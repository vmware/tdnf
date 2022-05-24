/*
 * Copyright (C) 2022 VMware, Inc. All Rights Reserved.
 *
 * Licensed under the GNU Lesser General Public License v2.1 (the "License");
 * you may not use this file except in compliance with the License. The terms
 * of the License are located in the COPYING file of this distribution.
 */

#include <stdio.h>
#include <time.h>

#include <sqlite3.h>

#include <rpm/rpmlib.h>
#include <rpm/rpmdb.h>
#include <rpm/rpmlog.h>
#include <rpm/rpmps.h>
#include <rpm/rpmts.h>

#include "history.h"

#define check_cond(COND) if(!(COND)) { \
    fprintf(stderr, "check_cond failed in %s line %d\n", \
        __FUNCTION__, __LINE__); \
    rc = -1; \
    ((void)(rc)); /* suppress "set but not used" warning */ \
    goto error; \
}

#define check_ptr(ptr) if(!(ptr)) { \
    fprintf(stderr, "check_ptr failed in %s line %d\n", \
        __FUNCTION__, __LINE__); \
    rc = -1; \
    ((void)(rc)); /* suppress "set but not used" warning */ \
    goto error; \
}

#define check_rc(rc) if((rc) != 0) { \
    fprintf(stderr, "check_rc failed in %s line %d\n", \
        __FUNCTION__, __LINE__); \
    goto error; \
}

#define check_db_rc(db, rc) if((rc) != SQLITE_OK) { \
    fprintf(stderr, \
        "check_db_rc failed with %s in %s line %d\n", \
        sqlite3_errmsg(db), __FUNCTION__, __LINE__); \
    ((void)(rc)); /* suppress "set but not used" warning */ \
    goto error; \
}

#define check_db_step(db, step) \
    if((step) != SQLITE_ROW && (step) != SQLITE_DONE) { \
        fprintf(stderr, \
            "check_db_rc failed with %s in %s line %d\n", \
            sqlite3_errmsg(db), __FUNCTION__, __LINE__); \
        rc = -1; \
        ((void)(rc)); /* suppress "set but not used" warning */ \
        goto error; \
    }

#define check_db_cmd(db, cmd) { \
    rc = (cmd); \
    check_db_rc(db, rc); \
}

#define safe_free(ptr) { if ((ptr) != NULL) { free(ptr); ptr = NULL; }}

#define map_isset(map, idx) (map[(idx)-1] != 0)
#define map_set(map, idx) { map[(idx)-1] = 1;}
#define map_unset(map, idx) { map[(idx)-1] = 0;}

static
int _cmp_int(const void *p1, const void *p2)
{
    return *(int *)p1 - *(int *)p2;
}

static
void sort_array(int *arr, int count)
{
    qsort(arr, count, sizeof(int), _cmp_int);
}

/*
 * Diff arr1 and arr2, with sizes count1 and count2. The arrays must be
 * sorted. The differences
 * will be stored in ponly1 and ponly2, which are dynamically allocated
 * and should be free'd by the caller. Counts of the created arrays
 * are stored in ponly1_count and ponly2_count.
 */
static
int diff_arrays(int *arr1, int count1,
                 int *arr2, int count2,
                 int **ponly1, int *ponly1_count,
                 int **ponly2, int *ponly2_count)
{
    int rc = 0;
    int i1 = 0, i2 = 0;
    int *only1 = NULL, *only2 = NULL;
    int only1_count = 0, only2_count = 0;

    check_ptr(only1 = (int *)calloc(count1, sizeof(int)));
    check_ptr(only2 = (int *)calloc(count2, sizeof(int)));

    while(i1 < count1 && i2 < count2) {
        if (arr1[i1] != arr2[i2]) {
            if(arr1[i1] > arr2[i2]) {
                only2[only2_count++] = arr2[i2++];
            } else {
                only1[only1_count++] = arr1[i1++];
            }
        } else {
            i1++;
            i2++;
        }
    }

    if(i1 == count1) {
        while (i2 < count2) {
            only2[only2_count++] = arr2[i2++];
        }
    } else {
        while (i1 < count1) {
            only1[only1_count++] = arr1[i1++];
        }
    }
    *ponly1 = only1;
    *ponly1_count = only1_count;

    *ponly2 = only2;
    *ponly2_count = only2_count;

    return 0;
error:
    safe_free(only1);
    safe_free(only2);
    return rc;
}

static
sqlite3 *init_db(const char *filename)
{
    sqlite3 *db = NULL;

    int rc = sqlite3_open(filename, &db);
    check_db_rc(db, rc);

error:
    return db;
}

/* Check if table exists. Returns SQLITE_ROW if it exists, SQLITE_DONE if not,
   or error code in case of error */
static
int db_table_exists(sqlite3 *db, const char *name)
{
    int rc = 0;
    sqlite3_stmt *res = NULL;

    rc = sqlite3_prepare_v2(db,
                            "SELECT * FROM sqlite_master "
                                "WHERE type='table' AND name=?;",
                            -1, &res, 0);
    check_db_rc(db, rc);

    sqlite3_bind_text(res, 1, name, -1, NULL);

    rc = sqlite3_step(res);
    sqlite3_finalize(res); res = NULL;

error:
    if (res)
        sqlite3_finalize(res);
    return rc;
}

/* count known rpms in db */
static
int db_rpms_count(sqlite3 *db, int *pcount)
{
    int rc = 0, step;
    sqlite3_stmt *res = NULL;
    const char *sql = "SELECT count(*) FROM rpms;";

    rc = sqlite3_prepare_v2(db,
                            sql,
                            -1, &res, 0);
    check_db_rc(db, rc);

    step = sqlite3_step(res);
    if (step == SQLITE_ROW) {
        *pcount = sqlite3_column_int(res, 0);
    }

error:
    if (res)
        sqlite3_finalize(res);
    return rc;
}

/* max id in a table (should be same as count unless there are gaps) */
static
int db_maxid(sqlite3 *db, const char *table_name, int *pmaxid)
{
    int rc = 0, step;
    sqlite3_stmt *res = NULL;
    char sql[256];

    snprintf(sql, sizeof(sql), "SELECT * FROM %s ORDER BY id DESC;", table_name);
    rc = sqlite3_prepare_v2(db,
                            sql,
                            -1, &res, 0);
    check_db_rc(db, rc);

    step = sqlite3_step(res);
    check_cond(step == SQLITE_ROW);

    *pmaxid = sqlite3_column_int(res, 0);
    sqlite3_finalize(res); res = NULL;
error:
    if (res)
        sqlite3_finalize(res);
    return rc;
}

/* max rpm id db (should be same as count unless there are gaps) */
static
int db_rpms_maxid(sqlite3 *db, int *pmaxid)
{
    return db_maxid(db, "rpms", pmaxid);
}

static
void db_free_nevra_map(struct history_nevra_map *hnm)
{
    if (hnm){
        if (hnm->idmap) {
            for (int i = 0; i < hnm->count; i++) {
                safe_free(hnm->idmap[i]);
            }
            free(hnm->idmap);
        }
        free(hnm);
    }
}

/*
 * Create a map with all known NEVRAs.
 */
static
struct history_nevra_map *db_nevra_map(sqlite3 *db)
{
    int rc;
    struct history_nevra_map *hnm = NULL;
    sqlite3_stmt *res = NULL;
    const char *sql_find = "SELECT * FROM rpms;";
    int count = 0;

    rc = db_rpms_maxid(db, &count);
    check_db_rc(db, rc);

    hnm = (struct history_nevra_map *)calloc(1, sizeof(struct history_nevra_map));
    check_ptr(hnm);

    hnm->idmap = (char **)calloc(count, sizeof(char *));
    check_ptr(hnm->idmap);

    hnm->count = count;

    rc = sqlite3_prepare_v2(db, sql_find, -1, &res, 0);
    check_db_rc(db, rc);
    for(int step = sqlite3_step(res); step == SQLITE_ROW; step = sqlite3_step(res)) {
        int id = sqlite3_column_int(res, 0);
        char *nevra = strdup((const char *)sqlite3_column_text(res, 1));
        check_ptr(nevra);
        check_cond((id > 0 && id <= count));
        /* map is zero based, rpm ids start with 1 */
        hnm->idmap[id - 1] = nevra;
    }
error:
    if (res)
        sqlite3_finalize(res);
    if (rc) {
        db_free_nevra_map(hnm); hnm = NULL;
    }
    return hnm;
}

static
int db_get_dict_entry(sqlite3 *db,
                      const char *table_name, const char *field_name,
                      const char *entry, int *pid,
                      int create)
{
    int rc = 0, step;
    sqlite3_stmt *res = NULL;
    char sql[256];

    snprintf(sql, sizeof(sql), "SELECT * FROM %s WHERE %s = ?;", table_name, field_name);

    rc = sqlite3_prepare_v2(db, sql, -1, &res, 0);
    check_db_rc(db, rc);

    sqlite3_bind_text(res, 1, entry, -1, NULL);
    step = sqlite3_step(res);
    if (step == SQLITE_ROW) {
        int id = sqlite3_column_int(res, 0);
        if (pid)
            *pid = id;
        sqlite3_finalize(res);
        /* we found it, return */
        return 0;
    }
    sqlite3_finalize(res); res = NULL;

    if (!create) {
        if (pid) *pid = 0;
        return 0;
    }

    /* add it to db */
    snprintf(sql, sizeof(sql), "INSERT INTO %s(%s) VALUES (?);", table_name, field_name);
    rc = sqlite3_prepare_v2(db, sql, -1, &res, 0);
    check_db_rc(db, rc);

    sqlite3_bind_text(res, 1, entry, -1, NULL);
    step = sqlite3_step(res);
    if (step == SQLITE_DONE) {
        int id = sqlite3_last_insert_rowid(db);
        if (pid)
            *pid = id;
    }

error:
    if (res)
        sqlite3_finalize(res);
    return rc;
}

/*
 * Add nevra to db if it's not there already. The id will be returned
 * in *pid if that's not NULL.
*/
static
int db_add_nevra(sqlite3 *db, const char *nevra, int *pid)
{
    return db_get_dict_entry(db, "rpms", "nevra", nevra, pid, 1);
}

/* read transaction items into ht */
static
int history_delta_read(sqlite3 *db, struct history_delta *hd, int id)
{
    int rc = 0;
    sqlite3_stmt *res = NULL;
    const char *sql = "SELECT * FROM trans_items WHERE trans_id = ? ORDER BY rpm_id;";
    int max_count = 0;

    /* array sizes cannot be larger than number of known rpms */
    rc = db_rpms_count(db, &max_count);
    check_db_rc(db, rc);

    hd->added_ids = (int *)calloc(max_count, sizeof(int));
    check_ptr(hd->added_ids);
    hd->removed_ids = (int *)calloc(max_count, sizeof(int));
    check_ptr(hd->removed_ids);

    hd->added_count = 0;
    hd->removed_count = 0;

    rc = sqlite3_prepare_v2(db, sql, -1, &res, 0);
    check_db_rc(db, rc);

    sqlite3_bind_int(res, 1, id);
    for(int step = sqlite3_step(res); step == SQLITE_ROW; step = sqlite3_step(res)) {
        int type = sqlite3_column_int(res, 2);
        int rpm_id = sqlite3_column_int(res, 3);
        if (type == HISTORY_ITEM_TYPE_ADD || type == HISTORY_ITEM_TYPE_SET) {
            check_cond(hd->added_count < max_count);
            hd->added_ids[hd->added_count++] = rpm_id;
        }
        else if (type == HISTORY_ITEM_TYPE_REMOVE) {
            check_cond(hd->removed_count < max_count);
            hd->removed_ids[hd->removed_count++] = rpm_id;
        }
    }

error:
    if (res)
        sqlite3_finalize(res);
    return rc;
}

/* play one delta transaction on installed_map */
static
int db_play_delta(sqlite3 *db, int trans_id,
                  char *installed_map, int map_size)
{
    int rc = 0;
    sqlite3_stmt *res = NULL;
    const char *sql = "SELECT * FROM trans_items WHERE trans_id = ? ORDER BY rpm_id;";

    rc = sqlite3_prepare_v2(db, sql, -1, &res, 0);
    check_db_rc(db, rc);

    sqlite3_bind_int(res, 1, trans_id);
    for(int step = sqlite3_step(res); step == SQLITE_ROW; step = sqlite3_step(res)) {
        int type = sqlite3_column_int(res, 2);
        int rpm_id = sqlite3_column_int(res, 3);
        check_cond(rpm_id > 0 && rpm_id <= map_size);
        check_cond(type == HISTORY_ITEM_TYPE_ADD || type == HISTORY_ITEM_TYPE_REMOVE);
        if (type == HISTORY_ITEM_TYPE_ADD) {
            map_set(installed_map, rpm_id);
        }
        else if (type == HISTORY_ITEM_TYPE_REMOVE) {
            map_unset(installed_map, rpm_id);
        }
    }

error:
    if (res)
        sqlite3_finalize(res);
    return rc;
}

/* play one set transaction on installed_map */
static
int db_play_set(sqlite3 *db, int trans_id,
                char *installed_map, int map_size)
{
    int rc = 0;
    sqlite3_stmt *res = NULL;
    const char *sql = "SELECT * FROM trans_items "
        "WHERE trans_id = ? ORDER BY rpm_id;";

    rc = sqlite3_prepare_v2(db, sql, -1, &res, 0);
    check_db_rc(db, rc);

    sqlite3_bind_int(res, 1, trans_id);
    for(int step = sqlite3_step(res); step == SQLITE_ROW; step = sqlite3_step(res)) {
        int type = sqlite3_column_int(res, 2);
        int rpm_id = sqlite3_column_int(res, 3);
        check_cond(rpm_id > 0 && rpm_id <= map_size);
        check_cond(type == HISTORY_ITEM_TYPE_SET);
        map_set(installed_map, rpm_id);
    }

error:
    if (res)
        sqlite3_finalize(res);
    return rc;
}

/* Replay all transactions on installed_map to reach trans_id by rewinding to
 * the last baseline state and applying all deltas before and
 * including trans_id.
 * installed_map must have been allocated.
 * Does not install RPMs or modify the db.
 */
static
int db_play_transaction(sqlite3 *db, int trans_id,
                        char *installed_map, int map_size)
{
    sqlite3_stmt *res = NULL;
    int step, rc = 0;

    /* find most recent HISTORY_TRANS_TYPE_BASE */
    rc = sqlite3_prepare_v2(db,
                            "SELECT * FROM transactions "
                                "WHERE type = ? ORDER BY id DESC;",
                            -1, &res, 0);
    check_db_rc(db, rc);

    sqlite3_bind_int(res, 1, HISTORY_TRANS_TYPE_BASE);
    int base_id = 0;
    for (step = sqlite3_step(res); step == SQLITE_ROW; step = sqlite3_step(res)) {
        base_id = sqlite3_column_int(res, 0);
        if (base_id <= trans_id)
            break;
    }
    /* make sure we have found a base line */
    check_cond(step == SQLITE_ROW);

    /* we found the most recent base state before trans_id,
       set to that state ... */
    rc = db_play_set(db, base_id, installed_map, map_size);
    check_rc(rc);
    /* ... then replay all deltas until we reach the desired trans_id */
    for (int id = base_id + 1; id <= trans_id; id++) {
        rc = db_play_delta(db, id, installed_map, map_size);
        check_rc(rc);
    }
error:
    if (res)
        sqlite3_finalize(res);
    return rc;
}

/* Update db with currently installed RPMs.
 * All ids will be saved to array pointed to by pids
 * (if not NULL), the array will be allocated.
 */
static
int db_update_rpms(rpmts ts, sqlite3 *db, int **pids, int *pcount)
{
    int rc = 0;
    int count = 0, i = 0;
    int *ids = NULL;
    Header h;
    rpmdbMatchIterator mi = NULL;
    char *nevra = NULL;

    if (pids) {
        /* count installed packages */
        rpmdbMatchIterator mi =
            rpmtsInitIterator(ts, RPMDBI_PACKAGES, NULL, 0);
        while ((h = rpmdbNextIterator(mi))) {
            count++;
        }
        rpmdbFreeIterator(mi);
        ids = (int *)calloc(count, sizeof(int));
    }

    char *sql = "CREATE TABLE IF NOT EXISTS "
        "rpms(Id INTEGER PRIMARY KEY AUTOINCREMENT, nevra TEXT);";
    rc = sqlite3_exec(db, sql, 0, 0, NULL);
    check_db_rc(db, rc);

    mi = rpmtsInitIterator(ts, RPMDBI_PACKAGES, NULL, 0);
    while ((h = rpmdbNextIterator(mi))) {
        nevra = headerGetAsString(h, RPMTAG_NEVRA);
        rc = db_add_nevra(db, nevra, ids ? &ids[i++] : NULL);
        check_db_rc(db, rc);
        safe_free(nevra);
    }
    rpmdbFreeIterator(mi);

    if (pids && pcount) {
        sort_array(ids, count);

        *pids = ids;
        *pcount = count;
    }
    return rc;
error:
    safe_free(nevra);
    if (mi)
        rpmdbFreeIterator(mi);
    if (ids)
        free(ids);
    return rc;
}

/* Helper function to add a transaction entry to the db.
   Does not add transaction items */
static
int db_add_transaction(sqlite3 *db, int *ptrans_id,
                       const char *cmdline, time_t timestamp,
                       const char *cookie, int type)
{
    int rc = 0, ret;
    sqlite3_stmt *res = NULL;

    rc = sqlite3_prepare_v2(db,
        "INSERT INTO transactions(cmdline, cookie, timestamp, type) VALUES (?, ?, ?, ?);",
        -1, &res, 0);
    check_db_rc(db, rc);

    sqlite3_bind_text(res, 1, cmdline, -1, NULL);
    sqlite3_bind_text(res, 2, cookie, -1, NULL);
    sqlite3_bind_int(res, 3, timestamp);
    sqlite3_bind_int(res, 4, type);

    ret = sqlite3_step(res);
    check_cond(ret == SQLITE_DONE);
    *ptrans_id = sqlite3_last_insert_rowid(db);

error:
    if (res)
        sqlite3_finalize(res);
    return rc;
}

/* Helper function to add transaction items */
static
int db_add_trans_items(sqlite3 *db, int trans_id, int type,
                       int *rpm_ids, int rpm_count)
{
    int rc = 0, ret;
    int i;
    sqlite3_stmt *res = NULL;

    for (i = 0; i < rpm_count; i++) {
        rc = sqlite3_prepare_v2(db,
            "INSERT INTO trans_items(trans_id, type, rpm_id) VALUES (?, ?, ?);",
            -1, &res, 0);
        check_db_rc(db, rc);

        sqlite3_bind_int(res, 1, trans_id);
        sqlite3_bind_int(res, 2, type);
        sqlite3_bind_int(res, 3, rpm_ids[i]);

        ret = sqlite3_step(res);
        check_cond(ret == SQLITE_DONE);

        sqlite3_finalize(res); res = NULL;
    }
error:
    if (res)
        sqlite3_finalize(res);
    return rc;
}

static
int db_set_auto_flag_byid(sqlite3 *db, int trans_id, int name_id, int value)
{
    int rc = 0, step;
    sqlite3_stmt *res = NULL;

    /* TODO: check if entry with trans_id and name_id already exists and update,
       instead of creating a new entry */
    rc = sqlite3_prepare_v2(db,
        "INSERT INTO flag_set(trans_id, name_id, value) VALUES (?, ?, ?);",
        -1, &res, 0);
    check_db_rc(db, rc);

    sqlite3_bind_int(res, 1, trans_id);
    sqlite3_bind_int(res, 2, name_id);
    sqlite3_bind_int(res, 3, value);

    step = sqlite3_step(res);
    check_cond(step == SQLITE_DONE);

error:
    if (res)
        sqlite3_finalize(res);
    return rc;
}

static
int db_set_auto_flag(sqlite3 *db, int trans_id, const char *name, int value)
{
    int rc = 0;
    int name_id;
    sqlite3_stmt *res = NULL;

    rc = sqlite3_exec(db,
        "CREATE TABLE IF NOT EXISTS "
            "names("
                "Id INTEGER PRIMARY KEY AUTOINCREMENT, "
                "name TEXT);",
        0, 0, NULL);
    check_db_rc(db, rc);

    rc = db_get_dict_entry(db, "names", "name", name, &name_id, 1);
    check_db_rc(db, rc);

    rc = sqlite3_exec(db,
        "CREATE TABLE IF NOT EXISTS "
            "flag_set("
                "Id INTEGER PRIMARY KEY AUTOINCREMENT,"
                "trans_id INTEGER,"
                "name_id INTEGER,"
                "value INTEGER);",
        0, 0, NULL);
    check_db_rc(db, rc);

    rc = db_set_auto_flag_byid(db, trans_id, name_id, value);
    check_rc(rc);

error:
    if (res)
        sqlite3_finalize(res);
    return rc;
}

static
int db_get_auto_flag_byid(sqlite3 *db, int trans_id, int name_id, int *pvalue)
{
    int rc = 0, step;
    sqlite3_stmt *res = NULL;

    /* last entry will set the value */
    /* shouldn't matter if we order by id or trans_id? */
    rc = sqlite3_prepare_v2(db,
                            "SELECT * FROM flag_set "
                                "WHERE name_id = ? AND trans_id <= ? "
                                "ORDER BY id DESC;",
                            -1, &res, 0);
    check_db_rc(db, rc);
    sqlite3_bind_int(res, 1, name_id);
    sqlite3_bind_int(res, 2, trans_id);

    step = sqlite3_step(res);

    if (step == SQLITE_ROW) { /* found */
        *pvalue = sqlite3_column_int(res, 3);
    } else {
        /* Not found is valid and means value is 0 (unset).
           This can happen although we found the name if trans_id is not the
           latest and an entry was added later. */
        *pvalue = 0;
    }
    sqlite3_finalize(res); res = NULL;
error:
    if (res)
        sqlite3_finalize(res);
    return rc;
}

static
int db_get_auto_flag(sqlite3 *db, int trans_id, const char *name, int *pvalue)
{
    int rc = 0;
    int name_id;
    sqlite3_stmt *res = NULL;

    rc = db_table_exists(db, "flag_set");
    if (rc == SQLITE_DONE) { /* no table */
        *pvalue = 0;
        return 0;
    }
    check_cond(rc == SQLITE_ROW);

    rc = db_table_exists(db, "names");
    if (rc == SQLITE_DONE) { /* no table */
        *pvalue = 0;
        return 0;
    }
    check_cond(rc == SQLITE_ROW);

    rc = db_get_dict_entry(db, "names", "name", name, &name_id, 0);
    check_db_rc(db, rc);

    if (name_id == 0) {
        /* not found is valid and means value is 0 (unset) */
        *pvalue = 0;
        return 0;
    }

    rc = db_get_auto_flag_byid(db, trans_id, name_id, pvalue);
    check_db_rc(db, rc);

error:
    if (res)
        sqlite3_finalize(res);
    return rc;
}

int history_set_auto_flag(struct history_ctx *ctx, const char *name, int value)
{
    int rc = 0;
    int oldval;

    check_ptr(name);
    check_cond(ctx->trans_id > 0);

    /* setting only when needed avoids cluttering the db */
    rc = db_get_auto_flag(ctx->db, ctx->trans_id, name, &oldval);
    check_rc(rc);

    if (oldval != value) {
        rc = db_set_auto_flag(ctx->db, ctx->trans_id, name, value);
        check_rc(rc);
    }
error:
    return rc;
}

int history_get_auto_flag(struct history_ctx *ctx, const char *name, int *pvalue)
{
    int rc = 0;

    check_ptr(name);
    check_cond(ctx->trans_id > 0);
    rc = db_get_auto_flag(ctx->db, ctx->trans_id, name, pvalue);
    check_rc(rc);
error:
    return rc;
}

/* restore flags to values from trans_id */
int history_restore_auto_flags(struct history_ctx *ctx, int trans_id)
{
    int rc = 0;
    int i, count;

    rc = db_table_exists(ctx->db, "names");
    if (rc == SQLITE_DONE) { /* no table => nothing to restore */
        return 0;
    }
    check_cond(rc == SQLITE_ROW);

    rc = db_maxid(ctx->db, "names", &count);
    check_rc(rc);

    for (i = 1; i <= count; i++) {
        int value, oldval;

        rc = db_get_auto_flag_byid(ctx->db, trans_id, i, &value);
        check_rc(rc);

        rc = db_get_auto_flag_byid(ctx->db, ctx->trans_id, i, &oldval);
        check_rc(rc);

        if (value != oldval) {
            rc = db_set_auto_flag_byid(ctx->db, ctx->trans_id, i, value);
            check_rc(rc);
        }
    }
error:
    return rc;
}

/* range is exclusive for from */
int history_replay_auto_flags(struct history_ctx *ctx, int from, int to)
{
    int rc = 0;
    int i, count;

    rc = db_table_exists(ctx->db, "names");
    if (rc == SQLITE_DONE) { /* no table => nothing to restore */
        return 0;
    }
    check_cond(rc == SQLITE_ROW);

    rc = db_maxid(ctx->db, "names", &count);
    check_rc(rc);

    for (i = 1; i <= count; i++) {
        int val_from, val_to;

        rc = db_get_auto_flag_byid(ctx->db, from, i, &val_from);
        check_rc(rc);
        rc = db_get_auto_flag_byid(ctx->db, to, i, &val_to);
        check_rc(rc);

        if (val_from != val_to)
        {
            int oldval;

            rc = db_get_auto_flag_byid(ctx->db, ctx->trans_id, i, &oldval);
            check_rc(rc);

            if (val_to != oldval) {
                rc = db_set_auto_flag_byid(ctx->db, ctx->trans_id, i, val_to);
                check_rc(rc);
            }
        }
    }
error:
    return rc;
}

/* Helper to set the ctx cookie, free'ing the old one if needed */
static
void history_set_cookie(struct history_ctx *ctx, const char *cookie)
{
    safe_free(ctx->cookie);
    ctx->cookie = strdup(cookie);
}

/* Helper to convert installed_map into list of ids.
   The list will be sorted. */
static
int *get_ids_from_map(char *map, int map_size, int *pcount)
{
    int rc = 0;
    int id;
    int *ids = NULL;
    int j = 0, count = 0;

    for (id = 1; id <= map_size; id++) {
        if (map_isset(map, id))
            count++;
    }
    check_ptr(ids = calloc(count, sizeof(int)));
    for (id = 1; id <= map_size; id++) {
        if (map_isset(map, id))
            ids[j++] = id;
    }
    *pcount = count;

error:
    if (rc)
        safe_free(ids);
    return ids;
}


/* Helper to convert installed_map into list of ids and set it for ctx */
static
int history_set_ids_from_map(struct history_ctx *ctx,
                             char *installed_map, int map_size)
{
    int rc = 0;

    safe_free(ctx->installed_ids);
    ctx->installed_ids =
        get_ids_from_map(installed_map, map_size, &ctx->installed_count);
    check_ptr(ctx->installed_ids);
error:
    return rc;
}

void history_free_delta(struct history_delta *hd)
{
    if (hd) {
        safe_free(hd->added_ids);
        safe_free(hd->removed_ids);
        free(hd);
    }
}

/* Get the delta from trans_id to current state. trans_id points to the state
   where that transaction has been completed. */
struct history_delta *history_get_delta(struct history_ctx *ctx, int trans_id)
{
    int rc = 0;
    char *installed_map = NULL;
    int map_size = 0, installed_count;
    struct history_delta *hd =
        (struct history_delta *)calloc(1, sizeof(struct history_delta));
    int *installed_ids = NULL;

    check_ptr(hd);

    /* TODO: store size in ctx and use that */
    rc = db_rpms_maxid(ctx->db, &map_size);
    check_rc(rc);

    installed_map = (char *)calloc(map_size, sizeof(char));
    check_ptr(installed_map);

    rc = db_play_transaction(ctx->db, trans_id, installed_map, map_size);
    check_rc(rc);

    installed_ids = get_ids_from_map(installed_map, map_size, &installed_count);
    check_ptr(installed_ids);

    rc = diff_arrays(ctx->installed_ids, ctx->installed_count,
                     installed_ids, installed_count,
                     &hd->removed_ids, &hd->removed_count,
                     &hd->added_ids, &hd->added_count);
    check_rc(rc);

error:
    safe_free(installed_ids);
    safe_free(installed_map);

    if (rc) {
        history_free_delta(hd);
        hd = NULL;
    }
    return hd;
}

/* Get the delta from trans_id0 to trans_id1. trans_id1 can be less than
   trans_id0, in which case the delta is reversed. Both ids point to the
   states where those transactions have been completed. */
struct history_delta *history_get_delta_range(struct history_ctx *ctx,
                                              int trans_id0, int trans_id1)
{
    int rc = 0;
    char *installed_map = NULL;
    int map_size = 0, installed_count0, installed_count1;
    struct history_delta *hd =
        (struct history_delta *)calloc(1, sizeof(struct history_delta));
    int *installed_ids0 = NULL, *installed_ids1 = NULL;

    check_ptr(hd);

    /* TODO: store size in ctx and use that */
    rc = db_rpms_maxid(ctx->db, &map_size);
    check_rc(rc);

    installed_map = (char *)calloc(map_size, sizeof(char));
    check_ptr(installed_map);

    rc = db_play_transaction(ctx->db, trans_id0, installed_map, map_size);
    check_rc(rc);

    installed_ids0 = get_ids_from_map(installed_map, map_size, &installed_count0);
    check_ptr(installed_ids0);

    safe_free(installed_map);

    installed_map = (char *)calloc(map_size, sizeof(char));
    check_ptr(installed_map);

    rc = db_play_transaction(ctx->db, trans_id1, installed_map, map_size);
    check_rc(rc);

    installed_ids1 = get_ids_from_map(installed_map, map_size, &installed_count1);
    check_ptr(installed_ids1);

    rc = diff_arrays(installed_ids1, installed_count1,
                     installed_ids0, installed_count0,
                     &hd->removed_ids, &hd->removed_count,
                     &hd->added_ids, &hd->added_count);
    check_rc(rc);

error:
    safe_free(installed_ids0);
    safe_free(installed_ids1);

    safe_free(installed_map);

    if (rc) {
        history_free_delta(hd);
        hd = NULL;
    }
    return hd;
}

void history_free_nevra_map(struct history_nevra_map *hnm)
{
    db_free_nevra_map(hnm);
}

struct history_nevra_map *history_nevra_map(struct history_ctx *ctx)
{
    return db_nevra_map(ctx->db);
}

char *history_get_nevra(struct history_nevra_map *hnm, int id)
{
    if (id > 0 && id <= hnm->count)
        return hnm->idmap[id - 1];
    return NULL;
}

/* set ctx to the state at trans_id */
int history_set_state(struct history_ctx *ctx, int trans_id)
{
    int rc = 0;
    char *installed_map = NULL;
    int map_size = 0;

    rc = db_rpms_maxid(ctx->db, &map_size);
    check_rc(rc);

    installed_map = (char *)calloc(map_size, sizeof(char));
    check_ptr(installed_map);

    rc = db_play_transaction(ctx->db, trans_id, installed_map, map_size);
    check_rc(rc);

    history_set_ids_from_map(ctx, installed_map, map_size);

    ctx->trans_id = trans_id;
error:
    safe_free(installed_map);

    return rc;
}

/* Update state in ctx and db by setting a baseline state.
 * This is used to initialize the db, but can be called later to set a new baseline.
 */
int history_record_state(struct history_ctx *ctx)
{
    int rc = 0;
    char *err_msg = NULL;
    int trans_id = 0;

    /* avoid unfinished transaction record on failure or crash */
    rc = sqlite3_exec(ctx->db, "BEGIN TRANSACTION;", 0, 0, NULL);
    check_db_rc(ctx->db, rc);

    rc = sqlite3_exec(ctx->db,
        "CREATE TABLE IF NOT EXISTS "
            "transactions(Id INTEGER PRIMARY KEY AUTOINCREMENT,"
                "cookie TEXT, cmdline TEXT,"
                "timestamp INTEGER, "
                "type INTEGER);",
        0, 0, NULL);
    check_db_rc(ctx->db, rc);

    rc = db_add_transaction(ctx->db, &trans_id, "(set)", time(NULL),
                            ctx->cookie, HISTORY_TRANS_TYPE_BASE);
    check_rc(rc);

    rc = sqlite3_exec(ctx->db,
        "CREATE TABLE IF NOT EXISTS "
            "trans_items(Id INTEGER PRIMARY KEY AUTOINCREMENT,"
            "trans_id INTEGER,"
            "type INTEGER,"
            "rpm_id INTEGER);",
        0, 0, &err_msg);
    check_db_rc(ctx->db, rc);

    rc = db_add_trans_items(ctx->db, trans_id, HISTORY_ITEM_TYPE_SET,
                            ctx->installed_ids, ctx->installed_count);
    check_rc(rc);

    ctx->trans_id = trans_id;
error:
    if (rc)
        sqlite3_exec(ctx->db, "ROLLBACK;", 0, 0, NULL);
    else
        sqlite3_exec(ctx->db, "COMMIT;", 0, 0, NULL);
    return rc;
}

void history_free_transactions(struct history_transaction *tas, int count)
{
    if (tas) {
        for(int i = 0; i < count; i++) {
            safe_free(tas[i].cookie);
            safe_free(tas[i].cmdline);
            safe_free(tas[i].delta.added_ids);
            safe_free(tas[i].delta.removed_ids);
        }
        free(tas);
    }
}

/*
   Get a list of transcactions and store them into buffer pointed to by ptas.
   The number will be stored into pcount. Optionally a range can be set with
   the from and to parameters. Either both have to be set to a range, or both
   need to be 0 for no range (all transactions). The list will be in reverse
   order if the reverse parameter is set.
*/
int history_get_transactions(struct history_ctx *ctx,
                             struct history_transaction **ptas,
                             int *pcount,
                             int reverse, int from, int to)
{
    sqlite3_stmt *res = NULL;
    int step, rc = 0;
    int i, count = 0;
    struct history_transaction *tas = NULL;
    char sql[256];

    check_cond(to >= from);

    if (from == 0 || to == 0) {
        rc = sqlite3_prepare_v2(ctx->db,
                                "SELECT * FROM transactions ORDER BY id DESC;",
                                -1, &res, 0);
        check_db_rc(ctx->db, rc);
        step = sqlite3_step(res);
        check_cond(step == SQLITE_ROW);

        count = sqlite3_column_int(res, 0);
        sqlite3_finalize(res); res = NULL;

        snprintf(sql, sizeof(sql),
            "SELECT * FROM transactions ORDER BY id%s;",
            reverse ? " DESC" : "");
        rc = sqlite3_prepare_v2(ctx->db,
                                sql,
                                -1, &res, 0);
        check_db_rc(ctx->db, rc);
    } else {
        count = to - from + 1;
        snprintf(sql, sizeof(sql),
            "SELECT * FROM transactions WHERE id BETWEEN ? AND ? ORDER BY id%s;",
            reverse ? " DESC" : "");
        rc = sqlite3_prepare_v2(ctx->db,
                                sql,
                                -1, &res, 0);
        check_db_rc(ctx->db, rc);
        sqlite3_bind_int(res, 1, from);
        sqlite3_bind_int(res, 2, to);
    }

    tas = (struct history_transaction *)calloc(count, sizeof(struct history_transaction));
    check_ptr(tas);

    for (i = 0, step = sqlite3_step(res);
         step == SQLITE_ROW;
         step = sqlite3_step(res), i++) {
        check_cond(i >= 0 && i < count);

        tas[i].id = sqlite3_column_int(res, 0);

        const char *cookie = (char *)sqlite3_column_text(res, 1);
        tas[i].cookie = cookie ? strdup(cookie) : NULL;

        const char *cmdline = (char *)sqlite3_column_text(res, 2);
        tas[i].cmdline = cmdline ? strdup(cmdline) : NULL;

        tas[i].timestamp = sqlite3_column_int(res, 3);
        tas[i].type = sqlite3_column_int(res, 4);
    }
    count = i;

    for (i = 0; i < count; i++) {
        history_delta_read(ctx->db, &tas[i].delta, tas[i].id);
    }

    *ptas = tas;
    *pcount = count;

error:
    if (res)
        sqlite3_finalize(res);
    if (rc)
        history_free_transactions(tas, count);
    return rc;
}

/*
   Update state in ctx and db to actual RPM state on system if it has changed
   by adding a delta transaction.
*/
int history_update_state(struct history_ctx *ctx, rpmts ts, const char *cmdline)
{
    int rc = 0;
    int trans_id;
    int *current_ids = NULL, current_count;
    int *added_ids = NULL, added_count;
    int *removed_ids = NULL, removed_count;
    char *cookie = NULL;

    cookie = rpmdbCookie(rpmtsGetRdb(ts));
    check_ptr(cookie);

    if (strcmp(ctx->cookie, cookie) == 0) {
        /* nothing changed */
        safe_free(cookie);
        return 0;
    }

    rc = db_update_rpms(ts, ctx->db, &current_ids, &current_count);
    check_rc(rc);

    rc = diff_arrays(ctx->installed_ids, ctx->installed_count,
                current_ids, current_count,
                &removed_ids, &removed_count,
                &added_ids, &added_count);
    check_rc(rc);

    /* avoid unfinished transaction record on failure or crash */
    rc = sqlite3_exec(ctx->db, "BEGIN TRANSACTION;", 0, 0, NULL);
    check_db_rc(ctx->db, rc);

    rc = db_add_transaction(ctx->db, &trans_id, cmdline, time(NULL),
                            cookie, HISTORY_TRANS_TYPE_DELTA);
    check_rc(rc);

    /* both added_count and removed_count can be 0 even if the cookie differed
       if there was a reinstall */
    if (added_count > 0) {
        rc = db_add_trans_items(ctx->db, trans_id, HISTORY_ITEM_TYPE_ADD,
                                added_ids, added_count);
        check_rc(rc);
    }
    if (removed_count > 0) {
        rc = db_add_trans_items(ctx->db, trans_id, HISTORY_ITEM_TYPE_REMOVE,
                                removed_ids, removed_count);
        check_rc(rc);
    }

    /* replace ctx->installed_ids */
    safe_free(ctx->installed_ids);
    ctx->installed_ids = current_ids;
    ctx->installed_count = current_count;

    history_set_cookie(ctx, cookie);
    ctx->trans_id = trans_id;
error:
    if (rc)
        sqlite3_exec(ctx->db, "ROLLBACK;", 0, 0, NULL);
    else
        sqlite3_exec(ctx->db, "COMMIT;", 0, 0, NULL);
    safe_free(added_ids);
    safe_free(removed_ids);
    safe_free(cookie);
    return rc;
}

/* sync history context to current state from ts */
int history_sync(struct history_ctx *ctx, rpmts ts)
{
    sqlite3_stmt *res = NULL;
    int step, rc = 0;
    char *cookie = NULL;
    int db_isfresh = 1;

    check_ptr(ts);

    cookie = rpmdbCookie(rpmtsGetRdb(ts));
    /* this fails if the rpm db isn't opened */
    check_ptr(cookie);

    step = db_table_exists(ctx->db, "transactions");

    if (step == SQLITE_ROW) {
        rc = sqlite3_prepare_v2(ctx->db,
                                "SELECT * FROM transactions ORDER BY id DESC;",
                                -1, &res, 0);
        check_db_rc(ctx->db, rc);

        step = sqlite3_step(res);
        if (step == SQLITE_ROW) {
            int id = sqlite3_column_int(res, 0);
            const char *cookie_db = (char *)sqlite3_column_text(res, 1);

            if (strcmp(cookie, cookie_db) != 0) {
                /*
                 * cookie has changed but not by us (probably rpm) -
                 * update state by adding a pseudo transaction.
                 */
                rc = history_set_state(ctx, id);
                check_rc(rc);
                history_set_cookie(ctx, cookie_db);
                rc = history_update_state(ctx, ts, "(unknown)");
                check_rc(rc);
            } else {
                /*
                 * No change, we can either update from db or read rpms.
                 * The former may need replaying history, the latter may be faster
                 */
                rc = db_update_rpms(ctx->ts, ctx->db,
                                    &(ctx->installed_ids), &ctx->installed_count);
                check_rc(rc);
                history_set_cookie(ctx, cookie_db);
                ctx->trans_id = id;
            }
            sqlite3_finalize(res); res = NULL;
            db_isfresh = 0;
        }
    } else {
        check_cond(step == SQLITE_DONE);
    }

    if (db_isfresh) {
        /* we are starting from scratch */
        history_set_cookie(ctx, cookie);

        rc = db_update_rpms(ts, ctx->db,
                            &(ctx->installed_ids), &ctx->installed_count);
        check_rc(rc);

        rc = history_record_state(ctx);
        check_rc(rc);
    }

error:
    if (res)
        sqlite3_finalize(res);
    safe_free(cookie);
    return rc;
}

struct history_ctx *create_history_ctx(const char *db_filename)
{
    int rc = 0, step;
    sqlite3 *db;
    struct history_ctx *ctx = NULL;
    sqlite3_stmt *res = NULL;

    db = init_db(db_filename);
    check_ptr(db);

    ctx = (struct history_ctx *)calloc(1, sizeof(struct history_ctx));
    check_ptr(ctx);

    ctx->db = db;

    step = db_table_exists(ctx->db, "transactions");
    check_db_step(db, step);

    if (step == SQLITE_ROW) {
        rc = sqlite3_prepare_v2(ctx->db,
                                "SELECT * FROM transactions ORDER BY id DESC;",
                                -1, &res, 0);
        check_db_rc(ctx->db, rc);

        step = sqlite3_step(res);
        if (step == SQLITE_ROW) {
            char *cookie = (char *)sqlite3_column_text(res, 1);
            if (cookie)
                history_set_cookie(ctx, cookie);
            ctx->trans_id = sqlite3_column_int(res, 0);
        }
    }
error:
    if (res)
        sqlite3_finalize(res);
    if (rc) {
        destroy_history_ctx(ctx);
        return NULL;
    }
    return ctx;
}

void destroy_history_ctx(struct history_ctx *ctx)
{
    if (ctx) {
        if(ctx->db)
            sqlite3_close(ctx->db);
        safe_free(ctx->installed_ids);
        safe_free(ctx->cookie);
        free(ctx);
    }
}
