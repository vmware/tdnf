#include "includes.h"

uint32_t
dup_argv(
    int argc,
    char* const* argv,
    char*** argvDup
    )
{
    uint32_t dwError = 0;
    int i = 0;
    char** dup = NULL;

    if(!argv || !argvDup)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = TDNFAllocateMemory(1, sizeof(char*) * argc, (void**)&dup);
    BAIL_ON_TDNF_ERROR(dwError);

    for(i = 0; i < argc; ++i)
    {
        dup[i] = strdup(argv[i]);
    }
    *argvDup = dup;

cleanup:
    return dwError;

error:
    if(argvDup)
    {
        *argvDup = NULL;
    }
    goto cleanup;
}

uint32_t
PMDUtilsFormatSize(
    uint32_t unSize,
    char** ppszFormattedSize
    )
{
    uint32_t dwError = 0;
    char* pszFormattedSize = NULL;
    char* pszSizes = "bkMG";
    double dSize = unSize;

    int nIndex = 0;
    int nLimit = strlen(pszSizes);
    double dKiloBytes = 1024.0;
    int nMaxSize = 25;

    if(!ppszFormattedSize)
    {
      dwError = ERROR_TDNF_INVALID_PARAMETER;
      BAIL_ON_TDNF_ERROR(dwError);
    }

    while(nIndex < nLimit && dSize > dKiloBytes)
    {
        dSize /= dKiloBytes;
        nIndex++;
    }

    dwError = TDNFAllocateMemory(1, nMaxSize, (void**)&pszFormattedSize);
    BAIL_ON_TDNF_ERROR(dwError);

    if(sprintf(pszFormattedSize, "%.2f %c", dSize, pszSizes[nIndex]) < 0)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    *ppszFormattedSize = pszFormattedSize;

cleanup:
    return dwError;

error:
    if(ppszFormattedSize)
    {
        *ppszFormattedSize = NULL;
    }
    TDNF_SAFE_FREE_MEMORY(pszFormattedSize);
    goto cleanup;
}

uint32_t
file_read_all_text(
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
ltrim(
    const char *pszStr
    )
{
    if(!pszStr) return NULL;
    while(isspace(*pszStr)) ++pszStr;
    return pszStr;
}

const char *
rtrim(
    const char *pszStart,
    const char *pszEnd
    )
{
    if(!pszStart || !pszEnd) return NULL;
    while(pszEnd > pszStart && isspace(*pszEnd)) pszEnd--;
    return pszEnd;
}

uint32_t
count_matches(
    const char *pszString,
    const char *pszFind,
    int *pnCount
    )
{
    uint32_t dwError = 0;
    int nCount = 0;
    int nOffset = 0;
    int nFindLength = 0;
    char *pszMatch = NULL;

    if(IsNullOrEmptyString(pszString) ||
       IsNullOrEmptyString(pszFind) ||
       !pnCount)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    nFindLength = strlen(pszFind);
    while((pszMatch = strcasestr(pszString + nOffset, pszFind)))
    {
        ++nCount;
        nOffset = pszMatch - pszString + nFindLength;
    }

    *pnCount = nCount;
cleanup:
    return dwError;

error:
    if(pnCount)
    {
        *pnCount = 0;
    }
    goto cleanup;
}

uint32_t
string_replace(
    const char *pszString,
    const char *pszFind,
    const char *pszReplace,
    char **ppszResult
    )
{
    uint32_t dwError = 0;
    char *pszResult = NULL;
    char *pszBoundary = NULL;
    int nCount = 0;
    int nResultLength = 0;
    int nFindLength = 0;
    int nReplaceLength = 0;
    int nOffset = 0;

    if(IsNullOrEmptyString(pszString) ||
       IsNullOrEmptyString(pszFind) ||
       !ppszResult)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = count_matches(pszString, pszFind, &nCount);
    BAIL_ON_TDNF_ERROR(dwError);

    if(nCount == 0)
    {
        dwError = ENOENT;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    nFindLength = strlen(pszFind);
    if(pszReplace)
    {
        nReplaceLength = strlen(pszReplace);
    }

    nResultLength = strlen(pszString) +
                    nCount * (nReplaceLength - nFindLength);

    dwError = TDNFAllocateMemory(1, sizeof(char) * (nResultLength + 1),
                                (void **)&pszResult);
    BAIL_ON_TDNF_ERROR(dwError);

    nOffset = 0;
    while((pszBoundary = strcasestr(pszString + nOffset, pszFind)))
    {
        int nLength = pszBoundary - (pszString + nOffset);

        strncat(pszResult, pszBoundary - nLength, nLength);
        if(pszReplace)
        {
            strcat(pszResult, pszReplace);
        }

        nOffset = pszBoundary - pszString + nFindLength;
    }

    strcat(pszResult, pszString + nOffset);

    *ppszResult = pszResult;
cleanup:
    return dwError;

error:
    if(ppszResult)
    {
        *ppszResult = NULL;
    }
    TDNF_SAFE_FREE_MEMORY(pszResult);
    goto cleanup;
}

uint32_t
make_array_from_string(
    const char *pszString,
    const char *pszSeparator,
    char ***pppszArray,
    int *pnCount
    )
{
    uint32_t dwError = 0;
    char **ppszArray = NULL;
    char *pszBoundary = NULL;
    int nOffset = 0;
    int nSepLength = 0;
    int nCount = 1;
    int nIndex = 0;

    if(IsNullOrEmptyString(pszString) ||
       IsNullOrEmptyString(pszSeparator) ||
       !pppszArray ||
       !pnCount)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = count_matches(pszString, pszSeparator, &nCount);
    BAIL_ON_TDNF_ERROR(dwError);

    ++nCount;

    dwError = TDNFAllocateMemory(1, sizeof(char **) * (nCount),
                                (void **)&ppszArray);
    BAIL_ON_TDNF_ERROR(dwError);

    nOffset = 0;
    nIndex = 0;
    nSepLength = strlen(pszSeparator);
    while((pszBoundary = strcasestr(pszString + nOffset, pszSeparator)))
    {
        int nLength = pszBoundary - (pszString + nOffset);
        dwError = TDNFAllocateMemory(1, sizeof(char) * (nLength + 1),
                                    (void **)&ppszArray[nIndex]);
        BAIL_ON_TDNF_ERROR(dwError);

        memcpy(ppszArray[nIndex], pszString + nOffset, nLength);

        nOffset = pszBoundary - pszString + nSepLength;
        ++nIndex;
    }

    dwError = TDNFAllocateString(pszString + nOffset, &ppszArray[nIndex]);
    BAIL_ON_TDNF_ERROR(dwError);

    *pppszArray = ppszArray;
    *pnCount = nCount;
cleanup:
    return dwError;

error:
    if(pppszArray)
    {
        *pppszArray = NULL;
    }
    if(pnCount)
    {
        *pnCount = 0;
    }
    TDNFFreeStringArrayWithCount(ppszArray, nCount);
    goto cleanup;
}
