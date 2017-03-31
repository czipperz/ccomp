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
#include "diagnostics.h"
#include "position.h"

void destroy_token(token* token) {
    assert(token);

    free(token->string.str);
}
void destroy_vec_token(vec_token* vtoken) {
    assert(vtoken);

    while (vtoken->num != 0) {
        --vtoken->num;
        destroy_token(vtoken->ptr + vtoken->num);
    }
    vtoken->cap = 0;
    free(vtoken->ptr);
}
void destroy_vec_vec_token(vec_vec_token* vvtoken) {
    assert(vvtoken);

    while (vvtoken->num != 0) {
        --vvtoken->num;
        destroy_vec_token(vvtoken->ptr + vvtoken->num);
    }
    vvtoken->cap = 0;
    free(vvtoken->ptr);
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
                          raw_position* rpos, vec_tagged_str* end) {
    assert(token);
    assert(fpos);
    rpos_bounds_check_end(rpos, end);

    while (isalnum(rpos_c(rpos)) || rpos_c(rpos) == '_') {
        char c = rpos_c(rpos);
        if (vec_push(&token->string, sizeof(char), &c)) {
            PRINT_MALLOC_ERROR();
            return 1;
        }
        if (forward_char(fpos, rpos, end)) {
            return 1;
        }
    }

    return 0;
}

int tokenize(vec_vec_token *vvtoken, const char* fname,
             vec_tagged_str* vvstr, vec_tagged_str* end) {
    file_position fpos;
    raw_position rpos;
    vec_token* vtoken;

    assert(vvtoken);
    assert(fname);
    assert(vvstr);
    assert(end);

    vtoken = vvtoken->ptr;
    assert(vtoken);

    fpos.file_name = fname;
    fpos.y = 0;
    fpos.x = 0;
    rpos.list = vvstr;
    rpos.buffer = 0;
    rpos.index = 0;

    if (vec_reserve(vvtoken, sizeof(vec_token), 1)) {
        return 1;
    }

    while (1) {
        token token;
        char current, next;
        while (1) {
            current = rpos_c(&rpos);
            if (!isspace(current)) {
                break;
            }
            if (forward_char(&fpos, &rpos, end)) {
                goto ret;
            }
        }
        if (isalpha(current) || current == '_') {
            memset(&token.string, 0, sizeof(token.string));
            if (str_push(&token.string, current)) {
                PRINT_MALLOC_ERROR();
                goto fail;
            }
            if (!forward_char(&fpos, &rpos, end)) {
                if (parse_var_name(&token, &fpos, &rpos, end)) {
                    free(token.string.str);
                    goto fail;
                }
            }
            goto add_it;
        }
        if (forward_char(&fpos, &rpos, end)) {
            if (rpos_c(&rpos) == ';') {
                token.tag = token_semicolon;
            } else if (rpos_c(&rpos) == '}') {
                token.tag = token_close_curly;
            } else {
                goto fail;
            }
            memset(&token.string, 0, sizeof(token.string));
            continue;
        }
        next = rpos_c(&rpos);
        if (isspace(next)) {
            if (single_parse(current, &token)) {
                goto add_it;
            } else {
                goto fail;
            }
        } else {
            assert(0);
        }

add_it:
        if (vec_push(vtoken, sizeof(token), &token)) {
            PRINT_MALLOC_ERROR();
            goto fail;
        }
        continue;
    }

ret:
    return 0;

fail:
    destroy_vec_vec_token(vvtoken);
    return 1;
}
