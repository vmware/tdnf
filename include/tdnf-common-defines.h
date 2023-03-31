/*
 * Copyright (C) 2020-2022 VMware, Inc. All Rights Reserved.
 *
 * Licensed under the GNU Lesser General Public License v2.1 (the "License");
 * you may not use this file except in compliance with the License. The terms
 * of the License are located in the COPYING file of this distribution.
 */

#ifndef __INC_TDNF_COMMON_DEFINES_H__
#define __INC_TDNF_COMMON_DEFINES_H__

/* Use this to get rid of variable/parameter unused warning */
#define UNUSED(var) ((void)(var))

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

#define IsNullOrEmptyString(str)    (!(str) || !(*str))

#define BAIL_ON_TDNF_ERROR(dwError)     \
    do {                                \
        if (dwError)                    \
        {                               \
            goto error;                 \
        }                               \
    } while(0)

#define BAIL_ON_TDNF_SYSTEM_ERROR(dwError)                  \
    do {                                                    \
        if (dwError)                                        \
        {                                                   \
            dwError = ERROR_TDNF_SYSTEM_BASE + dwError;     \
            goto error;                                     \
        }                                                   \
    } while(0)

#define BAIL_ON_TDNF_SYSTEM_ERROR_UNCOND(dwError)   \
    do {                                            \
        dwError = ERROR_TDNF_SYSTEM_BASE + dwError; \
        goto error;                                 \
    } while(0)

#define CHECK_JD_RC(rc)                \
{                                      \
    if ((rc) != 0) {                   \
        dwError = ERROR_TDNF_JSONDUMP; \
        BAIL_ON_CLI_ERROR(dwError);    \
    }                                  \
}

#define CHECK_JD_NULL(jd)              \
{                                      \
    if ((jd) == NULL) {                \
        dwError = ERROR_TDNF_JSONDUMP; \
        BAIL_ON_CLI_ERROR(dwError);    \
    }                                  \
}

#define JD_SAFE_DESTROY(jd) \
{                           \
    if (jd) {               \
        jd_destroy(jd);     \
        jd = NULL;          \
    }                       \
}

#define TDNF_SAFE_FREE_MEMORY(pMemory)          \
    do {                                        \
        if (pMemory) {                          \
            TDNFFreeMemory(pMemory);            \
            pMemory = NULL;                     \
        }                                       \
    } while(0)

#define TDNF_SAFE_FREE_STRINGARRAY(ppArray)     \
    do {                                        \
        if (ppArray)                            \
        {                                       \
            TDNFFreeStringArray(ppArray);       \
            ppArray = NULL;                     \
        }                                       \
    } while(0)

#define LOG_INFO    0
#define LOG_ERR     1
#define LOG_CRIT    2

#define pr_info(fmt, ...) \
    log_console(LOG_INFO, fmt, ##__VA_ARGS__)

#define pr_err(fmt, ...) \
    log_console(LOG_ERR, fmt, ##__VA_ARGS__)

#define pr_json(str) \
    fputs(str, stdout)

#define pr_jsonf(fmt, ...) \
    fprintf(stdout, fmt, ##__VA_ARGS__)

/*
 * If something needs to be printed (a prompt for example)
 * irrespective of 'quiet' option
 */
#define pr_crit(fmt, ...) \
    log_console(LOG_CRIT, fmt, ##__VA_ARGS__)

#endif /* __INC_TDNF_COMMON_DEFINES_H__ */
