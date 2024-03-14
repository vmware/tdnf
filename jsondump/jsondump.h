/*
 * Copyright (C) 2022-2023 VMware, Inc. All Rights Reserved.
 *
 * Licensed under the GNU General Public License v2 (the "License");
 * you may not use this file except in compliance with the License. The terms
 * of the License are located in the COPYING file of this distribution.
 */

#pragma once
#include <stdint.h>

struct json_dump{
    char *buf;
    unsigned int buf_size;
    unsigned int pos;
};

struct json_dump *jd_create(unsigned int size);
void jd_destroy(struct json_dump *jd);

int jd_map_start(struct json_dump *jd);
int jd_map_add_string(struct json_dump *jd, const char *key, const char *value);
int jd_map_add_int(struct json_dump *jd, const char *key, int value);
int jd_map_add_int64(struct json_dump *jd, const char *key, int64_t value);
int jd_map_add_bool(struct json_dump *jd, const char *key, int value);
int jd_map_add_null(struct json_dump *jd, const char *key);
int jd_map_add_fmt(struct json_dump *jd, const char *key, const char *format, ...);
int jd_map_add_child(struct json_dump *jd, const char *key, const struct json_dump *jd_child);

int jd_list_start(struct json_dump *jd);
int jd_list_add_string(struct json_dump *jd, const char *value);
int jd_list_add_int(struct json_dump *jd, int value);
int jd_list_add_int64(struct json_dump *jd, int64_t value);
int jd_list_add_bool(struct json_dump *jd, int value);
int jd_list_add_null(struct json_dump *jd);
int jd_list_add_fmt(struct json_dump *jd, const char *format, ...);
int jd_list_add_child(struct json_dump *jd, const struct json_dump *jd_child);
