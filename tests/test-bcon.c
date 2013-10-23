/*
 * @file test-bcon.c
 * @brief BCON (BSON C Object Notation) initial test
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
#include <string.h>
#include <stdio.h>

int main(int argc, char ** argv)
{
    int f = 10;
    int g = 100;
    int * foo = &f;
    int * bar = &g;
    int ** huh = &foo;
    bson_t * bson, ** bson_ptr;

    bcon_t * bcon = BCON(
        "foo", "bar",
        "bar", BCON_INT32(10),
        "bar2", BCON_PINT32(huh),
        "baz", BCON_DOC(
            "lol", BCON_ARRAY("hi", BCON_DOUBLE(5.5), BCON_DOUBLE(5.6), BCON_DOUBLE(5.7), "bye"),
        ),
        "binary", BCON_BINARY(BSON_SUBTYPE_BINARY, "12345678901", 11),
        "codewscope", BCON_CODEWSCOPE("print x;", "x", BCON_DOUBLE(10)),
        "code", BCON_CODEWSCOPE("print 10;"),
    );

    bcon_DUMP_AS_JSON(bcon);
    *huh = bar;
    bcon_DUMP_AS_JSON(bcon);

    bcon_t * broken_bcon = BCON(
        "foo", "bar",
        "bar", BCON_DOC(
            "foo",
        ),
        "foo",
    );

    bcon_DUMP(broken_bcon);

    bcon_t * insert_bcon = BCON(
        "state", BCON_DOC( "$in", BCON_ARRAY("NJ", "NY", "PA")),
    );

    bcon_DUMP_AS_JSON(insert_bcon);

    int i;
    char buf[100];

    bson = bson_new();
    bson_ptr = &bson;

    for (i = 0; i < 10; i++) {
        sprintf(buf, "%d", i);
        bson_append_int32(bson, buf, -1, i);
    }

    bcon_t * insert_bcon_w_bson = BCON(
        "state", BCON_DOC( "$in", BCON_RBSON_ARRAY(bson_ptr) ),
    );

    bcon_DUMP_AS_JSON(insert_bcon_w_bson);

    bson_destroy(bson);
    bson = bson_new();
    bson_ptr = &bson;

    for (i = 10; i < 20; i++) {
        sprintf(buf, "%d", i);
        bson_append_int32(bson, buf, -1, i);
    }

    bcon_DUMP_AS_JSON(insert_bcon_w_bson);

    bson_destroy(bson);

    return 0;
}
