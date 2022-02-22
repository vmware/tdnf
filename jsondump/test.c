/*
 * Copyright (C) 2022 VMware, Inc. All Rights Reserved.
 *
 * Licensed under the GNU General Public License v2 (the "License");
 * you may not use this file except in compliance with the License. The terms
 * of the License are located in the COPYING file of this distribution.
 */

#include <stdio.h>

#include "jsondump.h"

int main(int argc, char *argv[])
{
    struct json_dump *jd, *jd1;
    
    jd = jd_create(0);
    jd_map_start(jd);
    jd_map_add_string(jd, "foo", "bar");
    jd_map_add_string(jd, "goo", "car");
    jd_map_add_string(jd, "hoo", "\tdar\n");
    jd_map_add_fmt(jd, "ioo", "%d ears", 2);
    jd_map_add_null(jd, "nothing");
    jd_map_add_bool(jd, "yes", 1);
    jd_map_add_bool(jd, "no", 0);

    printf("%s\n", jd->buf);

    jd1 = jd_create(0);

    jd_map_start(jd1);
    jd_map_add_child(jd1, "nested", jd);
    
    printf("%s\n", jd1->buf);
    
    jd_destroy(jd);
    jd_destroy(jd1);
    
    jd = jd_create(0);
    jd_list_start(jd);
    for(int i = 0; i < 10; i++) {
        char buf[3];
        snprintf(buf, sizeof(buf), "%d", i);
        jd_list_add_string(jd, buf);
    }
    jd_list_add_null(jd);

    printf("%s\n", jd->buf);
    
    jd = jd_create(0);
    jd_list_start(jd);
    for(int i = 0; i < 10; i++) {
        jd_list_add_fmt(jd, "i=%d", i);
    }

    printf("%s\n", jd->buf);
    
    jd_destroy(jd);
    
    jd = jd_create(0);
    jd_list_start(jd);
    for(int i = 0; i < 10; i++) {
        jd_list_add_int(jd, i);
    }

    printf("%s\n", jd->buf);
    
    jd_destroy(jd);

    jd = jd_create(0);
    jd_list_start(jd);
    jd_list_add_bool(jd, 1);
    jd_list_add_bool(jd, 0);

    printf("%s\n", jd->buf);
    
    jd_destroy(jd);

    return 0;
}