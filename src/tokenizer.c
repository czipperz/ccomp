/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2017 Chris Gregory czipperz@gmail.com
 */

#include "tokenizer.h"

#include <assert.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "alloc.h"
#include "position.h"

void destroy_token(token* token) {
    assert(token);

    free(token->string.str);
}
void destroy_tokens(tokens* tokens) {
    int i;

    assert(tokens);

    for (i = 0; i != tokens->num_tokens; ++i) {
        destroy_token(tokens->tokens + i);
    }
    free(tokens->tokens);
}
void destroy_tokens_vec(tokens_vec* tokens_vec) {
    int i;

    assert(tokens_vec);

    for (i = 0; i != tokens_vec->num_tokens; ++i) {
        destroy_tokens(tokens_vec->tokens);
    }
    free(tokens_vec->tokens);
}

static int single_parse(char input, token* token) {
    const char* const singles_s = "(){}[]?:;=,";
    const token_tag singles_t[] =
        {token_open_paren,    token_close_paren, token_open_curly,
         token_open_curly,    token_open_square, token_close_square,
         token_question_mark, token_colon,       token_semicolon,
         token_assign,        token_comma};
    const char* const maybe_s = "&^|<>+-*/!~";
    const token_tag maybe_t[] = {token_band,    token_bxor,
                                 token_bor,     token_less,
                                 token_greater, token_plus,
                                 token_minus,   token_mult,
                                 token_div,     token_not,
                                 token_bnot};
    int i;

    assert(token);

    for (i = 0; i != sizeof(singles_t) / sizeof(*singles_t); ++i) {
        if (singles_s[i] == input) {
            token->tag = singles_t[i];
            memset(&token->string, 0, sizeof(token->string));
            return 1;
        }
    }
    for (i = 0; i != sizeof(maybe_t) / sizeof(*maybe_t);
         ++i) {
        if (maybe_s[i] == input) {
            token->tag = maybe_t[i];
            memset(&token->string, 0, sizeof(token->string));
            return 2;
        }
    }
    return 0;
}

static int parse_var_name(token* token, file_position* fpos,
                          raw_position* rpos,
                          tagged_str_list* strings_end) {
    assert(token);
    assert(fpos);
    assert(rpos);
    assert(strings_end);

    while (isalnum(rpos->buffer->str[rpos->index])) {
        REALLOC(token->string.str, token->string.len + 1, goto fail);
        ++token->string.len;
        token->string.str[token->string.len - 1] =
            rpos->buffer->str[rpos->index];
        forward_char(fpos, rpos, strings_end);
    }

    return 0;
fail:
    return 1;
}

int tokenize(tokens_vec *tokens_v, const char* fname,
             tagged_str_list* strs, tagged_str_list* strs_end) {
    file_position fpos;
    raw_position rpos;
    tokens* tokens = tokens_v->tokens;

    assert(tokens_v);
    assert(fname);
    assert(strs);
    assert(strs_end);

    fpos.file_name = fname;
    fpos.y = 0;
    fpos.x = 0;
    rpos.list = strs;
    rpos.buffer = strs->strs;
    rpos.index = 0;

    CALLOC(tokens_v->tokens, 1, sizeof(*tokens_v->tokens), return 1);
    tokens_v->num_tokens = 1;

    while (1) {
        token token;
        char current, next;
        while (1) {
            current = rpos.buffer->str[rpos.index];
            if (!isspace(current)) {
                break;
            }
            if (forward_char(&fpos, &rpos, strs_end)) {
                goto ret;
            }
        }
        if (isalpha(current) || current == '_') {
            if (forward_char(&fpos, &rpos, strs_end)) {
                return 1;
            }
            MALLOC(token.string.str, 1, goto fail);
            token.string.str[0] = current;
            token.string.len = 1;
            if (parse_var_name(&token, &fpos, &rpos, strs_end)) {
                free(token.string.str);
                goto fail;
            }
            goto add_it;
        }
        if (forward_char(&fpos, &rpos, strs_end)) {
            if (rpos.buffer->str[rpos.index] == ';') {
                token.tag = token_semicolon;
            } else if (rpos.buffer->str[rpos.index] == '}') {
                token.tag = token_close_curly;
            } else {
                goto fail;
            }
            memset(&token.string, 0, sizeof(token.string));
            continue;
        }
        next = rpos.buffer->str[rpos.index];
        if (isspace(next)) {
            if (single_parse(current, &token)) {
                goto add_it;
            } else {
                goto fail;
            }
        } else {
            fputs("ADD CASES", stderr);
            abort();
        }

add_it:
        REALLOC(tokens_v->tokens, sizeof(token) * tokens->num_tokens,
                { assert(0); });
        ++tokens_v->num_tokens;
        tokens->tokens[tokens->num_tokens - 1] = token;
        continue;
    }

ret:
    return 0;

fail:
    destroy_tokens_vec(tokens_v);
    return 1;
}
