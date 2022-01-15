/*
 * Copyright (C) 2020-2022 VMware, Inc. All Rights Reserved.
 *
 * Licensed under the GNU Lesser General Public License v2.1 (the "License");
 * you may not use this file except in compliance with the License. The terms
 * of the License are located in the COPYING file of this distribution.
 */

#include "includes.h"
#include <gpgme.h>

static
uint32_t
_TDNFVerifyResult(
    gpgme_ctx_t pContext
    )
{
    uint32_t dwError = 0;
    gpgme_signature_t pSig = NULL;
    gpgme_verify_result_t pResult = NULL;

    /* pContext release will free pResult. do not free otherwise */
    pResult = gpgme_op_verify_result(pContext);
    if (!pResult || !pResult->signatures)
    {
        dwError = ERROR_TDNF_GPG_VERIFY_RESULT;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    for (pSig = pResult->signatures; pSig; pSig = pSig->next)
    {
        if (pSig->status)
        {
            pr_err("repo md signature check: %s\n", gpgme_strerror (pSig->status));
            dwError = ERROR_TDNF_GPG_SIGNATURE_CHECK;
            break;
        }
    }
cleanup:
    return dwError;

error:
    goto cleanup;
}

uint32_t
TDNFRepoGPGCheckVerifyVersion(
    )
{
    uint32_t dwError = 0;
    const char *pszVersion = NULL;

    pszVersion = gpgme_check_version(NULL);
    if (!pszVersion)
    {
        dwError = ERROR_TDNF_GPG_VERSION_FAILED;
        BAIL_ON_TDNF_ERROR(dwError);
    }

error:
    return dwError;
}

uint32_t
TDNFVerifyRepoMDSignature(
    PTDNF_PLUGIN_HANDLE pHandle,
    const char *pszRepoMD,
    const char *pszRepoMDSig
    )
{
    uint32_t dwError = 0;
    FILE *fpRepoMD = NULL;
    FILE *fpRepoMDSig = NULL;
    gpgme_error_t nGPGError = 0;
    gpgme_ctx_t pContext = NULL;
    gpgme_protocol_t protocol = GPGME_PROTOCOL_OpenPGP;
    gpgme_data_t dataSig = NULL;
    gpgme_data_t dataText = NULL;

    if (!pHandle || IsNullOrEmptyString(pszRepoMD) ||
        IsNullOrEmptyString(pszRepoMDSig))
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    nGPGError = gpgme_new(&pContext);
    if (nGPGError)
    {
        pHandle->nGPGError = nGPGError;
        dwError = ERROR_TDNF_GPG_ERROR;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    gpgme_set_protocol (pContext, protocol);

    fpRepoMDSig = fopen(pszRepoMDSig, "rb");
    if (!fpRepoMDSig)
    {
        pr_err("repogpgcheck: failed to open %s\n", pszRepoMDSig);
        dwError = errno;
        BAIL_ON_TDNF_SYSTEM_ERROR(dwError);
    }

    dwError = gpgme_data_new_from_stream (&dataSig, fpRepoMDSig);
    if (dwError)
    {
        pHandle->nGPGError = nGPGError;
        dwError = ERROR_TDNF_GPG_ERROR;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    fpRepoMD = fopen(pszRepoMD, "rb");
    if (!fpRepoMD)
    {
        pr_err("repogpgcheck: failed to open %s\n", pszRepoMD);
        dwError = errno;
        BAIL_ON_TDNF_SYSTEM_ERROR(dwError);
    }

    dwError = gpgme_data_new_from_stream(&dataText, fpRepoMD);
    if (dwError)
    {
        pHandle->nGPGError = nGPGError;
        dwError = ERROR_TDNF_GPG_ERROR;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = gpgme_op_verify(pContext, dataSig, dataText, NULL);
    if (dwError)
    {
        pHandle->nGPGError = nGPGError;
        pr_err("gpg verify failed: %s\n", gpgme_strerror(dwError));
        dwError = ERROR_TDNF_GPG_ERROR;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = _TDNFVerifyResult(pContext);
    BAIL_ON_TDNF_ERROR(dwError);

cleanup:
    if (dataText)
    {
        gpgme_data_release(dataText);
    }
    if (dataSig)
    {
        gpgme_data_release(dataSig);
    }
    if (fpRepoMD)
    {
        fclose(fpRepoMD);
    }
    if (fpRepoMDSig)
    {
        fclose(fpRepoMDSig);
    }
    if (pContext)
    {
        gpgme_release(pContext);
    }
    return dwError;

error:
    goto cleanup;
}

uint32_t
TDNFVerifySignature(
    PTDNF_PLUGIN_HANDLE pHandle,
    const char *pcszRepoId,
    const char *pcszRepoMDUrl,
    const char *pcszRepoMDFile
    )
{
    uint32_t dwError = 0;
    char *pszRepoMDSigUrl = NULL;
    char *pszRepoMDSigFile = NULL;

    if (!pHandle || !pHandle->pTdnf || IsNullOrEmptyString(pcszRepoId) ||
        IsNullOrEmptyString(pcszRepoMDUrl) ||
        IsNullOrEmptyString(pcszRepoMDFile))
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = TDNFAllocateStringPrintf(&pszRepoMDSigUrl,
                  "%s%s",
                  pcszRepoMDUrl,
                  TDNF_REPO_METADATA_SIG_EXT);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFAllocateStringPrintf(&pszRepoMDSigFile,
                  "%s%s",
                  pcszRepoMDFile,
                  TDNF_REPO_METADATA_SIG_EXT);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFDownloadFile(pHandle->pTdnf,
                               pcszRepoId,
                               pszRepoMDSigUrl,
                               pszRepoMDSigFile,
                               pcszRepoId);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFVerifyRepoMDSignature(pHandle, pcszRepoMDFile, pszRepoMDSigFile);
    BAIL_ON_TDNF_ERROR(dwError);

cleanup:
    if (pszRepoMDSigFile)
    {
        unlink(pszRepoMDSigFile);
    }
    TDNF_SAFE_FREE_MEMORY(pszRepoMDSigUrl);
    TDNF_SAFE_FREE_MEMORY(pszRepoMDSigFile);
    return dwError;

error:
    pr_err("Error: %s %u\n", __FUNCTION__, dwError);
    goto cleanup;
}

static
uint32_t
TDNFHasRepo(
    PTDNF_PLUGIN_HANDLE pHandle,
    const char *pcszRepoId,
    int *pnHasRepo
    )
{
    uint32_t dwError = 0;
    PTDNF_REPO_GPG_CHECK_DATA pData = NULL;
    int nHasRepo = 0;

    if (!pHandle || IsNullOrEmptyString(pcszRepoId) || !pnHasRepo)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    for(pData = pHandle->pData; pData; pData = pData->pNext)
    {
        if (strcmp(pData->pszRepoId, pcszRepoId) == 0)
        {
            nHasRepo = 1;
            break;
        }
    }

    *pnHasRepo = nHasRepo;

error:
    return dwError;
}

uint32_t
TDNFRepoMDCheckSignature(
    PTDNF_PLUGIN_HANDLE pHandle,
    PTDNF_EVENT_CONTEXT pContext
    )
{
    uint32_t dwError = 0;
    const char *pcszRepoId = NULL;
    const char *pcszRepoMDUrl = NULL;
    const char *pcszRepoMDFile = NULL;
    int nHasRepo = 0;

    if (!pHandle || !pHandle->pTdnf || !pContext)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    /* we are looking for repo id first */
    dwError = TDNFEventContextGetItemString(
                  pContext,
                  TDNF_EVENT_ITEM_REPO_ID,
                  (const char **)&pcszRepoId);
    BAIL_ON_TDNF_ERROR(dwError);

    /* check if this repo id is in list for repo_gpgcheck */
    dwError = TDNFHasRepo(pHandle, pcszRepoId, &nHasRepo);
    BAIL_ON_TDNF_ERROR(dwError);

    /* if repo is not in list, return immediately */
    if (nHasRepo == 0)
    {
        goto cleanup;
    }

    dwError = TDNFEventContextGetItemString(
                  pContext,
                  TDNF_EVENT_ITEM_REPO_MD_URL,
                  (const char **)&pcszRepoMDUrl);
    BAIL_ON_TDNF_ERROR(dwError);
    dwError = TDNFEventContextGetItemString(
                  pContext,
                  TDNF_EVENT_ITEM_REPO_MD_FILE,
                  (const char **)&pcszRepoMDFile);
    BAIL_ON_TDNF_ERROR(dwError);
    /* download signature file and do detached signature check
       for repomd file
    */
    dwError = TDNFVerifySignature(pHandle,
                  pcszRepoId,
                  pcszRepoMDUrl,
                  pcszRepoMDFile);
    BAIL_ON_TDNF_ERROR(dwError);

cleanup:
    return dwError;

error:
    goto cleanup;
}

uint32_t
TDNFRepoGPGCheckReadConfig(
    PTDNF_PLUGIN_HANDLE pHandle,
    PTDNF_EVENT_CONTEXT pContext
    )
{
    uint32_t dwError = 0;
    int nEnabled = 0;
    PCONF_SECTION pSection = NULL;
    PTDNF_REPO_GPG_CHECK_DATA pData = NULL;

    if (!pHandle || !pHandle->pTdnf || !pContext)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    /* event has a repo section which has all the config data */
    dwError = TDNFEventContextGetItemPtr(
                  pContext,
                  TDNF_EVENT_ITEM_REPO_SECTION,
                  (const void **)&pSection);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFReadKeyValueBoolean(
                  pSection,
                  TDNF_REPO_CONFIG_REPO_GPGCHECK_KEY,
                  0,
                  &nEnabled);
    BAIL_ON_TDNF_ERROR(dwError);

    /*
     * if repo_gpgcheck is enabled, keep this repo id
     * section name is the repo id
    */
    if (nEnabled)
    {
        dwError = TDNFAllocateMemory(sizeof(*pData), 1, (void **)&pData);
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = TDNFAllocateString(pSection->pszName, &pData->pszRepoId);
        BAIL_ON_TDNF_ERROR(dwError);

        pData->pNext = pHandle->pData;
        pHandle->pData = pData;
    }
cleanup:
    return dwError;

error:
    TDNFFreeRepoGPGCheckData(pData);
    goto cleanup;
}
