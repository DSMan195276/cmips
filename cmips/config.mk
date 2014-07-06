# Program wide settings
EXE       := cmips
EXEC      := CMIPS
CMIPS_VERSION   := 0
CMIPS_SUBLEVEL  := 1
CMIPS_PATCH     := 0
CMIPS_VERSION_N := $(CMIPS_VERSION).$(CMIPS_SUBLEVEL).$(CMIPS_PATCH)

CMIPS_LIBFLAGS := -lreadline
CMIPS_CFLAGS  += -DCMIPS_VERSION=$(CMIPS_VERSION)       \
				 -DCMIPS_SUBLEVEL=$(CMIPS_SUBLEVEL)     \
				 -DCMIPS_PATCH=$(CMIPS_PATCH)           \
				 -DCMIPS_VERSION_N="$(CMIPS_VERSION_N)"

CMIPS_OBJS += ./src.o ./common/arg_parser.o ./cmips.o

