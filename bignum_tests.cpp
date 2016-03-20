#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>

#include "bignum.h"

void test_assign() {
	BigNum<10, 2> a;
	for (unsigned int i=0; i<100; ++i) {
		a = i;
		unsigned int v = a.value();
		assert(i == v);
	}
}

void test_add() {
	unsigned int i, j, k, l;
	BigNum<10, 2> a, b, c, d;
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
	BigNum<10, 3> a, b, c, d;
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
	BigNum<10, 2> a, b, c, d, e;
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
	digit_type remaind;
	BigNum<10, 3> a, b, c;
	for (j=1; j<10; ++j) {
		for (i=0; i<1000; ++i) {
			a = i;
			b = a / j;
			c = a.div(j, &remaind);
			k = b.value();
			l = c.value();
			assert(k == i / j);
			assert(l == i / j);
			assert(remaind == i % j);
		}
	}
}

void test_div2() {
	unsigned int i, j;
	BigNum<16, 2> a, b;
	for (i=0; i<256; ++i) {
		a = i;
		b = a.div2();
		j = b.value();
		//printf("%u\t%u\n", i, j);
		assert(j == i/2);
	}
}

void test_div_long() {
	unsigned int i, j, k, l;
	BigNum<10, 3> a, b, c, d;
	for (i=0; i<1000; ++i) {
		for (j=1; j<100; ++j) {
			a = i;
			b = j;
			c = a.div(b, &d);
			k = c.value();
			l = d.value();
			//printf("%u / %u = %u, %u\n", i, j, k, l);
			assert(k == i / j);
			assert(l == i % j);
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
	BigNum<16, 16> a, b;
	for (i=0; i<1024*64; ++i) {
		for (j=0; j<64; ++j) {
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
	typedef BigNum<10, 4> MyBigNum;
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
	typedef BigNum<16, 3> MyBigNum;
	//typedef BigNum<16, 4+1> MyBigNum;
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

void test_extended_binary_euclidean_large() {
	typedef BigNum<0x100000000llu, 686+1> MyBigNum;
	MyBigNum n, a, b, x, y, gcd;
	n = 2000; a = n.pow(2000);
	n = 1999; b = n.pow(1999);
	MyBigNum::extended_binary_euclidean(a, b, &x, &y, &gcd);
	//printf("x = "); x.printd(); printf("\n");
	//printf("y = "); y.printd(); printf("\n");
	//printf("gcd = "); gcd.printd(); printf("\n");
	assert(gcd == 1);
}

void test_linear_diophantine() {
	unsigned int i, j, k, m, n;
	typedef BigNum<16, 3> MyBigNum;
	MyBigNum a, b, c, x, y;
	for (i=1; i<16*3; ++i) {
		for (j=1; j<16*3; ++j) {
			if (i % 2 == 0 && j % 2 == 0) continue;
			for (k=1; k<16*3; ++k) {
				a = i;
				b = j;
				c = k;
				bool res = MyBigNum::linear_diophantine(a, b, c, &x, &y);
				if (res) {
					m = x.value();
					n = y.value();
					//printf("%u * %u - %u * %u = %u\n", i, m, j, n, k);
					assert(i * m - j * n == k);
				}
			}
		}
	}
}

void suite() {
	test_assign();
	test_add();
	test_mul();
	test_sub();
	test_div();
	test_div_long();
	test_div2();
	test_pow();
	test_sqrt();
	test_extended_binary_euclidean();
	test_extended_binary_euclidean_large();
	test_linear_diophantine();
}

int main() {
	suite();
	printf("Success\n");
	return 0;
}

