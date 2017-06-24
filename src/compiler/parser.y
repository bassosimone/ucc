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
#include<compiler/compiler.h>
%}

%error-verbose
%expect 14

%token STRING
%token IF
%token ELSE
%token EXEC
%token TO
%token TC
%token GO
%token GC
%token PV
%token P
%token ID
%left AND
%left OR
%left NOT
%left EQ
%left MAG
%left MIN
%left MAEQ
%left MIEQ
%left NEQ

%%

correct: functions;

functions:
|func
|functions func;

func: id param GO body GC
{
    debug("funz: id param GO body GC");
    code_eval_FUNC($1, $2, $4);
};

body:
{
    debug("body: <empty>");
    $$ = code_gen_NOP();
}
| if
{
    debug("body: if");
    $$ = $1;
}
| exec
{
    debug("body: exec");
    $$ = $1;
}
| body if 
{
    debug("body: body if");
    $$ = code_eval_BODY($1, $2);
}
| body exec
{
    debug("body: body exec");
    $$ = code_eval_BODY($1, $2);
}
| GO body GC
{
    debug("body: GO body GC");
    $$ = $2;
}
| PV
{
    debug("body: PV");
    $$ = code_gen_NOP();
};

if: IF TO conditions TC body
{
    debug("if: IF TO conditions TC body");
    $$ = code_eval_IF($3, $5);
}
| IF TO conditions TC body ELSE body
{
    debug("if: IF TO conditions TC body ELSE body");
    $$ = code_eval_IF_ELSE($3, $5, $7);
}
;

conditions: cond
{
    debug("conditions: cond");
    $$ = $1;
}
| conditions AND cond
{
    debug("conditions: conditions AND cond");
    $$ = code_eval_AND($1, $3);
}
| conditions OR cond
{
    debug("conditions: conditions OR cond");
    $$ = code_eval_OR($1, $3);
}
| NOT conditions
{
    debug("conditions: NOT conditions");
    $$ = code_eval_NOT($2);
};

cond: value EQ value
{
    debug("cond: value EQ value");
    $$ = code_gen_EQ($1, $3);
}
| value MAG value
{
    debug("cond: value MAG value");
    $$ = code_gen_MAG($1, $3);
}
| value MIN value
{
    debug("cond: value MIN value");
    $$ = code_gen_MIN($1, $3);
}
| value MAEQ value
{
    debug("cond: value MAEQ value");
    $$ = code_gen_MAEQ($1, $3);
}
| value MIEQ value
{
    debug("cond: value MIEQ value");
    $$ = code_gen_MIEQ($1, $3);
}
| value NEQ value
{
    debug("cond: value NEQ value");
    $$ = code_gen_NEQ($1, $3);
}
| value
{
    debug("cond: value");
    $$ = $1;
}
| TO conditions TC
{
    debug("cond: TO conditions TC");
    $$ = $2;
};

value: STRING
{
    debug("value: STRING");
    $$ = $1;
}
| id P id
{
    debug("value: id P id");
    $$ = code_eval_ID_P_ID($1, $3);
};

exec: EXEC TO STRING TC PV
{
    debug("exec: EXEC TO STRING TC PV");
    $$ = code_gen_EXEC($3);
};

param: TO id TC
{
    debug("param: TO id TC");
    $$ = $1;
};

id: ID
{
    debug("id: ID");
    $$ = $1;
};

