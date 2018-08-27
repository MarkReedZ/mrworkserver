#pragma once

#include "Python.h"
#include <stdbool.h>

#include <stdint.h>
#include <stddef.h>
#include <assert.h>

typedef void * JSOBJ;
//typedef char bool;
//enum { false, true };

enum JsonTag {
    JSON_NUMBER = 0,
    JSON_STRING,
    JSON_ARRAY,
    JSON_OBJECT,
    JSON_TRUE,
    JSON_FALSE,
    JSON_NULL = 0xF
};
typedef enum JsonTag JsonTag;

void *jsonParse(char *str, char **endptr, size_t len); //, char **endptr);
#ifdef __AVX2__
void *jParse(char *s, char **endptr, size_t len);
#endif

