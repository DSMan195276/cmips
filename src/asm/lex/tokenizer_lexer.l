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
#include "mips_emu/mips_emu.h"
#include "asm/tokenizer_lexer.h"

#define YY_NO_INPUT

#define YY_DECL enum asm_token yylex (struct tokenizer *state)

static void append_c(struct tokenizer *t, char ch);
static int stringcasecmp(const char *s1, const char *s2);
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

\.[a-z]+ {
    state->ident = strdup(yytext + 1);
    return TOK_DIRECTIVE;
}

[a-zA-Z][a-zA-Z0-9]*: {
    state->ident = strndup(yytext, strlen(yytext) - 1);
    return TOK_LABEL;
}

\$((zero|ZERO)|(at|AT)|[vV][01]|[aA][0-3]|[tT][0-9]|[sS][0-7]|[kK][01]|(gp|GP)|(sp|SP)|(fp|FP)|(ra|RA)) {
    int i;
    for (i = 0; i < 32; i++) {
        if (stringcasecmp(mips_reg_names_strs[i], yytext + 1) == 0) {
            state->val = i;
            break;
        }
    }
    return TOK_REGISTER;
}

\$([0-9]|[12][0-9]|3[0-1]) {
    state->val = strtol(yytext + 1, NULL, 10);
    return TOK_REGISTER;
}

0x[0-9a-fA-F] {
    state->val = strtol(yytext, NULL, 0);
    return TOK_HEXVAL;
}

-?[0-9]+ {
    state->val = strtol(yytext, NULL, 0);
    return TOK_INTEGER;
}

"/*"    BEGIN(comment);
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
    return TOK_IDENT;
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

[a-zA-Z][a-zA-Z0-9]* {
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

static int stringcasecmp(const char *s1, const char *s2)
{
    for (; *s1 && *s2; s1++, s2++)
        if ((*s1 | 32) != (*s2 | 32))
            return 1;

    if (*s1 || *s2)
        return 1;

    return 0;
}



