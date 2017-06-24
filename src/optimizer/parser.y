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

%{
#include<optimizer/optimizer.h>

static codeLine CodeLineCurrent;       /* Buffer for current .code line */
static gotLine GotLineCurrent;         /* Buffer for current .got line  */
%}

%error-verbose
%expect 0

%token  STRING
%token  REGNAME
%token  OFFSET
%token  ID
%token  VM_NOP
%token  VM_EXEC
%token  VM_EQ
%token  VM_MAG
%token  VM_MIN
%token  VM_MAEQ
%token  VM_MIEQ
%token  VM_NEQ
%token  VM_JTRUE
%token  VM_JFALSE
%token  VM_JMP
%token  VM_RETURN
%token  SECT_CODE
%token  SECT_GOT

%%

full: | SECT_GOT SECT_CODE | got code
{
    debug("full");
};

got: SECT_GOT body_SECT_GOT
{
    debug("SECT_GOT body_SECT_GOT");
};

body_SECT_GOT: ID OFFSET
{
    debug("body SECT_GOT");

    GotLineCurrent.id    = strdup($1);
    GotLineCurrent.start = atoi($2);

    got_line_add(&GotLineCurrent);
}
| body_SECT_GOT ID OFFSET
{
    debug("body SECT_GOT ricorsivo");

    GotLineCurrent.id    = strdup($2);
    GotLineCurrent.start = atoi($3);

    got_line_add(&GotLineCurrent);
};

code: SECT_CODE body_SECT_CODE 
{
    debug("sect_SECT_CODE body_SECT_CODE");
};

body_SECT_CODE: line_SECT_CODE 
{
    debug("body_SECT_CODE");
}
| body_SECT_CODE line_SECT_CODE
{
    debug("body_SECT_CODE recursive");
};

line_SECT_CODE: OFFSET VM_NOP 
{
    debug("line_SECT_CODE VM_NOP");

    CodeLineCurrent.offset = atoi($1);
    CodeLineCurrent.opcode = strdup("VM_NOP");
    CodeLineCurrent.jump   = -1;
    CodeLineCurrent.reg    = NULL;
    CodeLineCurrent.string = NULL;

    code_line_add(&CodeLineCurrent);
}
| OFFSET VM_RETURN 
{
    debug("line_SECT_CODE VM_RETURN");

    CodeLineCurrent.offset = atoi($1);
    CodeLineCurrent.opcode = strdup("VM_RETURN");
    CodeLineCurrent.jump   = -1;
    CodeLineCurrent.reg    = NULL;
    CodeLineCurrent.string = NULL;

    code_line_add(&CodeLineCurrent);
}
| OFFSET VM_JMP OFFSET 
{     
    int my_offset, target;

    debug("line_SECT_CODE VM_JMP");

    my_offset = atoi($1);
    target = atoi($3);

    /* Optimization: kill a jump that reference the following line. */
    if (target == my_offset + 1) {
        CodeLineCurrent.offset = my_offset;
        CodeLineCurrent.opcode = strdup("VM_NOP");
        CodeLineCurrent.jump   = -1;
        CodeLineCurrent.reg    = NULL;
        CodeLineCurrent.string = NULL;
    } else {
        CodeLineCurrent.offset = my_offset;
        CodeLineCurrent.opcode = strdup("VM_JMP");
        CodeLineCurrent.jump   = target;
        CodeLineCurrent.reg    = NULL;
        CodeLineCurrent.string = NULL;
    }

    code_line_add(&CodeLineCurrent);
}
| OFFSET VM_EQ REGNAME STRING 
{
    debug("line_SECT_CODE VM_EQ");

    CodeLineCurrent.offset = atoi($1);
    CodeLineCurrent.opcode = strdup("VM_EQ");
    CodeLineCurrent.jump   = -1;
    CodeLineCurrent.reg    = strdup($3);
    CodeLineCurrent.string = strdup($4);

    code_line_add(&CodeLineCurrent);
}
| OFFSET VM_NEQ REGNAME STRING 
{
    debug("line_SECT_CODE VM_NEQ");

    CodeLineCurrent.offset = atoi($1);
    CodeLineCurrent.opcode = strdup("VM_NEQ");
    CodeLineCurrent.jump   = -1;
    CodeLineCurrent.reg    = strdup($3);
    CodeLineCurrent.string = strdup($4);

    code_line_add(&CodeLineCurrent);
}
| OFFSET VM_MAG REGNAME STRING 
{
    debug("line_SECT_CODE VM_MAG");

    CodeLineCurrent.offset = atoi($1);
    CodeLineCurrent.opcode = strdup("VM_MAG");
    CodeLineCurrent.jump   = -1;
    CodeLineCurrent.reg    = strdup($3);
    CodeLineCurrent.string = strdup($4);

    code_line_add(&CodeLineCurrent);
}
| OFFSET VM_MIN REGNAME STRING 
{
    debug("line_SECT_CODE VM_MIN");

    CodeLineCurrent.offset = atoi($1);
    CodeLineCurrent.opcode = strdup("VM_MIN");
    CodeLineCurrent.jump   = -1;
    CodeLineCurrent.reg    = strdup($3);
    CodeLineCurrent.string = strdup($4);

    code_line_add(&CodeLineCurrent);
}
| OFFSET VM_MAEQ REGNAME STRING 
{
    debug("line_SECT_CODE VM_MAEQ");

    CodeLineCurrent.offset = atoi($1);
    CodeLineCurrent.opcode = strdup("VM_MAEQ");
    CodeLineCurrent.jump   = -1;
    CodeLineCurrent.reg    = strdup($3);
    CodeLineCurrent.string = strdup($4);

    code_line_add(&CodeLineCurrent);
}
| OFFSET VM_MIEQ REGNAME STRING 
{
    debug("line_SECT_CODE VM_MIEQ");

    CodeLineCurrent.offset = atoi($1);
    CodeLineCurrent.opcode = strdup("VM_MIEQ");
    CodeLineCurrent.jump   = -1;
    CodeLineCurrent.reg    = strdup($3);
    CodeLineCurrent.string = strdup($4);

    code_line_add(&CodeLineCurrent);
}
| OFFSET VM_EXEC STRING
{
    debug("line_SECT_CODE VM_EXEC");

    CodeLineCurrent.offset = atoi($1);
    CodeLineCurrent.opcode = strdup("VM_EXEC");
    CodeLineCurrent.jump   = -1;
    CodeLineCurrent.reg    = NULL;
    CodeLineCurrent.string = strdup($3);

    code_line_add(&CodeLineCurrent);
}
| OFFSET VM_JTRUE OFFSET OFFSET VM_JFALSE OFFSET
{
    int my_offset, target;

    debug("line_SECT_CODE VM_JTRUE (followed by VM_JFALSE)");

    /*
     * Analyze VM_JTRUE.
     */

    my_offset = atoi($1);
    target = atoi($3);

    /* Optimization: kill a jump that reference the following line or the
     * following next line. This extra optimization is possible because
     * we know that we have two conditional jumps one after the another: in
     * general it's not possible to kill a jump to my_offset +2 because we'll
     * break the flow. However, this situation makes it possible because
     * only one of the two conditional jumps will be taken:
     *
     *   1 VM_MAEQ   $3 "The Forest Of October"
     *   2 VM_JTRUE  4
     *   3 VM_JFALSE 7
     *   4 ...
     *
     * As you can see, in this case is safe to kill line 2.
     *
     * WARNING! The compiler (or the assembler programmer) MUST always
     * produce a packet of two conditional jumps, where the first is a
     * VM_JTRUE and the second is VM_JFALSE.
     */
    if (target == my_offset + 1 || target == my_offset + 2) {
        CodeLineCurrent.offset = my_offset;
        CodeLineCurrent.opcode = strdup("VM_NOP");
        CodeLineCurrent.jump   = -1;
        CodeLineCurrent.reg    = NULL;
        CodeLineCurrent.string = NULL;
    } else {
        CodeLineCurrent.offset = my_offset;
        CodeLineCurrent.opcode = strdup("VM_JTRUE");
        CodeLineCurrent.jump   = target;
        CodeLineCurrent.reg    = NULL;
        CodeLineCurrent.string = NULL;
    }
    code_line_add(&CodeLineCurrent);

    /*
     * Analyze VM_FALSE.
     */

    my_offset = atoi($4);
    target = atoi($6);

    /* Optimization: kill a jump that reference the following line. */
    if (target == my_offset + 1) {
        CodeLineCurrent.offset = my_offset;
        CodeLineCurrent.opcode = strdup("VM_NOP");
        CodeLineCurrent.jump   = -1;
        CodeLineCurrent.reg    = NULL;
        CodeLineCurrent.string = NULL;
    } else {
        CodeLineCurrent.offset = my_offset;
        CodeLineCurrent.opcode = strdup("VM_JFALSE");
        CodeLineCurrent.jump   = target;
        CodeLineCurrent.reg    = NULL;
        CodeLineCurrent.string = NULL;
    }
    code_line_add(&CodeLineCurrent);
};

%%

