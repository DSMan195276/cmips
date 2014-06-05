
include ./config.mk

EXEN := $(EXE)

all: $(EXEN) doc

SRCS := $(wildcard src/*.c)
OBJS := $(SRCS:.c=.o)

CLEAN_OBJS :=

DIRS := $(filter-out src/,$(sort $(dir $(wildcard src/*/))))

EXTRA_DEPS :=

define compile-dir
dir_name_$(1) := $$(patsubst %/,%,$(1))
dir_name_obj_$(1) := $$(dir_name_$(1)).o
OBJS += $$(dir_name_obj_$(1))

dir_name_srcs_$(1) := $$(wildcard $(1)*.c)
dir_name_lex_$(1) := $$(wildcard $(1)lex/*.l)

dir_name_objs_$(1) := $$(dir_name_srcs_$(1):.c=.o)
EXTRA_DEPS += $$(dir_name_objs_$(1):.o=.d)

dir_name_objs_$(1) += $$(dir_name_lex_$(1):.l=.o)

$$(dir_name_lex_$(1):.l=.c):

CLEAN_OBJS += $$(dir_name_objs_$(1)) $$(dir_name_lex_$(1):.l=.c)

$$(dir_name_obj_$(1)): $$(dir_name_objs_$(1))
	@echo " LD      $$@"
	$$(Q)$$(LD) -r $$(dir_name_objs_$(1)) -o $$@

endef

$(foreach dir,$(DIRS),$(eval $(call compile-dir,$(dir))))

DEPFILES := $(OBJS:.o=.d) $(EXTRA_DEPS)

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(MAKECMDGOALS),dist)
-include $(DEPFILES)
endif
endif

# Set configuration options
ifdef VERBOSE
	Q :=
else
	Q := @
endif

ifdef CMIPS_DEBUG
	CFLAGS += -DCMIPS_DEBUG -g
endif

.PHONY: all install clean dist doc install_$(EXE)

dist: clean
	$(Q)mkdir -p $(EXE)-$(VERSION_N)
	$(Q)cp -R Makefile README.md config.mk LICENSE ./doc ./include ./src $(EXE)-$(VERSION_N)
	@echo " TAR     $(EXE)-$(VERSION_N).tar"
	$(Q)tar -cf $(EXE)-$(VERSION_N).tar $(EXE)-$(VERSION_N)
	@echo " GZIP    $(EXE)-$(VERSION_N).tar.gz"
	$(Q)gzip -f $(EXE)-$(VERSION_N).tar
	@echo " RM      $(EXE)-$(VERSION_N).tar"
	$(Q)rm -fr $(EXE)-$(VERSION_N).tar
	@echo " RM      $(EXE)-$(VERSION_N)"
	$(Q)rm -fr $(EXE)-$(VERSION_N)

clean:
	$(Q)for file in $(OBJS) $(CLEAN_OBJS) $(DEPFILES) $(EXEN); do \
		if [ -f "$$file" ]; then \
			echo " RM      $$file"; \
			rm -f $$file; \
		fi \
	done

install: install_$(EXE) install_doc

install_$(EXE): $(EXEN)
	$(Q)mkdir -p $(BINDIR)
	$(Q)mkdir -p $(DOCDIR)
	$(Q)install -d $(BINDIR) $(DOCDIR)
	@echo " INSTALL README.md LICENSE"
	$(Q)install -m 644 README.md LICENSE $(DOCDIR)
	@echo " INSTALL $(EXEN)"
	$(Q)install -m 775 $(EXEN) $(BINDIR)

install_doc: doc doc/$(EXE).1
	$(Q)mkdir -p $(MAN1DIR)
	@echo " INSTALL doc/$(EXE).1"
	$(Q)install -m 444 doc/$(EXE).1 $(MAN1DIR)

$(EXEN): $(OBJS)
	@echo " CCLD    $@"
	$(Q)$(CC) $(LDFLAGS) $(OBJS) -o $@ $(LIBFLAGS)

%.c: %.l
	@echo " LEX     $@"
	$(Q)$(LEX) $(LFLAGS) -o $@ $<

%.o: %.c
	@echo " CC      $@"
	$(Q)$(CC) $(CFLAGS) -c $< -o $@

%.d: %.c
	@echo " CCDEP   $@"
	$(Q)$(CC) -MM -MP -MF $@ $(CFLAGS) $(LDFLAGS) $< -MT $*.o -MT $@

doc:

