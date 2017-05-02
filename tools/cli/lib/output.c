/*
 * Copyright (C) 2015 VMware, Inc. All Rights Reserved.
 *
 * Licensed under the GNU General Public License v2 (the "License");
 * you may not use this file except in compliance with the License. The terms
 * of the License are located in the COPYING file of this distribution.
 */

/*
 * Module   : output.c
 *
 * Abstract :
 *
 *            tdnf
 *
 *            command line tool
 *
 * Authors  : Priyesh Padmavilasom (ppadmavilasom@vmware.com)
 *
 */

#include "includes.h"

void
ShowConsoleProps(
    )
{
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);

    printf ("lines %d\n", w.ws_row);
    printf ("columns %d\n", w.ws_col);
}

uint32_t
GetColumnWidths(
    int nCount,
    int* pnColPercents,
    int* pnColWidths
    )
{
    uint32_t dwError = 0;
    struct winsize stWinSize = {0};
    int nConsoleWidth = 0;
    int nIndex = 0;

    if(!pnColPercents || !pnColWidths)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_CLI_ERROR(dwError);
    }

    dwError = ioctl(STDOUT_FILENO, TIOCGWINSZ, &stWinSize);
    if(dwError > 0)
    {
        nConsoleWidth = 80;
        dwError = 0;
    }
    else
    {
        nConsoleWidth = stWinSize.ws_col;
    }

    for(nIndex = 0; nIndex < nCount; nIndex++)
    {
        pnColWidths[nIndex] = (nConsoleWidth * pnColPercents[nIndex]) / 100;
    }
cleanup:
    return dwError;
error:
    goto cleanup;
}
