# Program wide settings
EXE       := cmips
EXEC      := CMIPS
VERSION   := 0
SUBLEVEL  := 1
PATCH     := 0
VERSION_N := $(VERSION).$(SUBLEVEL).$(PATCH)

# Compiler settings
CC      ?= cc
CFLAGS  += -Wall -I'./include' -O2 -std=c99             \
           -D$(EXEC)_VERSION=$(VERSION)                 \
           -D$(EXEC)_SUBLEVEL=$(SUBLEVEL)               \
           -D$(EXEC)_PATCH=$(PATCH)                     \
           -D$(EXEC)_VERSION_N="$(VERSION_N)"           \
		   -Wno-unused-result                           \
		   -D_GNU_SOURCE
LDFLAGS ?=
LIBFLAGS := -lreadline
LEX     ?= flex
LFLAGS  ?=
LD      ?= ld
PERL    ?= perl -w

# Install Paths
PREFIX  := /usr
BINDIR  := $(PREFIX)/bin
MANDIR  := $(PREFIX)/share/man
MAN1DIR := $(MANDIR)/man1
DOCDIR  := $(PREFIX)/share/doc/$(EXE)

# Configuration -- Uncomment lines to enable option

# Enable debugging
$(EXEC)_DEBUG := y

# Show all commands executed by the Makefile
# V := y

