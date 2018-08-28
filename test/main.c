#include "test.h"
#include "tokenizer_suite.h"
#include "parser_suite.h"

TEST_MAIN()
{
    TEST_RUN(tokenizer_suite);
    TEST_RUN(parser_suite);
    return 0;
}
