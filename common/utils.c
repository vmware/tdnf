#include "includes.h"

uint32_t
TDNFFileReadAllText(
    const char *pszFileName,
    char **ppszText
    )
{
    uint32_t dwError = 0;
    FILE *fp = NULL;
    char *pszText = NULL;
    int nLength = 0;
    int nBytesRead = 0;

    if(!pszFileName || !ppszText)
    {
        dwError = EINVAL;
        BAIL_ON_TDNF_SYSTEM_ERROR(dwError);
    }

    fp = fopen(pszFileName, "r");
    if(!fp)
    {
        dwError = ENOENT;
        BAIL_ON_TDNF_SYSTEM_ERROR(dwError);
    }
    fseek(fp, 0, SEEK_END);
    nLength = ftell(fp);

    dwError = TDNFAllocateMemory(1, nLength + 1, (void **)&pszText);
    BAIL_ON_TDNF_ERROR(dwError);

    if(fseek(fp, 0, SEEK_SET))
    {
        dwError = errno;
        BAIL_ON_TDNF_SYSTEM_ERROR(dwError);
    }

    nBytesRead = fread(pszText, 1, nLength, fp);
    if(nBytesRead != nLength)
    {
        dwError = EBADFD;
        BAIL_ON_TDNF_SYSTEM_ERROR(dwError);
    }

    *ppszText = pszText;
cleanup:
    if(fp)
    {
        fclose(fp);
    }
    return dwError;

error:
    if(ppszText)
    {
        *ppszText = NULL;
    }
    TDNF_SAFE_FREE_MEMORY(pszText);
    goto cleanup;
}

const char *
TDNFLeftTrim(
    const char *pszStr
    )
{
    if(!pszStr) return NULL;
    while(isspace(*pszStr)) ++pszStr;
    return pszStr;
}

const char *
TDNFRightTrim(
    const char *pszStart,
    const char *pszEnd
    )
{
    if(!pszStart || !pszEnd) return NULL;
    while(pszEnd > pszStart && isspace(*pszEnd)) pszEnd--;
    return pszEnd;
}