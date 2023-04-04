/*
 * Copyright (C) 2019-2023 VMware, Inc. All Rights Reserved.
 *
 * Licensed under the GNU Lesser General Public License v2.1 (the "License");
 * you may not use this file except in compliance with the License. The terms
 * of the License are located in the COPYING file of this distribution.
 */

#include "includes.h"

static bool isQuiet = false;
static bool isJson = false;
static bool isDnfCheckUpdateCompat = false;

void GlobalSetQuiet(int32_t val)
{
    if (val > 0)
    {
        isQuiet = true;
    }
}

void GlobalSetJson(int32_t val)
{
    if (val > 0)
    {
        isJson = true;
    }
}

void GlobalSetDnfCheckUpdateCompat(int32_t val)
{
    if (val > 0)
    {
        isDnfCheckUpdateCompat = true;
    }
}

bool GlobalGetDnfCheckUpdateCompat()
{
    return isDnfCheckUpdateCompat;
}

void log_console(int32_t loglevel, const char *format, ...)
{
    va_list args;
    FILE *stream = NULL;

    if (!format)
    {
        return;
    }

    va_start(args, format);

    switch (loglevel)
    {
    case LOG_INFO:
    case LOG_CRIT:
        if (isJson)
        {
            goto end;
        }
        if (loglevel == LOG_INFO && isQuiet)
        {
            goto end;
        }
        stream = stdout;
        break;

    case LOG_ERR:
        stream = stderr;
        break;

    default:
        goto end;
    }

    if (!stream)
    {
        /* just in case */
        goto end;
    }

    vfprintf(stream, format, args);
    fflush(stream);
end:
    va_end(args);
}
