# Program wide settings
EXE       := cmips
VERSION   := 0
SUBLEVEL  := 1
PATCH     := 0
VERSION_N := $(VERSION).$(SUBLEVEL).$(PATCH)

# Compiler settings
CC      ?= cc
CFLAGS  := -Wall -I'./include' -O2 -std=c99           \
           -DCMIPS_VERSION=$(VERSION)                 \
           -DCMIPS_SUBLEVEL=$(SUBLEVEL)               \
           -DCMIPS_PATCH=$(PATCH)                     \
           -DCMIPS_VERSION_N="$(VERSION_N)"           \
		   -D_GNU_SOURCE
LDFLAGS ?=
LIBFLAGS := -lreadline
LEX     := flex
LFLAGS  := -Pcfg_yy
LD      := ld


# Install Paths
PREFIX  := /usr
BINDIR  := $(PREFIX)/bin

# Configuration -- Uncomment lines to enable option

# Enable debugging
CMIPS_DEBUG := y

# Show all commands executed by the Makefile
# VERBOSE := y

