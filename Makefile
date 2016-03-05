CC=g++
LD=gcc
#LD=g++
STRIP=true
WARNINGS=-Wall -Wextra
DEBUG=
#DEBUG=-g -ggdb
COPTIM=-march=native -O2
#COPTIM=-O0
DEFINES=
INCLUDES=
CSTD=-std=c++11
CFLAGS=$(WARNINGS) $(DEBUG) $(COPTIM) $(DEFINES) $(INCLUDES) $(CSTD) -pipe
LDOPTIM=-Wl,-O1 -Wl,--as-needed
#LDOPTIM=
LIBFILES=-lm
LDFLAGS=$(WARNINGS) $(DEBUG) $(LDOPTIM) $(LIBFILES)
SRC_DIR=.
BUILD_DIR=build

bignum_tests: $(BUILD_DIR)/bignum_tests.o
	$(LD) -o $@ $^ $(LDFLAGS)
	$(STRIP) $@

$(BUILD_DIR)/bignum_tests.o: $(SRC_DIR)/bignum_tests.cpp $(SRC_DIR)/bignum.h Makefile
	$(CC) -o $@ $< -c $(CFLAGS)

clean:
	rm -rf $(BUILD_DIR)
	mkdir $(BUILD_DIR)

