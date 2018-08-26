#ifndef TOKENIZER_H_
#define TOKENIZER_H_

struct Token
{
    const char *begin;
    const char *end;
};

struct Token next_token(const char *str);

#endif  // TOKENIZER_H_
