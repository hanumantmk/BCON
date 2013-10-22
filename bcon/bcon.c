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
    "BCONT_END",
    "BCONT_NULL",
#include "bcon_enum_str.h"
    "BCONT_ERROR"
};

int bcon_to_bson_put_value(bson_t * bson, const char * key, void * in, bcon_type_t type);

bcon_type_t bcon_token(bcon_t ** stream, void ** out)
{
    bcon_type_t type;
    bcon_t * in;

    if ((*stream)->STRING == NULL) {
        type = BCONT_END;
    } else {
        if ((*stream)->STRING == BCON_MAGIC) {
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
            type = BCONT_STRING;
            *out = (void *)*stream;
        }
    }

    (*stream)++;

    return type;
}

int bcon_to__bson(bcon_t * in, bson_t * bson, int is_array)
{
    void * obj;
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
            assert(type == BCONT_STRING);

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

    if (r == 1) {
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
        case BCONT_DOUBLE:
            bson_append_double(bson, key, -1, *((double *)val));
            break;
        case BCONT_STRING:
            bson_append_utf8(bson, key, -1, *((char **)val), -1);
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
        case BCONT_OID: {
            bson_oid_t oid;

            if (val) {
                bson_oid_init_from_string(&oid, *((char **)val));
            } else {
                bson_oid_init(&oid, NULL);
            }

            bson_append_oid(bson, key, -1, &oid);
            break;
        }
        case BCONT_BOOL:
            bson_append_bool(bson, key, -1, *((bson_bool_t *)val));
            break;
        case BCONT_TIMESTAMP:
            bson_append_date_time(bson, key, -1, *((bson_int64_t *)val));
            break;
        case BCONT_NULL:
            bson_append_null(bson, key, -1);
            break;
        case BCONT_SYMBOL:
            bson_append_symbol(bson, key, -1, *((char **)val), -1);
            break;
        case BCONT_INT32:
            bson_append_int32(bson, key, -1, *((bson_int32_t *)val));
            break;
        case BCONT_INT64:
            bson_append_int64(bson, key, -1, *((bson_int64_t *)val));
            break;
        case BCONT_BIN: {
            bcon_binary_t * z = *((bcon_binary_t **)val);

            bson_append_binary(bson, key, -1, z->subtype, z->binary, z->length);
            break;
        }
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
    void * ptr;
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
            } else if (type == BCONT_STRING) {
                key = *(char **)ptr;

                utstring_printf(s, "%-*s\"%s\" : ", indent + 2, "", key);

                type = bcon_token(in, &ptr);
            } else {
                type = BCONT_ERROR;
            }
        }

        switch (type) {
            case BCONT_STRING:
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
            case BCONT_ERROR:
                utstring_printf(s, "<ERROR HERE>");
                return;
            case BCONT_END:
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
