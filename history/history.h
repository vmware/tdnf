/*
 * Copyright (C) 2022 VMware, Inc. All Rights Reserved.
 *
 * Licensed under the GNU Lesser General Public License v2.1 (the "License");
 * you may not use this file except in compliance with the License. The terms
 * of the License are located in the COPYING file of this distribution.
 */

#pragma once

#include <sqlite3.h>
#include <rpm/rpmlib.h>

#include "config.h"

#define HISTORY_TRANS_TYPE_BASE 0
#define HISTORY_TRANS_TYPE_DELTA 1

#define HISTORY_ITEM_TYPE_SET 0
#define HISTORY_ITEM_TYPE_ADD 1
#define HISTORY_ITEM_TYPE_REMOVE 2

struct history_ctx
{
    sqlite3 *db;
    int *installed_ids; /* installed ids must be sorted */
    int installed_count;
    char *cookie;
    int trans_id;
};

struct history_delta
{
    int *added_ids;
    int added_count;
    int *removed_ids;
    int removed_count;
};

struct history_flags_delta
{
    int *changed_ids;
    int *values;
    int count;
};

struct history_transaction
{
    int id;
    int type;
    char *cmdline;
    time_t timestamp;
    char *cookie;
    struct history_delta delta;
    struct history_flags_delta flags_delta;
};

struct history_nevra_map
{
    int count;
    char **idmap;
};

struct history_ctx *create_history_ctx(const char *db_filename);
void destroy_history_ctx(struct history_ctx *ctx);

int history_sync(struct history_ctx *ctx, rpmts ts);

char *history_nevra_from_id(struct history_ctx *ctx, int id);
struct history_nevra_map *history_nevra_map(struct history_ctx *ctx);
void history_free_nevra_map(struct history_nevra_map *);
char *history_get_nevra(struct history_nevra_map *hnm, int id);

void history_free_delta(struct history_delta *hd);
struct history_delta *history_get_delta(struct history_ctx *ctx, int trans_id);
struct history_delta *history_get_delta_range(struct history_ctx *ctx, int trans_id0, int trans_id1);

int history_add_transaction(struct history_ctx *ctx, const char *cmdline);
int history_record_state(struct history_ctx *ctx);
int history_update_state(struct history_ctx *ctx, rpmts ts, const char *cmdline);

int history_get_transactions(struct history_ctx *ctx,
                             struct history_transaction **ptas,
                             int *pcount,
                             int reverse, int from, int to);
void history_free_transactions(struct history_transaction *tas, int count);

int history_set_auto_flag(struct history_ctx *ctx, const char *name, int value);
int history_get_auto_flag(struct history_ctx *ctx, const char *name, int *pvalue);

int history_restore_auto_flags(struct history_ctx *ctx, int trans_id);
int history_replay_auto_flags(struct history_ctx *ctx, int from, int to);

void history_free_flags_delta(struct history_flags_delta * hfd);
struct history_flags_delta *
history_get_flags_delta(struct history_ctx *ctx, int from, int to);

