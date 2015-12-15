SCRATCH_ROOT := .
PATH_BIN = $(addprefix $(SCRATCH_ROOT)/,lib)

#-------------------------------------------------------------------------------
LTARGET := psfreader_c
#-------------------------------------------------------------------------------
.PHONY: default all
default: msg_targets
all: $(LTARGET)

.PHONY: msg_targets
msg_targets:
	@printf "Makefile targets:\n"
	@printf "   -clean: Clean-up build directories.\n"
	@printf "   -psfreader_c: Build PSF reader C-interface.\n"
	@printf "   -all: Build all.\n"

include ./MakeCppTools.mk
LPATH_INCLUDE += ./include
CPPFLAGS += -Wall
CPPFLAGS += -fPIC
CPPFLAGS += -O3

#Libraries
#-------------------------------------------------------------------------------
LIBREF_PSFCore := $(call CreateLibRef,psfcore,src)

#Add libraries:
$(call AddCppLibraryList,LIBREF_PSFCore)

#Shared object targets
#-------------------------------------------------------------------------------

#-----libpsfreader_c.so-----
sharedobjDep := $(call AppendPath,src/psf_c,psfreader_c.cpp)
sharedobjDep += $(call GetLibTargetList,LIBREF_PSFCore)

#PHONY: Always re-compile .so from scratch
.PHONY: psfreader_c
psfreader_c: $(sharedobjDep)
	$(CC) $(CFLAGS) $(CPPFLAGS) -shared -o $(call GetProgPath,libpsfreader_c.so) $^

#Convenient target aliases
#-------------------------------------------------------------------------------
$(call AddLibAlias,libpsfcore,$(LIBREF_PSFCore))

#Include dependencies of registered files
#-------------------------------------------------------------------------------
$(call IncludeDependencies)

.PHONY: msg_debug
msg_debug:
	@printf "COMPILE.cpp: !$(COMPILE.cpp)!\n"
	@echo
	@printf "PATH_BUILD: !$(PATH_BUILD)!\n"
	@printf "PATH_BIN: !$(PATH_BIN)!\n"
	@printf "MAKECMDGOALS: !$(MAKECMDGOALS)!\n"
	@printf "OUTPUT_OPTION: !$(OUTPUT_OPTION)!\n"
	@echo
	@printf "LPATH_INCLUDE: !$(LPATH_INCLUDE)!\n\n"
	@printf "LREL_CLEANUP: !$(LREL_CLEANUP)!\n\n"
	@printf "LABS_CLEANUP: !$(LABS_CLEANUP)!\n\n"
