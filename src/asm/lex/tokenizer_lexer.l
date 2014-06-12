/*
 * Copyright (C) 2014 Matt Kilgore
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */
%top {
#include "common.h"
}

%{
#include <string.h>
#include <stdlib.h>
#include "mips_emu.h"
#include "tokenizer_lexer.h"

#define YY_NO_INPUT

#define YY_DECL enum asm_token yylex (struct tokenizer *state)

static void append_c(struct tokenizer *t, char ch);
%}

%option noyywrap

%x comment
%x ident_str

%%

[ \t]+      /* Eat whitespace */

\n      state->line++;

("#"|"//")[^\n]*    /* One line comment */

";" { return TOK_SEMICOLON; }
"(" { return TOK_LPAREN; }
")" { return TOK_RPAREN; }
"," { return TOK_COMMA; }

\.(align|ALIGN) { state->val = DIR_align; return TOK_DIRECTIVE; }
\.(ascii|ASCII) { state->val = DIR_ascii; return TOK_DIRECTIVE; }
\.(asciiz|ASCIIZ) { state->val = DIR_asciiz; return TOK_DIRECTIVE; }
\.(byte|BYTE) { state->val = DIR_byte; return TOK_DIRECTIVE; }
\.(data|DATA) { state->val = DIR_data; return TOK_DIRECTIVE; }
\.(double|DOUBLE) { state->val = DIR_double; return TOK_DIRECTIVE; }
\.(extern|EXTERN) { state->val = DIR_extern; return TOK_DIRECTIVE; }
\.(float|FLOAT) { state->val = DIR_float; return TOK_DIRECTIVE; }
\.(globl|GLOBL) { state->val = DIR_globl; return TOK_DIRECTIVE; }
\.(half|HALF) { state->val = DIR_half; return TOK_DIRECTIVE; }
\.(kdata|KDATA) { state->val = DIR_kdata; return TOK_DIRECTIVE; }
\.(ktext|KTEXT) { state->val = DIR_ktext; return TOK_DIRECTIVE; }
\.(space|SPACE) { state->val = DIR_space; return TOK_DIRECTIVE; }
\.(text|TEXT) { state->val = DIR_text; return TOK_DIRECTIVE; }
\.(word|WORD) { state->val = DIR_word; return TOK_DIRECTIVE; }


\.?[a-zA-Z][a-zA-Z0-9]*: {
    if (state->ident)
        free(state->ident);
    state->ident = strndup(yytext, strlen(yytext) - 1);
    return TOK_LABEL;
}

0x[0-9a-fA-F]+ {
    state->val = strtol(yytext, NULL, 0);
    return TOK_INTEGER;
}

\$(zero|ZERO) { state->val = REG_ZERO; return TOK_REGISTER; }

\$(at|AT) { state->val = REG_AT; return TOK_REGISTER; }
\$(gp|GP) { state->val = REG_GP; return TOK_REGISTER; }
\$(sp|SP) { state->val = REG_SP; return TOK_REGISTER; }
\$(fp|FP) { state->val = REG_FP; return TOK_REGISTER; }
\$(ra|RA) { state->val = REG_RA; return TOK_REGISTER; }

\$[vV]0 { state->val = REG_V0; return TOK_REGISTER; }
\$[vV]1 { state->val = REG_V1; return TOK_REGISTER; }

\$[aA]0 { state->val = REG_A0; return TOK_REGISTER; }
\$[aA]1 { state->val = REG_A1; return TOK_REGISTER; }
\$[aA]2 { state->val = REG_A2; return TOK_REGISTER; }
\$[aA]3 { state->val = REG_A3; return TOK_REGISTER; }

\$[tT]0 { state->val = REG_T0; return TOK_REGISTER; }
\$[tT]1 { state->val = REG_T1; return TOK_REGISTER; }
\$[tT]2 { state->val = REG_T2; return TOK_REGISTER; }
\$[tT]3 { state->val = REG_T3; return TOK_REGISTER; }
\$[tT]4 { state->val = REG_T4; return TOK_REGISTER; }
\$[tT]5 { state->val = REG_T5; return TOK_REGISTER; }
\$[tT]6 { state->val = REG_T6; return TOK_REGISTER; }
\$[tT]7 { state->val = REG_T7; return TOK_REGISTER; }
\$[tT]8 { state->val = REG_T8; return TOK_REGISTER; }
\$[tT]9 { state->val = REG_T9; return TOK_REGISTER; }

\$[sS]0 { state->val = REG_S0; return TOK_REGISTER; }
\$[sS]1 { state->val = REG_S1; return TOK_REGISTER; }
\$[sS]2 { state->val = REG_S2; return TOK_REGISTER; }
\$[sS]3 { state->val = REG_S3; return TOK_REGISTER; }
\$[sS]4 { state->val = REG_S4; return TOK_REGISTER; }
\$[sS]5 { state->val = REG_S5; return TOK_REGISTER; }
\$[sS]6 { state->val = REG_S6; return TOK_REGISTER; }
\$[sS]7 { state->val = REG_S7; return TOK_REGISTER; }

\$[kK]0 { state->val = REG_K0; return TOK_REGISTER; }
\$[kK]1 { state->val = REG_K1; return TOK_REGISTER; }

\$([0-9]|[12][0-9]|3[0-1]) {
    state->val = strtol(yytext + 1, NULL, 10);
    return TOK_REGISTER;
}


-?[0-9]+ {
    state->val = strtol(yytext, NULL, 0);
    return TOK_INTEGER;
}

"/*"                    BEGIN(comment);
<comment>[^*\n]*
<comment>"*"+[^*/\n]*
<comment>\n             state->line++;
<comment>"*"+"/"        BEGIN(INITIAL);

"\""    {
    if (state->ident) {
        free(state->ident);
        state->ident = NULL;
    }
    state->id_len = 0;
    state->id_alloc = 0;
    BEGIN(ident_str);
}

<ident_str>\"  {
    append_c(state, '\0');
    BEGIN(INITIAL);
    return TOK_QUOTE_STRING;
}

<ident_str>\\[0-7]{1,3} {
    int result = strtol(yytext + 1, NULL, 8);
    append_c(state, (char)result);
}

<ident_str>"\\x"[0-9a-fA-F]{1,2} {
    int result = strtol(yytext + 2, NULL, 16);
    append_c(state, (char)result);
}

<ident_str>\\n { append_c(state, '\n'); }
<ident_str>\\r { append_c(state, '\r'); }
<ident_str>\\b { append_c(state, '\b'); }
<ident_str>\\f { append_c(state, '\f'); }
<ident_str>\\a { append_c(state, '\a'); }
<ident_str>\\t { append_c(state, '\t'); }
<ident_str>\\v { append_c(state, '\v'); }
<ident_str>[^\\\"\n]+ {
    char *yptr = yytext;
    while (*yptr) {
        append_c(state, *yptr);
        yptr++;
    }
}

\.?[a-zA-Z][a-zA-Z0-9]* {
    if (state->ident)
        free(state->ident);
    state->id_len = 0;
    state->id_alloc = 0;
    state->ident = strdup(yytext);
    return TOK_IDENT;
}

<<EOF>> {
    return TOK_EOF;
}

.

%%

static void append_c(struct tokenizer *t, char ch)
{
    t->id_len++;
    if (t->id_alloc < t->id_len) {
        t->id_alloc += 20;
        t->ident = realloc(t->ident, t->id_alloc);
    }
    t->ident[t->id_len - 1] = ch;
}

void fake_func(void)
{
    yyunput(0, 0);
}

