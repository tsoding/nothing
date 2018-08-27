#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "test.h"
#include "script/tokenizer.h"

TEST(tokenizer_number_list_test)
{
    struct Token token = next_token("(1 2 3)");
    ASSERT_STREQN("(", token.begin, (size_t) (token.end - token.begin));

    token = next_token(token.end);
    ASSERT_STREQN("1", token.begin, (size_t) (token.end - token.begin));

    token = next_token(token.end);
    ASSERT_STREQN("2", token.begin, (size_t) (token.end - token.begin));

    token = next_token(token.end);
    ASSERT_STREQN("3", token.begin, (size_t) (token.end - token.begin));

    token = next_token(token.end);
    ASSERT_STREQN(")", token.begin, (size_t) (token.end - token.begin));

    return 0;
}

TEST(tokenizer_string_list_test)
{
    struct Token token = next_token("(\"foo\" \"bar\" \"baz\")");
    ASSERT_STREQN("(", token.begin, (size_t) (token.end - token.begin));

    token = next_token(token.end);
    ASSERT_STREQN("\"foo\"", token.begin, (size_t) (token.end - token.begin));

    token = next_token(token.end);
    ASSERT_STREQN("\"bar\"", token.begin, (size_t) (token.end - token.begin));

    token = next_token(token.end);
    ASSERT_STREQN("\"baz\"", token.begin, (size_t) (token.end - token.begin));

    token = next_token(token.end);
    ASSERT_STREQN(")", token.begin, (size_t) (token.end - token.begin));

    return 0;
}

TEST_SUITE(tokenizer_suite)
{
    TEST_RUN(tokenizer_number_list_test);
    TEST_RUN(tokenizer_string_list_test);
    return 0;
}

TEST_MAIN()
{
    TEST_RUN(tokenizer_suite);
    return 0;
}
