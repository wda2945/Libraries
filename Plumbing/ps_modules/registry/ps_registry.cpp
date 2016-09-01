//
//  psRegistry.cpp
//  RobotFramework
//
//  Created by Martin Lane-Smith on 5/19/16.
//  Copyright © 2016 Martin Lane-Smith. All rights reserved.
//

#include <string.h>
#include <string>
#include <stdio.h>
#include "ps_registry.hpp"
#include "ps_config.h"
#include "pubsub/ps_pubsub_class.hpp"
#include "notify/ps_notify.hpp"

ps_registry& the_registry()
{
	static ps_registry the_registry;
	return the_registry;
}

ps_registry::ps_registry() : ps_root_class(std::string("registry"))
{
    the_broker().register_object(REGISTRY_UPDATE_PACKET, this);
    the_broker().register_object(REGISTRY_SYNC_PACKET, this);
}

//helpers
std::string get_domain_string(const char *_domain)
{
    char domain[REGISTRY_DOMAIN_LENGTH];
    strncpy(domain, _domain, REGISTRY_DOMAIN_LENGTH);
    domain[REGISTRY_DOMAIN_LENGTH-1] = '\0';
    return std::string(domain);
}
std::string get_name_string(const char *_name)
{
    char name[REGISTRY_NAME_LENGTH];
    strncpy(name, _name, REGISTRY_NAME_LENGTH);
    name[REGISTRY_NAME_LENGTH-1] = '\0';
    return std::string(name);
}

ps_registry_datatype_t ps_registry::get_registry_type(const char *domain, const char *name)
{
//    PS_DEBUG("reg: get type %s/%s", domain, name);
	LOCK_MUTEX(registryMtx);

    auto domain_pos = registry.find(get_domain_string(domain));   //find the domain set<>
    if (domain_pos == registry.end())
    {
    	UNLOCK_MUTEX(registryMtx);
        return PS_REGISTRY_UNKNOWN_TYPE;        //unknown domain
    }

    domain_set_t *domain_set = domain_pos->second;
    
    auto pos = domain_set->find(get_name_string(name));          //find the name object (ps_registry_entry_class)
    if (pos == domain_set->end())
    {
    	UNLOCK_MUTEX(registryMtx);
        return PS_REGISTRY_UNKNOWN_TYPE;        //unknown name
    }

    ps_registry_datatype_t type = pos->second.entry.value.datatype;
    
	UNLOCK_MUTEX(registryMtx);
    
    return type;
}

ps_registry_flags_t ps_registry::get_registry_flags(const char *domain, const char *name)
{
//    PS_DEBUG("reg: get flags %s/%s", domain, name);
	LOCK_MUTEX(registryMtx);

	auto domain_pos = registry.find(get_domain_string(domain));   //find the domain set<>
    if (domain_pos == registry.end())
    {
    	UNLOCK_MUTEX(registryMtx);
        return PS_REGISTRY_INVALID_FLAGS;        //unknown domain
    }
    
    domain_set_t *domain_set = domain_pos->second;
    
    auto pos = domain_set->find(get_name_string(name));          //find the name object (ps_registry_entry_class)
    if (pos == domain_set->end())
    {
    	UNLOCK_MUTEX(registryMtx);
    	return PS_REGISTRY_INVALID_FLAGS;        //unknown name
    }
    
    ps_registry_flags_t flags = pos->second.entry.value.flags;

	UNLOCK_MUTEX(registryMtx);

	return flags;
}

//add a new entry into the registry
ps_result_enum ps_registry::add_new_registry_entry(const char *_domain, const char *_name,
                                               ps_registry_datatype_t type, ps_registry_flags_t flags)
{
//    PS_DEBUG("reg: new %s/%s", _domain, _name);
	ps_registry_entry_t new_entry;

    domain_set_t *domain_set;
    std::string domain  = get_domain_string(_domain);
    std::string name    = get_name_string(_name);
    
	LOCK_MUTEX(registryMtx);

	auto domain_pos = registry.find(domain);   //get the domain set
    if (domain_pos == registry.end())
    {
        //new domain
        domain_set = new domain_set_t();
        registry.insert(std::make_pair(domain, domain_set));
    }
    else
    {
        domain_set = domain_pos->second;
    }
    
    auto pos = domain_set->find(name);   //get the name set
	if (pos == domain_set->end())
	{
        strncpy(new_entry.entry.domain, _domain, REGISTRY_DOMAIN_LENGTH);
        new_entry.entry.domain[REGISTRY_DOMAIN_LENGTH-1] = '\0';

        strncpy(new_entry.entry.name, _name, REGISTRY_NAME_LENGTH);
        new_entry.entry.name[REGISTRY_NAME_LENGTH-1] = '\0';
        
        new_entry.entry.value.datatype = type;
        new_entry.entry.value.flags = flags;
        new_entry.entry.value.source = SOURCE;
        new_entry.entry.value.serial = 1;
        this_checksum++;
        
        switch(type)
        {
            case PS_REGISTRY_TEXT_TYPE:
                new_entry.entry.value.string_value[0] = '\0';
                break;
            case PS_REGISTRY_INT_TYPE:
                new_entry.entry.value.int_value = 0;
                new_entry.entry.value.int_min = 0;
                new_entry.entry.value.int_max = 0;
                break;
            case PS_REGISTRY_REAL_TYPE:
                new_entry.entry.value.real_value = 0;
                new_entry.entry.value.real_min = 0;
                new_entry.entry.value.real_max = 0;
                break;
            case PS_REGISTRY_BOOL_TYPE:
                new_entry.entry.value.bool_value = false;
                break;
            default:
                break;
        }

		domain_set->insert(std::make_pair(name, new_entry));

        UNLOCK_MUTEX(registryMtx);

        notify_domain_observers(_domain, _name);
        
        if (!(flags & PS_REGISTRY_LOCAL))
        	the_broker().publish_packet(REGISTRY_UPDATE_PACKET, &new_entry.entry, sizeof(ps_update_packet_t));

        return PS_OK;
	}
	else
	{
    	UNLOCK_MUTEX(registryMtx);
		return PS_NAME_EXISTS;
	}
}

ps_result_enum ps_registry::set_new_registry_entry(const char *_domain, const char *_name, const ps_registry_struct_t& set_value)
{
//    PS_DEBUG("reg: new %s/%s", _domain, _name);
	ps_registry_entry_t new_entry;
    domain_set_t *domain_set;
    std::string domain = get_domain_string(_domain);
    std::string name    = get_name_string(_name);
    
	LOCK_MUTEX(registryMtx);

	auto domain_pos = registry.find(domain);   //get the domain set
    if (domain_pos == registry.end())
    {
        //new domain
        domain_set = new domain_set_t();
        registry.insert(std::make_pair(domain, domain_set));
    }
    else
    {
        domain_set = domain_pos->second;
    }
    
    auto pos = domain_set->find(name);   //get the name set
    if (pos == domain_set->end())
    {
        strncpy(new_entry.entry.domain, _domain, REGISTRY_DOMAIN_LENGTH);
        new_entry.entry.domain[REGISTRY_DOMAIN_LENGTH-1] = '\0';
        
        strncpy(new_entry.entry.name, _name, REGISTRY_NAME_LENGTH);
        new_entry.entry.name[REGISTRY_NAME_LENGTH-1] = '\0';
        
        new_entry.entry.value.datatype = set_value.datatype;
        new_entry.entry.value.flags = set_value.flags;
        new_entry.entry.value.source = SOURCE;
        new_entry.entry.value.serial = 1;
        this_checksum++;
        
        switch(set_value.datatype)
        {
            case PS_REGISTRY_TEXT_TYPE:
                strcpy(new_entry.entry.value.string_value, set_value.string_value);
                break;
            case PS_REGISTRY_INT_TYPE:
                new_entry.entry.value.int_min = set_value.int_min;
                new_entry.entry.value.int_max = set_value.int_max;
                new_entry.entry.value.int_value = set_value.int_value;
                break;
            case PS_REGISTRY_REAL_TYPE:
                new_entry.entry.value.real_min = set_value.real_min;
                new_entry.entry.value.real_max = set_value.real_max;
                new_entry.entry.value.real_value = set_value.real_value;
                break;
            case PS_REGISTRY_BOOL_TYPE:
                new_entry.entry.value.bool_value = set_value.bool_value;
                break;
            default:
                break;
        }
        
        domain_set->insert(std::make_pair(name, new_entry));
        
        UNLOCK_MUTEX(registryMtx);

        notify_domain_observers(_domain, _name);

        if (!(set_value.flags & PS_REGISTRY_LOCAL))
        		the_broker().publish_packet(REGISTRY_UPDATE_PACKET, &new_entry.entry, sizeof(ps_update_packet_t));

        return PS_OK;
    }
    else
    {
    	UNLOCK_MUTEX(registryMtx);
        return PS_NAME_EXISTS;
    }
}

//copy value only into the registry
ps_result_enum ps_registry::set_registry_entry(const char *_domain, const char *_name, const ps_registry_struct_t &new_value)
{
//    PS_DEBUG("reg: set %s/%s", _domain, _name);
	ps_update_packet_t 			*update_pkt;
	std::set<ps_observer_class*> _observers;

    domain_set_t *domain_set;
    std::string domain = get_domain_string(_domain);
    std::string name    = get_name_string(_name);
    
	LOCK_MUTEX(registryMtx);

    auto domain_pos = registry.find(domain);   //get the domain set
    if (domain_pos == registry.end())
    {
    	UNLOCK_MUTEX(registryMtx);
        return PS_NAME_NOT_FOUND;
    }
    else
    {
        domain_set = domain_pos->second;
    }
    
    auto pos = domain_set->find(name);
	if (pos == domain_set->end())
	{
    	UNLOCK_MUTEX(registryMtx);
		return PS_NAME_NOT_FOUND;
	}
	else
	{
        //copy value if type matches
        ps_result_enum reply = copy_data(pos->second.entry.value, new_value);
        pos->second.entry.value.serial++;
        this_checksum++;
        
        _observers = pos->second.observers;
        update_pkt = &pos->second.entry;

        UNLOCK_MUTEX(registryMtx);

        //notify observers
        for (auto obs : _observers)
        {
            (obs->observer_callback)(domain.c_str(), name.c_str(), obs->arg);
        }

        notify_domain_observers(_domain, _name);
        
        if (!(pos->second.entry.value.flags & PS_REGISTRY_LOCAL))
        	the_broker().publish_packet(REGISTRY_UPDATE_PACKET, update_pkt, sizeof(ps_update_packet_t));

        return reply;
	}
}

//copy entry from the registry
ps_result_enum ps_registry::get_registry_entry(const char *_domain, const char *_name, ps_registry_struct_t *get_value)
{
    PS_DEBUG("reg: get %s/%s", _domain, _name);
    domain_set_t *domain_set;
    std::string domain = get_domain_string(_domain);
    std::string name    = get_name_string(_name);
    
	LOCK_MUTEX(registryMtx);

	auto domain_pos = registry.find(domain);   //get the domain set
    if (domain_pos == registry.end())
    {
    	UNLOCK_MUTEX(registryMtx);
        return PS_NAME_NOT_FOUND;
    }
    else
    {
        domain_set = domain_pos->second;
    }
    
    auto pos = domain_set->find(name);
    if (pos == domain_set->end())
    {
    	UNLOCK_MUTEX(registryMtx);
        return PS_NAME_NOT_FOUND;
    }
    else
	{
        //copy min, max, value
		copy_all_data(*get_value, pos->second.entry.value);
	}
	UNLOCK_MUTEX(registryMtx);
	return PS_OK;
}

ps_result_enum ps_registry::set_observer(const char *_domain, const char *_name, ps_registry_callback_t *callback, const void *arg)
{
    PS_DEBUG("reg: set_observer: %s/%s", _domain, _name);
    domain_set_t *domain_set;
    std::string domain = get_domain_string(_domain);
    std::string name    = get_name_string(_name);
    
	LOCK_MUTEX(registryMtx);

	auto domain_pos = registry.find(domain);   //get the domain set
    if (domain_pos == registry.end())
    {
        //make an entry for domain
        domain_set = new domain_set_t();
        registry.insert(std::make_pair(domain, domain_set));
    }
    else
    {
        domain_set = domain_pos->second;
    }

    auto pos = domain_set->find(name);
    if (pos != domain_set->end())
    {
        ps_observer_class *obs = new ps_observer_class(callback, arg);
        pos->second.observers.insert(obs);
        UNLOCK_MUTEX(registryMtx);
        return PS_OK;
    }
    else if (name == "any")
    {
        //make an entry
        ps_registry_entry_t new_entry;
        
        strncpy(new_entry.entry.domain, _domain, REGISTRY_DOMAIN_LENGTH);
        new_entry.entry.domain[REGISTRY_DOMAIN_LENGTH-1] = '\0';
        
        strncpy(new_entry.entry.name, _name, REGISTRY_NAME_LENGTH);
        new_entry.entry.name[REGISTRY_NAME_LENGTH-1] = '\0';
        
        new_entry.entry.value.source = SOURCE;
        new_entry.entry.value.flags  = PS_REGISTRY_LOCAL;
        
        domain_set->insert(std::make_pair(name, new_entry));

        ps_observer_class *obs = new ps_observer_class(callback, arg);
        
        pos = domain_set->find(name);
        pos->second.observers.insert(obs);
        
        UNLOCK_MUTEX(registryMtx);
        return PS_OK;
    }
    else
    {
        PS_DEBUG("reg: set_observer: %s/%s not found", domain.c_str(), name.c_str());
    	UNLOCK_MUTEX(registryMtx);
        return PS_NAME_NOT_FOUND;
    }
}

void ps_registry::notify_domain_observers(const char *_domain, const char *_name)
{
    domain_set_t *domain_set;
    std::set<ps_observer_class*> _observers;
    
    std::string domain = get_domain_string(_domain);
    std::string name    = get_name_string(_name);
	LOCK_MUTEX(registryMtx);

	auto domain_pos = registry.find(domain);   //get the domain set
    if (domain_pos != registry.end())
    {
        domain_set = domain_pos->second;
        
        auto pos = domain_set->find("any");
        if (pos != domain_set->end())
        {
            _observers = pos->second.observers;
        }
    }
    
    UNLOCK_MUTEX(registryMtx);
    
    //notify observers
    for (auto obs : _observers)
    {
        (obs->observer_callback)(domain.c_str(), name.c_str(), obs->arg);
    }
}

ps_result_enum ps_registry::interate_domain(const char *_domain, ps_registry_callback_t *callback, const void *arg)
{
    PS_DEBUG("reg: iterate: %s", _domain);
    
    domain_set_t *domain_set;
    std::string domain = get_domain_string(_domain);
    
	LOCK_MUTEX(registryMtx);

	auto domain_pos = registry.find(domain);   //get the domain set
    if (domain_pos == registry.end())
    {
    	UNLOCK_MUTEX(registryMtx);
        return PS_NAME_NOT_FOUND;
    }
    else
    {
        domain_set = domain_pos->second;
    }
    UNLOCK_MUTEX(registryMtx);

    for (auto pos = domain_set->begin(); pos != domain_set->end(); pos++)
    {
        (callback)(domain.c_str(), pos->first.c_str(), arg);
    }
    return PS_OK;
}

ps_result_enum ps_registry::reset_registry()
{
	LOCK_MUTEX(registryMtx);

	registry.clear();

	UNLOCK_MUTEX(registryMtx);
    return PS_OK;
}


void ps_registry::calculate_checksum()
{
    int i;
    for (i=0; i< SRC_COUNT; i++)
    {
        checksums[i] = 0;
    }

	LOCK_MUTEX(registryMtx);

	for (auto domain_pos : registry)
    {
        for (auto name_pos : *domain_pos.second)
        {
            int src    = name_pos.second.entry.value.source;
            uint8_t flags  = name_pos.second.entry.value.flags;

            if ((src < SRC_COUNT) && ((flags & PS_REGISTRY_LOCAL) == 0))
            {
                checksums[src] += name_pos.second.entry.value.serial;
            }
        }
    }

    this_checksum = checksums[SOURCE];

	UNLOCK_MUTEX(registryMtx);
}

//send a sync packet containing the checksums of registry entries from each source
ps_result_enum ps_registry::send_registry_sync()
{
    calculate_checksum();
    return the_broker().publish_packet(REGISTRY_SYNC_PACKET, checksums, sizeof(int) * SRC_COUNT);
}

//process received packets, either a sync packet or an update
void ps_registry::message_handler(ps_packet_source_t packet_source, ps_packet_type_t   packet_type, const void *msg, int length)
{
    if (packet_source != SOURCE)
    {
        switch(packet_type)
        {
            case REGISTRY_SYNC_PACKET:
                //receive registry sync packet
            {
                char timeBuff[20];
                const time_t now = time(NULL);
                struct tm *timestruct = localtime(&now);
                
                snprintf(timeBuff, 20, "%02i:%02i:%02i",
                         timestruct->tm_hour, timestruct->tm_min, timestruct->tm_sec);
                
                if (ps_registry_set_text("Registry", "Last Sync Rx", timeBuff) == PS_NAME_NOT_FOUND)
                {
                    ps_registry_add_new("Registry", "Last Sync Rx", PS_REGISTRY_TEXT_TYPE, PS_REGISTRY_LOCAL + PS_REGISTRY_SRC_WRITE);
                    ps_registry_set_text("Registry", "Last Sync Rx", timeBuff);
                }
                
                PS_DEBUG("reg: rx sync packet");
                calculate_checksum();

                const int *csums = static_cast<const int*>(msg);

                if ((int)((SOURCE+1) * sizeof(int)) < length)
                {
                    if (csums[SOURCE] != checksums[SOURCE])
                    {
                        PS_DEBUG("reg: checksum %i from %i. Should be %i", csums[SOURCE], packet_source, checksums[SOURCE]);
                        resend_all_updates();
                        the_notifier().ps_republish_conditions();
                    }
                }
                else
                {
                    PS_ERROR("reg: sync packet too short");
                }
            }
                break;
            case REGISTRY_UPDATE_PACKET:
                //receive registry update
                if (length >= (int) sizeof(ps_update_packet_t))
                {
                    ps_update_packet_t *pkt = (ps_update_packet_t*) msg;
                    ps_registry_entry_t new_entry;
                    
                    PS_DEBUG("reg: rx update %s/%s", pkt->domain, pkt->name);

                    std::set<ps_observer_class*> _observers;
                    
                    domain_set_t *domain_set;
                    std::string domain {pkt->domain};
                    std::string name   {pkt->name};
                    
                    LOCK_MUTEX(registryMtx);
                    
                    auto domain_pos = registry.find(domain);   //get the domain set
                    if (domain_pos == registry.end())
                    {
                        //new domain
                        domain_set = new domain_set_t();
                        registry.insert(std::make_pair(domain, domain_set));
                    }
                    else
                    {
                        domain_set = domain_pos->second;
                    }
                    
                    auto pos = domain_set->find(name);
                    if (pos == domain_set->end())
                    {
                        //new name entry
                        memcpy(&new_entry.entry, pkt, sizeof(ps_update_packet_t));
                        domain_set->insert(std::make_pair(name, new_entry));
                        pos = domain_set->find(name);
                    }
                    
                    //copy value if type matches
                    copy_data(pos->second.entry.value, pkt->value);
                    pos->second.entry.value.serial = pkt->value.serial;
                    
                    _observers = pos->second.observers;
                    
                    UNLOCK_MUTEX(registryMtx);
                    
                    //notify observers
                    for (auto obs : _observers)
                    {
                        (obs->observer_callback)(pkt->domain, pkt->name, obs->arg);
                    }
                    
                    notify_domain_observers(pkt->domain, pkt->name);

                }
                else
                {
                    PS_DEBUG("reg: rx bad update packet");
                }

                break;
            default:
                break;
        }
    }
}

//resend all the entries I own except local-only
void ps_registry::resend_all_updates()
{
	LOCK_MUTEX(registryMtx);

	for (auto domain_pos : registry)
    {
        for (auto name_pos : *domain_pos.second)
        {
            auto src    = name_pos.second.entry.value.source;
            auto flags  = name_pos.second.entry.value.flags;
            if ((src == SOURCE) && ((flags & PS_REGISTRY_LOCAL) == 0))
            {
                 the_broker().publish_packet(REGISTRY_UPDATE_PACKET, &name_pos.second.entry, sizeof(ps_update_packet_t));
            }
        }
    }
	UNLOCK_MUTEX(registryMtx);
}

ps_result_enum ps_registry::load_registry_method(const char *path)
{
    FILE *fp = fopen(path, "r");
    if (fp == 0)
        return PS_NAME_NOT_FOUND;
    
    char *buff = NULL;
    size_t buffsize = 0;
    
    PS_DEBUG("reg: pre-loading from %s", path);
    
	while (getline(&buff, &buffsize, fp) > 0)
    {
        char value_type[20] {""};
        sscanf(buff, "%19s", value_type);
        
        char domain[100];
        char name[100];
        
        if (strcmp(value_type, "real_setting") == 0)
        {
            ps_registry_struct_t val;
            
            sscanf(buff, "%19s %99s %99s %f %f %f", value_type, domain, name,
                   &val.real_min, &val.real_max,  &val.real_value);
            
            val.datatype = PS_REGISTRY_REAL_TYPE;
            val.flags = PS_REGISTRY_ANY_WRITE;
            
            ps_registry_set_new(domain, name, val);
            
            PS_DEBUG(">> real %s %s %f %f %f", domain, name, val.real_min, val.real_max,  val.real_value);
        }
        else if (strcmp(value_type, "int_setting") == 0)
        {
            ps_registry_struct_t val;

            sscanf(buff, "%19s %99s %99s %i %i %i", value_type, domain, name,
                   &val.int_min, &val.int_max,  &val.int_value);

            val.datatype = PS_REGISTRY_INT_TYPE;
            val.flags = PS_REGISTRY_ANY_WRITE;
            
            ps_registry_set_new(domain, name, val);
            
            PS_DEBUG(">> int %s %s %i %i %i", domain, name,  val.int_min, val.int_max, val.int_value);
        }
        else if (strcmp(value_type, "bool_option") == 0)
        {
            int v;
            
            sscanf(buff, "%19s %99s %99s %i", value_type, domain, name, &v);
            
            ps_registry_add_new(domain, name, PS_REGISTRY_BOOL_TYPE, PS_REGISTRY_ANY_WRITE);
            
            ps_registry_set_bool(domain, name, v);
            
            PS_DEBUG(">> bool %s %s %i", domain, name, v);
        }
        if (strcmp(value_type, "real") == 0)
        {
            float v;
            ps_registry_flags_t flags;

            sscanf(buff, "%19s %c %99s %99s %f", value_type, &flags, domain, name, &v);

            ps_registry_add_new(domain, name, PS_REGISTRY_REAL_TYPE, flags);
            
            ps_registry_set_real(domain, name,  v);

            PS_DEBUG(">> real %s %s %f", domain, name, v);
        }
        else if (strcmp(value_type, "int") == 0)
        {
            int v;
            ps_registry_flags_t flags;

            sscanf(buff, "%19s %c %99s %99s %i", value_type, &flags, domain, name, &v);

            ps_registry_add_new(domain, name, PS_REGISTRY_INT_TYPE, flags);

            ps_registry_set_int(domain, name,  v);
            
            PS_DEBUG(">> int %s %s %i", domain, name, v);
        }
        else if (strcmp(value_type, "bool") == 0)
        {
            int v;
            ps_registry_flags_t flags;

            sscanf(buff, "%19s %c %99s %99s %i", value_type, &flags, domain, name, &v);
            
            ps_registry_add_new(domain, name, PS_REGISTRY_BOOL_TYPE, flags);
            
            ps_registry_set_bool(domain, name, v);
            
            PS_DEBUG(">> bool %s %s %i", domain, name, v);
        }
        else if (strcmp(value_type, "text") == 0)
        {
            char value[100];
            ps_registry_flags_t flags;

            sscanf(buff, "%19s %c %99s %99s %99s", value_type, &flags, domain, name, value);
            
            ps_registry_add_new(domain, name, PS_REGISTRY_TEXT_TYPE, flags);
            ps_registry_set_text(domain, name, value);
            
            PS_DEBUG(">> text %s %s %s", domain, name, value);
        }
        //else ignore
    }
    
    fclose(fp);
    return PS_OK;
}

ps_result_enum ps_registry::save_registry_method(const char *path, const char *domain)
{
    FILE *fp;
    
    fp = fopen(path, "w");
    if (fp == 0)
        return PS_NAME_NOT_FOUND;
    
	LOCK_MUTEX(registryMtx);

	for (auto domain_pos : registry)
    {
        if ((domain_pos.first == std::string(domain)) || (domain == NULL))
        {
            PS_DEBUG("reg: saving <%s> to %s", domain_pos.first.c_str(), path);
            
            for (auto name_pos : *domain_pos.second)
            {
                switch (name_pos.second.entry.value.datatype)
                {
                    case PS_REGISTRY_TEXT_TYPE:
                        fprintf(fp, "text %s %s %s\n",
                                domain_pos.first.c_str(),
                                name_pos.first.c_str(),
                                name_pos.second.entry.value.string_value);
                        PS_DEBUG("text %s %s %s",
                                domain_pos.first.c_str(),
                                name_pos.first.c_str(),
                                name_pos.second.entry.value.string_value);
                        break;
                    case PS_REGISTRY_INT_TYPE:
                        fprintf(fp, "int %s %s %i %i %i\n",
                                domain_pos.first.c_str(),
                                name_pos.first.c_str(),
                                name_pos.second.entry.value.int_min,
                                name_pos.second.entry.value.int_max,
                                name_pos.second.entry.value.int_value);
                        PS_DEBUG("int %s %s %i %i %i",
                                domain_pos.first.c_str(),
                                name_pos.first.c_str(),
                                name_pos.second.entry.value.int_min,
                                name_pos.second.entry.value.int_max,
                                name_pos.second.entry.value.int_value);
                        break;
                    case PS_REGISTRY_REAL_TYPE:
                        fprintf(fp, "real %s %s %f %f %f\n",
                                domain_pos.first.c_str(),
                                name_pos.first.c_str(),
                                name_pos.second.entry.value.real_min,
                                name_pos.second.entry.value.real_max,
                                name_pos.second.entry.value.real_value);
                        PS_DEBUG("real %s %s %f %f %f",
                                domain_pos.first.c_str(),
                                name_pos.first.c_str(),
                                name_pos.second.entry.value.real_min,
                                name_pos.second.entry.value.real_max,
                                name_pos.second.entry.value.real_value);
                        break;
                    case PS_REGISTRY_BOOL_TYPE:
                        fprintf(fp, "text %s %s %i\n",
                                domain_pos.first.c_str(),
                                name_pos.first.c_str(),
                                name_pos.second.entry.value.bool_value);
                        PS_DEBUG("text %s %s %i",
                                domain_pos.first.c_str(),
                                name_pos.first.c_str(),
                                name_pos.second.entry.value.bool_value);
                        break;
                    default:
                        break;
                }
            }
        }
    }

    fclose(fp);
	UNLOCK_MUTEX(registryMtx);
    return PS_OK;
}

//////////////////////C API


//////////////Adding new entries

//just add with no data
ps_result_enum ps_registry_add_new(const char *domain, const char *name,
                                   ps_registry_datatype_t type, ps_registry_flags_t flags)
{
    return the_registry().add_new_registry_entry(domain, name, type, flags);
}
//add and set data
ps_result_enum ps_registry_set_new(const char *domain, const char *name, ps_registry_struct_t data)
{
    return the_registry().set_new_registry_entry(domain, name, data);
}

ps_registry_datatype_t ps_registry_get_type(const char *domain, const char *name)
{
    return the_registry().get_registry_type(domain, name);
}

ps_registry_flags_t     ps_registry_get_flags(const char *domain, const char *name)
{
    return the_registry().get_registry_flags(domain, name);
}

/////////////Changing values of existing entries

ps_result_enum ps_registry_set(const char *domain, const char *name, const ps_registry_struct_t data)
{
    return the_registry().set_registry_entry(domain, name, data);
}

ps_result_enum ps_registry_set_text(const char *domain, const char *name, const char *string_value){
    ps_registry_struct_t data;
    data.datatype = PS_REGISTRY_TEXT_TYPE;
    strncpy(data.string_value, string_value, REGISTRY_TEXT_LENGTH);
    data.string_value[REGISTRY_TEXT_LENGTH-1] = '\0';
    return ps_registry_set(domain, name, data);
}
ps_result_enum ps_registry_set_string(const char *domain, const char *name, std::string string_value){
    ps_registry_struct_t data;
    data.datatype = PS_REGISTRY_TEXT_TYPE;
    strncpy(data.string_value, string_value.c_str(), REGISTRY_TEXT_LENGTH);
    data.string_value[REGISTRY_TEXT_LENGTH-1] = '\0';
    return ps_registry_set(domain, name, data);
}
ps_result_enum ps_registry_set_int(const char *domain, const char *name, int _value){
    ps_registry_struct_t data;
    data.datatype = PS_REGISTRY_INT_TYPE;
    data.int_value = _value;
    return ps_registry_set(domain, name, data);
}
ps_result_enum ps_registry_set_real(const char *domain, const char *name, float _value){
    ps_registry_struct_t data;
    data.datatype = PS_REGISTRY_REAL_TYPE;
    data.real_value = _value;
    return ps_registry_set(domain, name, data);
}
ps_result_enum ps_registry_set_bool(const char *domain, const char *name, bool _value){
    ps_registry_struct_t data;
    data.datatype = PS_REGISTRY_BOOL_TYPE;
    data.bool_value = _value;
    return ps_registry_set(domain, name, data);
}

////////////Getting current values

ps_result_enum ps_registry_get(const char *domain, const char *name, ps_registry_struct_t *data)
{
    return the_registry().get_registry_entry(domain, name, data);
}

ps_result_enum ps_registry_get_text(const char *domain, const char *name, char *buff, int len){
	ps_registry_struct_t value;
	ps_result_enum reply = ps_registry_get(domain, name, &value);

	if (reply == PS_OK)
	{
		if (value.datatype == PS_REGISTRY_TEXT_TYPE)
		{
			strncpy(buff, value.string_value, len);
			return PS_OK;
		}
		else
		{
			return PS_WRONG_DATA_TYPE;
		}
	}
	return reply;
}

ps_result_enum ps_registry_get_string(const char *domain, const char *name, std::string *_value){
    ps_registry_struct_t value;
    ps_result_enum reply = ps_registry_get(domain, name, &value);

	if (reply == PS_OK)
	{
		if (value.datatype == PS_REGISTRY_TEXT_TYPE)
		{
            *_value = std::string(value.string_value);
			return PS_OK;
		}
		else
		{
			return PS_WRONG_DATA_TYPE;
		}
	}
	return reply;
}

ps_result_enum ps_registry_get_int(const char *domain, const char *name, int *buff){
    ps_registry_struct_t value;
    ps_result_enum reply = ps_registry_get(domain, name, &value);
    
    if (reply == PS_OK)
    {
        if (value.datatype == PS_REGISTRY_INT_TYPE)
        {
			*buff = value.int_value;
			return PS_OK;
		}
		else
		{
			return PS_WRONG_DATA_TYPE;
		}
	}
	return reply;
}

ps_result_enum ps_registry_get_real(const char *domain, const char *name, float *buff){
    ps_registry_struct_t value;
    ps_result_enum reply = ps_registry_get(domain, name, &value);
    
    if (reply == PS_OK)
    {
        if (value.datatype == PS_REGISTRY_REAL_TYPE)
        {
            *buff = value.real_value;
            return PS_OK;
        }
        else
        {
            return PS_WRONG_DATA_TYPE;
        }
    }
    return reply;
}

ps_result_enum ps_registry_get_bool(const char *domain, const char *name, bool *buff){
    ps_registry_struct_t value;
    ps_result_enum reply = ps_registry_get(domain, name, &value);
    
    if (reply == PS_OK)
    {
        if (value.datatype == PS_REGISTRY_BOOL_TYPE)
        {
            *buff = value.bool_value;
            return PS_OK;
        }
        else
        {
            return PS_WRONG_DATA_TYPE;
        }
    }
    return reply;
}

ps_result_enum ps_registry_set_observer(const char *domain, const char *name, ps_registry_callback_t *callback, void *arg)
{
	return the_registry().set_observer(domain, name, callback, arg);
}

ps_result_enum ps_registry_iterate_domain(const char *domain, ps_registry_callback_t *callback, void *arg)
{
    return the_registry().interate_domain(domain, callback, arg);
}

ps_result_enum load_registry(const char *path)
{
    return the_registry().load_registry_method(path);
}
ps_result_enum save_registry(const char *path, const char *domain)
{
    return the_registry().save_registry_method(path, domain);
}

ps_result_enum send_registry_sync()
{
	return the_registry().send_registry_sync();
}
