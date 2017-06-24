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
#include<optimizer/optimizer.h>       /* WARNING! Must be first.       */
#include<optimizer/parser.h>

%}

NEWLINE \r|\n|\r\n

STRING \"[^\"]*\"

REGNAME "$"[0-5]

OFFSET [0-9]+

ID [A-Za-z_][A-Za-z0-9_]*

%%


{NEWLINE} {
     debug("NEWLINE:");
}

{STRING} {
    yylval = strdup(yytext);
    debug("STRING: %s", yytext);
    return STRING;
}

{REGNAME} {
    yylval = strdup(yytext);
    debug("REGNAME: %s", yytext);
    return REGNAME;
}

{OFFSET} {
    yylval = strdup(yytext);
    debug("OFFSET: %s", yytext);
    return OFFSET;
}

"VM_NOP" {
    debug("VM_NOP: %s", yytext);
    return VM_NOP;
}

"VM_EXEC" {
    debug("VM_EXEC: %s", yytext);
    return VM_EXEC;
}

"VM_EQ" {
    debug("VM_EQ: %s", yytext);
    return VM_EQ;
}

"VM_MAG" {
    debug("VM_MAG: %s", yytext);
    return VM_MAG;
}

"VM_MIN" {
    debug("VM_MIN: %s", yytext);
    return VM_MIN;
}

"VM_MAEQ" {
    debug("VM_MAEQ: %s", yytext);
    return VM_MAEQ;
}

"VM_MIEQ" {
    debug("VM_MIEQ: %s", yytext);
    return VM_MIEQ;
}

"VM_NEQ" {
    debug("VM_NEQ: %s", yytext);
    return VM_NEQ;
}

"VM_JTRUE" {
    debug("VM_JTRUE: %s", yytext);
    return VM_JTRUE;
}

"VM_JFALSE" {
    debug("VM_JFALSE: %s", yytext);
    return VM_JFALSE;
}

"VM_JMP" {
    debug("VM_JMP: %s", yytext);
    return VM_JMP;
}

"VM_RETURN" {
    debug("VM_RETURN: %s", yytext);
    return VM_RETURN;
}

".code" {
    debug("SECT_CODE: %s", yytext);
    return SECT_CODE;
}

".got" {
    debug("SECT_GOT: %s", yytext);
    return SECT_GOT;
}

{ID} {
    yylval = strdup(yytext);
    debug("ID: %s", yytext);
    return ID;
}

" "|\t ;

%%
