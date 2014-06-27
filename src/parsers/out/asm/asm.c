/*
 * Copyright (C) 2013 Matt Kilgore
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */
#include "common.h"

#include <stdio.h>

#include "mips/inst.h"
#include "parser.h"

int parser_write_asm_file(struct parser *parser, FILE *file)
{
    int i;
    be32 inst;
    int cpu_inst;
    const struct inst_desc *desc;

    fprintf(file, ".data\n");
    for (i = 0; i < parser->data.len; i++)
        fprintf(file, ".byte 0x%02x\n", (int)parser->data.data[i]);
    fprintf(file, "\n");

    for (i = 0; i < parser->text.len; i += 4) {
        int op;
        inst = ((be32 *)parser->text.data)[i / sizeof(be32)];
        cpu_inst = be32_to_cpu(inst);

        desc = inst_ids;
        op = INST_OPCODE(cpu_inst);

        if (op == OP_SPECIAL) {
            int func = INST_R_FUNC(cpu_inst);
            for (; desc->g.ident != NULL; desc++) {
                if (desc->func == func) {
                    printf("%s\n", desc->g.ident);
                    break;
                }
            }
            break;
        }

        for (; desc->g.ident != NULL; desc++) {
            if (desc->opcode == op) {
                printf("%s\n", desc->g.ident);
                break;
            }
        }
    }


    return 0;
}

