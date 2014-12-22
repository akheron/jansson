####################################################################################################
#
# Make xml_parser library
#
####################################################################################################

PROJECT_DIR = .
BUILD_DIR ?= ../build-internal
export BUILD_DIR

SOURCE_DIR = $(PROJ_DIR)/src
INCLUDE_DIR = $(PROJ_DIR)/src

# Include base makefile after setting directories
#
-include $(BUILD_DIR)/Makefile.base.mk

######################################################################################
### Local information

PROJECT_BASE_NAME = jansson

#  177-D: function "json_object_set" was declared but never referenced
#  550-D: variable "value" was set but never used
# 1293-D: assignment in condition
#
DIAGFLAGS += --diag_remark=177,550,1293

include $(BUILD_DIR)/Makefile.library.mk

# Make sure we're using our config.h -- this has to be done after the include above.
#
CONFIG_H = $(SOURCE_DIR)/jansson_config.h

$(CONFIG_H): $(PROJ_DIR)/armcc/jansson_config.h
	@cp $< $@

$(OBJS): $(CONFIG_H)

clean: code_clean archive_clean depclean
	rm $(CONFIG_H)
