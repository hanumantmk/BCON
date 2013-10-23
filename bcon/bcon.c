/*
 * @file bcon.c
 * @brief BCON (BSON C Object Notation) Implementation
 */

/*    Copyright 2009-2013 MongoDB Inc.
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

#include "bcon.h"
#include <error.h>
#include "inc/utstring.h"

char * BCON_MAGIC = "BCON_MAGIC";

static char * BCON_TYPE_ENUM_STR[] = {
#include "bcon_enum_str.h"
    "BCONT_END",
    "BCONT_ERROR"
};

int bcon_to_bson_put_value(bson_t * bson, const char * key, void * in, bcon_type_t type);

bcon_type_t bcon_token(bcon_t ** stream, void ** out)
{
    bcon_type_t type;
    bcon_t * in;

    if ((*stream)->UTF8 == NULL) {
        type = BCONT_END;
    } else {
        if ((*stream)->UTF8 == BCON_MAGIC) {
            (*stream)++;

            type = (*stream)->type;

            (*stream)++;

            in = *stream;
            switch(type) {
#include "bcon_indirection.h"
                default:
                    type = BCONT_ERROR;
                    break;
            };
        } else {
            type = BCONT_UTF8;
            *out = (void *)*stream;
        }
    }

    (*stream)++;

    return type;
}

int bcon_to__bson(bcon_t * in, bson_t * bson, int is_array)
{
    void * obj = NULL;
    bcon_type_t type;

    int i = 0;
    char i_str[100];
    const char * key;

    while (1) {
        if (is_array) {
            sprintf(i_str, "%d", i);
            key = i_str;
        } else {
            type = bcon_token(&in, &obj);

            if (type == BCONT_END) return 0;

            // TODO deal with indirection here
            assert(type == BCONT_UTF8);

            key = *((char **)obj);
        }

        type = bcon_token(&in, &obj);

        switch(type) {
            case BCONT_END:
                return is_array ? 0 : 1;
                break;
            default:
                if (bcon_to_bson_put_value(bson, key, obj, type)) return 1;
                break;
        }

        i++;
    }

    return 0;
}

char * bcon_to_bson(bcon_t * in, bson_t * bson)
{
    int r = bcon_to__bson(in, bson, 0);

    if (r) {
        return bcon_dump(in);
    } else {
        return NULL;
    }
}

int bcon_to_bson_put_value(bson_t * bson, const char * key, void * val, bcon_type_t type)
{
    bson_t child;

    bcon_type_t val_type = type;

    switch(val_type) {
        case BCONT_UTF8:
            bson_append_utf8(bson, key, -1, *((char **)val), -1);
            break;
        case BCONT_DOUBLE:
            bson_append_double(bson, key, -1, *((double *)val));
            break;
        case BCONT_BCON_DOCUMENT:
            bson_append_document_begin(bson, key, -1, &child);
            if (bcon_to__bson(*((bcon_t **)val), &child, 0)) return 1;
            bson_append_document_end(bson, &child);
            break;
        case BCONT_BCON_ARRAY:
            bson_append_array_begin(bson, key, -1, &child);
            if (bcon_to__bson(*((bcon_t **)val), &child, 1)) return 1;
            bson_append_array_end(bson, &child);
            break;
        case BCONT_BIN: {
            bcon_binary_t * z = *((bcon_binary_t **)val);

            bson_append_binary(bson, key, -1, z->subtype, z->binary, z->length);
            break;
        }
        case BCONT_UNDEFINED:
            bson_append_undefined(bson, key, -1);
            break;
        case BCONT_BSON_OID:
            bson_append_oid(bson, key, -1, *((bson_oid_t **)val));
            break;
        case BCONT_BOOL:
            bson_append_bool(bson, key, -1, *((bson_bool_t *)val));
            break;
        case BCONT_DATE_TIME:
            bson_append_timeval(bson, key, -1, *((struct timeval **)val));
            break;
        case BCONT_NULL:
            bson_append_null(bson, key, -1);
            break;
        case BCONT_BCON_REGEX: {
            bcon_regex_t * r = *((bcon_regex_t **)val);

            bson_append_regex(bson, key, -1, r->regex, r->flags);
            break;
        }
        case BCONT_BCON_DBPOINTER: {
            bcon_dbpointer_t * db = *((bcon_dbpointer_t **)val);

            bson_append_dbpointer(bson, key, -1, db->collection, db->oid);
            break;
        }
        case BCONT_BCON_CODE: {
            bcon_code_t * code = *((bcon_code_t **)val);

            bson_append_code(bson, key, -1, code->code);
            break;
        }
        case BCONT_SYMBOL:
            bson_append_symbol(bson, key, -1, *((char **)val), -1);
            break;
        case BCONT_BCON_CODEWSCOPE: {
            bcon_code_t * code = *((bcon_code_t **)val);

            bson_t * child = bson_new();
            int r = bcon_to__bson(code->scope, child, 0);

            if (! r) bson_append_code_with_scope(bson, key, -1, code->code, child);

            bson_destroy(child);

            if (r) return r;

            break;
        }
        case BCONT_INT32:
            bson_append_int32(bson, key, -1, *((bson_int32_t *)val));
            break;
        case BCONT_BCON_TIMESTAMP: {
            bcon_timestamp_t * ts = *((bcon_timestamp_t **)val);

            bson_append_timestamp(bson, key, -1, ts->timestamp, ts->increment);
            break;
        }
        case BCONT_INT64:
            bson_append_int64(bson, key, -1, *((bson_int64_t *)val));
            break;
        case BCONT_MAXKEY:
            bson_append_maxkey(bson, key, -1);
            break;
        case BCONT_MINKEY:
            bson_append_minkey(bson, key, -1);
            break;
        case BCONT_BSON_ARRAY: {
            bson_t * child = *((bson_t **)val);

            bson_append_array(bson, key, -1, child);
            break;
        }
        case BCONT_BSON_DOCUMENT: {
            bson_t * child = *((bson_t **)val);

            bson_append_document(bson, key, -1, child);
            break;
        }
        default:
            return 1;
            break;
    }

    return 0;
}

void bcon_DUMP_AS_JSON(bcon_t * in)
{
    bson_t * bson = bson_new();
    char * err_str = bcon_to_bson(in, bson);

    if (err_str) {
        printf("ERROR: %s\n", err_str);
        free(err_str);
    } else {
        char * json = bson_as_json(bson, NULL);
        printf("%s\n", json);
        free(json);
    }

    bson_destroy(bson);
}

void bcon__DUMP(bcon_t ** in, int is_array, UT_string * s, int indent)
{
    bcon_type_t type;
    void * ptr = NULL;
    char * key;
    bcon_t * child;
    int keep_going = 1;

    utstring_printf(s, "%s\n", is_array ? "[" : "{");

    while (keep_going) {
        if (is_array) {
            type = bcon_token(in, &ptr);

            if (! (type == BCONT_ERROR || type == BCONT_END)) {
                utstring_printf(s, "%-*s", indent + 2, "");
            }
        } else {
            type = bcon_token(in, &ptr);

            if (type == BCONT_END) {
                keep_going = 0;
                break;
            } else if (type == BCONT_UTF8) {
                key = *(char **)ptr;

                utstring_printf(s, "%-*s\"%s\" : ", indent + 2, "", key);

                type = bcon_token(in, &ptr);
            } else {
                type = BCONT_ERROR;
            }
        }

        switch (type) {
            case BCONT_UTF8:
                utstring_printf(s, "\"%s\"", *(char **)ptr);
                break;
            case BCONT_BCON_DOCUMENT:
                child = *(bcon_t **)ptr;
                bcon__DUMP(&child, 0, s, indent + 2);
                break;
            case BCONT_BCON_ARRAY:
                child = *(bcon_t **)ptr;
                bcon__DUMP(&child, 1, s, indent + 2);
                break;
            case BCONT_BCON_CODEWSCOPE: {
                bcon_code_t * code = *(bcon_code_t **)ptr;

                utstring_printf(s, "%s(", BCON_TYPE_ENUM_STR[type]);
                bcon__DUMP(&code->scope, 0, s, indent + 2);
                utstring_printf(s, "%-*s)", indent, "");
                break;
            }
            case BCONT_ERROR:
                utstring_printf(s, "<ERROR HERE>");
                return;
            case BCONT_END:
                if (! is_array) {
                    utstring_printf(s, "<ERROR HERE>");
                    return;
                }
                keep_going = 0;
                break;
            default:
                utstring_printf(s, "%s", BCON_TYPE_ENUM_STR[type]);
                break;
        }

        if (keep_going) utstring_printf(s, ",\n");
    }

    utstring_printf(s, "%-*s%s", indent, "", is_array ? "]" : "}");
}

char * bcon_dump(bcon_t * in)
{
    UT_string s;
    utstring_init(&s);

    bcon__DUMP(&in, 0, &s, 0);

    return utstring_body(&s);
}

void bcon_DUMP(bcon_t * in)
{
    char * str = bcon_dump(in);

    printf("%s\n", str);

    free(str);
}
