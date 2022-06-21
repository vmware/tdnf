/*
 * Copyright (C) 2022 VMware, Inc. All Rights Reserved.
 *
 * Licensed under the GNU General Public License v2 (the "License");
 * you may not use this file except in compliance with the License. The terms
 * of the License are located in the COPYING file of this distribution.
 */

#include <stdio.h>

#include "jsondump.h"

#define CHECK_RC(x) if (x) {fprintf(stderr, "FAIL: rc != 0 in line %d\n", __LINE__); }
#define CHECK_NULL(x) if (x == NULL) {fprintf(stderr, "FAIL: NULL returned in line %d\n", __LINE__); }

int main(void)
{
    struct json_dump *jd, *jd1;

    /* flat map with all inds of values */
    jd = jd_create(0);
    CHECK_NULL(jd);

    CHECK_RC(jd_map_start(jd));
    CHECK_RC(jd_map_add_string(jd, "foo", "bar"));
    CHECK_RC(jd_map_add_string(jd, "goo", "car"));
    CHECK_RC(jd_map_add_string(jd, "hoo", "\tdar\n"));
    CHECK_RC(jd_map_add_fmt(jd, "ioo", "%d ears", 2));
    CHECK_RC(jd_map_add_null(jd, "nothing"));
    CHECK_RC(jd_map_add_bool(jd, "yes", 1));
    CHECK_RC(jd_map_add_bool(jd, "no", 0));

    printf("%s\n", jd->buf);

    /* nested map in map */
    jd1 = jd_create(0);
    CHECK_NULL(jd1);

    CHECK_RC(jd_map_start(jd1));
    CHECK_RC(jd_map_add_child(jd1, "nested", jd));

    printf("%s\n", jd1->buf);

    jd_destroy(jd);
    jd_destroy(jd1);

    /* list with strings */
    jd = jd_create(0);
    CHECK_NULL(jd);

    CHECK_RC(jd_list_start(jd));
    for(int i = 0; i < 10; i++) {
        char buf[3];
        snprintf(buf, sizeof(buf), "%d", i);
        jd_list_add_string(jd, buf);
    }
    jd_list_add_null(jd);

    printf("%s\n", jd->buf);

    jd_destroy(jd);

    /* list with format strings */
    jd = jd_create(0);
    CHECK_NULL(jd);

    CHECK_RC(jd_list_start(jd));
    for(int i = 0; i < 10; i++) {
        CHECK_RC(jd_list_add_fmt(jd, "i=%d", i));
    }

    printf("%s\n", jd->buf);

    jd_destroy(jd);

    /* list of ints */
    jd = jd_create(0);
    CHECK_NULL(jd);

    CHECK_RC(jd_list_start(jd));
    for(int i = 0; i < 10; i++) {
        CHECK_RC(jd_list_add_int(jd, i));
    }

    printf("%s\n", jd->buf);

    jd_destroy(jd);

    /* list of bools */
    jd = jd_create(0);
    CHECK_NULL(jd);

    CHECK_RC(jd_list_start(jd));
    CHECK_RC(jd_list_add_bool(jd, 1));
    CHECK_RC(jd_list_add_bool(jd, 0));

    printf("%s\n", jd->buf);

    jd_destroy(jd);

    return 0;
}