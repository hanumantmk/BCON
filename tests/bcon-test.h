#ifndef BCON_TEST_H
#define BCON_TEST_H

#include <check.h>
#include "bcon.h"

void bcon_eq_bson(bcon_t * bcon, bson_t * expected);
extern void add_tests(Suite * s);

#endif
