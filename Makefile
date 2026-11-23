CC ?= cc
PKG_CONFIG ?= pkg-config
BUILD_DIR ?= build
TARGET := $(BUILD_DIR)/qqbot
SRC := src/main.c src/qq/webhook.c
CFLAGS += -Isrc
CFLAGS += -std=c11 -D_POSIX_C_SOURCE=200809L -Wall -Wextra -Wpedantic -O2 $(shell $(PKG_CONFIG) --cflags libmicrohttpd libcurl sqlite3 libcjson openssl 2>/dev/null)
LDLIBS += $(shell $(PKG_CONFIG) --libs libmicrohttpd libcurl sqlite3 libcjson openssl 2>/dev/null) -lpthread

.PHONY: all clean run check-deps
all: $(TARGET)

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
