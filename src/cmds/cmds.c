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
#include "input.h"
#include "mips.h"
#include "asm.h"
#include "cmds.h"

static void dump_regs(int argc, char **argv)
{
    mips_dump_regs(&cmips_emu.r);
}

static void dump_seg_mem(int argc, char **argv)
{
    printf("Text segment (%d):\n", cmips_emu.mem.text.size);
    dump_mem(cmips_emu.mem.text.block, cmips_emu.mem.text.size, cmips_emu.mem.text.addr);

    printf("Data segment (%d):\n", cmips_emu.mem.data.size);
    dump_mem(cmips_emu.mem.data.block, cmips_emu.mem.data.size, cmips_emu.mem.data.addr);
}

static void run_inst(int argc, char **argv)
{

}

static void run_code(int argc, char **argv)
{
    mips_run(&cmips_emu);
}

static void reset_code(int argc, char **argv)
{
    mips_reset_emu(&cmips_emu);
}

static void load_file(int argc, char **argv)
{
    if (argc != 1) {
        printf("Please give load-file a single argument that is the file to load\n");
        return ;
    }

    mips_load_file(&cmips_emu, argv[0]);
}

static void script(int argc, char **argv)
{
    if (argc != 1) {
        printf("Please give run-script a single argument that is the file to run\n");
        return ;
    }

    run_script(argv[0]);
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
    { "reset", reset_code, "Reset the currently loaded code.", "" },
    { "run-inst", run_inst, "Run a single instruction given as an argument.", "<instruction>" },
    { "dump-mem", dump_seg_mem, "Dump the current contents of memory", "" },
    { "run-script", script, "Run a command script", "<filename>" },

    { "help", help, "Display help information.", "" },
    { "quit", exit_cmd, "Exit the program.", "" },
    { "exit", exit_cmd, "Exit the program.", "" },
    { NULL, NULL, NULL, NULL},
};

