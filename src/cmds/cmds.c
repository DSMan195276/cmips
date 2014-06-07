/*
 * Copyright (C) 2013 Matt Kilgore
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */
#include "common.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cmips.h"
#include "mips_emu.h"
#include "asm.h"
#include "cmds.h"

static void dump_regs(int argc, char **argv)
{
    mips_dump_regs(&cmips_emu.r);
}

static void run_inst(int argc, char **argv)
{

}

static void load_file(int argc, char **argv)
{
    if (argc != 1) {
        printf("Please give load-file a single argument that is the file to load\n");
        return ;
    }

    asm_gen_from_file(&cmips_asm_gen, argv[0]);
}

static void run_code(int argc, char **argv)
{
    int i;
    for (i = 0; i < cmips_asm_gen.text_size / 4; i++)
        mips_run_inst(&cmips_emu, ((uint32_t *)cmips_asm_gen.text)[i]);
}

static void help(int argc, char **argv)
{
    char buf[50];
    struct cmips_cmd *cmd;
    for (cmd = cmips_cmds; cmd->cmd_id != NULL; cmd++) {
        memset(buf, 0, sizeof(buf));
        strcpy(buf, cmd->cmd_id);
        strcat(buf, " ");
        strcat(buf + strlen(cmd->cmd_id) + 1, cmd->args);
        printf("%-30s - %s\n", buf, cmd->help);
    }
}

struct cmips_cmd cmips_cmds[] = {
    { "dump-regs", dump_regs, "Dump current contents of cpu registers.", "" },
    { "load-asm-file", load_file, "Load an assembly file, takes a filename as an argument.", "<filename>" },
    { "run", run_code, "Run currently loaded code.", "" },
    { "run-inst", run_inst, "Run a single instruction given as an argument.", "<instruction>" },

    { "help", help, "Display help information.", "" },
    { "quit", NULL, "Exit the program.", "" },
    { "exit", NULL, "Exit the program.", "" },
    { NULL, NULL, NULL, NULL},
};

