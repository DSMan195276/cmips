
include ./config.mk

all: $(EXE) doc

SRCS := $(wildcard src/*.c) $(LEX_CS)
OBJS := $(SRCS:.c=.o)

CLEAN_OBJS :=

DIRS := $(filter-out ./src/,$(sort $(dir $(wildcard ./src/*/))))

define compile-dir
dir_name_$(1) := $$(patsubst ./%/,%,$(1))
dir_name_obj_$(1) := $$(dir_name_$(1)).o
OBJS += $$(dir_name_obj_$(1))

dir_name_srcs_$(1) := $$(wildcard $(1)*.c)
dir_name_objs_$(1) := $$(dir_name_srcs_$(1):.c=.o)

CLEAN_OBJS += $$(dir_name_objs_$(1))

$$(dir_name_obj_$(1)): $$(dir_name_objs_$(1))
	@echo " LD      $$@"
	$$(Q)$$(LD) -r $$(dir_name_objs_$(1)) -o $$@
endef

$(foreach dir,$(DIRS),$(eval $(call compile-dir,$(dir))))

# Set configuration options
ifdef VERBOSE
	Q :=
else
	Q := @
endif

ifdef CMIPS_DEBUG
	CFLAGS += -DCMIPS_DEBUG -g
endif

.PHONY: all install clean dist

dist: clean
	$(Q)mkdir -p $(EXE)-$(VERSION_N)
	$(Q)cp -R Makefile README.md config.mk LICENSE ./doc ./include ./src $(EXE)-$(VERSION_N)
	$(Q)tar -cf $(EXE)-$(VERSION_N).tar $(EXE)-$(VERSION_N)
	$(Q)gzip $(EXE)-$(VERSION_N).tar
	$(Q)rm -fr $(EXE)-$(VERSION_N)
	@echo " Created $(EXE)-$(VERSION_N).tar.gz"

clean:
	@echo " RM      $(OBJS) $(CLEAN_OBJS)"
	$(Q)rm -f $(OBJS) $(CLEAN_OBJS)
	@echo " RM      $(EXE)"
	$(Q)rm -f $(EXE)

$(EXE): $(OBJS)
	@echo " CCLD    $@"
	$(Q)$(CC) $(LDFLAGS) $(OBJS) -o $@

%.o: %.c
	@echo " CC      $@"
	$(Q)$(CC) $(CFLAGS) -c $< -o $@

%.c: %.l
	@echo " LEX     $@"
	$(Q)$(LEX) $(LFLAGS) -o $@ $<

doc:

