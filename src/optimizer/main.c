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

/**
 * @file optimizer/main.c
 * Optimizer main file.
 */

#include <optimizer/optimizer.h>

/**
 * @defgroup optimpl Optimizer implementation
 * @ingroup optimizer
 * @{
 * Memory management strategy we use: cancer. We allocate memory for each
 * string that we need and we never deallocate, until we run out of memory.
 * Given the limitate scope of this example program, I guess that we're
 * not going to run out of memory. The day this happens, give us a call
 * and we'll fix this issue.
 *
 * Important: Only strings that have a meaningful semantic value should
 * be passed to the parser (e.g. a particular register yes, a particular
 * command no: infact a particular command has no more semantic than
 * itself).
 */

/** Head of linked list of code lines. */
static codeListNode *CodeHead;
/** Tail in linked list of code lines. */
static codeListNode *CodeTail;
/** Head in linked list of got lines. */
static gotListNode *GotHead;
/** Tail in linked list of got lines. */
static gotListNode *GotTail;

/** Delete useless lines from the code and print the output. */
static void optimize_code(void)
{
    codeListNode *code;
    gotListNode *got;
    int *vec, n;

    /* Make sure we don't segfault if the input was empty. */
    if (CodeTail == NULL) {
        fprintf(stderr, "[warning] Nothing of interesting to optimize\n");
        return;
    }

    /* Allocate a vector to exchange line numbers. */
    vec = calloc(CodeTail->content.offset, sizeof (int));

    /* Filter out VM_NOP lines. */
    for (code = CodeHead, n = 0; code != NULL; code = code->nextPtr) {
        if (strcmp(code->content.opcode, "VM_NOP") != 0) {
            /* Register that line has changed its offset. */
            vec[code->content.offset] = n;
            /* Assign the line its new offset number. */
            code->content.offset = n;
            /* Point to next instruction. */
            ++n;
        } else {
            /* Let VM_NOP pretend to be next non-VM_NOP instruction. This
             * makes it possible for a function to start with a NOP sled! */
            vec[code->content.offset] = n;
        }
    }

    /* Rewrite jump targets. */
    for (code = CodeHead; code != NULL; code = code->nextPtr) {
        if (strcmp(code->content.opcode, "VM_JMP") == 0 ||
            strcmp(code->content.opcode, "VM_JFALSE") == 0 ||
            strcmp(code->content.opcode, "VM_JTRUE") == 0)
        {
            code->content.jump = vec[code->content.jump];
        }
    }

    /* Rewrite global offset table. */
    for (got = GotHead; got != NULL; got = got->nextPtr) {
        got->content.start = vec[got->content.start];
    }

    /* Print global offset table. */
    printf(".got\n");
    for (; GotHead != NULL; GotHead = GotHead->nextPtr) {
        printf("%s %d\n", GotHead->content.id, GotHead->content.start);
    }

    /* Print .code section. */
    printf(".code\n");
    for (; CodeHead != NULL; CodeHead = CodeHead->nextPtr) {
        if (strcmp("VM_NOP",CodeHead->content.opcode) == 0) {
            /* Don't include VM_NOP in the result. */
            continue;
        }
        printf("%d %s ", CodeHead->content.offset, CodeHead->content.opcode);
        if (CodeHead->content.jump != -1) {
            printf("%d ", CodeHead->content.jump);
        } else if(CodeHead->content.reg != NULL) {
            printf("%s %s", CodeHead->content.reg, CodeHead->content.string);
        } else if (strcmp("VM_EXEC", CodeHead->content.opcode) == 0) {
            printf("%s", CodeHead->content.string);
        }
        printf("\n");
    }
}

/**
 * @}
 */

void code_line_add(codeLine *current)
{
    codeListNode* n = malloc(sizeof (codeListNode));

    if (n != NULL) {
        n->content.offset = current->offset;
        n->content.opcode = current->opcode;
        n->content.jump = current->jump;
        n->content.reg = current->reg;
        n->content.string = current->string;
    } else {
        fprintf(stderr, "Out of memory\n");
        exit(1);
    }

    if (CodeTail != NULL){
        CodeTail->nextPtr = n;
        CodeTail = n;
    } else {
        CodeTail = CodeHead = n;
    }
}

void got_line_add(gotLine *current)
{
    gotListNode* n = malloc(sizeof (gotListNode));

    if (n != NULL) {
        n->content.id = current->id;
        n->content.start = current->start;
    } else {
        fprintf(stderr, "Out of memory\n");
        exit(1);
    }

    if(GotTail != NULL){
        GotTail->nextPtr = n;
        GotTail = n;
    } else {
        GotTail = GotHead = n;
    }
}

int yywrap(void)
{
    return 1;
}

void yyerror (char const * str)
{
    fprintf(stderr, "%s\n", str);
    exit(1);
}

int main(int argc, char ** argv)
{
    char * prog = argv[0];
    ++argv, --argc;

    if (argc > 0) {
        for (; argc > 0; ++argv, --argc) {
            yyin = fopen(argv[0], "r");
            if (!yyin) {
                fprintf(stderr, "%s: error - can't open %s", prog, argv[0]);
                exit(1);
            }
            yyparse();
            fclose(yyin);
        }
    } else {
        yyin = stdin;
        yyparse();
    }

    optimize_code();

    return 0;
}
