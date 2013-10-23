/*
 * @file bcon.h
 * @brief BCON (BSON C Object Notation) Declarations
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

#ifndef BCON_H_
#define BCON_H_

#include <bson.h>

#include "bcon_pp.h"

#define BCON_ADD_BRACKETS(v) { v }
#define BCON(...) ((bcon_t []){ BCON_MACRO_MAP( BCON_ADD_BRACKETS, (,), __VA_ARGS__, 0 ) })
#include "bcon_sub_symbols.h"
#define BCON_DOC(...) BCON_BCON_DOCUMENT(BCON( __VA_ARGS__ ))
#define BCON_ARRAY(...) BCON_BCON_ARRAY(BCON( __VA_ARGS__ ))
#define BCON_BINARY(subtype, binary, length) BCON_BIN(((bcon_binary_t []){{subtype, (uint8_t *)(binary), length}}))
#define BCON_CODEWSCOPE(code, ...) BCON_BCON_CODEWSCOPE(((bcon_code_t []){{code, BCON( __VA_ARGS__ )}}))
#define BCON_CODE(code) BCON_BCON_CODEWSCOPE(((bcon_code_t []){{code, 0}}))

extern char * BCON_MAGIC;

typedef enum {
#include "bcon_enum.h"
    BCONT_END,
    BCONT_ERROR,
} bcon_type_t;

typedef struct bcon_binary {
    bson_subtype_t subtype;
    bson_uint8_t * binary;
    bson_uint32_t length;
} bcon_binary_t;

typedef struct bcon_regex {
    char * regex;
    char * flags;
} bcon_regex_t;

typedef struct bcon_code {
    char * code;
    union bcon * scope;
} bcon_code_t;

typedef struct bcon_dbpointer {
    char * collection;
    bson_oid_t * oid;
} bcon_dbpointer_t;

typedef struct bcon_timestamp {
    bson_uint32_t timestamp;
    bson_uint32_t increment;
} bcon_timestamp_t;

typedef union bcon {
#include "bcon_union.h"

    bcon_type_t type;
} bcon_t;

char * bcon_dump(bcon_t * in);
char * bcon_to_bson(bcon_t * in, bson_t * bson);
void bcon_DUMP(bcon_t * in);
void bcon_DUMP_AS_JSON(bcon_t * in);

#endif
