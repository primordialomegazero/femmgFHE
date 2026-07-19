CC = gcc
CFLAGS = -std=c11 -O3 -Wno-deprecated-declarations
INCLUDES = -I include -I src/schnorr -I src/bitcoin -I src/fractal
LDFLAGS = -lssl -lcrypto

all: test_quick test_halimaw test_oneshot

test_quick: src/schnorr/schnorr.c test/quick_test.c
	$(CC) $(CFLAGS) $(INCLUDES) $^ $(LDFLAGS) -o $@

test_halimaw: src/schnorr/schnorr.c src/bitcoin/shrincs.c src/fractal/schnorr_fractal.c src/fractal/shrincs_fractal.c test/halimaw_ultimate.c
	$(CC) $(CFLAGS) $(INCLUDES) $^ $(LDFLAGS) -o $@

test_oneshot: src/schnorr/schnorr.c src/bitcoin/shrincs.c src/fractal/schnorr_fractal.c src/fractal/shrincs_fractal.c test/oneshot_fullblown.c
	$(CC) $(CFLAGS) $(INCLUDES) $^ $(LDFLAGS) -o $@

examples: example_basic

example_basic: src/schnorr/schnorr.c examples/basic_sign.c
	$(CC) $(CFLAGS) $(INCLUDES) $^ $(LDFLAGS) -o $@

clean:
	rm -f test_quick test_halimaw test_oneshot example_basic
