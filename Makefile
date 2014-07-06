include ./config.mk

srctree := .
objtree := .

EXECUTABLE := ./$(EXE)

# This is our default target - The default is the first target in the file so
# we need to define this fairly high-up.
all: real-all

PHONY += all install clean dist real-all

# Predefine this variable. It contains a list of extra files to clean. Ex.
CLEAN_LIST :=

# List of files that dependency files should be generated for
DEPS :=

# Current project being compiled (Ex. cmips, ncmips, ...) - Blank for core
PROJ :=
EXES :=

# Set configuration options
ifdef V
	Q :=
else
	Q := @
endif

ifdef CMIPS_DEBUG
	CPPFLAGS += -DCMIPS_DEBUG
	CFLAGS += -g
	ASFLAGS += -g
	LDFLAGS += -g
endif


# This includes everything in the 'include' folder of the $(objtree)
# This is so that the code can reference generated include files
CPPFLAGS += -I'$(objtree)/include/'

define create_link_rule
$(1): $(2)
	@echo " LD      $$@"
	$$(Q)$$(LD) -r $(2) -o $$@
endef

define create_cc_rule
ifneq ($(3),)
ifneq ($$(wildcard $(2)),)
$(1): $(2)
	@echo " CC      $$@"
	$$(Q)$$(CC) $$(CFLAGS) $$(CPPFLAGS) $(3) -c $$< -o $$@
endif
endif
endef

# Traverse into tree
define subdir_inc
objtree := $$(objtree)/$(1)
srctree := $$(srctree)/$(1)

subdir-y :=
objs-y :=
clean-list-y :=

_tmp := $$(shell mkdir -p $$(objtree))
include $$(srctree)/Makefile

CLEAN_LIST += $$(patsubst %,$$(objtree)/%,$$(objs-y)) $$(patsubst %,$$(objtree)/%,$$(clean-list-y)) $$(objtree).o
DEPS += $$(patsubst %,$$(objtree)/%,$$(objs-y))

objs := $$(patsubst %,$$(objtree)/%,$$(objs-y)) $$(patsubst %,$$(objtree)/%.o,$$(subdir-y))

$$(foreach obj,$$(patsubst %,$$(objtree)/%,$$(objs-y)),$$(eval $$(call create_cc_rule,$$(obj),$$(obj:.o=.c),$$($$(PROJ)_CFLAGS))))

$$(eval $$(call create_link_rule,$$(objtree).o,$$(objs)))

$$(foreach subdir,$$(subdir-y),$$(eval $$(call subdir_inc,$$(subdir))))

srctree := $$(patsubst %/$(1),%,$$(srctree))
objtree := $$(patsubst %/$(1),%,$$(objtree))
endef


# Include the base directories for source files - That is, the generic 'src'
$(eval $(call subdir_inc,src))
$(eval $(call subdir_inc,common))

define proj_ccld_rule
$(1): $(2) | $$(objtree)/bin
	@echo " CCLD    $$@"
	$$(Q)$$(CC) $(3) $(2) -o $$@ $(4)
endef

define proj_inc
include $(1)/config.mk
PROG := $$(objtree)/bin/$$(EXE)
PROJ := $$(EXEC)
EXES += $$(PROG)

$$(eval $$(call proj_ccld_rule,$$(PROG),$$($$(EXEC)_OBJS),$$($$(EXEC)_CFLAGS),$$($$(EXEC)_LIBFLAGS)))
$$(eval $$(call subdir_inc,$$(EXE)))
CLEAN_LIST += $$(PROG)
endef

$(eval $(call proj_inc,cmips))
CLEAN_LIST += $(objtree)/bin

# Include tests
ifneq (,$(filter $(MAKECMDGOALS),check clean))
include ./test/Makefile
endif


# Actual entry
real-all: $(EXES)

dist: clean
	$(Q)mkdir -p $(EXE)-$(VERSION_N)
	$(Q)cp -R Makefile README.md config.mk LICENSE ./doc ./include ./src ./test $(EXE)-$(VERSION_N)
	$(Q)tar -cf $(EXE)-$(VERSION_N).tar $(EXE)-$(VERSION_N)
	$(Q)gzip $(EXE)-$(VERSION_N).tar
	$(Q)rm -fr $(EXE)-$(VERSION_N)
	@echo " Created $(EXE)-$(VERSION_N).tar.gz"

clean:
	$(Q)for file in $(CLEAN_LIST); do \
		if [ -e $$file ]; then \
			echo " RM      $$file"; \
			rm -rf $$file; \
		fi \
	done

$(objtree)/bin:
	@echo " MKDIR   $@"
	$(Q)$(MKDIR) $@

$(objtree)/%.o: $(srctree)/%.c
	@echo " CC      $@"
	$(Q)$(CC) $(CPPFLAGS) $(CFLAGS) $(CFLAGS_$@) -c $< -o $@

$(objtree)/.%.d: $(srctree)/%.c
	$(Q)$(CC) -MM -MP -MF $@ $(CPPFLAGS) $(CFLAGS) $< -MT $(objtree)/$*.o -MT $@

DEP_LIST := $(foreach dep,$(DEPS),$(dir $(dep)).$(notdir $(dep)))
DEP_LIST := $(DEP_LIST:.o=.d)

ifneq ($(MAKECMDGOALS),clean)
-include $(DEP_LIST)
endif
CLEAN_LIST += $(DEP_LIST)

.PHONY: $(PHONY)

