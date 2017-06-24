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
#include<compiler/compiler.h>      /* WARNING! Must be first.          */
#include<compiler/parser.h>
%}

%x comment

NEWLINE \r|\n|\r\n

STRING \"[^\"]*\"

COMMENT    "//".*{NEWLINE}

IF if

ELSE else

EXEC exec

TO "("

TC ")"

GO "{"

GC "}"

PV ";"

P "."

ID [A-Za-z_][A-Za-z0-9_]*

AND &&

OR "||"

NOT !

EQ ==

MAG >

MIN <

MAEQ >=

MIEQ <=
NEQ !=

%%

{NEWLINE} {
    ;
}

{STRING} {
    yylval = p_node_create(yytext);
    debug("STRING: %s", yytext);
    return STRING;
}

{COMMENT} {
    ;
}

"/*" {
    BEGIN(comment);
}

<comment>[^*/]* {
    ;
}

<comment>\* {
    ;
}

<comment>\/ {
    ;
}

<comment>"*/" {
    BEGIN(INITIAL);
}

{IF} {
    return IF;
}

{ELSE} {
    return ELSE;
}

{EXEC} {
    return EXEC;
}

{TO} {
    return TO;
}

{TC} {
    return TC;
}

{GO} {
    return GO;
}

{GC} {
    return GC;
}

{PV} {
    return PV;
}

{P} {
    return P;
}

{ID} {
    debug("ID: %s", yytext);
    yylval = p_node_create(yytext);
    return ID;
}

{AND} {
    return AND;
}

{OR} {
    return OR;
}

{NOT} {
    return NOT;
}

{EQ} {
    return EQ;
}

{MAG} {
    return MAG;
}

{MIN} {
    return MIN;
}

{MAEQ} {
    return MAEQ;
}

{MIEQ} {
    return MIEQ;
}

{NEQ} {
    return NEQ;
}

" "|\t ;

%%

