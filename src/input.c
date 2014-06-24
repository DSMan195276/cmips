/*
 * Copyright (C) 2013 Matt Kilgore
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */
#include "common.h"

#include <stdlib.h>
#include <stdio.h>
#include <readline/readline.h>
#include <readline/history.h>

#include "cmips.h"
#include "cmds.h"

static const char *prompt = "cmips> ";

static int exit_inp_loop = 0;

void exit_cmd(int argc, char **argv)
{
    exit_inp_loop = 1;
}

static char **parse_line(char *line, int *argc)
{
    char **lines = NULL;
    int sflag = 0, i, len, last;

    len = strlen(line);
    *argc = 0;

    for (i = 0; i < len; i++) {
         if (!isspace(line[i]) && !sflag) {
             (*argc)++;
             lines = realloc(lines, *argc * sizeof(*lines));
             last = i;
             sflag = 1;
         } else if (isspace(line[i]) && sflag) {
             line[i] = '\0';
             lines[*argc - 1] = line + last;
             sflag = 0;
         }
    }

    if (sflag) {
        lines[*argc - 1] = line + last;
        sflag = 0;
    }

    return lines;
}

static char *match_func(const char *line, int state)
{
    static int list_index, len;
    const char *name;

    if (!state) {
        list_index = 0;
        len = strlen(line);
    }

    while ((name = cmips_cmds[list_index].cmd_id)) {
        list_index++;

        if (strncmp(name, line, len) == 0)
            return strdup(name);
    }

    return NULL;
}

static char **complete_line(const char *line, int start, int end)
{
    char **matches;

    matches = NULL;

    if (start == 0)
        matches = rl_completion_matches((char *)line, match_func);

    return matches;
}

static void run_cmd(char *line)
{
    struct cmips_cmd *cmd;
    char **lines;
    int args;

    lines = parse_line(line, &args);

    if (lines == NULL)
        return ;

    for (cmd = cmips_cmds; cmd->cmd_id != NULL; cmd++) {
        if (strcmp(lines[0], cmd->cmd_id) == 0) {
            (cmd->cmd) (args - 1, lines + 1);
            break;
        }
    }

    if (cmd->cmd_id == NULL)
        printf("Unknown command '%s'\n", lines[0]);

    free(lines);

    return ;
}

int run_script(const char *s_file)
{
    char *line = NULL;
    size_t len;
    FILE *script;

    script = fopen(s_file, "r");

    if (!script) {
        printf("Error opening script '%s'\n", s_file);
        return -1;
    }

    while ((len = getline(&line, &len, script)) != -1 && !exit_inp_loop) {
        if (line[len - 1] == '\n')
            line[len - 1] = '\0';
        if (len == 1)
            continue ;
        printf("%s%s\n", prompt, line);
        run_cmd(line);
    }

    fclose(script);
    free(line);

    return 0;
}

void run_input_loop(void)
{
    char *line = NULL;

    if (cmips_arg_state.cmd_script) {
        run_script(cmips_arg_state.cmd_script);
        return ;
    }

    rl_attempted_completion_function = complete_line;

    do {
        line = readline(prompt);
        if (line && *line)
            add_history(line);
        else
            continue;

        run_cmd(line);

        if (line)
            free(line);
    } while (!exit_inp_loop);

    clear_history();
}

