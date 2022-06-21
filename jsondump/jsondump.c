/*
 * Copyright (C) 2022 VMware, Inc. All Rights Reserved.
 *
 * Licensed under the GNU General Public License v2 (the "License");
 * you may not use this file except in compliance with the License. The terms
 * of the License are located in the COPYING file of this distribution.
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

#include "jsondump.h"

#define SIZE_INC 256

/*
 * This is a very simple libary to simplify creating json strings.
 * It only allows for creating json output. It is not intended
 * for access of these json data in a structured way, and there is
 * no parsing back into json data. There are other libraries
 * for that purpose, but they are too complex if all you need is
 * a simple dump of json.
 *
 * Look at test.c for usage examples.
 */

/* Example 'make_message' from man 3 snprintf,
 * modified to accept va_list.
 * The allocated string returned must be free'd by the caller.
 */
static
char *_alloc_vsprintf(const char *fmt, va_list ap)
{
    int size = 0;
    char *p = NULL;
    va_list aq;
    va_copy(aq, ap);

    /* Determine required size */
    size = vsnprintf(p, size, fmt, ap);

    if (size < 0)
        return NULL;

    size++;             /* For '\0' */
    p = (char *)calloc(1, size);
    if (p == NULL)
        return NULL;

    size = vsnprintf(p, size, fmt, aq);
    va_end(aq);

    if (size < 0) {
        free(p);
        return NULL;
    }

    return p;
}

/*
 * Create a context to hold a json buffer. After use it needs to be
 * free'd using json_destroy().
 * The buffer will be allocated with size bytes, or SIZE_INC bytes if 0.
 */

struct json_dump *jd_create(unsigned int size)
{
    struct json_dump *jd = NULL;

    jd = calloc(1, sizeof(struct json_dump));
    if (!jd)
        return NULL;

    if (size == 0)
        size = SIZE_INC;

    jd->buf = (char *)calloc(size, sizeof(char));
    if (!jd->buf) {
        jd_destroy(jd);
        return NULL;
    }
    jd->buf_size = size;

    return jd;
}

void jd_destroy(struct json_dump *jd)
{
    if (jd) {
        if (jd->buf)
            free(jd->buf);
        free(jd);
    }
}

/* make sure we have at least add_size bytes left in buffer by reallocating
 * if needed */
static
int _jd_realloc(struct json_dump *jd, unsigned int add_size)
{
    if (jd == NULL)
        return -1;

    if (jd->pos + add_size >= jd->buf_size - 1) {
        add_size += SIZE_INC;
        jd->buf = (char *)realloc((void *)jd->buf, jd->buf_size + add_size);
        if (!jd->buf)
            return -1;
        jd->buf_size = jd->buf_size + add_size;
    }
    return 0;
}

/*
 * Convert string to json format, escaping characters if needed and add
 * quotes.
 * The string will be allocated using calloc() and it's the caller's
 * responsibilty to free it.
 */

char *jsonify_string(const char *str)
{
    char *p;
    const char *q = str;
    char *buf;

    /* allocate for worst case - every char escaped plus quotes + nul */
    buf = calloc(strlen(str)*2+3, sizeof(char));
    if(!buf)
        return NULL;

    p = buf;
    *p++ = '\"';
    while(*q) {
        switch (*q) {
            case '"':
                *p++ = '\\';
                *p++ = '"';
                break;
            case '\\':
                *p++ = '\\';
                *p++ = '\\';
                break;
            case '\b':
                *p++ = '\\';
                *p++ ='b';
                break;
            case '\f':
                *p++ = '\\';
                *p++ ='f';
                break;
            case '\n':
                *p++ = '\\';
                *p++ ='n';
                break;
            case '\r':
                *p++ = '\\';
                *p++ ='r';
                break;
            case '\t':
                *p++ = '\\';
                *p++ ='t';
                break;
            default:
                *p++ = *q;
        }
        q++;
    }
    *p++ = '\"';
    *p = 0;
    return buf;
}

/* add an empty map to the buffer */
int jd_map_start(struct json_dump *jd)
{
    if(_jd_realloc(jd, 2))
        return -1;
    jd->buf[jd->pos++] = '{';
    jd->buf[jd->pos++] = '}';
    jd->buf[jd->pos] = 0;

    return 0;
}

/* before adding an item to a map, we need to rewind and replace the
 * closing bracket with a comma. */
static
void _jd_map_prep_append(struct json_dump *jd)
{
    if (jd->pos > 1 && jd->buf[jd->pos-1] == '}') {
        jd->pos--;
        if (jd->buf[jd->pos-1] != '{')
            jd->buf[jd->pos++] = ',';
    }
}

/* helper to add a key value pair with the string 'as-is'
 * (w/out jsonifying and quotes)
 */
static
int _jd_map_add_raw(struct json_dump *jd, const char *key, const char *value)
{
    int add_size = strlen(key) + strlen(value) + 5; /* lengths plus quotes plus colon plus closing bracket plus nul*/
    int l;

    if (jd == NULL)
        return -1;

    if(_jd_realloc(jd, add_size))
        return -1;
    _jd_map_prep_append(jd);

    l = snprintf(&(jd->buf[jd->pos]), jd->buf_size - jd->pos, "\"%s\":%s}", key, value);
    if (l < 0)
        return -1;
    jd->pos += l;
    return 0;
}

/*
 * Add a string value to a map with key. The string will be processed
 * to convert it to json format. The string can be NULL, in which case
 * a json 'null' will be added instead.
 */
int jd_map_add_string(struct json_dump *jd, const char *key, const char *value)
{
    int rc;

    if (!value)
        return jd_map_add_null(jd, key);

    char *json_value = jsonify_string(value);
    if (json_value == NULL)
        return -1;

    rc = _jd_map_add_raw(jd, key, json_value);

    free(json_value);

    return rc;
}

int jd_map_add_int(struct json_dump *jd, const char *key, int value)
{
    char buf[22]; /* 22 = length of 2^64 + 1 */

    if (snprintf(buf, sizeof(buf), "%d", value) < 0)
        return -1;
    return _jd_map_add_raw(jd, key, buf);
}

int jd_map_add_bool(struct json_dump *jd, const char *key, int value)
{
    return _jd_map_add_raw(jd, key, value ? "true": "false");
}

int jd_map_add_null(struct json_dump *jd, const char *key)
{
    return _jd_map_add_raw(jd, key, "null");
}

int jd_map_add_fmt(struct json_dump *jd, const char *key, const char *format, ...)
{
    va_list args;
    char *buf;
    int rc;

    va_start(args, format);
    buf = _alloc_vsprintf(format, args);
    va_end(args);

    if (buf == NULL)
        return -1;

    rc = jd_map_add_string(jd, key, buf);

    free(buf);

    return rc;
}

int jd_map_add_child(struct json_dump *jd, const char *key, struct json_dump *jd_child)
{
    return _jd_map_add_raw(jd, key, jd_child->buf);
}

/* create an empty list */
int jd_list_start(struct json_dump *jd)
{
    if(_jd_realloc(jd, 2))
        return -1;
    jd->buf[jd->pos++] = '[';
    jd->buf[jd->pos++] = ']';
    jd->buf[jd->pos] = 0;

    return 0;
}

static
int _jd_list_prep_append(struct json_dump *jd)
{
    if (jd->pos > 1 && jd->buf[jd->pos-1] == ']') {
        jd->pos--;
        if (jd->buf[jd->pos-1] != '[')
            jd->buf[jd->pos++] = ',';
    }
    return 0;
}

/* similar to _jd_map_add_raw() but for a list */
static
int _jd_list_add_raw(struct json_dump *jd, const char *value)
{
    int add_size = strlen(value) + 2; /* length plus closing bracket plus nul */
    int l;

    if (jd == NULL)
        return -1;

    if(_jd_realloc(jd, add_size))
        return -1;
    _jd_list_prep_append(jd);

    l = snprintf(&(jd->buf[jd->pos]), jd->buf_size - jd->pos, "%s]", value);
    if (l < 0)
        return -1;
    jd->pos += l;

    return 0;
}

/* similar to jd_map_add_string() but for a list */
int jd_list_add_string(struct json_dump *jd, const char *value)
{
    char *json_value;
    int rc;

    if (!value)
        return jd_list_add_null(jd);

    json_value = jsonify_string(value);
    if (json_value == NULL)
        return -1;

    rc = _jd_list_add_raw(jd, json_value);

    free(json_value);

    return rc;
}

int jd_list_add_int(struct json_dump *jd, int value)
{
    char buf[22]; /* 22 = length of 2^64 + 1 */

    if (snprintf(buf, sizeof(buf), "%d", value) < 0)
        return -1;
    return _jd_list_add_raw(jd, buf);
}

int jd_list_add_bool(struct json_dump *jd, int value)
{
    return _jd_list_add_raw(jd, value? "true" : "false");
}

int jd_list_add_null(struct json_dump *jd)
{
    return _jd_list_add_raw(jd, "null");
}

int jd_list_add_fmt(struct json_dump *jd, const char *format, ...)
{
    va_list args;
    char *buf = NULL;
    int rc;

    va_start(args, format);
    buf = _alloc_vsprintf(format, args);
    va_end(args);

    if (buf == NULL)
        return -1;

    rc = jd_list_add_string(jd, buf);

    free(buf);

    return rc;
}

int jd_list_add_child(struct json_dump *jd, struct json_dump *jd_child)
{
    return _jd_list_add_raw(jd, jd_child->buf);
}

