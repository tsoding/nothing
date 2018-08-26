#ifndef TOKENIZER_H_
#define TOKENIZER_H_

struct Token
{
    const char *begin;
    const char *end;
};

inline struct Token token(const char *begin, const char *end)
{
    struct Token token = {
        .begin = begin,
        .end = end
    };

    return token;
}

struct Token next_token(const char *str);

#endif  // TOKENIZER_H_
