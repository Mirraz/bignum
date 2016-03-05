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
	test_sqrt();
	test_extended_binary_euclidean();
}

int main() {
	suite();
	printf("Success\n");
	return 0;
}

