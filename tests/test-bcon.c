/*
 * @file test-bcon.c
 * @brief BCON (BSON C Object Notation) Tests
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

#include <check.h>
#include "bcon.h"

void bcon_eq_bson(bcon_t * bcon, bson_t * expected)
{
    char * bson_json, * expected_json, * err_str;
    int unequal;

    bson_t * bson = bson_new();

    err_str = bcon_to_bson(bcon, bson);
    ck_assert_msg(err_str == NULL, "Error in bcon_to_bson: (%s)", err_str);
    if (err_str) free(err_str);

    unequal = (expected->len != bson->len);
    if (! unequal) unequal = memcmp(bson_get_data(expected), bson_get_data(bson), expected->len);

    if (unequal) {
        bson_json = bson_as_json(bson, NULL);
        expected_json = bson_as_json(expected, NULL);
    }
    ck_assert_msg(! unequal, "bson objects unequal: (%s) != (%s)", bson_json, expected_json);
    if (unequal) {
        free(bson_json);
        free(expected_json);
    }

    bson_destroy(bson);
    bson_destroy(expected);
}

START_TEST(test_utf8)
{
    bson_t * bson = bson_new();
    bson_append_utf8(bson, "foo", -1, "bar", -1);

    bcon_t * bcon = BCON(
        "foo", "bar",
    );

    bcon_eq_bson(bcon, bson);
}
END_TEST

START_TEST(test_double)
{
    bson_t * bson = bson_new();
    bson_append_double(bson, "foo", -1, 1.1);

    bcon_t * bcon = BCON(
        "foo", BCON_DOUBLE(1.1),
    );

    bcon_eq_bson(bcon, bson);
}
END_TEST

START_TEST(test_bcon_document)
{
    bson_t * bson = bson_new();
    bson_t child;
    bson_append_document_begin(bson, "foo", -1, &child);
    bson_append_utf8(&child, "bar", -1, "baz", -1);
    bson_append_document_end(bson, &child);

    bcon_t * bcon = BCON(
        "foo", BCON_DOC(
            "bar", "baz",
        ),
    );

    bcon_eq_bson(bcon, bson);
}
END_TEST

START_TEST(test_bcon_array)
{
    bson_t * bson = bson_new();
    bson_t child;
    bson_append_array_begin(bson, "foo", -1, &child);
    bson_append_utf8(&child, "0", -1, "bar", -1);
    bson_append_utf8(&child, "1", -1, "baz", -1);
    bson_append_array_end(bson, &child);

    bcon_t * bcon = BCON(
        "foo", BCON_ARRAY( "bar", "baz" ),
    );

    bcon_eq_bson(bcon, bson);
}
END_TEST

START_TEST(test_binary)
{
    bson_t * bson = bson_new();
    bson_append_binary(bson, "foo", -1, BSON_SUBTYPE_BINARY, (bson_uint8_t *)"deadbeef", 8);

    bcon_t * bcon = BCON(
        "foo", BCON_BINARY(BSON_SUBTYPE_BINARY, "deadbeef", 8),
    );

    bcon_eq_bson(bcon, bson);
}
END_TEST

START_TEST(test_undefined)
{
    bson_t * bson = bson_new();
    bson_append_undefined(bson, "foo", -1);

    bcon_t * bcon = BCON(
        "foo", BCON_UNDEFINED,
    );

    bcon_eq_bson(bcon, bson);
}
END_TEST

START_TEST(test_oid)
{
    bson_t * bson = bson_new();
    bson_oid_t oid;
    bson_oid_init(&oid, NULL);

    bson_append_oid(bson, "foo", -1, &oid);

    bcon_t * bcon = BCON(
        "foo", BCON_BSON_OID(&oid),
    );

    bcon_eq_bson(bcon, bson);
}
END_TEST

START_TEST(test_bool)
{
    bson_t * bson = bson_new();

    bson_append_bool(bson, "foo", -1, 1);

    bcon_t * bcon = BCON(
        "foo", BCON_BOOL(1),
    );

    bcon_eq_bson(bcon, bson);

    bson = bson_new();

    bson_append_bool(bson, "foo", -1, 0);

    bcon = BCON(
        "foo", BCON_BOOL(0),
    );

    bcon_eq_bson(bcon, bson);
}
END_TEST

START_TEST(test_date_time)
{
    bson_t * bson = bson_new();

    struct timeval tv = { .tv_sec = 1231111, .tv_usec = 12311 };

    bson_append_timeval(bson, "foo", -1, &tv);

    bcon_t * bcon = BCON(
        "foo", BCON_DATE_TIME(&tv),
    );

    bcon_eq_bson(bcon, bson);
}
END_TEST

START_TEST(test_null)
{
    bson_t * bson = bson_new();

    bson_append_null(bson, "foo", -1);

    bcon_t * bcon = BCON(
        "foo", BCON_NULL,
    );

    bcon_eq_bson(bcon, bson);
}
END_TEST

START_TEST(test_regex)
{
    bson_t * bson = bson_new();

    bson_append_regex(bson, "foo", -1, "^foo|bar$", "i");

    bcon_t * bcon = BCON(
        "foo", BCON_REGEX("^foo|bar$", "i"),
    );

    bcon_eq_bson(bcon, bson);
}
END_TEST

START_TEST(test_dbpointer)
{
    bson_t * bson = bson_new();

    bson_oid_t oid;
    bson_oid_init(&oid, NULL);

    bson_append_dbpointer(bson, "foo", -1, "collection", &oid);

    bcon_t * bcon = BCON(
        "foo", BCON_DBPOINTER("collection", &oid),
    );

    bcon_eq_bson(bcon, bson);
}
END_TEST

START_TEST(test_code)
{
    bson_t * bson = bson_new();

    bson_append_code(bson, "foo", -1, "print 10;");

    bcon_t * bcon = BCON(
        "foo", BCON_CODE("print 10;"),
    );

    bcon_eq_bson(bcon, bson);
}
END_TEST

START_TEST(test_symbol)
{
    bson_t * bson = bson_new();

    bson_append_symbol(bson, "foo", -1, "deadbeef", -1);

    bcon_t * bcon = BCON(
        "foo", BCON_SYMBOL("deadbeef"),
    );

    bcon_eq_bson(bcon, bson);
}
END_TEST

START_TEST(test_code_with_scope)
{
    bson_t * bson = bson_new();
    bson_t * scope = bson_new();
    bson_append_double(scope, "x", -1, 10);

    bson_append_code_with_scope(bson, "foo", -1, "print x;", scope);

    bcon_t * bcon = BCON(
        "foo", BCON_CODEWSCOPE("print x;", "x", BCON_DOUBLE(10)),
    );

    bcon_eq_bson(bcon, bson);

    bson_destroy(scope);
}
END_TEST

START_TEST(test_int32)
{
    bson_t * bson = bson_new();
    bson_append_int32(bson, "foo", -1, 100);

    bcon_t * bcon = BCON(
        "foo", BCON_INT32(100),
    );

    bcon_eq_bson(bcon, bson);
}
END_TEST

START_TEST(test_timestamp)
{
    bson_t * bson = bson_new();

    bson_append_timestamp(bson, "foo", -1, 100, 1000);

    bcon_t * bcon = BCON(
        "foo", BCON_TIMESTAMP(100, 1000),
    );

    bcon_eq_bson(bcon, bson);
}
END_TEST

START_TEST(test_int64)
{
    bson_t * bson = bson_new();
    bson_append_int64(bson, "foo", -1, 100);

    bcon_t * bcon = BCON(
        "foo", BCON_INT64(100),
    );

    bcon_eq_bson(bcon, bson);
}
END_TEST

START_TEST(test_maxkey)
{
    bson_t * bson = bson_new();
    bson_append_maxkey(bson, "foo", -1);

    bcon_t * bcon = BCON(
        "foo", BCON_MAXKEY,
    );

    bcon_eq_bson(bcon, bson);
}
END_TEST

START_TEST(test_minkey)
{
    bson_t * bson = bson_new();
    bson_append_minkey(bson, "foo", -1);

    bcon_t * bcon = BCON(
        "foo", BCON_MINKEY,
    );

    bcon_eq_bson(bcon, bson);
}
END_TEST

START_TEST(test_bson_document)
{
    bson_t * bson = bson_new();
    bson_t * child = bson_new();
    bson_append_utf8(child, "bar", -1, "baz", -1);
    bson_append_document(bson, "foo", -1, child);

    bcon_t * bcon = BCON(
        "foo", BCON_BSON_DOCUMENT(child),
    );

    bcon_eq_bson(bcon, bson);

    bson_destroy(child);
}
END_TEST

START_TEST(test_bson_array)
{
    bson_t * bson = bson_new();
    bson_t * child = bson_new();
    bson_append_utf8(child, "0", -1, "baz", -1);
    bson_append_array(bson, "foo", -1, child);

    bcon_t * bcon = BCON(
        "foo", BCON_BSON_ARRAY(child),
    );

    bcon_eq_bson(bcon, bson);

    bson_destroy(child);
}
END_TEST

void
add_basic_tests(Suite * s)
{
    TCase * core = tcase_create("Basic");
    tcase_add_test(core, test_utf8);
    tcase_add_test(core, test_double);
    tcase_add_test(core, test_bcon_document);
    tcase_add_test(core, test_bcon_array);
    tcase_add_test(core, test_binary);
    tcase_add_test(core, test_undefined);
    tcase_add_test(core, test_oid);
    tcase_add_test(core, test_bool);
    tcase_add_test(core, test_date_time);
    tcase_add_test(core, test_null);
    tcase_add_test(core, test_regex);
    tcase_add_test(core, test_dbpointer);
    tcase_add_test(core, test_code);
    tcase_add_test(core, test_symbol);
    tcase_add_test(core, test_code_with_scope);
    tcase_add_test(core, test_int32);
    tcase_add_test(core, test_timestamp);
    tcase_add_test(core, test_int64);
    tcase_add_test(core, test_maxkey);
    tcase_add_test(core, test_minkey);
    tcase_add_test(core, test_bson_document);
    tcase_add_test(core, test_bson_array);
    suite_add_tcase(s, core);

    return;
}

int main(int argc, char ** argv)
{
    int num_failed;

    Suite * s = suite_create("BconTest");

    add_basic_tests(s);

    SRunner * sr = srunner_create(s);
    srunner_run_all(sr, CK_NORMAL);
    num_failed = srunner_ntests_failed(sr);
    srunner_free(sr);

    return (num_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
