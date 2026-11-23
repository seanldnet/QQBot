CC ?= cc
PKG_CONFIG ?= pkg-config
BUILD_DIR ?= build
TARGET := $(BUILD_DIR)/qqbot
SRC := src/main.c src/qq/webhook.c
SDK_SRC := sdk/src/qqbot_sdk.c
SDK_LIB := $(BUILD_DIR)/libqqbot_sdk.so
CFLAGS += -Isrc
CFLAGS += -std=c11 -D_POSIX_C_SOURCE=200809L -Wall -Wextra -Wpedantic -O2 $(shell $(PKG_CONFIG) --cflags libmicrohttpd libcurl sqlite3 libcjson openssl 2>/dev/null)
LDLIBS += $(shell $(PKG_CONFIG) --libs libmicrohttpd libcurl sqlite3 libcjson openssl 2>/dev/null) -lpthread

.PHONY: all sdk clean run check-deps
all: $(TARGET) sdk

sdk: $(SDK_LIB)

$(SDK_LIB): $(SDK_SRC) sdk/include/qqbot_sdk.h
	@mkdir -p $(BUILD_DIR)
	$(CC) -std=c11 -D_POSIX_C_SOURCE=200809L -DQQBOT_SDK_BUILD -fPIC -shared -Wall -Wextra -Wpedantic -Isdk/include $(shell $(PKG_CONFIG) --cflags libcurl) -o $@ $(SDK_SRC) $(shell $(PKG_CONFIG) --libs libcurl)

$(TARGET): $(SRC)
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDLIBS)

check-deps:
	@for p in libmicrohttpd libcurl sqlite3 libcjson openssl; do \
	  pkg-config --exists $$p || { echo "Missing dependency: $$p"; exit 1; }; \
	done

run: $(TARGET)
	./$(TARGET)

clean:
	rm -rf $(BUILD_DIR)
