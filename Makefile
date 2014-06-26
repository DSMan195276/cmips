
include ./config.mk

srctree := .
objtree := .

EXE_OBJ := $(EXE).o
EXECUTABLE := $(EXE)

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

ifdef AUTO_CMDS
	CPPFLAGS += -DAUTO_CMD_LIST
endif


# This includes everything in the 'include' folder of the $(objtree)
# This is so that the code can reference generated include files
CPPFLAGS += -I'$(objtree)/include/'

# Traverse into tree
define subdir_inc
objtree := $$(objtree)/$(1)
srctree := $$(srctree)/$(1)

pfix := $$(subst /,_,$$(objtree))_
DIRINC_y :=
OBJS_y :=
CLEAN_LIST_y :=

_tmp := $$(shell mkdir -p $$(objtree))
include $$(srctree)/Makefile

CLEAN_LIST += $$(patsubst %,$$(objtree)/%,$$(CLEAN_LIST_y)) $$(patsubst %,$$(objtree)/%,$$(OBJS_y))

DEPS += $$(patsubst %,$$(objtree)/%,$$(OBJS_y))

$(1)tre := $$(objtree).o
$(1)objs := $$(patsubst %,$$(objtree)/%,$$(OBJS_y)) $$(patsubst %,$$(objtree)/%.o,$$(DIRINC_y))

$$($(1)tre): $$($(1)objs)
	@echo " LD      $$@"
	$(Q)$(LD) -r $$($(1)objs) -o $$@

$$(foreach subdir,$$(DIRINC_y),$$(eval $$(call subdir_inc,$$(subdir))))

srctree := $$(patsubst %/$(1),%,$$(srctree))
objtree := $$(patsubst %/$(1),%,$$(objtree))
pfix := $$(subst /,_,$$(objtree))_
endef


# Include the base directories for source files - That is, the generic 'src'
$(eval $(call subdir_inc,src))

REAL_OBJS_y += src.o

define compile_file

_tmp := $$(subst /,_,$$(basename $(1)))_y
ifdef $$(_tmp)

CLEAN_LIST += $$($$(_tmp))

$(1)_t := $$($$(_tmp))

$(1): $(1)_t
	@echo " LD      $$@"
	$$(Q)$$(LD) -r -o $$@ $(1)_t

endif

endef

$(foreach file,$(REAL_OBJS_y),$(eval $(call compile_file,$(file))))


ifneq ($(MAKECMDGOALS),clean)
ifneq ($(MAKECMDGOALS),dist)
-include $(DEPS:.o=.d)
endif
endif

CLEAN_LIST += $(DEPS:.o=.d)


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
	$(Q)for file in $(REAL_OBJS_y) $(CLEAN_LIST) $(EXE_OBJ) $(EXECUTABLE); do \
		echo " RM      $$file"; \
		rm -f $$file; \
	done

$(EXECUTABLE): $(EXE_OBJ)
	@echo " CCLD    $@"
	$(Q)$(CC) $(LDFLAGS) -o $@ $< $(LIBFLAGS)

$(EXE_OBJ): $(REAL_OBJS_y)
	@echo " LD      $@"
	$(Q)$(LD) -r $(REAL_OBJS_y) -o $@

$(objtree)/%.o: $(srctree)/%.c
	@echo " CC      $@"
	$(Q)$(CC) $(CPPFLAGS) $(CFLAGS) $(CFLAGS_$@) -c $< -o $@

$(objtree)/%.d: $(srctree)/%.c
	@echo " CCDEP   $@"
	$(Q)$(CC) -MM -MP -MF $@ $(CPPFLAGS) $< -MT $(objtree)/$*.o -MT $@


.PHONY: $(PHONY)

