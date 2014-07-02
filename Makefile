
include ./config.mk

srctree := .
objtree := .

EXECUTABLE := ./$(EXE)

# This is our default target - The default is the first target in the file so
# we need to define this fairly high-up.
all: real-all

PHONY += all install clean dist real-all

# This variable defines any extra targets that should be build by 'all'
EXTRA_TARGETS :=

# This is the internal list of objects to compile into the file .o
REAL_OBJS_y :=

# Predefine this variable. It contains a list of extra files to clean. Ex.
CLEAN_LIST :=

DEPS :=

# Set configuration options
ifdef V
	Q :=
else
	Q := @
endif

ifdef $(EXEC)_DEBUG
	CPPFLAGS += -D$(EXEC)_DEBUG
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
	$(Q)$(LD) -r $(2) -o $$@
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

$$(eval $$(call create_link_rule,$$(objtree).o,$$(objs)))

$$(foreach subdir,$$(subdir-y),$$(eval $$(call subdir_inc,$$(subdir))))

srctree := $$(patsubst %/$(1),%,$$(srctree))
objtree := $$(patsubst %/$(1),%,$$(objtree))
endef


# Include the base directories for source files - That is, the generic 'src'
$(eval $(call subdir_inc,src))
REAL_OBJS_y += src.o

# Include tests
ifneq (,$(filter $(MAKECMDGOALS),check clean))
include ./test/Makefile
endif


# Actual entry
real-all: $(EXECUTABLE)

dist: clean
	$(Q)mkdir -p $(EXE)-$(VERSION_N)
	$(Q)cp -R Makefile README.md config.mk LICENSE ./doc ./include ./src ./test $(EXE)-$(VERSION_N)
	$(Q)tar -cf $(EXE)-$(VERSION_N).tar $(EXE)-$(VERSION_N)
	$(Q)gzip $(EXE)-$(VERSION_N).tar
	$(Q)rm -fr $(EXE)-$(VERSION_N)
	@echo " Created $(EXE)-$(VERSION_N).tar.gz"

clean:
	$(Q)for file in $(CLEAN_LIST) $(EXECUTABLE); do \
		if [ -e $$file ]; then \
			echo " RM      $$file"; \
			rm -rf $$file; \
		fi \
	done

$(EXECUTABLE): $(REAL_OBJS_y)
	@echo " CCLD    $@"
	$(Q)$(CC) $(LDFLAGS) -o $@ $(REAL_OBJS_y) $(LIBFLAGS)

$(objtree)/%.o: $(srctree)/%.c
	@echo " CC      $@"
	$(Q)$(CC) $(CPPFLAGS) $(CFLAGS) $(CFLAGS_$@) -c $< -o $@

$(objtree)/.%.d: $(srctree)/%.c
	$(Q)$(CC) -MM -MP -MF $@ $(CPPFLAGS) $< -MT $(objtree)/$*.o -MT $@

DEP_LIST := $(join $(foreach dir,$(DEPS),$(dir $(dir))),$(foreach file,$(DEPS),.$(notdir $(file))))
DEP_LIST := $(DEP_LIST:.o=.d)

ifneq ($(MAKECMDGOALS),clean)
-include $(DEP_LIST)
endif
CLEAN_LIST += $(DEP_LIST)

.PHONY: $(PHONY)

