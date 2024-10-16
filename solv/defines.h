#ifndef __SOLV_DEFINES_H__
#define __SOLV_DEFINES_H__

#define SYSTEM_REPO_NAME "@System"
#define CMDLINE_REPO_NAME "@cmdline"
#define SOLV_COOKIE_IDENT "tdnf"
#define TDNF_SOLVCACHE_DIR_NAME "solvcache"
#define SOLV_COOKIE_LEN   32

#define SOLV_NEVRA_UNINSTALLED 0
#define SOLV_NEVRA_INSTALLED   1

#define BAIL_ON_TDNF_LIBSOLV_ERROR(dwError) \
    do {                                                           \
        if (dwError)                                               \
        {                                                          \
            goto error;                                            \
        }                                                          \
    } while(0)

typedef struct {
    // frequently changed values
    char * pszElementBuffer;
    int nBufferLen;
    int nInPackage;
    int nPrintPackage;
    int nTimeFound;

    // managed values
    int nBufferMaxLen;
    int nDepth;
    int nPrevElement; // enum 0 -> start, 1 -> data, 2 -> end

    //set and forget on creation
    time_t nSearchTime;
    FILE * pbOutfile;
} TDNFFilterData;

#define TDNF_MAX_FILTER_INPUT_THRESHOLD 500000000
#define TDNF_DEFAULT_TIME_FILTER_BUFF_SIZE 16000

#define BAIL_ON_TDNF_TIME_FILTER_ERROR(dwError) \
    do {                                                           \
        if (dwError)                                               \
        {                                                          \
            goto error;                                            \
        }                                                          \
    } while(0)

#endif /* __SOLV_DEFINES_H__ */
