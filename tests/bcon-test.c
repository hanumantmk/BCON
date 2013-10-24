/*
 * @file bcon_test.c
 * @brief BCON (BSON C Object Notation) Test library
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

#include "bcon-test.h"

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

int main(int argc, char ** argv)
{
    int num_failed;

    Suite * s = suite_create("BconTest");

    add_tests(s);

    SRunner * sr = srunner_create(s);
    srunner_run_all(sr, CK_NORMAL);
    num_failed = srunner_ntests_failed(sr);
    srunner_free(sr);

    return (num_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
