
include $(BUILD_DIR)/makefile.d/base.mk

CFLAGS += -Isrc -DHAVE_STDINT_H=1

JANSSON_A = $(OUTPUT_DIR)/libjansson.a

HEADERS += src/jansson.h
HEADERS += src/jansson_config.h
HEADERS += src/hashtable.h
HEADERS += src/strbuffer.h
HEADERS += src/utf.h
HEADERS += src/jansson_private.h

OBJS += $(OUTPUT_DIR)/dump.o
OBJS += $(OUTPUT_DIR)/error.o
OBJS += $(OUTPUT_DIR)/hashtable.o
OBJS += $(OUTPUT_DIR)/hashtable_seed.o
OBJS += $(OUTPUT_DIR)/load.o
OBJS += $(OUTPUT_DIR)/memory.o
OBJS += $(OUTPUT_DIR)/pack_unpack.o
OBJS += $(OUTPUT_DIR)/strbuffer.o
OBJS += $(OUTPUT_DIR)/strconv.o
OBJS += $(OUTPUT_DIR)/utf.o
OBJS += $(OUTPUT_DIR)/value.o

src/jansson_config.h: cppunit/jansson_config.h
	cp $< $@

$(OUTPUT_DIR):
	$(VERBOSE)mkdir -p $(OUTPUT_DIR)

$(OUTPUT_DIR)/%.o: src/%.c $(HEADERS) | $(OUTPUT_DIR)
	$(CC) $(CFLAGS) $(PROFILE_FLAGS) -c $< -o $@

$(JANSSON_A): $(HEADERS) $(OBJS) | $(OUTPUT_DIR)
	$(AR) src $(JANSSON_A) $(OBJS)

clean:
	rm -rf $(OUTPUT_DIR) src/jansson_config.h

all: $(JANSSON_A)