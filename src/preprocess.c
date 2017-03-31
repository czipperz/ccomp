/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2017 Chris Gregory czipperz@gmail.com
 */

#include "preprocess.h"

#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "alloc.h"
#include "str.h"
#include "position.h"
#include "diagnostics.h"

typedef struct definition {
    str name;
    /* Possibly NULL: */ str definition;
} definition;
typedef struct vec_definition {
    definition* ptr;
    size_t len;
    size_t cap;
} vec_definition;
typedef struct header_guard {
    str fname;
    /* Possibly NULL (#pragma once): */ str macro;
} header_guard;
typedef struct vec_header_guard {
    header_guard* ptr;
    size_t len;
    size_t cap;
} vec_header_guard;

typedef enum directive {
    define_directive = 1, undef_directive,
    include_directive,
    if_directive, ifdef_directive, ifndef_directive,
    else_directive, elif_directive,
    endif_directive,
    line_directive, error_directive, pragma_directive
} directive;

/* Process instruction at point:
   #|define -> #define| , return define_directive */
static int
process_instruction(file_position *fpos, raw_position *rpos,
                    vec_tagged_str* buffers_end, directive *directive) {
    int ret = 0;
    char temp[8];
    int temp_index = 0;
    file_position bfpos = *fpos;
    raw_position brpos = *rpos;
    while (1) {
        if (forward_char(&bfpos, &brpos, buffers_end) ||
            !isalnum(rpos_c(&brpos)) ||
            temp_index == 7) {
            temp[temp_index] = 0;
            if (strcmp(temp, "define") == 0) {
                *directive = define_directive;
            } else if (strcmp(temp, "undef") == 0) {
                *directive = undef_directive;
            } else if (strcmp(temp, "include") == 0) {
                *directive = include_directive;
            } else if (strcmp(temp, "if") == 0) {
                *directive = if_directive;
            } else if (strcmp(temp, "ifdef") == 0) {
                *directive = ifdef_directive;
            } else if (strcmp(temp, "ifndef") == 0) {
                *directive = ifndef_directive;
            } else if (strcmp(temp, "else") == 0) {
                *directive = else_directive;
            } else if (strcmp(temp, "elif") == 0) {
                *directive = elif_directive;
            } else if (strcmp(temp, "endif") == 0) {
                *directive = endif_directive;
            } else if (strcmp(temp, "line") == 0) {
                *directive = line_directive;
            } else if (strcmp(temp, "error") == 0) {
                *directive = error_directive;
            } else if (strcmp(temp, "pragma") == 0) {
                *directive = pragma_directive;
            } else {
                print_error(&bfpos,
                            "Invalid preprocessing directive %s",
                            temp);
                ret = 1;
                goto ret;
            }
            break;
        } else {
            temp[temp_index] = rpos_c(&brpos);
            ++temp_index;
        }
    }
    *fpos = bfpos;
    *rpos = brpos;
ret:
    return ret;
}

static int delete_block_comments(vec_vec_tagged_str *buffers) {
    file_position fpos;
    raw_position rpos;

    /* Go back to beginning of file */
    rpos.list = buffers->ptr;
    rpos.buffer = 0;
    rpos.index = 0;
    fpos = rpos_str(&rpos)->fpos;

    do {
        file_position bfpos;
        raw_position brpos;

        /* pos is before the potential `/` and `*` */
        bfpos = fpos;
        brpos = rpos;

top:
        /* Goto the next character so we can check / and * at once. */
        if (forward_char(&bfpos, &brpos, buffers->ptr + buffers->len)) {
            return 0;
        }

        rpos_bounds_check_end(&rpos, buffers->ptr + buffers->len);
        rpos_bounds_check_end(&brpos, buffers->ptr + buffers->len);
        if (rpos_c(&rpos) != '/' || rpos_c(&brpos) != '*') {
            /* If we aren't actually at a comment, repeat. */
            fpos = bfpos;
            rpos = brpos;
            goto top;
        }

        /* Put bpos after the / and * */
        if (forward_char(&bfpos, &brpos, buffers->ptr + buffers->len)) {
            goto unterminated_c;
        }

        print_warning(&fpos, "Position of |/*");

        /* Now look for `*` and `/` */
        while (1) {
            rpos_bounds_check(&brpos);
            if (rpos_c(&brpos) != '*') {
                if (forward_char(&bfpos, &brpos, buffers->ptr + buffers->len)) {
                    goto unterminated_c;
                }
                continue;
            }

            if (forward_char(&bfpos, &brpos, buffers->ptr + buffers->len)) {
                goto unterminated_c;
            }
            rpos_bounds_check(&brpos);
            if (rpos_c(&brpos) != '/') {
                continue;
            }

            print_warning(&bfpos, "Position of *|/");

            /* Go forward one to get brpos after `*` and `/` */
            if (forward_char(&bfpos, &brpos, buffers->ptr + buffers->len)) {
                /* We are at end of file so delete after rpos */
                str_set_len(&rpos_str(&rpos)->str, rpos.index);

                /* Now delete between them. */
                while (1) {
                    ++rpos.buffer;
                    if (rpos.buffer == rpos.list->len) {
                        ++rpos.list;
                        if (rpos.list == buffers->ptr + buffers->len) {
                            break;
                        }
                        rpos.buffer = 0;
                    }
                    free(rpos_str(&rpos));
                }
                assert(rpos.list == buffers->ptr + buffers->len);
                return 0;
            } else if (rpos.buffer != brpos.buffer) {
                /* Delete end of rpos's str */
                str_set_len(&rpos_str(&rpos)->str, rpos.index);

                /* Delete start of the last str */
                {
                    str* b = &rpos_str(&brpos)->str;
                    b->len -= brpos.index;
                    memmove(b->str, b->str + brpos.index,
                            b->len * sizeof(char));
                }

                rpos.index = 0;

                /* Delete between rpos's and brpos's initial strs */
                ++rpos.buffer;
                if (rpos.list + 1 < brpos.list) {
                    /* Delete lists between them */
                    {
                        vec_tagged_str* itr = rpos.list + 1;
                        while (itr < brpos.list) {
                            destroy_vec_tagged_str(itr);
                            ++itr;
                        }
                    }

                    /* Move backwards and subtract from length. */
                    buffers->len -= brpos.list - buffers->ptr;
                    memmove(rpos.list + 1, brpos.list,
                            sizeof(vec_tagged_str) * buffers->len);

                    /* Delete strs after rpos in its list */
                    while (rpos.buffer < --rpos.list->len) {
                        free(rpos.list->ptr[rpos.list->len].str.str);
                    }

                    /* Delete strs before brpos in its list */
                    {
                        size_t i;
                        for (i = 0; i != brpos.buffer; ++i) {
                            free(brpos.list->ptr[i].str.str);
                        }
                    }
                    brpos.list->len -= brpos.buffer;
                    memmove(brpos.list->ptr,
                            brpos.list->ptr + brpos.buffer,
                            sizeof(tagged_str) * brpos.list->len);
                    brpos.buffer = 0;
                } else {
                    /* Delete strs between rpos and brpos in the lists */
                    size_t i;
                    assert(rpos.buffer < brpos.buffer);
                    for (i = rpos.buffer + 1; i != brpos.buffer;
                         ++i) {
                        free(brpos.list->ptr[i].str.str);
                    }
                    brpos.list->len -= brpos.buffer - rpos.buffer - 1;
                    memmove(rpos_str(&rpos) + 1, rpos_str(&brpos),
                            sizeof(tagged_str) *
                                (brpos.list->len - brpos.buffer));
                    brpos.buffer = rpos.buffer + 1;
                }

                /* Finally, go to bpos, past the end of the comment */
                rpos = brpos;
                fpos = bfpos;

                /* Assert post conditions: we are at start of the
                 * buffer the comment end used to be in. */
                assert(rpos.list == brpos.list);
                assert(rpos.list != buffers->ptr + buffers->len);
                assert(rpos.buffer == brpos.buffer);
                assert(rpos.index == 0);
            } else {
                /* Here the comment start and end are in the same
                 * string.  We have to split the string in two. */

                /* Make the new string */
                tagged_str str;
                str.str.len = str.str.cap = brpos.list->len - brpos.index;
                str.str.str = malloc(sizeof(char) * (str.str.cap + 1));
                if (!str.str.str) {
                    PRINT_MALLOC_ERROR();
                    return 1;
                }
                memcpy(str.str.str,
                       rpos_str(&brpos)->str.str,
                       rpos_str(&brpos)->str.len + 1 - brpos.index);

                str.fpos = rpos_str(&rpos)->fpos;

                if (vec_insert(rpos.list, sizeof(tagged_str), rpos.buffer + 1, &str)) {
                    free(str.str.str);
                    PRINT_MALLOC_ERROR();
                    return 1;
                }
                ++rpos.buffer;
            }
            break;
        }

    } while (!forward_char(&fpos, &rpos, buffers->ptr + buffers->len));

    return 0;

unterminated_c:
    print_error(&fpos, "Unterminated comment.");
    return 1;
}

static int evaluate_preprocessor_commands(vec_vec_tagged_str *buffers) {
    file_position fpos;
    raw_position rpos;
    char previous = '\n';

    /* Go back to beginning of file */
    rpos.list = buffers->ptr;
    rpos.buffer = 0;
    rpos.index = 0;
    fpos = rpos_str(&rpos)->fpos;
    do {
        /* Skip whitespace at beginning of line */
        if (forward_through_blanks(&fpos, &rpos, buffers->ptr + buffers->len)) {
            break;
        }
        rpos_bounds_check(&rpos);
        /* Recognize preprocessor commands */
        if (rpos_c(&rpos) == '#' && previous == '\n') {
            directive directive;
            /* Skip whitespace after the # */
            if (forward_through_blanks(&fpos, &rpos,
                                       buffers->ptr + buffers->len)) {
                break;
            }
            if (!process_instruction(&fpos, &rpos,
                                     buffers->ptr + buffers->len,
                                     &directive)) {
                printf("Read directive %d\n", directive);
            }
        }
        rpos_bounds_check(&rpos);
        previous = rpos_c(&rpos);
    } while (!forward_char(&fpos, &rpos, buffers->ptr + buffers->len));

    return 0;
}

static int preprocess_(vec_vec_tagged_str *buffers,
                       vec_definition *definitions,
                       vec_header_guard *header_guards) {
    if (delete_block_comments(buffers)) {
        return 1;
    }

    if (evaluate_preprocessor_commands(buffers)) {
        return 1;
    }

    return 0;
}

int preprocess(vec_vec_tagged_str *buffers) {
    vec_definition definitions = VEC_INIT;
    vec_header_guard header_guards = VEC_INIT;

    int ret;

    assert(buffers);

    ret = preprocess_(buffers, &definitions, &header_guards);

    while (definitions.len != 0) {
        --definitions.len;
        free(definitions.ptr[definitions.len].name.str);
        free(definitions.ptr[definitions.len].definition.str);
    }
    free(definitions.ptr);

    while (header_guards.len != 0) {
        --header_guards.len;
        free(header_guards.ptr[header_guards.len].fname.str);
        free(header_guards.ptr[header_guards.len].macro.str);
    }
    free(header_guards.ptr);

    return ret;
}
