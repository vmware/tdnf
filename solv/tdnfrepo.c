
/*
 * Copyright (C) 2015-2023 VMware, Inc. All Rights Reserved.
 *
 * Licensed under the GNU Lesser General Public License v2.1 (the "License");
 * you may not use this file except in compliance with the License. The terms
 * of the License are located in the COPYING file of this distribution.
 */

#include "includes.h"

// #### XML FILTER CODE ####

/***
* Resize the buffer specified by ppszCharBuffer and update pnBufferMaxLen
* to the length of the newly resized buffer if the nLengthToAdd would overflow
* the buffer.
***/
uint32_t 
checkAndResizeBuffer(char ** ppszCharBuffer, int * pnBufferMaxLen, int nLengthToAdd) {
    uint32_t dwError = 0;
    char * pszTempCharBuffer = NULL;
    if (ppszCharBuffer == NULL || *ppszCharBuffer == NULL || pnBufferMaxLen == NULL || *pnBufferMaxLen <= 0 || nLengthToAdd < 0) {
        dwError = ERROR_TDNF_TIME_FILTER_GENERAL;
        BAIL_ON_TDNF_TIME_FILTER_ERROR(dwError);
    }

    // calculate new max length
    int nTempMaxLen = *pnBufferMaxLen;
    int nBufferContentLen = strlen(*ppszCharBuffer);
    while (nBufferContentLen + nLengthToAdd + 1 >= nTempMaxLen)
    {
        nTempMaxLen *= 2;
    }
    if (nTempMaxLen >= TDNF_MAX_FILTER_INPUT_THRESHOLD)
    {
        dwError = ERROR_TDNF_TIME_FILTER_GENERAL;
        BAIL_ON_TDNF_TIME_FILTER_ERROR(dwError);
    }

    // only realloc if the size changed
    if (nTempMaxLen != *pnBufferMaxLen)
    {
        pszTempCharBuffer = realloc(*ppszCharBuffer, nTempMaxLen);
        if (!pszTempCharBuffer)
        {
            dwError = ERROR_TDNF_TIME_FILTER_MEMORY;
            BAIL_ON_TDNF_TIME_FILTER_ERROR(dwError);
        }
        //set expanded char buffer 
        *ppszCharBuffer = pszTempCharBuffer;
        *pnBufferMaxLen = nTempMaxLen;
    }

cleanup:
    return dwError;
error:
    pr_err("An error occurred during buffer resizing with the following code: %u\n", dwError);
    goto cleanup;
}

/***
* allocate a new string in ppszDestStr location with the linted description,
* all '&', '<', and '>' characters will be replaced with the xml escape
* character versions of each in line.
***/
uint32_t
xmlEscapeCharLinter(const char * pszStringToEscape, char ** ppszDestStr) {
    uint32_t dwError = 0;
    const char * amp = "&amp;";
    const char * gt = "&gt;";
    const char * lt = "&lt;";

    if (pszStringToEscape == NULL || ppszDestStr == NULL)
    {
        dwError = ERROR_TDNF_TIME_FILTER_GENERAL;
        BAIL_ON_TDNF_TIME_FILTER_ERROR(dwError);
    }

    // allocate new string for linted string
    int nStrToLintLen = (strlen(pszStringToEscape) + 1); // add one for null char
    char * pszLintedStr = malloc(nStrToLintLen * sizeof(char));
    if (!pszLintedStr)
    {
        dwError = ERROR_TDNF_TIME_FILTER_MEMORY;
        BAIL_ON_TDNF_TIME_FILTER_ERROR(dwError);
    }
    bzero(pszLintedStr, nStrToLintLen * sizeof(char));
    int nOffset = 0;
    int nLintedSize = nStrToLintLen;

    // Loop through string to lint looking for chars in need of escaping
    for (int i = 0; i < nStrToLintLen; i++)
    {
        char * pszCharToAdd = NULL;
        int nAddStrlen = 1;
        // check current char for escape character
        switch (pszStringToEscape[i])
        {
            case '&':
                pszCharToAdd = amp;
                break;
            case '>':
                pszCharToAdd = gt;
                break;
            case '<':
                pszCharToAdd = lt;
                break;
        }

        //resize buffer if needed
        if (pszCharToAdd != NULL)
        {
            nAddStrlen = strlen(pszCharToAdd);
        }
        dwError = checkAndResizeBuffer(&pszLintedStr, &nLintedSize, nAddStrlen);
        if (dwError)
        {
            BAIL_ON_TDNF_TIME_FILTER_ERROR(dwError);
        }

        // add linted char
        if (pszCharToAdd == NULL)
        {
            pszLintedStr[i + nOffset] = pszStringToEscape[i];
        }
        else
        {
            strcat(pszLintedStr, pszCharToAdd);
            nOffset += nAddStrlen - 1; // minus 1 to account for the original space used by the character
        }
    }

    // set Dest to linted string if all done
    *ppszDestStr = pszLintedStr;
    
cleanup:
    return dwError;
error:
    pr_err("An error occurred during escape character linting with the following code: %u\n", dwError);
    goto cleanup;
}

/***
* allocate a new buffer to location pszElementBuffer of the size
* nElementBufferMax or greater (in the case resizing is needed).
* a formatted start element with the name and attrs specified will be
* placed in the newly allocated buffer.
***/
uint32_t 
addElementStartToBuffer(char ** pszElementBuffer, int * nElementBufferMax, const char * pszElementName, const char ** ppszAttrs) {
    uint32_t dwError = 0;

    if (pszElementBuffer == NULL || nElementBufferMax == NULL || *nElementBufferMax < 0)
    {
        dwError = ERROR_TDNF_TIME_FILTER_GENERAL;
        BAIL_ON_TDNF_TIME_FILTER_ERROR(dwError);
    }

    // set default buffer max length
    if (*nElementBufferMax == 0)
    {
        *nElementBufferMax = TDNF_DEFAULT_TIME_FILTER_BUFF_SIZE;
    }
    *pszElementBuffer = malloc(*nElementBufferMax * sizeof(char));
    
    char * pszLintedAttrVal = NULL;
    char * pszTempBuffer = NULL;
    dwError = checkAndResizeBuffer(pszElementBuffer, nElementBufferMax, strlen(pszElementName) + 2);
    if (dwError)
    {
        BAIL_ON_TDNF_TIME_FILTER_ERROR(dwError);
    }
    sprintf(*pszElementBuffer, "<%s", pszElementName);
    for (int i = 0; ppszAttrs[i]; i += 2)
    {
        dwError = xmlEscapeCharLinter(ppszAttrs[i+1], &pszLintedAttrVal);
        if (dwError)
        {
            BAIL_ON_TDNF_TIME_FILTER_ERROR(dwError);
        }
        int nTempBufferLen = strlen(pszLintedAttrVal) + strlen(ppszAttrs[i]) + 5;
        dwError = checkAndResizeBuffer(pszElementBuffer, nElementBufferMax, nTempBufferLen);
        if (dwError)
        {
            BAIL_ON_TDNF_TIME_FILTER_ERROR(dwError);
        }
        pszTempBuffer = malloc(sizeof(char) * nTempBufferLen);
        if (!pszTempBuffer)
        {
            dwError = ERROR_TDNF_TIME_FILTER_MEMORY;
            BAIL_ON_TDNF_TIME_FILTER_ERROR(dwError);
        }
        sprintf(pszTempBuffer, " %s=\"%s\"", ppszAttrs[i], pszLintedAttrVal);
        strcat(*pszElementBuffer, pszTempBuffer);
        
        // free temp variables
        free(pszTempBuffer);
        pszTempBuffer = NULL;
        free(pszLintedAttrVal);
        pszLintedAttrVal = NULL;
    }
    strcat(*pszElementBuffer, ">");
    
cleanup:
    if (pszLintedAttrVal)
    {
        free(pszLintedAttrVal);
    }
    if (pszTempBuffer)
    {
        free(pszTempBuffer);
    }
    return dwError;
error:
    pr_err("An error occurred during start element generation with the following code: %u\n", dwError);
    goto cleanup;
}

/***
 * 
 ***/
uint32_t 
addElementEndToBuffer(char ** pszElementBuffer, int * nElementBufferMaxLen, const char * pszElementName) {
    uint32_t dwError = 0;
    if (pszElementBuffer == NULL || nElementBufferMaxLen == NULL || *nElementBufferMaxLen < 0)
    {
        dwError = ERROR_TDNF_TIME_FILTER_GENERAL;
        BAIL_ON_TDNF_TIME_FILTER_ERROR(dwError);
    }

    if (*nElementBufferMaxLen == 0 )
    {
        *nElementBufferMaxLen = TDNF_DEFAULT_TIME_FILTER_BUFF_SIZE;
    }
    *pszElementBuffer = malloc(*nElementBufferMaxLen * sizeof(char));
    
    dwError = checkAndResizeBuffer(pszElementBuffer, nElementBufferMaxLen, strlen(pszElementName) + 4);
    if (dwError)
    {
        BAIL_ON_TDNF_TIME_FILTER_ERROR(dwError);
    }
    sprintf(*pszElementBuffer, "</%s>", pszElementName);
    
cleanup:
    return dwError;
error:
    pr_err("An error occurred during end element generation with the following code: %u\n", dwError);
    goto cleanup;
}

/***
 * 
 ***/
uint32_t 
printElementStartToFile(FILE * pbOutfile, const char * pszElementName, const char ** ppszAttrs) {
    uint32_t dwError = 0;
    if (pbOutfile == NULL)
    {
        dwError = ERROR_TDNF_TIME_FILTER_GENERAL;
        BAIL_ON_TDNF_TIME_FILTER_ERROR(dwError);
    }

    int nStartElementBufferLength = TDNF_DEFAULT_TIME_FILTER_BUFF_SIZE;
    char * pszStartElement = NULL;

    dwError = addElementStartToBuffer(&pszStartElement, &nStartElementBufferLength, pszElementName, ppszAttrs);
    if (dwError)
    {
        BAIL_ON_TDNF_TIME_FILTER_ERROR(dwError);
    }
    fprintf(pbOutfile, "%s", pszStartElement);
    if (ferror(pbOutfile))
    {
        dwError = ERROR_TDNF_TIME_FILTER_IO;
        BAIL_ON_TDNF_TIME_FILTER_ERROR(dwError);
    }

cleanup:
    if (pszStartElement)
    {
        free(pszStartElement);
    }
    return dwError;
error:
    pr_err("An error occurred during start element printing with the following code: %u\n", dwError);
    goto cleanup;
}

/***
 * 
 ***/
uint32_t 
printElementEndToFile(FILE * pbOutfile, const char * pszElementName) {
    uint32_t dwError = 0;
    if (pbOutfile == NULL)
    {
        dwError = ERROR_TDNF_TIME_FILTER_GENERAL;
        BAIL_ON_TDNF_TIME_FILTER_ERROR(dwError);
    }

    int nEndElementBufferLength = TDNF_DEFAULT_TIME_FILTER_BUFF_SIZE;
    char * pszEndElement = NULL;

    dwError = addElementEndToBuffer(&pszEndElement, &nEndElementBufferLength, pszElementName);
    if (dwError)
    {
        BAIL_ON_TDNF_TIME_FILTER_ERROR(dwError);
    }
    fprintf(pbOutfile, "%s", pszEndElement);
    if (ferror(pbOutfile))
    {
        dwError = ERROR_TDNF_TIME_FILTER_IO;
        BAIL_ON_TDNF_TIME_FILTER_ERROR(dwError);
    }

cleanup:
    if (pszEndElement)
    {
        free(pszEndElement);
    }
    return dwError;
error:
    pr_err("An error occurred during end element printing with the following code: %u\n", dwError);
    goto cleanup;
}

/***
 * 
 ***/
void 
TDNFFilterStartElement(void *userData, const char * name, const char ** attrs) {
    uint32_t dwError = 0;
    char * pszStartElementBuffer = NULL;
    // load tracking data
    TDNFFilterData * pTracking = (TDNFFilterData *)userData;
    int nAddNewLineAfterStart = pTracking->nPrevElement == 0;
    char szNewLineBuffer[2];
    if (nAddNewLineAfterStart)
    {
        sprintf(szNewLineBuffer, "\n");
    }
    else
    {
        bzero(szNewLineBuffer, sizeof(szNewLineBuffer)); // don't assume memory zero'd
    }

    // increment depth
    pTracking->nDepth += 1;
    pTracking->nPrevElement = 0;

    // new package to parse or currently parsing package info
    if (strcmp(name, "package") == 0 || pTracking->nInPackage)
    {
        pTracking->nInPackage = 1;

        // already found/checked time
        if (pTracking->nTimeFound && pTracking->nPrintPackage)
        {
            fprintf(pTracking->pbOutfile, "%s", szNewLineBuffer);
            if (ferror(pTracking->pbOutfile))
            {
                dwError = ERROR_TDNF_TIME_FILTER_IO;
                BAIL_ON_TDNF_TIME_FILTER_ERROR(dwError);
            }

            dwError = printElementStartToFile(pTracking->pbOutfile, name, attrs);
            if (dwError)
            {
                BAIL_ON_TDNF_TIME_FILTER_ERROR(dwError);
            }
        }
        else
        { // still checking for time
            if (strcmp(name, "time") == 0)
            {
                // time found
                // validate file POSIX time
                for (int i = 0; attrs[i]; i += 2)
                {
                    if (strcmp(attrs[i], "file") == 0)
                    { 
                        // file time is the time the package is published to the repo
                        // when this is less than our search time, allow the package to be
                        // printed to the temp repo file, otherwise the current package
                        // can be discarded.
                        errno = 0;
                        char * pszSnapshotTimeEnd = NULL;
                        long nCurrentPackageTime = strtoll(attrs[i+1], pszSnapshotTimeEnd, 10);
                        if (errno || pszSnapshotTimeEnd == attrs[i+1])
                        {
                            dwError = ERROR_TDNF_TIME_FILTER_PARSE;
                            BAIL_ON_TDNF_TIME_FILTER_ERROR(dwError);
                        }
                        pTracking->nPrintPackage = (nCurrentPackageTime <= pTracking->nSearchTime);
                        pTracking->nTimeFound = 1;
                        break;
                    }
                }
                if (pTracking->nPrintPackage)
                {
                    // print buffer when time is found
                    fprintf(pTracking->pbOutfile, "%s", pTracking->pszElementBuffer);
                    if (ferror(pTracking->pbOutfile))
                    {
                        dwError = ERROR_TDNF_TIME_FILTER_IO;
                        BAIL_ON_TDNF_TIME_FILTER_ERROR(dwError);
                    }

                    fprintf(pTracking->pbOutfile, "%s", szNewLineBuffer);
                    if (ferror(pTracking->pbOutfile))
                    {
                        dwError = ERROR_TDNF_TIME_FILTER_IO;
                        BAIL_ON_TDNF_TIME_FILTER_ERROR(dwError);
                    }

                    // print time element
                    dwError = printElementStartToFile(pTracking->pbOutfile, name, attrs);
                    if (dwError)
                    {
                        BAIL_ON_TDNF_TIME_FILTER_ERROR(dwError);
                    }
                }
            }
            else if (!pTracking->nTimeFound)
            {
                // if we haven't found a time yet, the element must be stored
                // add to file buffer
                int nStartElementBufferSize = TDNF_DEFAULT_TIME_FILTER_BUFF_SIZE;
                pszStartElementBuffer = NULL;

                dwError = addElementStartToBuffer(&pszStartElementBuffer, &nStartElementBufferSize, name, attrs);
                if (dwError)
                {
                    BAIL_ON_TDNF_TIME_FILTER_ERROR(dwError);
                }
                int nLenToAdd = strlen(pszStartElementBuffer);
                nLenToAdd += strlen(szNewLineBuffer); // +1 if newLine character present

                dwError = checkAndResizeBuffer(&pszStartElementBuffer, &nStartElementBufferSize, strlen(szNewLineBuffer));
                if (dwError)
                {
                    BAIL_ON_TDNF_TIME_FILTER_ERROR(dwError);
                }
                strcat(pszStartElementBuffer, szNewLineBuffer);
                
                dwError = checkAndResizeBuffer(&(pTracking->pszElementBuffer), &(pTracking->nBufferMaxLen), nLenToAdd);
                if (dwError)
                {
                    BAIL_ON_TDNF_TIME_FILTER_ERROR(dwError);
                }
                strcat(pTracking->pszElementBuffer, pszStartElementBuffer);
            }
        }
    }
    else
    { // not in a package or parsing a new package
        fprintf(pTracking->pbOutfile, "%s", szNewLineBuffer);
        if (ferror(pTracking->pbOutfile))
        {
            dwError = ERROR_TDNF_TIME_FILTER_IO;
            BAIL_ON_TDNF_TIME_FILTER_ERROR(dwError);
        }
        // output line
        dwError = printElementStartToFile(pTracking->pbOutfile, name, attrs);
        if (dwError)
        {
            BAIL_ON_TDNF_TIME_FILTER_ERROR(dwError);
        }
    }
cleanup:
    if (pszStartElementBuffer)
    {
        free(pszStartElementBuffer);
    }
    return;
error:
    pr_err("An error occurred during start element parsing with the following code: %u\n", dwError);
    goto cleanup;
}

/***
 * 
 ***/
void 
TDNFFilterEndElement(void * userData, const char * name) {
    uint32_t dwError = 0;
    char * pszElementBuffer = NULL;
    // load tracking data
    TDNFFilterData * pTracking = (TDNFFilterData *)userData;

    // decrement depth
    pTracking->nDepth -= 1;
    pTracking->nPrevElement = 2;

    if (!pTracking->nInPackage || pTracking->nPrintPackage)
    {
        // print end element to file
        dwError = printElementEndToFile(pTracking->pbOutfile, name);
        if (dwError)
        {
            BAIL_ON_TDNF_TIME_FILTER_ERROR(dwError);
        }

    } 
    else if (pTracking->nInPackage && !pTracking->nTimeFound)
    {
        int nEndElementBufferLen = TDNF_DEFAULT_TIME_FILTER_BUFF_SIZE;
        pszElementBuffer = NULL;

        // add end element to buffer
        dwError = addElementEndToBuffer(&pszElementBuffer, &nEndElementBufferLen, name);
        if (dwError)
        {
            BAIL_ON_TDNF_TIME_FILTER_ERROR(dwError);
        }
        int nEndElementLen = strlen(pszElementBuffer);

        dwError = checkAndResizeBuffer(&(pTracking->pszElementBuffer), &(pTracking->nBufferMaxLen), nEndElementLen);
        if (dwError)
        {
            BAIL_ON_TDNF_TIME_FILTER_ERROR(dwError);
        }
        strcat(pTracking->pszElementBuffer, pszElementBuffer);

    } // else do nothing

    if (strcmp(name, "package") == 0)
    { // on end package, reset tracking function
        // reset userData
        pTracking->nBufferLen = 0;
        bzero(pTracking->pszElementBuffer, pTracking->nBufferMaxLen);
        pTracking->nInPackage = 0;
        pTracking->nPrintPackage = 0;
        pTracking->nTimeFound = 0;
    }
cleanup:
    if (pszElementBuffer)
    {
        free(pszElementBuffer);
    }
    return;
error:
    pr_err("An error occurred during end element parsing with the following code: %u\n", dwError);
    goto cleanup;
}

/***
 * 
 ***/
void 
TDNFFilterCharDataHandler(void * userData, const char * content, int length) {
    uint32_t dwError = 0;
    // load tracking data
    TDNFFilterData * pTracking = (TDNFFilterData *)userData;
    pTracking->nPrevElement = 1;

    char * pszCharData = malloc((length + 1) * sizeof(char));
    if (!pszCharData)
    {
        dwError = ERROR_TDNF_TIME_FILTER_MEMORY;
        BAIL_ON_TDNF_TIME_FILTER_ERROR(dwError);
    }
    bzero(pszCharData, (length + 1) * sizeof(char));
    strncpy(pszCharData, content, length);
    char * pszLintedCharData = NULL;
    dwError = xmlEscapeCharLinter(pszCharData, &pszLintedCharData);
    if (dwError)
    {
        BAIL_ON_TDNF_TIME_FILTER_ERROR(dwError);
    }

    // check params 
    if (!pTracking->nInPackage || pTracking->nPrintPackage)
    {
        // print to file
        fprintf(pTracking->pbOutfile, "%s", pszLintedCharData);
        if (ferror(pTracking->pbOutfile))
        {
            dwError = ERROR_TDNF_TIME_FILTER_IO;
            BAIL_ON_TDNF_TIME_FILTER_ERROR(dwError);
        }
    }
    else if (pTracking->nInPackage && !pTracking->nTimeFound)
    {
        // add to buffer
        dwError = checkAndResizeBuffer(&(pTracking->pszElementBuffer), &(pTracking->nBufferMaxLen), strlen(pszLintedCharData));
        if (dwError)
        {
            BAIL_ON_TDNF_TIME_FILTER_ERROR(dwError);
        }
        strcat(pTracking->pszElementBuffer, pszLintedCharData);
    } // else do nothing (skipped package)

cleanup:
    if (pszLintedCharData)
    {
        free(pszLintedCharData);
    }
    if (pszCharData)
    {
        free(pszCharData);
    }
    return;
error:
    pr_err("An error occurred during char data handling with the following code: %u\n", dwError);
    goto cleanup;
}

/***
 * 
 ***/
char * 
TDNFFilterFile(const char * pszInFilePath, const char * pszSnapshotTime) {
    // vars
    uint32_t dwError = 0;
    TDNFFilterData pData;
    bzero(&pData, sizeof(TDNFFilterData));
    time_t nSnapshotTime;
    bzero(&nSnapshotTime, sizeof(time_t));
    XML_Parser bParser;
    bzero(&bParser, sizeof(XML_Parser));
    FILE * pbInFile = NULL;
    FILE * pbOutFile = NULL;
    char pszTimeExtension[100];
    char * pszOutFilePath = NULL;

    // convert snapshot string to time for use by the parser and the temp file name
    errno = 0;
    char * pszSnapshotTimeEnd = NULL;
    nSnapshotTime = strtoll(pszSnapshotTime, &pszSnapshotTimeEnd, 10);
    if (errno || pszSnapshotTimeEnd == pszSnapshotTime)
    {
        dwError = ERROR_TDNF_TIME_FILTER_PARSE;
        BAIL_ON_TDNF_TIME_FILTER_ERROR(dwError);
    }

    //create output file ending
    sprintf(pszTimeExtension, "-%lld.xml", nSnapshotTime);

    // find total extension length
    int nInFileExtLen = 4; // len of ".xml"
    char * pszFileExt = strrchr(pszInFilePath, '.');
    if (strcmp(pszFileExt, ".xml") != 0)
    {
        nInFileExtLen += strlen(pszFileExt);
    }

    // calculate outfile length and allocate
    int nInFileLen = strlen(pszInFilePath);
    int nOutFileLen = (nInFileLen - nInFileExtLen) + strlen(pszTimeExtension) + 1;
    pszOutFilePath = malloc(nOutFileLen * sizeof(char));
    if (!pszOutFilePath)
    {
        dwError = ERROR_TDNF_TIME_FILTER_MEMORY;
        BAIL_ON_TDNF_TIME_FILTER_ERROR(dwError);
    }
    bzero(pszOutFilePath, nOutFileLen * sizeof(char));

    // use infile path + timestamp as new output file
    strncpy(pszOutFilePath, pszInFilePath, nInFileLen - nInFileExtLen); // remove extension to be added with the name
    strcat(pszOutFilePath, pszTimeExtension);

    // init vars, load files
    pbInFile = solv_xfopen(pszInFilePath, "r");
    if (!pbInFile)
    {
        dwError = ERROR_TDNF_TIME_FILTER_IO;
        BAIL_ON_TDNF_TIME_FILTER_ERROR(dwError);
    }
    pbOutFile = fopen(pszOutFilePath, "w");
    if (!pbOutFile)
    {
        dwError = ERROR_TDNF_TIME_FILTER_IO;
        BAIL_ON_TDNF_TIME_FILTER_ERROR(dwError);
    }

    pData.nBufferMaxLen = TDNF_DEFAULT_TIME_FILTER_BUFF_SIZE;
    pData.pszElementBuffer = (char *)malloc(pData.nBufferMaxLen * sizeof(char));
    if (!pData.pszElementBuffer)
    {
        dwError = ERROR_TDNF_TIME_FILTER_MEMORY;
        BAIL_ON_TDNF_TIME_FILTER_ERROR(dwError);
    }
    bzero(pData.pszElementBuffer, pData.nBufferMaxLen);
    pData.pbOutfile = pbOutFile;
    pData.nSearchTime = nSnapshotTime;
    pData.nDepth = 0;
    pData.nBufferLen = 0;
    pData.nInPackage = 0;
    pData.nPrintPackage = 0;
    pData.nTimeFound = 0;

    //create parser
    bParser = XML_ParserCreate(NULL);
    if (!bParser)
    {
        dwError = ERROR_TDNF_TIME_FILTER_PARSE;
        BAIL_ON_TDNF_TIME_FILTER_ERROR(dwError);
    }

    XML_SetUserData(bParser, &pData);
    XML_SetElementHandler(bParser, TDNFFilterStartElement, TDNFFilterEndElement);
    XML_SetCharacterDataHandler(bParser, TDNFFilterCharDataHandler);

    //parse XML
    fprintf(pbOutFile, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
    if (ferror(pbOutFile))
    {
        dwError = ERROR_TDNF_TIME_FILTER_IO;
        BAIL_ON_TDNF_TIME_FILTER_ERROR(dwError);
    }
    int nInputEof;
    do
    {
        void * pszXMLParseBuffer = XML_GetBuffer(bParser, BUFSIZ);
        if (!pszXMLParseBuffer)
        {
            fprintf(stderr, "Couldn't allocate memory for buffer\n");
            dwError = ERROR_TDNF_TIME_FILTER_MEMORY;
            BAIL_ON_TDNF_TIME_FILTER_ERROR(dwError);
        }

        const size_t len = fread(pszXMLParseBuffer, 1, BUFSIZ - 1, pbInFile);
        ((char *)pszXMLParseBuffer)[len] = '\0';
        if (ferror(pbInFile))
        {
            dwError = ERROR_TDNF_TIME_FILTER_IO;
            BAIL_ON_TDNF_TIME_FILTER_ERROR(dwError);
        }

        nInputEof = feof(pbInFile);

        if (XML_ParseBuffer(bParser, (int)len, nInputEof) == XML_STATUS_ERROR)
        {
            fprintf(stderr,
                "Parse error at line %lu:\n%s\n",
                XML_GetCurrentLineNumber(bParser),
                XML_ErrorString(XML_GetErrorCode(bParser)));
            dwError = ERROR_TDNF_TIME_FILTER_PARSE;
            BAIL_ON_TDNF_TIME_FILTER_ERROR(dwError);
        }
    } while (!nInputEof);

cleanup:
    if (pData.pszElementBuffer) {
        free(pData.pszElementBuffer);
    }

    if (bParser)
    {
        XML_ParserFree(bParser);
    }

    if (pbOutFile)
    {
        fclose(pbOutFile);
    }

    if (pbInFile)
    {
        fclose(pbInFile);
    }

    return pszOutFilePath;
error:
    pr_err("An error occurred during snapshot filtering with the following code: %u\n", dwError);
    goto cleanup;
}
// #### END XML SNAPSHOT FILTER CODE ####

uint32_t
SolvLoadRepomd(
    Repo* pRepo,
    const char* pszRepomd
    )
{
    uint32_t dwError = 0;
    FILE *fp = NULL;
    if( !pRepo || IsNullOrEmptyString(pszRepomd))
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);
    }

    fp = fopen(pszRepomd, "r");
    if (fp == NULL)
    {
        dwError = ERROR_TDNF_SOLV_IO;
        BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);
    }
    if (repo_add_repomdxml(pRepo, fp, 0))
    {
        dwError = ERROR_TDNF_SOLV_IO;
        BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);
    }
cleanup:
    if(fp != NULL)
    {
        fclose(fp);
    }
    return dwError;

error:
    goto cleanup;
}

uint32_t
SolvLoadRepomdPrimary(
    Repo* pRepo,
    const char* pszPrimary
    )
{
    uint32_t dwError = 0;
    FILE *fp = NULL;
    if( !pRepo || IsNullOrEmptyString(pszPrimary))
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);
    }

    fp = solv_xfopen(pszPrimary, "r");
    if (fp == NULL)
    {
        dwError = ERROR_TDNF_SOLV_IO;
        BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);
    }
    if (repo_add_rpmmd(pRepo, fp, 0, 0))
    {
        dwError = ERROR_TDNF_SOLV_IO;
        BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);
    }
cleanup:
    if(fp != NULL)
    {
        fclose(fp);
    }
    return dwError;

error:
    goto cleanup;

}

uint32_t
SolvLoadRepomdFilelists(
    Repo* pRepo,
    const char* pszFilelists
    )
{
    uint32_t dwError = 0;
    FILE *fp = NULL;
    if(!pRepo || IsNullOrEmptyString(pszFilelists))
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);
    }
    fp = solv_xfopen(pszFilelists, "r");
    if (fp == NULL)
    {
        dwError = ERROR_TDNF_SOLV_IO;
        BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);
    }
    if (repo_add_rpmmd(pRepo, fp, "FL", REPO_EXTEND_SOLVABLES))
    {
        dwError = ERROR_TDNF_SOLV_FAILED;
        BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);
    }
cleanup:
    if(fp != NULL)
        fclose(fp);
    return dwError;

error:
    goto cleanup;
}

uint32_t
SolvLoadRepomdUpdateinfo(
    Repo* pRepo,
    const char* pszUpdateinfo
    )
{
    uint32_t dwError = 0;
    FILE *fp = NULL;
    if( !pRepo || IsNullOrEmptyString(pszUpdateinfo))
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);
    }
    fp = solv_xfopen(pszUpdateinfo, "r");
    if (fp == NULL)
    {
        dwError = ERROR_TDNF_SOLV_IO;
        BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);
    }
    if (repo_add_updateinfoxml(pRepo, fp, 0))
    {
        dwError = ERROR_TDNF_SOLV_IO;
        BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);
    }
cleanup:
    if(fp != NULL)
    {
        fclose(fp);
    }
    return dwError;

error:
    goto cleanup;
}

uint32_t
SolvLoadRepomdOther(
    Repo* pRepo,
    const char* pszOther
    )
{
    uint32_t dwError = 0;
    FILE *fp = NULL;
    if( !pRepo || IsNullOrEmptyString(pszOther))
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);
    }

    fp = solv_xfopen(pszOther, "r");
    if (fp == NULL)
    {
        dwError = ERROR_TDNF_SOLV_IO;
        BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);
    }
    if (repo_add_rpmmd(pRepo, fp, 0, REPO_EXTEND_SOLVABLES))
    {
        dwError = ERROR_TDNF_SOLV_IO;
        BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);
    }
cleanup:
    if(fp != NULL)
    {
        fclose(fp);
    }
    return dwError;

error:
    goto cleanup;

}

uint32_t
SolvReadYumRepo(
    Repo *pRepo,
    const char *pszRepoName,
    const char *pszRepomd,
    const char *pszPrimary,
    const char *pszFilelists,
    const char *pszUpdateinfo,
    const char *pszOther,
    const char *pszSnapshotTime
    )
{
    uint32_t dwError = 0;
    // new vars for Filter
    char * tempPrimaryRepoFile = NULL;
    // end new vars
    if(!pRepo || !pszRepoName || !pszRepomd || !pszPrimary)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);
    }

    dwError = SolvLoadRepomd(pRepo, pszRepomd);
    BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);


    // Run filter if option present
    if (pszSnapshotTime != NULL){
        tempPrimaryRepoFile = TDNFFilterFile(pszPrimary, pszSnapshotTime);
        if (tempPrimaryRepoFile == NULL)
        {
            dwError = ERROR_TDNF_TIME_FILTER_GENERAL;
            BAIL_ON_TDNF_TIME_FILTER_ERROR(dwError);
        }
        pszPrimary = tempPrimaryRepoFile;
    }
    // End filter code

    dwError = SolvLoadRepomdPrimary(pRepo, pszPrimary);
    BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);

    if(pszFilelists)
    {
        dwError = SolvLoadRepomdFilelists(pRepo, pszFilelists);
        BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);
    }

    if(pszUpdateinfo)
    {
        dwError = SolvLoadRepomdUpdateinfo(pRepo, pszUpdateinfo);
        BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);
    }

    if(pszOther)
    {
        dwError = SolvLoadRepomdOther(pRepo, pszOther);
        BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);
    }


cleanup:
    if(tempPrimaryRepoFile != NULL)
    {
        remove(tempPrimaryRepoFile);
        free(tempPrimaryRepoFile);
    }

    return dwError;

error:
    goto cleanup;
}

uint32_t
SolvCountPackages(
    PSolvSack pSack,
    uint32_t* pdwCount
    )
{
    uint32_t dwError = 0;
    uint32_t dwCount = 0;
    const Pool* pool = 0;
    Id p = 0;
    if(!pSack || !pSack->pPool || !pdwCount)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);
    }
    pool = pSack->pPool;
    FOR_POOL_SOLVABLES(p)
    {
        dwCount++;
    }
    *pdwCount = dwCount;
cleanup:
    return dwError;
error:
    goto cleanup;

}

static
uint32_t
readRpmsFromDir(
    Repo *pRepo,
    const char *pszDir
)
{
    uint32_t dwError = 0;
    DIR *pDir = NULL;
    struct dirent *pEnt = NULL;
    char *pszPath = NULL;

    pDir = opendir(pszDir);
    if(pDir == NULL) {
        dwError = errno;
        BAIL_ON_TDNF_SYSTEM_ERROR(dwError);
    }
    while ((pEnt = readdir (pDir)) != NULL ) {
        int isDir;

        if (pEnt->d_name[0] == '.') {
            /* skip '.', '..', but also any dir name starting with '.' */
            continue;
        }

        dwError = TDNFJoinPath(
                      &pszPath,
                      pszDir,
                      pEnt->d_name,
                      NULL);
        BAIL_ON_TDNF_ERROR(dwError);

        dwError = TDNFIsDir(pszPath, &isDir);
        BAIL_ON_TDNF_ERROR(dwError);

        if (isDir) {
            dwError = readRpmsFromDir(pRepo, pszPath);
            BAIL_ON_TDNF_ERROR(dwError);
        } else if (strcmp(&pEnt->d_name[strlen(pEnt->d_name)-4], ".rpm") == 0) {
            if(!repo_add_rpm(pRepo,
                             pszPath,
                             REPO_REUSE_REPODATA|REPO_NO_INTERNALIZE)) {
                dwError = ERROR_TDNF_INVALID_PARAMETER;
                BAIL_ON_TDNF_ERROR(dwError);
            }
        }
        TDNF_SAFE_FREE_MEMORY(pszPath);
    }
cleanup:
    if(pDir) {
        closedir(pDir);
    }
    TDNF_SAFE_FREE_MEMORY(pszPath);

    return dwError;

error:
    goto cleanup;
}

uint32_t
SolvReadRpmsFromDirectory(
    Repo *pRepo,
    const char *pszDir
    )
{
    uint32_t dwError = 0;

    if(!pRepo || IsNullOrEmptyString(pszDir))
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);
    }

    dwError = readRpmsFromDir(pRepo, pszDir);
    BAIL_ON_TDNF_ERROR(dwError);

    repo_internalize(pRepo);

cleanup:
    return dwError;
error:
    goto cleanup;
}

uint32_t
SolvReadInstalledRpms(
    Repo* pRepo,
    const char *pszCacheFileName
    )
{
    uint32_t dwError = 0;
    FILE *pCacheFile = NULL;
    int  dwFlags = 0;

    if(!pRepo)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);
    }

    if(pszCacheFileName && access(pszCacheFileName, F_OK) == 0)
    {
        /* coverity[toctou] */
        pCacheFile = fopen(pszCacheFileName, "r");

        if(!pCacheFile)
        {
            dwError = errno;
            BAIL_ON_TDNF_SYSTEM_ERROR(dwError);
        }
    }

    dwFlags = REPO_REUSE_REPODATA | RPM_ADD_WITH_HDRID | REPO_USE_ROOTDIR;
    dwError = repo_add_rpmdb_reffp(pRepo, pCacheFile, dwFlags);

    if (dwError)
    {
        dwError = ERROR_TDNF_SOLV_IO;
        BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);
    }

cleanup:
    if (pCacheFile)
        fclose(pCacheFile);
    return dwError;

error:
    goto cleanup;
}

uint32_t
SolvCalculateCookieForFile(
    const char *pszFilePath,
    unsigned char *pszCookie
    )
{
    FILE *fp = NULL;
    int32_t nLen = 0;
    uint32_t dwError = 0;
    Chksum *pChkSum = NULL;
    char buf[BUFSIZ] = {0};

    if (!pszFilePath)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);
    }

    fp = fopen(pszFilePath, "r");
    if (!fp)
    {
        dwError = ERROR_TDNF_SOLV_IO;
        BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);
    }

    pChkSum = solv_chksum_create(REPOKEY_TYPE_SHA256);
    if (!pChkSum)
    {
        dwError = ERROR_TDNF_SOLV_CHKSUM;
        BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);
    }
    solv_chksum_add(pChkSum, SOLV_COOKIE_IDENT, strlen(SOLV_COOKIE_IDENT));

    while ((nLen = fread(buf, 1, sizeof(buf) - 1, fp)) > 0)
    {
          solv_chksum_add(pChkSum, buf, nLen);
          memset(buf, 0, sizeof(buf));
    }
    solv_chksum_free(pChkSum, pszCookie);

cleanup:
    if (fp)
    {
        fclose(fp);
    }

    return dwError;

error:
    goto cleanup;
}

/* Create a name for the repo cache path based on repo name and
   a hash of the url.
*/
uint32_t
SolvCreateRepoCacheName(
    const char *pszName,
    const char *pszUrl,
    char **ppszCacheName
    )
{
    uint32_t dwError = 0;
    Chksum *pChkSum = NULL;
    unsigned char pCookie[SOLV_COOKIE_LEN] = {0};
    char pszCookie[9] = {0};
    char *pszCacheName = NULL;

    if (!pszName || !pszUrl || !ppszCacheName)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);
    }

    pChkSum = solv_chksum_create(REPOKEY_TYPE_SHA256);
    if (!pChkSum)
    {
        dwError = ERROR_TDNF_SOLV_CHKSUM;
        BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);
    }
    solv_chksum_add(pChkSum, pszUrl, strlen(pszUrl));
    solv_chksum_free(pChkSum, pCookie);

    snprintf(pszCookie, sizeof(pszCookie), "%.2x%.2x%.2x%.2x",
             pCookie[0], pCookie[1], pCookie[2], pCookie[3]);

    dwError = TDNFAllocateStringPrintf(&pszCacheName, "%s-%s", pszName, pszCookie);
    BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);

    *ppszCacheName = pszCacheName;
cleanup:
    return dwError;

error:
    TDNF_SAFE_FREE_MEMORY(pszCacheName);
    goto cleanup;
}

uint32_t
SolvGetMetaDataCachePath(
    PSOLV_REPO_INFO_INTERNAL pSolvRepoInfo,
    char** ppszCachePath
    )
{
    char *pszCachePath = NULL;
    uint32_t dwError = 0;
    Repo *pRepo = NULL;

    if (!pSolvRepoInfo || !pSolvRepoInfo->pRepo || !ppszCachePath)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);
    }
    pRepo = pSolvRepoInfo->pRepo;
    if (!IsNullOrEmptyString(pRepo->name))
    {
        dwError = TDNFAllocateStringPrintf(
                      &pszCachePath,
                      "%s/%s/%s.solv",
                      pSolvRepoInfo->pszRepoCacheDir,
                      TDNF_SOLVCACHE_DIR_NAME,
                      pRepo->name);
        BAIL_ON_TDNF_ERROR(dwError);
    }
    *ppszCachePath = pszCachePath;
cleanup:
    return dwError;
error:
    TDNF_SAFE_FREE_MEMORY(pszCachePath);
    goto cleanup;
}

uint32_t
SolvAddSolvMetaData(
    PSOLV_REPO_INFO_INTERNAL pSolvRepoInfo,
    const char *pszTempSolvFile
    )
{
    uint32_t dwError = 0;
    Repo *pRepo = NULL;
    FILE *fp = NULL;
    int i = 0;

    if (!pSolvRepoInfo || !pSolvRepoInfo->pRepo || !pszTempSolvFile)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);
    }

    pRepo = pSolvRepoInfo->pRepo;
    if (!pRepo->pool)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);
    }

    for (i = pRepo->start; i < pRepo->end; i++)
    {
         if (pRepo->pool->solvables[i].repo != pRepo)
         {
             break;
         }
    }
    if (i < pRepo->end)
    {
        goto cleanup;
    }
    fp = fopen (pszTempSolvFile, "r");
    if (fp == NULL)
    {
        dwError = errno;
        BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);
    }
    repo_empty(pRepo, 1);
    if (repo_add_solv(pRepo, fp, SOLV_ADD_NO_STUBS))
    {
        dwError = ERROR_TDNF_ADD_SOLV;
        BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);
    }

cleanup:
    if (fp != NULL)
    {
        fclose(fp);
    }
    return dwError;
error:
    goto cleanup;
}

uint32_t
SolvUseMetaDataCache(
    const PSolvSack pSack,
    PSOLV_REPO_INFO_INTERNAL pSolvRepoInfo,
    int       *nUseMetaDataCache
    )
{
    uint32_t dwError = 0;
    FILE *fp = NULL;
    Repo *pRepo = NULL;
    const unsigned char *pszCookie = NULL;
    unsigned char pszTempCookie[32];
    char *pszCacheFilePath = NULL;

    if (!pSack || !pSolvRepoInfo || !pSolvRepoInfo->pRepo)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);
    }
    pRepo = pSolvRepoInfo->pRepo;
    pszCookie = pSolvRepoInfo->nCookieSet ? pSolvRepoInfo->cookie : NULL;

    dwError = SolvGetMetaDataCachePath(pSolvRepoInfo, &pszCacheFilePath);
    BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);

    if (IsNullOrEmptyString(pszCacheFilePath))
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);
    }
    fp = fopen(pszCacheFilePath, "r");
    if (fp == NULL)
    {
        dwError = ERROR_TDNF_SOLV_CACHE_NOT_CREATED;
        BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);
    }
    // Reading the cookie from cached Solv File
    if (fseek (fp, -sizeof(pszTempCookie), SEEK_END) || fread (pszTempCookie, sizeof(pszTempCookie), 1, fp) != 1)
    {
        dwError = ERROR_TDNF_SOLV_IO;
        BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);
    }
    // compare the calculated cookie with the one read from Solv file
    if (pszCookie && memcmp (pszCookie, pszTempCookie, sizeof(pszTempCookie)) != 0)
    {
        dwError = ERROR_TDNF_SOLV_IO;
        BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);
    }
    rewind(fp);
    if (repo_add_solv(pRepo, fp, 0))
    {
        dwError = ERROR_TDNF_ADD_SOLV;
        BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);
    }
    *nUseMetaDataCache = 1;

cleanup:
    if (fp != NULL)
    {
       fclose(fp);
    }
    TDNF_SAFE_FREE_MEMORY(pszCacheFilePath);
    return dwError;
error:
    if (dwError == ERROR_TDNF_SOLV_CACHE_NOT_CREATED)
    {
        dwError = 0;
    }
    goto cleanup;
}

uint32_t
SolvCreateMetaDataCache(
    const PSolvSack pSack,
    PSOLV_REPO_INFO_INTERNAL pSolvRepoInfo
    )
{
    uint32_t dwError = 0;
    Repo *pRepo = NULL;
    FILE *fp = NULL;
    int fd = 0;
    char *pszSolvCacheDir = NULL;
    char *pszTempSolvFile = NULL;
    char *pszCacheFilePath = NULL;
    mode_t mask = 0;

    if (!pSack || !pSolvRepoInfo)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);
    }

    pRepo = pSolvRepoInfo->pRepo;
    dwError = TDNFJoinPath(
                  &pszSolvCacheDir,
                  pSolvRepoInfo->pszRepoCacheDir,
                  TDNF_SOLVCACHE_DIR_NAME,
                  NULL);
    BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);

    if (access(pszSolvCacheDir, W_OK | X_OK))
    {
        if(errno != ENOENT)
        {
            dwError = errno;
        }
        BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);

        dwError = TDNFUtilsMakeDirs(pszSolvCacheDir);
        if (dwError == ERROR_TDNF_ALREADY_EXISTS)
        {
            dwError = 0;
        }
        BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);
    }

    pszTempSolvFile = solv_dupjoin(pszSolvCacheDir, "/", ".newsolv-XXXXXX");
    mask = umask(S_IRUSR | S_IWUSR | S_IRWXG);
    umask(mask);
    fd = mkstemp(pszTempSolvFile);
    if (fd < 0)
    {
        dwError = ERROR_TDNF_SOLV_IO;
        BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);
    }
    fchmod(fd, 0444);
    fp = fdopen(fd, "w");
    if (fp == NULL)
    {
        dwError = ERROR_TDNF_SOLV_IO;
        BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);
    }
    if (repo_write(pRepo, fp))
    {
        dwError = ERROR_TDNF_REPO_WRITE;
        BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);
    }

    if (pSolvRepoInfo->nCookieSet)
    {
        if (fwrite(pSolvRepoInfo->cookie, SOLV_COOKIE_LEN, 1, fp) != 1)
        {
            dwError = ERROR_TDNF_SOLV_IO;
            BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);
        }
    }

    if (fclose(fp))
    {
        fp = NULL;/* so that error branch will not attempt to close again */
        dwError = ERROR_TDNF_SOLV_IO;
        BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);
    }
    fp = NULL;
    dwError = SolvAddSolvMetaData(pSolvRepoInfo, pszTempSolvFile);
    BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);

    dwError = SolvGetMetaDataCachePath(pSolvRepoInfo, &pszCacheFilePath);
    BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);

    if (IsNullOrEmptyString(pszCacheFilePath))
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);
    }

    if (rename(pszTempSolvFile, pszCacheFilePath) == -1)
    {
        dwError = ERROR_TDNF_SYSTEM_BASE + errno;
        BAIL_ON_TDNF_LIBSOLV_ERROR(dwError);
    }
    unlink(pszTempSolvFile);
cleanup:
    TDNF_SAFE_FREE_MEMORY(pszTempSolvFile);
    TDNF_SAFE_FREE_MEMORY(pszSolvCacheDir);
    TDNF_SAFE_FREE_MEMORY(pszCacheFilePath);
    return dwError;
error:
    if (fp != NULL)
    {
        fclose(fp);
        unlink(pszTempSolvFile);
    }
    else if (fd > 0)
    {
        close(fd);
        unlink(pszTempSolvFile);
    }
    goto cleanup;
}
