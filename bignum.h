#ifndef BIGNUM_H
#define BIGNUM_H

#include <stdio.h>
#include <stdint.h>
#ifndef NDEBUG
#  include <inttypes.h>
#endif
#include <math.h>

typedef uint_fast16_t len_type;
typedef uint32_t      digit_type;
typedef uint_fast64_t operation_type;
typedef uint_fast16_t dec_len_type;
typedef uint8_t       dec_digit_type;
#ifndef NDEBUG
#  define LEN_PRINT "%" PRIuFAST16
#  define DIGIT_PRINT "%" PRIu32
#endif

template<operation_type BASE, len_type MAX_LEN, dec_len_type MAX_DECIMAL_LEN>
class BigNum {
	static_assert(BASE <= UINT32_MAX, "BASE is too large");
	static_assert(MAX_LEN < UINT_FAST16_MAX, "MAX_LEN is too large");
	
private:
	len_type len;
	digit_type digits[MAX_LEN];
	
public:
	BigNum() : len(0) {}
	
	// n may be > BASE
	BigNum(operation_type n) {
		len_type i = 0;
		while (n > 0) {
			assert(i < MAX_LEN);
			digits[i++] = n % BASE;
			n /= BASE;
		}
		len = i;
	}
	
	BigNum(const len_type b_len, const digit_type b_digits[]) : len(b_len) {
		for (len_type i=0; i<b_len; ++i) digits[i] = b_digits[i];
	}
	
	BigNum(const BigNum &b) : BigNum(b.len, b.digits) {}
	
	digit_type value() const {
		if (len == 0) return 0;
		operation_type n = 0;
		for (len_type i=len-1;; --i) {
			n += digits[i];
			if (i == 0) break;
			n *= BASE;
		}
		return n;
	}
	
	static void swap(BigNum &a, BigNum &b) {
		// TODO: not efficient
		BigNum tmp;
		tmp = a; a = b; b = tmp;
	}
	
	#ifndef NDEBUG
	void fdump(FILE *stream) const {
		fprintf(stream, LEN_PRINT "[", len);
		if (len > 0) {
			for (len_type i=len-1;; --i) {
				fprintf(stream, DIGIT_PRINT, digits[i]);
				if (i != 0) fprintf(stream, " ");
				else break;
			}
		}
		fprintf(stream, "]");
	}
	
	void dump() const {
		fdump(stdout);
	}
	#endif
	
	// print decimal
	void fprintd(FILE *stream) const {
		BigNum cur(*this);
		dec_digit_type decimal[MAX_DECIMAL_LEN];
		dec_len_type i = 0;
		digit_type denom;
		while (cur > 0) {
			cur = cur.div(10, &denom);
			decimal[i++] = denom;
		}
		if (i == 0) {
			fputc('0', stream);
			return;
		}
		--i;
		while (true) {
			fputc(decimal[i] + '0', stream);
			if (i == 0) break;
			--i;
		}
	}
	
	// print decimal
	void printd() const {
		fprintd(stdout);
	}
	
	bool operator <(const digit_type b) const {
		assert(b < BASE);
		if (len > 1) return false;
		if (len == 1) return digits[0] < b;
		return 0 < b; // if (len == 0)
	}
	
	bool operator <=(const digit_type b) const {
		assert(b < BASE);
		if (len > 1) return false;
		if (len == 1) return digits[0] <= b;
		return true; // if (len == 0)
	}
	
	bool operator >(const digit_type b) const {
		assert(b < BASE);
		if (len > 1) return true;
		if (len == 1) return digits[0] > b;
		return false; // if (len == 0)
	}
	
	bool operator >=(const digit_type b) const {
		assert(b < BASE);
		if (len > 1) return true;
		if (len == 1) return digits[0] >= b;
		return 0 >= b; // if (len == 0)
	}
	
	bool operator ==(const digit_type b) const {
		assert(b < BASE);
		if (len > 1) return false;
		if (len == 1) return digits[0] == b;
		return 0 == b; // if (len == 0)
	}
	
	bool operator !=(const digit_type b) const {
		assert(b < BASE);
		if (len > 1) return true;
		if (len == 1) return digits[0] != b;
		return 0 != b; // if (len == 0)
	}
	
	bool operator <(const BigNum &b) const {
		if (len != b.len) return len < b.len;
		if (len == 0) return false;
		for (len_type i=len-1;; --i) {
			if (digits[i] < b.digits[i]) return true;
			if (digits[i] > b.digits[i]) return false;
			if (i == 0) break;
		}
		return false;
	}
	
	bool operator <=(const BigNum &b) const {
		if (len != b.len) return len < b.len;
		if (len == 0) return true;
		for (len_type i=len-1;; --i) {
			if (digits[i] < b.digits[i]) return true;
			if (digits[i] > b.digits[i]) return false;
			if (i == 0) break;
		}
		return true;
	}
	
	bool operator >(const BigNum &b) const {
		if (len != b.len) return len > b.len;
		if (len == 0) return false;
		for (len_type i=len-1;; --i) {
			if (digits[i] < b.digits[i]) return false;
			if (digits[i] > b.digits[i]) return true;
			if (i == 0) break;
		}
		return false;
	}
	
	bool operator >=(const BigNum &b) const {
		if (len != b.len) return len > b.len;
		if (len == 0) return true;
		for (len_type i=len-1;; --i) {
			if (digits[i] < b.digits[i]) return false;
			if (digits[i] > b.digits[i]) return true;
			if (i == 0) break;
		}
		return true;
	}
	
	bool operator ==(const BigNum &b) const {
		if (len != b.len) return false;
		for (len_type i=0; i<len; ++i) {
			if (digits[i] != b.digits[i]) return false;
		}
		return true;
	}
	
	bool operator !=(const BigNum &b) const {
		if (len != b.len) return true;
		for (len_type i=0; i<len; ++i) {
			if (digits[i] == b.digits[i]) return false;
		}
		return true;
	}
	
	BigNum operator+(const BigNum &b) const {
		BigNum result;
		operation_type overflow = 0;
		operation_type summ;
		len_type i;
		for (i = 0; i<len && i<b.len; ++i) {
			summ = (operation_type)digits[i] + (operation_type)b.digits[i] + overflow;
			if (summ < BASE) {overflow = 0;}
			else {summ -= BASE; overflow = 1;}
			result.digits[i] = summ;
		}
		for (; i<b.len; ++i) {
			summ = (operation_type)b.digits[i] + overflow;
			if (summ < BASE) {overflow = 0;}
			else {summ -= BASE; overflow = 1;}
			result.digits[i] = summ;
		}
		for (; i<len && overflow > 0; ++i) {
			summ = (operation_type)digits[i] + overflow;
			if (summ < BASE) {overflow = 0;}
			else {summ -= BASE; overflow = 1;}
			result.digits[i] = summ;
		}
		if (overflow == 0) {
			for (; i<len; ++i) {
				result.digits[i] = digits[i];
			}
		} else {
			assert(i < MAX_LEN);
			result.digits[i++] = overflow;
		}
		result.len = i;
		return result;
	}
	
	// b may be > BASE
	BigNum operator+(const operation_type b) const {
		BigNum num_b(b);
		return *this + num_b;
	}
	
	BigNum& operator +=(const BigNum &b) {
		// TODO: not efficient
		BigNum result = (*this) + b;
		(*this) = result;
		return *this;
	}
	
	// b may be > BASE
	BigNum& operator +=(const operation_type b) {
		BigNum num_b(b);
		return *this += num_b;
	}
	
	// self += b * BASE^exp * coef
	void add_mul(const BigNum &b, const len_type exp, const operation_type coef) {
		assert(exp <= b.len + exp); // detect overflow
		assert(b.len + exp <= MAX_LEN);
		assert(coef < BASE);
		if (b.len == 0 || coef == 0) return;
		operation_type overflow = 0;
		operation_type res;
		len_type i, j;
		for (i = len; i<exp; ++i) digits[i] = 0;
		for (i = exp, j = 0; i<len && j<b.len; ++i, ++j) {
			res = (operation_type)digits[i] + (operation_type)b.digits[j] * coef + overflow;
			digits[i] = res % BASE;
			overflow = res / BASE;
		}
		for (; j<b.len; ++i, ++j) {
			res = (operation_type)b.digits[j] * coef + overflow;
			digits[i] = res % BASE;
			overflow = res / BASE;
		}
		for (; i<len && overflow > 0; ++i) {
			res = (operation_type)digits[i] + overflow;
			digits[i] = res % BASE;
			overflow = res / BASE;
		}
		if (overflow > 0) {
			assert(i < MAX_LEN);
			digits[i++] = overflow;
		}
		if (len < i) len = i;
	}
	
	BigNum operator*(const BigNum &b) const {
		assert(len <= len + b.len); // detect overflow
		assert(len + b.len <= MAX_LEN + 1);
		BigNum result(0);
		for (len_type i=0; i<b.len; ++i) {
			result.add_mul(*this, i, b.digits[i]);
		}
		return result;
	}
	
	// b may be > BASE
	BigNum operator*(const operation_type b) const {
		BigNum num_b(b);
		return *this * num_b;
	}
	
	BigNum& operator *=(const BigNum &b) {
		// TODO: not efficient
		BigNum result = (*this) * b;
		(*this) = result;
		return *this;
	}
	
	// b may be > BASE
	BigNum& operator *=(const operation_type b) {
		BigNum num_b(b);
		return *this *= num_b;
	}
	
	BigNum operator-(const BigNum &b) const {
		assert(b.len <= len);
		BigNum result(0);
		if (len == 0) return result;
		operation_type carry = 0;
		operation_type subtr, res;
		len_type i, j = 0;
		for (i=0; i<b.len; ++i) {
			subtr = (operation_type)b.digits[i] + carry;
			if ((operation_type)digits[i] >= subtr) {
				res = (operation_type)digits[i] - subtr;
				carry = 0;
			} else {
				res = (operation_type)digits[i] + (BASE - subtr);
				carry = 1;
			}
			if (res > 0) j = i + 1;
			result.digits[i] = res;
		}
		for (; i<len && carry > 0; ++i) {
			if ((operation_type)digits[i] >= carry) {
				res = (operation_type)digits[i] - carry;
				carry = 0;
				if (res > 0) j = i + 1;
			} else {
				res = (operation_type)digits[i] + (BASE - carry);
				carry = 1;
				j = i + 1;
			}
			result.digits[i] = res;
		}
		if (i < len) {
			for (; i<len; ++i) {
				result.digits[i] = digits[i];
			}
			j = len;
		}
		assert(carry == 0);
		assert(i > 0);
		result.len = j;
		return result;
	}
	
	// b may be > BASE
	BigNum operator-(const operation_type b) const {
		BigNum num_b(b);
		return *this - num_b;
	}
	
	BigNum& operator -=(const BigNum &b) {
		// TODO: not efficient
		BigNum result = (*this) - b;
		(*this) = result;
		return *this;
	}
	
	// b may be > BASE
	BigNum& operator -=(const operation_type b) {
		BigNum num_b(b);
		return *this -= num_b;
	}
	
	BigNum div(const digit_type b, digit_type *denom) const {
		assert(b < BASE);
		assert(b > 0);
		BigNum result(0);
		if (len == 0) {
			*denom = 0;
			return result;
		}
		result.len = len;
		if (digits[len-1] < b) --result.len;
		operation_type carry = 0;
		operation_type res;
		for (len_type i=len-1;; --i) {
			res = (operation_type)digits[i] + carry * BASE;
			carry = res % b;
			result.digits[i] = res / b;
			assert(result.digits[i] < BASE);
			if (i == 0) break;
		}
		*denom = carry;
		return result;
	}
	
	BigNum operator/(const digit_type b) const {
		digit_type denom;
		return div(b, &denom);
	}
	
	void div2() {
		// TODO: not efficient
		BigNum result = (*this) / 2;
		(*this) = result;
	}
	
	static_assert(BASE % 2 == 0, "BASE is not even");
	
	bool is_even() const {
		return (len == 0 || !(digits[0] & 1));
	}
	
	bool is_odd() const {
		return (len != 0 && (digits[0] & 1));
	}
	
	static BigNum square_root(const BigNum &n) {
		assert(n.len <= MAX_LEN - 1);
		if (n.len == 0) return 0;
		if (n.len == 1) return (digit_type)sqrt(n.digits[0]);
		
		BigNum l, r;
		digit_type leading = n.digits[n.len-1];
		if (n.len % 2 == 1) {
			l.len = r.len = (n.len+1) / 2;
			assert(l.len > 0);
			for (len_type i=0; i<l.len-1; ++i) l.digits[i] = r.digits[i] = 0;
			
			l.digits[l.len-1] = floor(sqrt(leading));
			r.digits[r.len-1] = ceil (sqrt(leading+1));
		} else {
			l.len = r.len = n.len / 2;
			assert(l.len > 0);
			for (len_type i=0; i<l.len-1; ++i) l.digits[i] = r.digits[i] = 0;
			
			digit_type l_leading = floor(sqrt(leading) * sqrt(BASE));
			assert(l_leading < BASE);
			l.digits[l.len-1] = l_leading;
			
			digit_type r_leading = (
				leading + 1 == BASE ?
				BASE :
				ceil(sqrt(leading+1) * sqrt(BASE))
			);
			assert(r_leading <= BASE);
			if (r_leading == BASE) {
				r.digits[r.len-1] = 0;
				++r.len;
				r_leading = 1;
			}
			r.digits[r.len-1] = r_leading;
		}
		
		BigNum m, m_sq;
		while (l + 1 < r) {
			m = (r + l) / 2;
			m_sq = m * m;
			if (m_sq < n) {
				l = m;
			} else if (n < m_sq) {
				r = m;
			} else {
				l = r = m;
				break;
			}
		}
		
		assert(r <= l + 1);
		assert(l * l <= n);
		assert(r.len > MAX_LEN / 2 || n <= r * r);
		
		return l;
	}
	
	// solves a*x - b*y = gcd(a,b)
	// a, b > 0 and are not both even
	static void extended_binary_euclidean(
		const BigNum &a, const BigNum &b,
		BigNum *x, BigNum *y, BigNum *gcd
	) {
		assert(a > 0 && b > 0);
		assert(!( a.is_even() && b.is_even() ));
		assert(a.len < MAX_LEN && b.len < MAX_LEN);
		
		BigNum r0 = a, r1 = b;
		BigNum s0 = 1, s1 = b;
		BigNum t0 = 0, t1 = a - 1;
		BigNum tmp;
		
		while (r0.is_even()) {
			r0.div2();
			if (s0.is_even() && t0.is_even()) {
				s0.div2();
				t0.div2();
			} else {
				s0 += b; assert(s0.is_even()); s0.div2();
				t0 += a; assert(t0.is_even()); t0.div2();
			}
		}
		
		while (1) {
			while (r1.is_even()) {
				r1.div2();
				if (s1.is_even() && t1.is_even()) {
					s1.div2();
					t1.div2();
				} else {
					s1 += b; assert(s1.is_even()); s1.div2();
					t1 += a; assert(t1.is_even()); t1.div2();
				}
			}
			if (r0 > r1) {
				swap(r0, r1);
				swap(s0, s1);
				swap(t0, t1);
			} else if (r0 == r1) {
				break;
			}
			r1 -= r0;
			if (s1 >= s0 && t1 >= t0) {
				s1 -= s0;
				t1 -= t0;
			} else {
				assert(s0 <= b);
				assert(t0 <= a);
				s1 += b - s0;
				t1 += a - t0;
			}
		};
		
		//assert(a * s1 - b * t1 == r1);
		assert(s1 < b || t1 < a);
		*x = s1;
		*y = t1;
		*gcd = r1;
	}
};

#endif/*BIGNUM_H*/

