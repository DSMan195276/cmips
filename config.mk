# Compiler settings
CC      ?= cc
CFLAGS  += -Wall -I'./include' -O2 -std=c99 \
		   -D_GNU_SOURCE -Wno-unused-result
LDFLAGS ?=
LEX     ?= flex
LFLAGS  ?=
LD      ?= ld
PERL    ?= perl -w
MKDIR   ?= mkdir

# Install Paths
PREFIX  := /usr
BINDIR  := $(PREFIX)/bin
MANDIR  := $(PREFIX)/share/man
MAN1DIR := $(MANDIR)/man1
DOCDIR  := $(PREFIX)/share/doc/$(EXE)

# Show all commands executed by the Makefile
# V := y

# Turn on debugging
# CMIPS_DEBUG := y

