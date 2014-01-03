####################################################################################################
#
# Make xml_parser library
#
####################################################################################################

BUILD_DIR ?= ../build-internal
PROJECT_DIR = .
export BUILD_DIR

SOURCE_DIR = $(PROJ_DIR)/src
INCLUDE_DIR = $(PROJ_DIR)/src

# Include base makefile after setting directories
#
-include $(BUILD_DIR)/Makefile.base.mk


# Only use implicit rules created in this file.
#
.SUFFIXES:

# We begin by specifying the default make target:  this target will be made if make
# is run without any parameters.
#
.DEFAULT_GOAL := all


test:
	$(MAKE) -f Makefile.test test

testclean:
	$(MAKE) -f Makefile.test clean

######################################################################################
### Local information

# Turn on exceptions
#
PROJECTFLAGS += --exceptions
PROJECTFLAGS += -DUSING_EXCEPTIONS=1
LDPROJECTFLAGS += --exceptions

ifeq "$(USING_OPTIMIZATION)" "1"
PROJECTFLAGS += -O2 -Otime
else
PROJECTFLAGS += -O0
AUTOMATED_TESTING ?= 1
endif

PROJECT_BASE_NAME = jansson


include $(BUILD_DIR)/Makefile.library.mk

#
# 1294 assignment in condition
#
CFLAGS += --diag_suppress=1293

# Make sure we're using our config.h -- this has to be done after the include above.
#
CONFIG_H = $(SOURCE_DIR)/jansson_config.h

$(CONFIG_H):
	@cp $(PROJ_DIR)/armcc/jansson_config.h $(CONFIG_H)

$(OBJS): $(CONFIG_H)

clean: code_clean archive_clean depclean
	rm $(CONFIG_H)
