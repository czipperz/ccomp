/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2017 Chris Gregory czipperz@gmail.com
 */

#include "preprocess.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

#include "alloc.h"
#include "str.h"
#include "position.h"
#include "diagnostics.h"

typedef struct definition {
    str name;
    /* Possibly NULL: */ str definition;
} definition;
typedef struct header_guard {
    str fname;
    /* Possibly NULL (#pragma once): */ str macro;
} header_guard;

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
                    tagged_str_list* buffers_end, directive *directive) {
    int ret = 0;
    char temp[8];
    int temp_index = 0;
    file_position bfpos = *fpos;
    raw_position brpos = *rpos;
    while (1) {
        if (forward_char(&bfpos, &brpos, buffers_end) ||
            !isalnum(brpos.buffer->str[brpos.index]) ||
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
            temp[temp_index] = brpos.buffer->str[brpos.index];
            ++temp_index;
        }
    }
    *fpos = bfpos;
    *rpos = brpos;
ret:
    return ret;
}

/* Delete "\\\n"s */
static int combine_lines(tagged_str_list* buffers,
                         tagged_str_list* buffers_end) {
    file_position fpos;
    raw_position rpos;

    /* Go back to beginning of file */
    rpos.list = buffers;
    rpos.buffer = buffers->strs;
    rpos.index = 0;
    fpos = rpos.buffer->fpos;

    do {
        assert(rpos.index < rpos.buffer->len);
        if (rpos.buffer->str[rpos.index] == '\\') {
            file_position bfpos = fpos;
            raw_position brpos = rpos;
            if (forward_char(&bfpos, &brpos, buffers_end)) {
                break;
            } else if (brpos.buffer->str[brpos.index] == '\n') {
                assert(brpos.index < brpos.buffer->len);

                if (forward_char(&bfpos, &brpos, buffers_end)) {
                    /* The end is at the end of the file.  Delete
                     * everything past rpos and stop. */
                    tagged_str_list* iterator;
                    for (iterator = rpos.list + 1;
                         iterator != buffers_end; ++iterator) {
                        destroy_tagged_str_list(iterator);
                    }

                    rpos.buffer->len = rpos.index;
                } else if (rpos.buffer == brpos.buffer) {
                    /* If these two buffers are equivalent, then we
                     * need to make a new string to hold the contents
                     * after the deletion.  This is required such that
                     * fpos stays valid so the next steps have correct
                     * diagnostics information. */
                    tagged_str* backup = rpos.list->strs;
                    tagged_str str;

                    /* Make the new string */
#warning str.fpos might be incorrect
                    str.fpos = fpos;
                    str.len = brpos.buffer->len - brpos.index;
                    MALLOC(str.str, str.len, { return 1; });
                    memcpy(str.str, brpos.buffer->str + brpos.index, str.len);

                    /* Reserve a slot for the extra string */
                    if (vector_reserve_another((void**) &rpos.list->strs,
                                               &rpos.list->len,
                                               &rpos.list->cap,
                                               sizeof(*rpos.list))) {
                        free(str.str);
                        return 1;
                    }

                    /* Repoint into the (possibly) new array.  From
                     * this point on, brpos is dead except for its
                     * index. */
                    rpos.buffer = rpos.list->strs + (backup - rpos.buffer);

                    /* Insert the new string */
                    memmove((brpos.buffer - backup) + rpos.buffer + 1,
                            (brpos.buffer - backup) + rpos.buffer,
                            (rpos.list->len - 1 -
                             (rpos.buffer - rpos.list->strs)) *
                            sizeof(struct tagged_str));
                    *((brpos.buffer - backup) + rpos.buffer) = str;
                } else {
                    /* The two buffers are not equivalent.  Delete
                     * all of those inbetween and then remove the end
                     * of rpos and beginning of brpos. */
                    assert(rpos.list <= brpos.list);
                    if (rpos.list != brpos.list) {
                        tagged_str* iterator;
                        if (rpos.list + 1 < brpos.list) {
                            tagged_str_list* list_iterator;
                            /* Delete lists inbetween neither are
                             * in. */
                            for (list_iterator = rpos.list + 1;
                                 list_iterator < brpos.list;
                                 ++list_iterator) {
                                destroy_tagged_str_list(
                                    list_iterator);
                            }
                            memmove(rpos.list + 1, brpos.list,
                                    (buffers_end - brpos.list) *
                                    sizeof(struct tagged_str_list));
                            brpos.list = rpos.list + 1;
                        }

                        /* Delete strs after rpos in its list */
                        for (iterator = rpos.buffer + 1;
                             iterator !=
                             rpos.list->strs + rpos.list->len;
                             ++iterator) {
                            free(iterator->str);
                        }
                        rpos.list->len =
                            rpos.buffer + 1 - rpos.list->strs;

                        /* Delete strs before brpos in its list */
                        iterator = brpos.buffer;
                        while (iterator != brpos.list->strs) {
                            --iterator;
                            free(iterator->str);
                        }
                        memmove(brpos.list->strs, brpos.buffer,
                                (brpos.list->len -
                                 (brpos.buffer - brpos.list->strs)) *
                                sizeof(struct tagged_str));

                        /* Delete past rpos in its str */
                        rpos.buffer->len = rpos.index;
                        shrink_tagged_str(rpos.buffer);

                        /* Delete before brpos in its str.  Walks the
                         * tagged fpos forward then moves the string
                         * back. */
                        walk_fpos(&brpos.buffer->fpos,
                                  brpos.buffer->str,
                                  brpos.index);
                        memmove(brpos.buffer->str,
                                brpos.buffer->str + brpos.index,
                                (brpos.buffer->len -= brpos.index) *
                                sizeof(char));
                        shrink_tagged_str(brpos.buffer);
                    } else {
                        tagged_str* iterator;
                        /* Delete strs between the two buffers */
                        for (iterator = rpos.buffer + 1;
                             iterator != brpos.buffer; ++iterator) {
                            free(iterator->str);
                        }
                        memmove(rpos.buffer + 1, brpos.buffer,
                                (brpos.list->strs + brpos.list->len -
                                 brpos.buffer) *
                                sizeof(struct tagged_str));
                    }
                }
            }
        }
    } while (!forward_char(&fpos, &rpos, buffers_end));

    return 0;
}

static int delete_block_comments(tagged_str_list* buffers,
                                 tagged_str_list* buffers_end) {
    file_position fpos;
    raw_position rpos;

    /* Go back to beginning of file */
    rpos.list = buffers;
    rpos.buffer = buffers->strs;
    rpos.index = 0;
    fpos = rpos.buffer->fpos;

    do {
        file_position bfpos;
        raw_position brpos;

        /* pos is before the potential `/` and `*` */
        bfpos = fpos;
        brpos = rpos;

top:
        /* Goto the next character so we can check / and * at once. */
        if (forward_char(&bfpos, &brpos, buffers_end)) {
            return 0;
        }

        assert(rpos.index < rpos.buffer->len);
        assert(brpos.index < brpos.buffer->len);
        if (rpos.buffer->str[rpos.index] != '/' ||
            brpos.buffer->str[brpos.index] != '*') {
            /* If we aren't actually at a comment, repeat. */
            fpos = bfpos;
            rpos = brpos;
            goto top;
        }

        /* Put bpos after the / and * */
        if (forward_char(&bfpos, &brpos, buffers_end)) {
            goto unterminated_c;
        }

        print_warning(&fpos, "Position of |/*");

        /* Now look for `*` and `/` */
        while (1) {
            assert(brpos.index < brpos.buffer->len);
            if (brpos.buffer->str[brpos.index] != '*') {
                if (forward_char(&bfpos, &brpos, buffers_end)) {
                    goto unterminated_c;
                }
                continue;
            }

            if (forward_char(&bfpos, &brpos, buffers_end)) {
                goto unterminated_c;
            }
            assert(brpos.index < brpos.buffer->len);
            if (brpos.buffer->str[brpos.index] != '/') {
                continue;
            }

            print_warning(&bfpos, "Position of *|/");

            /* Go forward one to get brpos after `*` and `/` */
            if (forward_char(&bfpos, &brpos, buffers_end)) {
                /* We are at end of file so delete after rpos */
                rpos.buffer->len = rpos.index;

                while (1) {
                    ++rpos.buffer;
                    if (rpos.buffer ==
                        rpos.list->strs + rpos.list->len) {
                        ++rpos.list;
                        if (rpos.list == buffers_end) {
                            break;
                        }
                        rpos.buffer = rpos.list->strs;
                    }
                    free(rpos.buffer->str);
                }
                assert(rpos.list == buffers_end);
                return 0;
            } else if (rpos.buffer != brpos.buffer) {
                /* Delete end of rpos's str */
                rpos.buffer->len = rpos.index;
                rpos.buffer->str[rpos.buffer->len] = 0;

                /* Delete start of the last str */
                memmove(brpos.buffer->str,
                        brpos.buffer->str + brpos.index + 1,
                        (brpos.buffer->len -=
                         (brpos.index + 1 - rpos.index)) *
                        sizeof(char));

                rpos.index = 0;

                /* Delete between rpos's and brpos's initial strs */
                ++rpos.buffer;
                while (1) {
                    if (rpos.buffer ==
                        rpos.list->strs + rpos.list->len) {
                        /* If we are at the end of our sub array, go
                         * to the next sub array. */
                        ++rpos.list;
                        if (rpos.list == buffers_end) {
                            break;
                        }
                        rpos.buffer = rpos.list->strs;
                    }
                    if (rpos.buffer == brpos.buffer) {
                        /* Finish up by */
                        #warning fix
                        memmove(brpos.list->strs, brpos.buffer,
                                (--rpos.list->len -
                                 (rpos.buffer - rpos.list->strs)) *
                                sizeof(struct tagged_str_list));
                        break;
                    }
                    free(rpos.buffer->str);
                    ++rpos.buffer;
                    --rpos.list->len;
                }

                /* Assert post conditions: we are at start of the
                 * buffer the comment end used to be in. */
                assert(rpos.list == brpos.list);
                if (rpos.list != buffers_end) {
                    assert(rpos.buffer == brpos.buffer);
                }
                assert(rpos.index == 0);
            } else {
                /* Here the comment start and end are in the same
                 * buffer.  We have to split the buffer in two. */

                int offset = rpos.buffer - rpos.list->strs;

                /* Explicitly test the predicates of the data structure. */
                assert(rpos.buffer >= rpos.list->strs);
                assert(rpos.buffer < rpos.list->strs + rpos.list->len);

                /* Allocate space for another buffer.  After this
                 * `brpos.buffer` is invalid. */
                REALLOC(rpos.list->strs,
                        sizeof(struct tagged_str_list) *
                            (rpos.list->len + 1),
                        { return 1; });
                ++rpos.list->len;
                rpos.buffer = rpos.list->strs + offset;

                /* Explicitly test the predicates of the data structure. */
                assert(rpos.buffer >= rpos.list->strs);
                assert(rpos.buffer < rpos.list->strs + rpos.list->len);

                /* Move elements back after it. */
                assert(rpos.list->strs + rpos.list->len >=
                       rpos.buffer + 1);
                memmove(rpos.buffer + 2, rpos.buffer + 1,
                        rpos.list->strs + rpos.list->len -
                            (rpos.buffer + 1));

                /* Create the new buffer. */
                MALLOC(rpos.buffer[1].str,
                       sizeof(char) * (rpos.buffer->len - rpos.index),
                       { return 1; });

                strcpy(rpos.buffer[1].str,
                       rpos.buffer->str + brpos.index);

                rpos.buffer->len = rpos.index;
                rpos.buffer->str[rpos.buffer->len] = 0;
            }
            break;
        }

    } while (!forward_char(&fpos, &rpos, buffers_end));

    return 0;

unterminated_c:
    print_error(&fpos, "Unterminated comment.");
    return 1;
}

static int evaluate_preprocessor_commands(tagged_str_list* buffers,
                                          tagged_str_list* buffers_end) {
    file_position fpos;
    raw_position rpos;
    char previous = '\n';

    /* Go back to beginning of file */
    rpos.list = buffers;
    rpos.buffer = buffers->strs;
    rpos.index = 0;
    fpos = rpos.buffer->fpos;
    do {
        /* Skip whitespace at beginning of line */
        if (forward_through_blanks(&fpos, &rpos, buffers_end)) {
            break;
        }
        assert(rpos.index < rpos.buffer->len);
        /* Recognize preprocessor commands */
        if (rpos.buffer->str[rpos.index] == '#' && previous == '\n') {
            directive directive;
            /* Skip whitespace after the # */
            if (forward_through_blanks(&fpos, &rpos, buffers_end)) {
                break;
            }
            if (!process_instruction(&fpos, &rpos, buffers_end,
                                     &directive)) {
                printf("Read directive %d\n", directive);
            }
        }
        assert(rpos.index < rpos.buffer->len);
        previous = rpos.buffer->str[rpos.index];
    } while (!forward_char(&fpos, &rpos, buffers_end));

    return 0;
}

static int preprocess_(tagged_str_list* buffers, tagged_str_list* buffers_end,
                       definition* *definitions, int *num_d,
                       header_guard* *header_guards, int *num_hg) {
    int ret = 0;

    if (combine_lines(buffers, buffers_end)) {
        ret = 1;
        goto cleanup;
    }

    if (delete_block_comments(buffers, buffers_end)) {
        ret = 1;
        goto cleanup;
    }

    if (evaluate_preprocessor_commands(buffers, buffers_end)) {
        ret = 1;
        goto cleanup;
    }

cleanup:
    /* Cleanup definitions */ {
        definition* definition = *definitions;
        for (; definition != *definitions + *num_d; ++definition) {
            free(definition->name.str);
            free(definition->definition.str);
        }
        free(*definitions);
    }
    /* Cleanup header_guards */ {
        header_guard* hg = *header_guards;
        for (; hg != *header_guards + *num_hg; ++hg) {
            free(hg->fname.str);
            free(hg->macro.str);
        }
        free(*header_guards);
    }
    return ret;
}

int preprocess(tagged_str_list* *buffers, tagged_str_list* buffers_end) {
    definition* definitions = 0;
    int num_d = 0;
    header_guard* header_guards = 0;
    int num_hg = 0;

    int i;

    int ret;

    assert(buffers);
    assert(buffers_end);

    ret = preprocess_(*buffers, buffers_end, &definitions,
                      &num_d, &header_guards, &num_hg);

    for (i = 0; i != num_d; ++i) {
        free(definitions[i].name.str);
        free(definitions[i].definition.str);
    }
    free(definitions);

    for (i = 0; i != num_d; ++i) {
        free(header_guards[i].fname.str);
        free(header_guards[i].macro.str);
    }
    free(header_guards);

    return ret;
}
