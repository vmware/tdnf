/*
 * Copyright (C) 2015-2023 VMware, Inc. All Rights Reserved.
 *
 * Licensed under the GNU Lesser General Public License v2.1 (the "License");
 * you may not use this file except in compliance with the License. The terms
 * of the License are located in the COPYING file of this distribution.
 */

#ifndef __TDNFDEFINES_H__
#define __TDNFDEFINES_H__

#ifdef __cplusplus
extern "C" {
#endif

//TDNF specific errors(1000 to 1999)
#define ERROR_TDNF_BASE                     1000
//TDNF errors 1000 to 1299
//A package was required in command args in order to process
//and was not found. Could not proceed
#define ERROR_TDNF_PACKAGE_REQUIRED         1001
//There was an error reading the tdnf conf file
//usually at /etc/tdnf/tdnf.conf
#define ERROR_TDNF_CONF_FILE_LOAD           1002
//There was an error loading a repo file
#define ERROR_TDNF_REPO_FILE_LOAD           1003
//invalid repo file
#define ERROR_TDNF_INVALID_REPO_FILE        1004
//could not open repo dir
#define ERROR_TDNF_REPO_DIR_OPEN            1005
//Repository handle operation failed
#define ERROR_TDNF_REPO_PERFORM             1006
//Repository getinfo operation failed
#define ERROR_TDNF_REPO_GETINFO             1007
//No repositories configured. Check /etc/yum.repos.d
#define ERROR_TDNF_NO_REPOS                 1008
//Repo was not found
#define ERROR_TDNF_REPO_NOT_FOUND           1009
//Configuration data was not loaded
#define ERROR_TDNF_INVALID_CONF             1010
#define ERROR_TDNF_NO_MATCH                 1011
//There were no enabled repos
#define ERROR_TDNF_NO_ENABLED_REPOS         1012
#define ERROR_TDNF_PACKAGELIST_EMPTY        1013
#define ERROR_TDNF_GOAL_CREATE              1014
#define ERROR_TDNF_INVALID_RESOLVE_ARG      1015
#define ERROR_TDNF_CLEAN_UNSUPPORTED        1016
#define ERROR_TDNF_NO_DOWNGRADES            1017
#define ERROR_TDNF_AUTOERASE_UNSUPPORTED    1018
//Config settings error
#define ERROR_TDNF_SET_PROXY                1020
#define ERROR_TDNF_SET_PROXY_USERPASS       1021
#define ERROR_TDNF_NO_DISTROVERPKG          1022
#define ERROR_TDNF_DISTROVERPKG_READ        1023
//
#define ERROR_TDNF_INVALID_ALLOCSIZE        1024
#define ERROR_TDNF_STRING_TOO_LONG          1025
//alter errors
#define ERROR_TDNF_ALREADY_INSTALLED        1026
#define ERROR_TDNF_NO_UPGRADE_PATH          1027
#define ERROR_TDNF_NO_DOWNGRADE_PATH        1028
//
#define ERROR_TDNF_METADATA_EXPIRE_PARSE    1029
#define ERROR_TDNF_PROTECTED                1030
#define ERROR_TDNF_SELF_ERASE               1030
#define ERROR_TDNF_ERASE_NEEDS_INSTALL      1031
#define ERROR_TDNF_OPERATION_ABORTED        1032
// for invalid input on interactive questions
#define ERROR_TDNF_INVALID_INPUT            1033
// cache only set, but no cache available
#define ERROR_TDNF_CACHE_DISABLED           1034
#define ERROR_TDNF_DOWNGRADE_NOT_ALLOWED    1035
// cache directory out of memory
#define ERROR_TDNF_CACHE_DIR_OUT_OF_DISK_SPACE 1036
// There are duplicate repo id
#define ERROR_TDNF_DUPLICATE_REPO_ID        1037

//curl errors
#define ERROR_TDNF_CURL_INIT                  1200
#define ERROR_TDNF_CURL_BASE                  1201
#define ERROR_TDNF_CURLE_UNSUPPORTED_PROTOCOL 1202
#define ERROR_TDNF_CURLE_FAILED_INIT          1203
#define ERROR_TDNF_CURLE_URL_MALFORMAT        1204
#define ERROR_TDNF_CURL_END                   1299

#define ERROR_TDNF_SOLV_BASE                1300
// general runtime error
#define ERROR_TDNF_SOLV_FAILED        (ERROR_TDNF_SOLV_BASE + 1)
// client programming error
#define ERROR_TDNF_SOLV_OP            (ERROR_TDNF_SOLV_BASE + 2)
// error propagated from libsolv
#define ERROR_TDNF_SOLV_LIBSOLV       (ERROR_TDNF_SOLV_BASE + 3)
// I/O error
#define ERROR_TDNF_SOLV_IO            (ERROR_TDNF_SOLV_BASE + 4)
// cache write error
#define ERROR_TDNF_SOLV_CACHE_WRITE   (ERROR_TDNF_SOLV_BASE + 5)
// ill-formed query
#define ERROR_TDNF_SOLV_QUERY         (ERROR_TDNF_SOLV_BASE + 6)
// unknown arch
#define ERROR_TDNF_SOLV_ARCH          (ERROR_TDNF_SOLV_BASE + 7)
// validation check failed
#define ERROR_TDNF_SOLV_VALIDATION    (ERROR_TDNF_SOLV_BASE + 8)
// ill-specified selector
#define ERROR_TDNF_SOLV_SELECTOR      (ERROR_TDNF_SOLV_BASE + 9)
// goal found no solutions
#define ERROR_TDNF_SOLV_NO_SOLUTION   (ERROR_TDNF_SOLV_BASE + 10)
// the capability was not available
#define ERROR_TDNF_SOLV_NO_CAPABILITY (ERROR_TDNF_SOLV_BASE + 11)
// Solv Checksum Error
#define ERROR_TDNF_SOLV_CHKSUM        (ERROR_TDNF_SOLV_BASE + 12)
// Solv file write failed
#define ERROR_TDNF_REPO_WRITE         (ERROR_TDNF_SOLV_BASE + 13)
// Solv File not created
#define ERROR_TDNF_SOLV_CACHE_NOT_CREATED  (ERROR_TDNF_SOLV_BASE + 14)
// Add solv file to repo failed
#define ERROR_TDNF_ADD_SOLV            (ERROR_TDNF_SOLV_BASE + 15)
//Repo errors 1400 to 1469
#define ERROR_TDNF_REPO_BASE                 1400
#define ERROR_TDNF_SET_SSL_SETTINGS          1401

//RPM Errors 1470 to 1599
#define ERROR_TDNF_RPM_BASE                  1470
#define ERROR_TDNF_RPMRC_NOTFOUND            1471
#define ERROR_TDNF_RPMRC_FAIL                1472
#define ERROR_TDNF_RPMRC_NOTTRUSTED          1473
#define ERROR_TDNF_RPMRC_NOKEY               1474
//
#define ERROR_TDNF_RPMTS_CREATE_FAILED       1501
#define ERROR_TDNF_RPMTS_BAD_ROOT_DIR        1502
#define ERROR_TDNF_RPMTS_SET_CB_FAILED       1503
#define ERROR_TDNF_RPMTS_KEYRING_FAILED      1504
#define ERROR_TDNF_INVALID_PUBKEY_FILE       1505
#define ERROR_TDNF_CREATE_PUBKEY_FAILED      1506
#define ERROR_TDNF_KEYURL_INVALID            1507
#define ERROR_TDNF_KEYURL_UNSUPPORTED        1508
#define ERROR_TDNF_RPM_HEADER_CONVERT_FAILED 1509
#define ERROR_TDNF_RPM_NOT_SIGNED            1510
#define ERROR_TDNF_RPMTD_CREATE_FAILED       1511
#define ERROR_TDNF_RPM_GET_RSAHEADER_FAILED  1512
#define ERROR_TDNF_RPM_GPG_PARSE_FAILED      1513
#define ERROR_TDNF_RPM_GPG_NO_MATCH          1514
#define ERROR_TDNF_RPM_CHECK                 1515
#define ERROR_TDNF_SETOPT_NO_EQUALS          1516
#define ERROR_TDNF_PLUGINS_DISABLED          1517
#define ERROR_TDNF_NO_PLUGIN_CONF_DIR        1518
#define ERROR_TDNF_PLUGIN_LOAD_ERROR         1519
#define ERROR_TDNF_OPT_NOT_FOUND             1520
#define ERROR_TDNF_PLUGIN_NO_MORE_EVENTS     1521
#define ERROR_TDNF_NO_PLUGIN_ERROR           1522
#define ERROR_TDNF_NO_GPGKEY_CONF_ENTRY      1523
#define ERROR_TDNF_URL_INVALID               1524
//RPM Transaction
#define ERROR_TDNF_TRANSACTION_FAILED        1525
#define ERROR_TDNF_RPMTS_OPENDB_FAILED       1526

#define ERROR_TDNF_SIZE_MISMATCH             1527
#define ERROR_TDNF_CHECKSUM_MISMATCH         1528

#define ERROR_TDNF_RPMTS_FDDUP_FAILED        1529

/* event context */
#define ERROR_TDNF_EVENT_CTXT_ITEM_NOT_FOUND      1551
#define ERROR_TDNF_EVENT_CTXT_ITEM_INVALID_TYPE   1552

// No search results found
#define ERROR_TDNF_NO_SEARCH_RESULTS    1599
#define ERROR_TDNF_SYSTEM_BASE          1600
//System errors 1600 and up
#define ERROR_TDNF_PERM                 (ERROR_TDNF_SYSTEM_BASE + EPERM)
#define ERROR_TDNF_INVALID_PARAMETER    (ERROR_TDNF_SYSTEM_BASE + EINVAL)
#define ERROR_TDNF_OUT_OF_MEMORY        (ERROR_TDNF_SYSTEM_BASE + ENOMEM)
#define ERROR_TDNF_NO_DATA              (ERROR_TDNF_SYSTEM_BASE + ENODATA)
#define ERROR_TDNF_FILE_NOT_FOUND       (ERROR_TDNF_SYSTEM_BASE + ENOENT)
#define ERROR_TDNF_ACCESS_DENIED        (ERROR_TDNF_SYSTEM_BASE + EACCES)
#define ERROR_TDNF_ALREADY_EXISTS       (ERROR_TDNF_SYSTEM_BASE + EEXIST)
#define ERROR_TDNF_INVALID_ADDRESS      (ERROR_TDNF_SYSTEM_BASE + EFAULT)
#define ERROR_TDNF_CALL_INTERRUPTED     (ERROR_TDNF_SYSTEM_BASE + EINTR)
#define ERROR_TDNF_FILESYS_IO           (ERROR_TDNF_SYSTEM_BASE + EIO)
#define ERROR_TDNF_SYM_LOOP             (ERROR_TDNF_SYSTEM_BASE + ELOOP)
#define ERROR_TDNF_NAME_TOO_LONG        (ERROR_TDNF_SYSTEM_BASE + ENAMETOOLONG)
#define ERROR_TDNF_CALL_NOT_SUPPORTED   (ERROR_TDNF_SYSTEM_BASE + ENOSYS)
#define ERROR_TDNF_INVALID_DIR          (ERROR_TDNF_SYSTEM_BASE + ENOTDIR)
#define ERROR_TDNF_OVERFLOW             (ERROR_TDNF_SYSTEM_BASE + EOVERFLOW)

#define ERROR_TDNF_JSONDUMP 1700

#define ERROR_TDNF_HISTORY_ERROR 1801
#define ERROR_TDNF_HISTORY_NODB 1802


#define ERROR_TDNF_PLUGIN_BASE          2000

#define ERROR_TDNF_BASEURL_DOES_NOT_EXISTS             2500
#define ERROR_TDNF_CHECKSUM_VALIDATION_FAILED          2501
#define ERROR_TDNF_METALINK_RESOURCE_VALIDATION_FAILED 2502

//FIPS error
#define ERROR_TDNF_FIPS_MODE_FORBIDDEN       2600

#define CMD_INSTALL "install"

#ifdef __cplusplus
}
#endif

#endif//__TDNFDEFINES_H__
