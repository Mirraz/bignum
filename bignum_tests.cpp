#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>

#include "bignum.h"

void test_assign() {
	BigNum<10, 2, 2> a;
	for (unsigned int i=0; i<100; ++i) {
		a = i;
		unsigned int v = a.value();
		assert(i == v);
	}
}

void test_add() {
	unsigned int i, j, k, l;
	BigNum<10, 2, 2> a, b, c, d;
	for (i=0; i<=99; ++i) {
		for (j=0; j<=99; ++j) {
			if (i+j >= 100) continue;
			a = i;
			b = j;
			c = a + j;
			d = a + b;
			k = c.value();
			l = d.value();
			assert(k == i + j);
			assert(l == i + j);
		}
	}
}

void test_mul() {
	unsigned int i, j, k, l;
	BigNum<10, 3, 3> a, b, c, d;
	for (i=0; i<=99; ++i) {
		for (j=0; j<=99; ++j) {
			if (i*j >= 1000) continue;
			a = i;
			b = j;
			c = a * j;
			d = a * b;
			k = c.value();
			l = d.value();
			assert(k == i * j);
			assert(l == i * j);
		}
	}
}

void test_sub() {
	unsigned int i, j, k, l;
	BigNum<10, 2, 2> a, b, c, d, e;
	for (i=0; i<=99; ++i) {
		for (j=0; j<=99; ++j) {
			if (i < j) continue;
			a = i;
			b = j;
			c = a - j;
			k = c.value();
			assert(k == i - j);
			d = a - b;
			l = d.value();
			assert(l == i - j);
			e = i - j;
			assert(d == e);
		}
	}
}

void test_div() {
	unsigned int i, j, k, l;
	digit_type denom;
	BigNum<10, 3, 3> a, b, c;
	for (j=1; j<10; ++j) {
		for (i=0; i<1000; ++i) {
			a = i;
			b = a / j;
			c = a.div(j, &denom);
			k = b.value();
			l = c.value();
			assert(k == i / j);
			assert(l == i / j);
			assert(denom == i % j);
		}
	}
}

int my_pow(const uint_fast64_t base, uint_fast8_t exp, uint_fast64_t *power) {
	uint_fast64_t result = 1;
	uint_fast8_t mask = 1;
	while (mask <= exp) mask <<= 1; // XXX my overflow
	mask >>= 1;
	while (mask > 0) {
		if (result > UINT32_MAX) return 1;
		result *= result;
		if (mask & exp) {
			//if (result > ceil(UINT64_MAX / base)) return 1;
			if (base > 0 && result > ((UINT64_MAX - 1) / base) + 1) return 1;
			result *= base;
		}
		mask >>= 1;
	}
	*power = result;
	return 0;
}

void test_pow() {
	uint_fast64_t i, j, k, l;
	BigNum<16, 16, 19> a, b;
	for (i=0; i<1024*64; ++i) {
		for (j=0; j<64; ++j) {
			if (i == 0 && j == 0) continue;
			if (my_pow(i, j, &k)) continue;
			a = i;
			b = a.pow(j);
			l = b.value();
			assert(l == k);
		}
	}
}

void test_sqrt() {
	unsigned int i, j, k;
	typedef BigNum<10, 4, 4> MyBigNum;
	MyBigNum a, b;
	for (i=0; i<1000; ++i) {
		a = i;
		b = MyBigNum::square_root(a);
		j = b.value();
		k = sqrt(i);
		assert(j == k);
	}
}

void test_extended_binary_euclidean() {
	unsigned int i, j, k, l, g;
	typedef BigNum<16, 3, 3> MyBigNum;
	//typedef BigNum<16, 4+1, 6> MyBigNum;
	MyBigNum a, b, x, y, gcd;
	for (i=1; i<256; ++i) {
	//for (i=1024*3; i<1024*4; ++i) {
		for (j=1; j<256; ++j) {
		//for (j=1024*3; j<1024*4; ++j) {
			if (i % 2 == 0 && j % 2 == 0) continue;
			a = i;
			b = j;
			MyBigNum::extended_binary_euclidean(a, b, &x, &y, &gcd);
			k = x.value();
			l = y.value();
			g = gcd.value();
			assert(i * k - j * l == g);
		}
	}
}

void suite() {
	test_assign();
	test_add();
	test_mul();
	test_sub();
	test_div();
	test_pow();
	test_sqrt();
	test_extended_binary_euclidean();
}

int main() {
	suite();
	printf("Success\n");
	return 0;
}

