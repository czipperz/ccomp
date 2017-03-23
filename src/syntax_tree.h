/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2017 Chris Gregory czipperz@gmail.com
 */

#ifndef HEADER_GUARD_SYNTAX_TREE_H
#define HEADER_GUARD_SYNTAX_TREE_H

#include <stdlib.h>

#include "str.h"

typedef struct type type;

typedef struct integral_type {
    enum {
        int_type, long_type, long_long_type, short_type,
        char_type, float_type, double_type, long_double_type
    } basic_type;
    enum {
        type_unsigned_flag = 1, type_volatile_flag = 2,
        type_const_flag = 4
    } flags;
} integral_type;

struct type {
    union {
        integral_type integral_type;
        /* NOT owning: */ struct struct_type* structure_type;
        /* NOT owning: */ struct union_type* union_type;
        /* NOT owning: */ type* typedef_type;
    } data;
    enum {
        char_type_tag, integral_type_tag, structure_type_tag,
        union_type_tag, typedef_type_tag, void_tag
    } type_tag;
    int indirection;
};

typedef struct expression {
    union {
        struct {
            struct expression* left;
            struct expression* right;
        } binary_expression;
        struct {
            struct expression* expr;
        } unary_expression;
        struct {
            integral_type type;
#warning Need to add representation of number
        } integral_expression;
        struct {
            str name;
        } variable_usage_expression;
        struct {
            type type;
            struct expression* value;
        } cast_expression;
    } data;
    enum {
        cmp_eq_tag, cmp_less_tag, cmp_less_eq_tag,
        cmp_greater_tag, cmp_greater_eq_tag, not_tag,
        plus_tag, plus_inplace_tag, minus_tag, minus_inplace_tag,
        mult_tag, mult_inplace_tag, div_tag, div_inplace_tag,
        left_shift_tag, left_shift_inplace_tag,
        right_shift_tag, right_shift_inplace_tag,
        and_tag, or_tag, band_tag, bor_tag, bxor_tag, bnot_tag,
        variable_usage_tag, integral_value_tag, undefined_value_tag,
        cast_tag
    } expression_tag;
} expression;

typedef struct statement {
    union {
        struct {
            expression init, test, loop;
            struct statement* body;
        } for_statement;
        struct {
            expression test;
            struct statement* body;
        } while_statement;
        struct {
            str label;
        } goto_statement;
        struct {
            expression test;
            struct statement* body;
            /* Possibly NULL: */ struct statement* else_statement;
        } if_statement;
        struct {
            expression value;
        } return_statement;
        struct {
            type type;
            str name;
            expression value;
        } variable_statement;
        expression expression_statement;
    } data;
    enum {
        if_tag, for_tag, do_tag, while_tag, goto_tag,
        return_tag, variable_tag, expression_tag
    } type;
} statement;

typedef struct function {
    statement* statements;
    int num_statements;
} function;

typedef struct member_type {
    str name;
    type type;
} struct_member_type, union_member_type;

typedef struct {
    /* Possibly NULL: */ str name;
    struct member_type* members;
    int num_members;
} struct_type, union_type;

typedef struct syntax_tree {
    function* functions;
    int num_functions;
} syntax_tree;

void destroy_member_type(struct member_type*);
void destroy_structure_type(struct_type*);
static void destroy_union_type(union_type* type) {
    destroy_structure_type(type);
}
static void destroy_type(type* type) {}
void destroy_expression(expression*);
void destroy_statement(statement*);
void destroy_function(function*);
void destroy_syntax_tree(syntax_tree*);

static void free_type(type* type) {
    destroy_type(type);
    free(type);
}
static void free_expression(expression* expression) {
    destroy_expression(expression);
    free(expression);
}
static void free_statement(statement* statement) {
    destroy_statement(statement);
    free(statement);
}
static void free_function(function* function) {
    destroy_function(function);
    free(function);
}
static void free_syntax_tree(syntax_tree* syntax_tree) {
    destroy_syntax_tree(syntax_tree);
    free(syntax_tree);
}

#endif
