/*
 * Copyright (C) 2021 VMware, Inc. All Rights Reserved.
 *
 * Licensed under the GNU Lesser General Public License v2.1 (the "License");
 * you may not use this file except in compliance with the License. The terms
 * of the License are located in the COPYING file of this distribution.
 */

/*
 * Module   : list.c
 *
 * Abstract :
 *
 *            tdnfclientlib
 *
 *            client library
 *
 * Authors  : Nitesh Kumar (kunitesh@vmware.com)
 */

#include "includes.h"

TDNF_ML_LIST*
TDNFMergeList(
    TDNF_ML_LIST* listA,
    TDNF_ML_LIST* listB
    )
{
    uint32_t dwError = 0;
    TDNF_ML_LIST* mergedList = NULL;
    TDNF_ML_URL_INFO *urlA = NULL;
    TDNF_ML_URL_INFO *urlB = NULL;

    if ((listA == NULL) && (listB == NULL))
    {
        return NULL;
    }

    // Base cases
    if (listA == NULL)
    {
        return (listB);
    }

    if (listB == NULL)
    {
        return (listA);
    }

    //Compare the URL Preference to sort the URL List in descending order.
    urlA = listA->data;
    urlB = listB->data;

    if ((urlA == NULL) || (urlB == NULL))
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    if (urlA->preference >= urlB->preference)
    {
        mergedList = listA;
        mergedList->next = TDNFMergeList(listA->next, listB);
    }
    else
    {
        mergedList = listB;
        mergedList->next = TDNFMergeList(listA, listB->next);
    }

    return mergedList;

error:
    return NULL;
}

/* This function is used to split the nodes of the given
 * list into two halves,and store the reference in frontRef
 * and backRef. If the length is odd, the extra node should
 * go in the front list.
 */
void
TDNFFrontBackSplit(
    TDNF_ML_LIST* currHead,
    TDNF_ML_LIST** frontRef,
    TDNF_ML_LIST** backRef
    )
{
    TDNF_ML_LIST* slowPtr = NULL;
    TDNF_ML_LIST* fastPtr = NULL;

    if (!currHead || !currHead->next ||
        !frontRef || !backRef)
    {
        return;
    }

    slowPtr = currHead;
    fastPtr = currHead->next;

    while (fastPtr != NULL)
    {
        fastPtr = fastPtr->next;
        if (fastPtr != NULL)
        {
            slowPtr = slowPtr->next;
            fastPtr = fastPtr->next;
        }
    }

    *frontRef = currHead;
    *backRef = slowPtr->next;
    slowPtr->next = NULL;
}

void
TDNFSortListOnPreference(
    TDNF_ML_LIST** headUrl
    )
{
    TDNF_ML_LIST* head = NULL;
    TDNF_ML_LIST* listA = NULL;
    TDNF_ML_LIST* listB = NULL;

    if (!headUrl)
    {
        return;
    }

    head = *headUrl;
    if ((head == NULL) || (head->next == NULL))
    {
        return;
    }

    TDNFFrontBackSplit(head, &listA, &listB);

    TDNFSortListOnPreference(&listA);
    TDNFSortListOnPreference(&listB);

    *headUrl = TDNFMergeList(listA, listB);

}

/* This function is used to append the list with
 * new node at last.
 */
uint32_t TDNFAppendList(
    TDNF_ML_LIST** head_ref,
    void *new_data
    )
{
    TDNF_ML_LIST* new_node = NULL;
    uint32_t dwError = 0;

    if (!new_data || !head_ref)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = TDNFAllocateMemory(1, sizeof(TDNF_ML_LIST), (void**)&new_node);
    BAIL_ON_TDNF_ERROR(dwError);

    new_node->data  = new_data;

    if (*head_ref == NULL)
    {
        *head_ref = new_node;
    }
    else
    {
        TDNF_ML_LIST *last_node = *head_ref;
        while(last_node->next != NULL)
        {
            last_node = last_node->next;
        }
        last_node->next = new_node;
    }

cleanup:
    return dwError;

error:
    pr_err("Memory allocation failed for new node error: %d\n", dwError);
    goto cleanup;
}

/* This function is used to delete the full list.
 * Also call the respective free function to free the
 * data ptr.
 */
void
TDNFDeleteList(
    TDNF_ML_LIST** head_ref,
    TDNF_ML_FREE_FUNC free_func
    )
{
    TDNF_ML_LIST* current = NULL;
    TDNF_ML_LIST* next = NULL;

    if (!head_ref || !free_func)
    {
        return;
    }

    current = *head_ref;
    while (current != NULL)
    {
        next = current->next;
        (*free_func)(current->data);
        TDNF_SAFE_FREE_MEMORY(current);
        current = next;
    }
    *head_ref = NULL;
}
