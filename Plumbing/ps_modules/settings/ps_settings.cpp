//
//  ps_settings.cpp
//  RobotFramework
//
//  Created by Martin Lane-Smith on 5/19/16.
//  Copyright © 2016 Martin Lane-Smith. All rights reserved.
//

#include "settings/ps_settings.hpp"

ps_result_enum ps_settings::set_string(char *name, char *value)
{
    return PS_OK;
}

ps_result_enum ps_settings::get_string(char *name, char *buffer, size_t length)
{
    return PS_NAME_NOT_FOUND;
}


ps_result_enum ps_settings::set_real(char *name, double value)
{
    return PS_OK;
}

ps_result_enum ps_settings::get_real(char *name, double *value)
{
    return PS_NAME_NOT_FOUND;
}


ps_result_enum ps_settings::set_integer(char *name, int32_t value)
{
    return PS_OK;
}

ps_result_enum ps_settings::get_integer(char *name, int32_t *value)
{
    return PS_NAME_NOT_FOUND;
}


ps_result_enum ps_settings::set_boolean(char *name, bool value)
{
    return PS_OK;
}

ps_result_enum ps_settings::get_boolean(char *name, bool *value)
{
    return PS_NAME_NOT_FOUND;
}
