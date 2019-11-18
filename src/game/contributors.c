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

#define CONTRIBUTORS_NUMBER 22

static const char * const CONTRIBUTORS_NAMES[CONTRIBUTORS_NUMBER] = {
    "rexim",
    "RIscRIpt",
    "TheKnarf",
    "PaprikaX33",
    "abridgewater",
    "The-Renaissance",
    "rafaelffilho",
    "bepzi",
    "zhiayang",
    "valignatev",
    "travisstaloch",
    "voldyman",
    "pi-pi3",
    "kolumb",
    "engelju",
    "ear",
    "destroycomputers",
    "Nykseli",
    "NilsIrl",
    "Minichota",
    "Funkschy",
    "ForNeVeR"
};

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
    if (contributors == NULL) {
        RETURN_LT(lt, NULL);
    }
    contributors->lt = lt;

    contributors->names = PUSH_LT(
        lt,
        create_dynarray(sizeof(const char*)),
        destroy_dynarray);
    if (contributors->names == NULL) {
        RETURN_LT(lt, NULL);
    }

    char name_string[CONTRIBUTORS_MAX_LENGTH];

    for (int i = 0; i < CONTRIBUTORS_NUMBER; i++) {

        snprintf(name_string, CONTRIBUTORS_MAX_LENGTH, "%s", CONTRIBUTORS_NAMES[i]);
        const char *name = PUSH_LT(lt, string_duplicate(trim_endline(name_string), NULL), free);
        if (name == NULL) {
            RETURN_LT(lt, NULL);
        }

        dynarray_push(contributors->names, &name);
    }

    return contributors;
}

void destroy_contributors(Contributors *contributors)
{
    trace_assert(contributors);
    RETURN_LT0(contributors->lt);
}

const char **contributors_names(const Contributors *contributors)
{
    trace_assert(contributors);
    return dynarray_data(contributors->names);
}

size_t contributors_count(const Contributors *contributors)
{
    trace_assert(contributors);
    return dynarray_count(contributors->names);
}
