/*
 * Copyright (C) 2013 Matt Kilgore
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */
#include "common.h"

#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <ncurses.h>

#include "mips.h"
#include "emu.h"

struct emulator cmips_emu;

enum color_pairs {
    MAIN_WIN = 1,
    CUR_LINE,
    DIS_WIN
};

WINDOW *dis_win, *reg_win, *main_win;

static void catch_errs(const char *err,va_list args)
{
    vprintf(err, args);
}

static void load_regs(WINDOW *win, struct mips_regs *regs)
{
    int i;

    mvwprintw(win, 1, 1, "zero: 0x%08x  at: 0x%08x", regs->regs[0], regs->regs[1]);
    for (i = 0; i < 30; i++)
        mvwprintw(win, i / 5 + 2, (i % 5) * 16 + 1, "%s: 0x%08x", mips_reg_names_strs[i + 2], regs->regs[i + 2]);
    mvwprintw(win, 8, 1, "hi: 0x%08x  lo: 0x%08x  pc: 0x%08x", regs->hi, regs->lo, regs->pc);
    wrefresh(win);
}

static void load_disassembly(WINDOW *win, struct emulator *emu)
{
    be32 inst;
    uint32_t addr;
    int rows, cols, i, k;
    getmaxyx(win, rows, cols);

    char buf[cols - 12];
    addr = cmips_emu.r.pc - (rows / 2 - 1) * 4;

    wattron(win, COLOR_PAIR(DIS_WIN));

    for (i = 0; i < rows - 2; i++, addr += 4) {
        mem_read_from_addr(&cmips_emu.mem, addr, sizeof(be32), &inst);
        mips_disassemble_inst(be32_to_cpu(inst), buf);

        if (i == rows / 2 - 1)
            wattron(win, COLOR_PAIR(CUR_LINE));

        for (k = 0; k < cols - 2; k++)
            mvwaddch(win, i + 1, k + 1, ' ');

        mvwprintw(win, i + 1, 1, "0x%08x %s", addr, buf);

        if (i == rows / 2 - 1)
            wattron(win, COLOR_PAIR(DIS_WIN));
    }

    wattroff(win, COLOR_PAIR(DIS_WIN));
    wrefresh(win);
}

static void load_output(WINDOW *win, int outfd)
{
    char buf[20];
    ssize_t len;

    while ((len = read(outfd, buf, sizeof(buf))) > 0) {
        buf[len] = '\0';
        wprintw(win, "%s", buf);
    }
    wrefresh(win);
}

int main(int argc, char **argv)
{
    int outputfd[2];
    struct timespec tim;

    if (argc <= 1) {
        printf("Please supply program with a assembly program to run\n");
        return 0;
    }

    initscr();
    raw();
    keypad(stdscr, 1);
    noecho();

    start_color();
    use_default_colors();
    init_pair(MAIN_WIN, -1, -1);
    init_pair(CUR_LINE, COLOR_GREEN, -1);
    init_pair(DIS_WIN, -1, -1);

    emulator_init(&cmips_emu);
    cmips_emu.err_disp = catch_errs;
    emulator_load_from_file(&cmips_emu, argv[1]);

    reg_win = subwin(stdscr, 10, COLS, 0, 0);
    dis_win = subwin(stdscr, LINES / 2 - 10, COLS, 10, 0);
    main_win = subwin(stdscr, LINES / 2, COLS, LINES / 2, 0);

    box(reg_win, ACS_VLINE, ACS_HLINE);
    load_regs(reg_win, &cmips_emu.r);
    box(dis_win, ACS_VLINE, ACS_HLINE);
    load_disassembly(dis_win, &cmips_emu);

    pipe(outputfd);
    fcntl(outputfd[0], F_SETFL, fcntl(outputfd[0], F_GETFL) | O_NONBLOCK);
    cmips_emu.outfd = outputfd[1];

    wmove(main_win, 0, 0);

    memset(&tim, 0, sizeof(struct timespec));

    while (!cmips_emu.stop_prog) {
        tim.tv_nsec = 100000000;
        nanosleep(&tim, NULL);

        emulator_run_next_inst(&cmips_emu);
        load_regs(reg_win, &cmips_emu.r);
        load_disassembly(dis_win, &cmips_emu);
        load_output(main_win, outputfd[0]);
    }

    close(outputfd[0]);
    close(outputfd[1]);

    wprintw(main_win, "\nProg done\n");
    wrefresh(main_win);
    refresh();
    getch();

    emulator_clear(&cmips_emu);

    endwin();
    return 0;
}

