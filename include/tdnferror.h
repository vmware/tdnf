/*
      * Copyright (C) 2014-2015 VMware, Inc. All rights reserved.
      *
      * Header : tdnferror.h
      *
      * Abstract :
      *
      *            tdnfclientlib
      *
      *            public header
      *
      * Authors  : Priyesh Padmavilasom (ppadmavilasom@vmware.com)
      *
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

//Hawkey errors 1300 to 1399
#define ERROR_TDNF_HAWKEY_BASE          1300
// general runtime error
#define ERROR_TDNF_HAWKEY_FAILED        (ERROR_TDNF_HAWKEY_BASE + HY_E_FAILED)
// client programming error
#define ERROR_TDNF_HAWKEY_OP            (ERROR_TDNF_HAWKEY_BASE + HY_E_OP)
// error propagated from libsolv
#define ERROR_TDNF_HAWKEY_LIBSOLV       (ERROR_TDNF_HAWKEY_BASE + HY_E_LIBSOLV)
// I/O error
#define ERROR_TDNF_HAWKEY_IO            (ERROR_TDNF_HAWKEY_BASE + HY_E_IO)
// cache write error
#define ERROR_TDNF_HAWKEY_CACHE_WRITE   (ERROR_TDNF_HAWKEY_BASE + HY_E_CACHE_WRITE)
// ill-formed query
#define ERROR_TDNF_HAWKEY_QUERY         (ERROR_TDNF_HAWKEY_BASE + HY_E_QUERY)
// unknown arch
#define ERROR_TDNF_HAWKEY_ARCH          (ERROR_TDNF_HAWKEY_BASE + HY_E_ARCH)
// validation check failed
#define ERROR_TDNF_HAWKEY_VALIDATION    (ERROR_TDNF_HAWKEY_BASE + HY_E_VALIDATION)
// ill-specified selector
#define ERROR_TDNF_HAWKEY_SELECTOR      (ERROR_TDNF_HAWKEY_BASE + HY_E_SELECTOR)
// goal found no solutions
#define ERROR_TDNF_HAWKEY_NO_SOLUTION   (ERROR_TDNF_HAWKEY_BASE + HY_E_NO_SOLUTION)
// the capability was not available
#define ERROR_TDNF_HAWKEY_NO_CAPABILITY (ERROR_TDNF_HAWKEY_BASE + HY_E_NO_CAPABILITY)
//Repo errors 1400 to 1469
#define ERROR_TDNF_REPO_BASE                 1400

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

//RPM Transaction
#define ERROR_TDNF_TRANS_INCOMPLETE     1525
#define ERROR_TDNF_TRANS_PKG_NOT_FOUND  1526

//System errors 1600 and up
#define ERROR_TDNF_SYSTEM_BASE          1600
// No search results found
#define ERROR_TDNF_NO_SEARCH_RESULTS    1601
#define ERROR_TDNF_INVALID_PARAMETER    (ERROR_TDNF_SYSTEM_BASE + EINVAL)
#define ERROR_TDNF_OUT_OF_MEMORY        (ERROR_TDNF_SYSTEM_BASE + ENOMEM)
#define ERROR_TDNF_NO_DATA              (ERROR_TDNF_SYSTEM_BASE + ENODATA)
#define ERROR_TDNF_FILE_NOT_FOUND       (ERROR_TDNF_SYSTEM_BASE + ENOENT)
#define ERROR_TDNF_ACCESS_DENIED        (ERROR_TDNF_SYSTEM_BASE + EACCES)

#define CMD_INSTALL "install"

#ifdef __cplusplus
}
#endif

#endif//__TDNFDEFINES_H__
