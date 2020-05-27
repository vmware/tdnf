#ifndef __SOLV_DEFINES_H__
#define __SOLV_DEFINES_H__

#define SYSTEM_REPO_NAME "@System"
#define CMDLINE_REPO_NAME "@commandline"
#define SOLV_COOKIE_IDENT "tdnf"
#define TDNF_SOLVCACHE_DIR_NAME "solvcache"
#define SOLV_COOKIE_LEN   32

#define BAIL_ON_TDNF_LIBSOLV_ERROR(dwError) \
    do {                                                           \
        if (dwError)                                               \
        {                                                          \
            goto error;                                            \
        }                                                          \
    } while(0)

#endif /* __SOLV_DEFINES_H__ */
