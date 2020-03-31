/*
 * Copyright (C) 2019 VMware, Inc. All Rights Reserved.
 *
 * Licensed under the GNU General Public License v2 (the "License");
 * you may not use this file except in compliance with the License. The terms
 * of the License are located in the COPYING file of this distribution.
 */

#pragma once

typedef struct _PY_TDNF_
{
    PyObject_HEAD
}PY_TDNF, *PPY_TDNF;

typedef struct _PY_TDNF_BASE_
{
    PyObject_HEAD
}PY_TDNF_BASE, *PPY_TDNF_BASE;

typedef struct _PY_TDNF_REPODATA
{
    PyObject_HEAD
    PyObject *id;
    PyObject *name;
    PyObject *baseurl;
    PyObject *metalink;
    int enabled;
}PY_TDNF_REPODATA, *PPY_TDNF_REPODATA;
