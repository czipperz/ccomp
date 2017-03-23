/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2017 Chris Gregory czipperz@gmail.com
 */

#include "syntax_tree.h"
#include "str.h"

void destroy_member_type(struct member_type* member_type) {
    free(member_type->name.str);
    destroy_type(&member_type->type);
}

void destroy_structure_type(struct_type* structure_type) {
    struct_member_type* member;
    free(structure_type->name.str);
    for (member = structure_type->members;
         member !=
             structure_type->members + structure_type->num_members;
         ++member) {
        destroy_member_type(member);
    }
    free(structure_type->members);
}

void destroy_expression(expression* expression) {
    switch (expression->expression_tag) {
    case not_tag:
    case bnot_tag:
        free_expression(expression->data.unary_expression.expr);
        break;
    case integral_value_tag:
        break;
    case undefined_value_tag:
        break;
    case variable_usage_tag:
        free(expression->data.variable_usage_expression.name.str);
        break;
    case cast_tag:
        destroy_type(&expression->data.cast_expression.type);
        free_expression(expression->data.cast_expression.value);
        break;
    default:
        free_expression(expression->data.binary_expression.left);
        free_expression(expression->data.binary_expression.right);
        break;
    }
}

void destroy_statement(statement* statement) {
    switch (statement->type) {
    case for_tag:
        destroy_expression(&statement->data.for_statement.init);
        destroy_expression(&statement->data.for_statement.test);
        destroy_expression(&statement->data.for_statement.loop);
        free_statement(statement->data.for_statement.body);
        break;
    case while_tag:
    case do_tag:
        destroy_expression(&statement->data.while_statement.test);
        free_statement(statement->data.while_statement.body);
        break;
    case goto_tag:
        free(statement->data.goto_statement.label.str);
        break;
    case if_tag:
        destroy_expression(&statement->data.if_statement.test);
        free_statement(statement->data.if_statement.body);
        if (statement->data.if_statement.else_statement) {
            free_statement(statement->data.if_statement.else_statement);
        }
        break;
    case return_tag:
        destroy_expression(&statement->data.return_statement.value);
        break;
    case variable_tag:
        destroy_type(&statement->data.variable_statement.type);
        free(statement->data.variable_statement.name.str);
        destroy_expression(&statement->data.variable_statement.value);
        break;
    case expression_tag:
        destroy_expression(&statement->data.expression_statement);
        break;
    }
}

void destroy_function(function* function) {
    statement* statement;
    for (statement = function->statements;
            statement !=
            function->statements + function->num_statements;
            ++statement) {
        destroy_statement(statement);
    }
    free(function->statements);
}

void destroy_syntax_tree(syntax_tree* syntax_tree) {
    function* function;
    for (function = syntax_tree->functions;
         function !=
         syntax_tree->functions + syntax_tree->num_functions;
         ++function) {
        destroy_function(function);
    }
    free(syntax_tree->functions);
}
