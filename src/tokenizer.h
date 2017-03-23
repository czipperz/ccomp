/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2017 Chris Gregory czipperz@gmail.com
 */

#ifndef HEADER_GUARD_TOKENIZER_H
#define HEADER_GUARD_TOKENIZER_H

#include "str.h"
#include "position.h"

typedef enum token_tag {
    token_increment, token_decrement, token_open_paren,
    token_close_paren, token_open_square, token_close_square,
    token_open_curly, token_close_curly,
    token_number, token_var_name, token_question_mark, token_colon,
    token_semicolon, token_comma, token_string, token_char,
    token_assign,
    token_bxor, token_bxor_inplace,
    token_bnot, token_bnot_inplace,
    token_not, token_not_inplace,
    token_bor, token_bor_inplace,
    token_band, token_band_inplace,
    token_or, token_or_inplace,
    token_and, token_and_inplace,
    token_right_shift, token_right_shift_inplace,
    token_left_shift, token_left_shift_inplace,
    token_plus, token_plus_inplace,
    token_minus, token_minus_inplace,
    token_mult, token_mult_inplace,
    token_div, token_div_inplace,
    token_mod, token_mod_inplace,
    token_equals, token_not_equals,
    token_greater, token_greater_equals,
    token_less, token_less_equals,

    /* Keywords */ token_return, token_goto, token_auto, token_break,
    token_struct, token_enum, token_union, token_const,
    token_volatile, token_default, token_if, token_else, token_extern,
    token_do, token_while, token_for, token_static, token_register,
    token_sizeof, token_typedef,token_case, token_switch,

    /* Types */ token_double, token_float, token_int, token_long,
    token_void, token_short, token_unsigned, token_signed
} token_tag;

typedef struct token {
    token_tag tag;
    /* Possibly NULL: */ str string;
} token;

typedef struct tokens {
    token* tokens;
    int num_tokens;
} tokens;

typedef struct tokens_vec {
    tokens* tokens;
    int num_tokens;
} tokens_vec;

void destroy_token(token*);
void destroy_tokens(tokens*);
void destroy_tokens_vec(tokens_vec*);

int tokenize(tokens_vec* tokens, const char* file_name,
             tagged_str_list* strs, tagged_str_list* num_strs);

#endif
