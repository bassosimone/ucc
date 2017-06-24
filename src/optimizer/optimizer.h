/* Copyright 2007 Andrea Autiero, Simone Basso.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this client except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include <string.h>
#include<stdlib.h>
#include <stdio.h>

/**
 * @file optimizer/optimizer.h
 * Optimizer main header.
 */

/**
 * @defgroup optimizer Optimizer
 * @{
 *
 * The <b>optimizer</b> is a module that works on <b>compiler</b> output,
 * trying to make the code faster and smaller. It builds two linked lists,
 * one with <b>global offset table</b> entries and one with <b>code</b>
 * lines. The algorithm used to optimize is roughly as follows:
 *
 * <ul>
 *   <li>Replace with <code>VM_NOP</code> all redundant instructions;</li>
 *   <li>Rewrite the code, removing all <code>VM_NOP</code>.
 * </ul>
 *
 * There are two cases of redundant instructions:
 *
 * <ul>
 *   <li>A <code>VM_JMP</code> that targets next instruction;</li>
 *   <li>A <code>VM_JTRUE</code>-<code>VM_JFALSE</code> pair.</li>
 * </ul>
 *
 * The latter, in particular, is rather common, because the compiler translates
 * a comparison instruction in three instructions: one for comparing, one in
 * case of success and one in case of failure. For instance:
 *
 * <pre>
 *   1 VM_MAEQ    $1, "The Forest Of October"    # Comparison
 *   2 VM_JTRUE   4                              # Success
 *   3 VM_JFALSE  20                             # Failure
 * </pre>
 *
 * This is rather inefficient, because there is always an unneeded jump, either
 * in the true or in the false case, depending on the flow of the original
 * program. Therefore, the optimizer will remove such unneded jump, making the
 * code faster and smaller.
 *
 * It is very important to note that compiler and optimizer agrees that there
 * will always be a <code>VM_JFALSE</code> after a <code>VM_JTRUE</code>. For
 * this reason, the optimizer parser will reject a source file that breaks this
 * rule. An alternative design, e.g. allowing unpaired conditional jumps, is
 * possible, but slower. Infact we'd need to scan the code in search for
 * paired jumps, while with current design we optimize in the semantic rule.
 */

/** A line in the <code>.code</code> section. */
typedef struct codeLine {
    /** Offset of this instruction in the code. */
    int offset;
    /** Assembly operative code. */
    char *opcode;
    /** Jump target (for jump instructions only). */
    int jump;
    /** Register involved in the operation. */
    char *reg;
    /** String involved in the operation. */
    char *string;
} codeLine;

/** Linked list of lines in the <code>.code</code> section. */
typedef struct codeListNode {
    /** Line in the <code>.code</code> section. */
    struct codeLine content;
    /** Next line in linked list. */
    struct codeListNode *nextPtr;
} codeListNode;

/** Line in the <code>.got</code> section. */
typedef struct gotLine {
    /** Function name. */
    char *id;
    /** Function offset in code. */
    int start;
} gotLine;

/** Linked list of lines in the <code>.got</code> section. */
typedef struct gotListNode {
    /** Line in the <code>.got</code> section. */
    struct gotLine content;
    /** Next line in linked list. */
    struct gotListNode *nextPtr;
} gotListNode;

/** Add data from a code line to the proper linked list. */
void code_line_add(codeLine *current);

/** Add data from a got line to the proper linked list. */
void got_line_add(gotLine *current);

/** This is the type that will be used in parser and scanner. */
#define YYSTYPE const char*

/**
 * @}
 */

extern int yylex(void);

extern int yyparse(void);

extern FILE *yyin;

extern void yyerror(char const * str);

#ifdef DEBUG
#include<stdarg.h>
#endif

static inline void debug(const char *fmt, ...)
{
#ifdef DEBUG
    va_list ap;
    fputs("debug: ", stderr);
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fputc('\n', stderr);
    va_end(ap);
#endif
}
