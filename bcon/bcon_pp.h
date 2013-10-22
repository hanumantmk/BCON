#include "bcon_gen_macros.h"

#define BCON_MACRO_MAP(m, sep,...) BCON_MACRO_MAP_(BCON_MACRO_COUNT(__VA_ARGS__), m, sep, __VA_ARGS__)
#define BCON_MACRO_MAP_(...) BCON_MACRO_MAP__(__VA_ARGS__)
#define BCON_MACRO_MAP__(N, m, sep, ...) BCON_MACRO_MAP_ ## N(m, sep, __VA_ARGS__)
#define BCON_MACRO_CONCAT(...) __VA_ARGS__
#define BCON_MACRO_MAP_1(m, sep, v) m(v)
#define BCON_MACRO_MAP_0(...)
