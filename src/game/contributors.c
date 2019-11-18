#include <stdio.h>
#include <stdlib.h>

#include "system/stacktrace.h"
#include "system/lt.h"
#include "system/lt_adapters.h"
#include "system/nth_alloc.h"
#include "system/line_stream.h"
#include "system/str.h"
#include "dynarray.h"

#include "./contributors.h"

#define CONTRIBUTORS_MAX_LENGTH 512

struct Contributors
{
    Lt *lt;
    Dynarray *names;
};

Contributors *create_contributors()
{
    Lt *lt = create_lt();

    Contributors *contributors = PUSH_LT(
        lt,
        nth_calloc(1, sizeof(Contributors)),
        free);
    return contributors;
}

void destroy_contributors(Contributors *contributors)
{
    RETURN_LT0(contributors->lt);
}

const char **contributors_names(const Contributors *contributors)
{
    return dynarray_data(contributors->names);
}

size_t contributors_count(const Contributors *contributors)
{
    return dynarray_count(contributors->names);
}
