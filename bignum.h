#ifndef BIGNUM_H
#define BIGNUM_H

#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#ifndef NDEBUG
#  include <inttypes.h>
#endif
#include <math.h>
#include <algorithm>

typedef uint_fast16_t len_type;
typedef uint32_t      digit_type;
typedef uint_fast64_t operation_type;
typedef uint_fast16_t dec_len_type;
typedef uint8_t       dec_digit_type;
typedef uint_fast8_t  pow_exp_type;
#define LEN_TYPE_MAX UINT_FAST16_MAX
#define DIGIT_TYPE_MAX UINT32_MAX
#define DEC_LEN_TYPE_MAX UINT_FAST16_MAX
#define LEN_TYPE_MAX_MASK (((len_type)1)<<15)
#ifndef NDEBUG
#  define LEN_PRINT "%" PRIuFAST16
#  define DIGIT_PRINT "%" PRIu32
#endif

constexpr pow_exp_type log_floor_constexpr(operation_type base, operation_type pow) {
	return (
		pow < base ?
		0 :
		log_floor_constexpr(base, pow/base) + 1
	);
}

constexpr bool exact_pow_constexpr(operation_type base, operation_type pow) {
	return (
		pow % base != 0 ?
		false :
		(
			pow == base ?
			true :
			exact_pow_constexpr(base, pow/base)
		)
	);
}

template<
	operation_type BASE,
	len_type MAX_LEN,
	bool IS_BASE_DECIMAL = exact_pow_constexpr(10, BASE),
	// MAX_DECIMAL_LEN may be not optimal
	// optimal MAX_DECIMAL_LEN = ceil( log(BASE) / log(10) * MAX_LEN )
	dec_len_type MAX_DECIMAL_LEN = (
		IS_BASE_DECIMAL ?
		0 :
		((dec_len_type)log_floor_constexpr(10, BASE) + 1) * (dec_len_type)MAX_LEN
	),
	pow_exp_type BASE_DECIMAL_LEN = (
		IS_BASE_DECIMAL ?
		log_floor_constexpr(10, BASE) :
		0
	)
>
class BigNum {
	static_assert(BASE > 1, "BASE is too small");
	static_assert(BASE - 1 <= DIGIT_TYPE_MAX, "BASE is too large");
	static_assert(MAX_LEN > 0, "MAX_LEN is too small");
	static_assert(MAX_LEN < LEN_TYPE_MAX, "MAX_LEN is too large");
	static_assert(IS_BASE_DECIMAL == !(MAX_DECIMAL_LEN > 0), "MAX_DECIMAL_LEN is wrong");
	static_assert(IS_BASE_DECIMAL == (BASE_DECIMAL_LEN > 0), "BASE_DECIMAL_LEN is wrong");
	static_assert(MAX_DECIMAL_LEN < DEC_LEN_TYPE_MAX, "MAX_DECIMAL_LEN is too large");
	
private:
	digit_type digits[MAX_LEN];
	len_type len;
	
public:
	BigNum() : len(0) {}
	
	// n may be > BASE
	BigNum(operation_type n) {
		// TODO: not efficient if always n < BASE
		len_type i = 0;
		while (n > 0) {
			assert(i < MAX_LEN);
			digits[i++] = n % BASE;
			n /= BASE;
		}
		len = i;
	}
	
private:
	void assign(const len_type b_len, const digit_type b_digits[]) {
		assert(b_len <= MAX_LEN);
		len = b_len;
		std::copy(b_digits, b_digits+b_len, digits);
	}
	
	void assign(const BigNum &b) {
		assign(b.len, b.digits);
	}
	
public:
	BigNum(const len_type b_len, const digit_type b_digits[]) {
		assign(b_len, b_digits);
	}
	
	BigNum(const BigNum &b) : BigNum(b.len, b.digits) {}
	
	BigNum(BigNum &&b) : BigNum(b.len, b.digits) {
		b.len = 0;
	}
	
	BigNum& operator=(const BigNum &b) {
		if (this != &b) {
			assign(b);
		}
		return *this;
	}
	
	BigNum& operator=(BigNum &&b) {
		if (this != &b) {
			assign(b);
			b.len = 0;
		}
		return *this;
	}
	
	template<operation_type B, len_type L, bool ID, dec_len_type DL, pow_exp_type BDL>
	BigNum<B, L, ID, DL, BDL> clone_template() const {
		return BigNum<B, L, ID, DL, BDL>(len, digits);
	}
	
	template<operation_type B, len_type L, bool ID, dec_len_type DL, pow_exp_type BDL>
	BigNum(const BigNum<B, L, ID, DL, BDL> &b) :
		BigNum(
			b.clone_template<
				BASE,
				MAX_LEN,
				IS_BASE_DECIMAL,
				MAX_DECIMAL_LEN,
				BASE_DECIMAL_LEN
			>()
		) {}
	
	operation_type value() const {
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
		BigNum tmp(a); a = b; b = tmp;
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
		if (len == 0) {
			fputc('0', stream);
			return;
		}
		if (IS_BASE_DECIMAL) {
			digit_type cur, dec_digit_mask, dec_digit;
			bool leading_zeros = true;
			for (len_type i=len-1;; --i) {
				cur = digits[i];
				dec_digit_mask = BASE / 10;
				for (pow_exp_type j=0; j<BASE_DECIMAL_LEN; ++j) {
					dec_digit = cur / dec_digit_mask;
					assert(dec_digit < 10);
					cur %= dec_digit_mask;
					dec_digit_mask /= 10;
					if (leading_zeros) leading_zeros = (dec_digit == 0);
					if (!leading_zeros)
						fputc(dec_digit + '0', stream);
				}
				assert(dec_digit_mask == 0);
				assert(cur == 0);
				if (i == 0) break;
			}
		} else if (BASE > 10) {
			BigNum cur(*this);
			dec_digit_type decimal[MAX_DECIMAL_LEN];
			dec_len_type i = 0;
			digit_type remaind;
			while (cur > 0) {
				assert(i < MAX_DECIMAL_LEN);
				cur = cur.div(10, &remaind);
				decimal[i++] = remaind;
			}
			assert(i > 0);
			--i;
			while (true) {
				fputc(decimal[i] + '0', stream);
				if (i == 0) break;
				--i;
			}
		} else {
			// TODO
			static_assert(BASE >= 10, "BASE is less than 10");
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
	
	// may be result === a
	static void add_static(BigNum &result, const BigNum &a, const BigNum &b) {
		operation_type overflow = 0;
		operation_type summ;
		len_type i;
		for (i = 0; i<a.len && i<b.len; ++i) {
			summ = (operation_type)a.digits[i] + (operation_type)b.digits[i] + overflow;
			if (summ < BASE) {overflow = 0;}
			else {summ -= BASE; overflow = 1;}
			result.digits[i] = summ;
		}
		const BigNum &l = (a.len < b.len ? b : a); // longest
		for (; i<l.len && overflow > 0; ++i) {
			summ = (operation_type)l.digits[i] + overflow;
			if (summ < BASE) {overflow = 0;}
			else {summ -= BASE; overflow = 1;}
			result.digits[i] = summ;
		}
		if (overflow == 0) {
			if (&result == &l) {
				i = l.len;
			} else {
				for (; i<l.len; ++i) result.digits[i] = l.digits[i];
			}
		} else {
			assert(i < MAX_LEN);
			result.digits[i++] = overflow;
		}
		result.len = i;
	}
	
	BigNum operator+(const BigNum &b) const {
		BigNum result;
		add_static(result, *this, b);
		return result;
	}
	
	BigNum& operator +=(const BigNum &b) {
		add_static(*this, *this, b);
		return *this;
	}
	
	// b may be > BASE
	BigNum operator+(const operation_type b) const {
		return *this + BigNum(b);
	}
	
	// b may be > BASE
	BigNum& operator +=(const operation_type b) {
		return *this += BigNum(b);
	}
	
	// self += b * BASE^exp * coef
	void add_mul_assign(const BigNum &b, const len_type exp, const operation_type coef) {
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
			result.add_mul_assign(*this, i, b.digits[i]);
		}
		return result;
	}
	
	BigNum& operator *=(const BigNum &b) {
		BigNum result = (*this) * b;
		(*this) = result;
		return *this;
	}
	
	// b may be > BASE
	BigNum operator*(const operation_type b) const {
		return *this * BigNum(b);
	}
	
	// b may be > BASE
	BigNum& operator *=(const operation_type b) {
		return *this *= BigNum(b);
	}
	
	// may be result === a
	static void sub_static(BigNum &result, const BigNum &a, const BigNum &b) {
		assert(b.len <= a.len);
		if (a.len == 0) {result.len = 0; return;}
		operation_type carry = 0;
		operation_type subtr, res;
		len_type i, j = 0;
		for (i=0; i<b.len; ++i) {
			subtr = (operation_type)b.digits[i] + carry;
			if ((operation_type)a.digits[i] >= subtr) {
				res = (operation_type)a.digits[i] - subtr;
				carry = 0;
			} else {
				res = (operation_type)a.digits[i] + (BASE - subtr);
				carry = 1;
			}
			if (res > 0) j = i + 1;
			result.digits[i] = res;
		}
		for (; i<a.len && carry > 0; ++i) {
			if ((operation_type)a.digits[i] >= carry) {
				res = (operation_type)a.digits[i] - carry;
				carry = 0;
				if (res > 0) j = i + 1;
			} else {
				res = (operation_type)a.digits[i] + (BASE - carry);
				carry = 1;
				j = i + 1;
			}
			result.digits[i] = res;
		}
		if (i < a.len) {
			if (&result == &a) {
				i = a.len;
			} else {
				for (; i<a.len; ++i) result.digits[i] = a.digits[i];
			}
			j = a.len;
		}
		assert(carry == 0);
		assert(i > 0);
		result.len = j;
	}
	
	BigNum operator-(const BigNum &b) const {
		BigNum result;
		sub_static(result, *this, b);
		return result;
	}
	
	BigNum& operator -=(const BigNum &b) {
		sub_static(*this, *this, b);
		return *this;
	}
	
	// b may be > BASE
	BigNum operator-(const operation_type b) const {
		return *this - BigNum(b);
	}
	
	// b may be > BASE
	BigNum& operator -=(const operation_type b) {
		return *this -= BigNum(b);
	}
	
	// may be result === a
	static void div_static(BigNum &result, const BigNum &a, const digit_type b, digit_type *remaind) {
		assert(b < BASE);
		assert(b > 0);
		if (a.len == 0) {
			*remaind = 0;
			result.len = 0;
			return;
		}
		len_type result_len = a.len;
		if (a.digits[a.len-1] < b) --result_len;
		operation_type carry = 0;
		operation_type res;
		for (len_type i=a.len-1;; --i) {
			res = (operation_type)a.digits[i] + carry * BASE;
			carry = res % b;
			result.digits[i] = res / b;
			assert(result.digits[i] < BASE);
			if (i == 0) break;
		}
		*remaind = carry;
		result.len = result_len;
	}
	
	BigNum div(const digit_type b, digit_type *remaind) const {
		BigNum result;
		div_static(result, *this, b, remaind);
		return result;
	}
	
	BigNum operator/(const digit_type b) const {
		digit_type remaind;
		return div(b, &remaind);
	}
	
	BigNum& operator/=(const digit_type b) {
		digit_type remaind;
		div_static(*this, *this, b, &remaind);
		return *this;
	}
	
	// may be result === a
	static void div2_static(BigNum &result, const BigNum &a) {
		if (!(BASE & (BASE-1))) { // BASE is power of 2
			if (a.len == 0) {result.len = 0; return;}
			digit_type carry = 0;
			bool next_carry;
			for (len_type i=a.len-1;; --i) {
				next_carry = a.digits[i] & 1;
				result.digits[i] = carry | (a.digits[i] >> 1);
				carry = (next_carry ? BASE/2 : 0);
				if (i == 0) break;
			}
			if (result.digits[a.len-1] == 0) result.len = a.len-1;
			else result.len = a.len;
		} else {
			digit_type remaind;
			div_static(result, a, 2, &remaind);
		}
	}
	
	BigNum div2() const {
		BigNum result;
		div2_static(result, *this);
		return result;
	}
	
	void div2_assign() {
		div2_static(*this, *this);
	}
	
	static_assert(BASE % 2 == 0, "BASE is not even");
	
	bool is_even() const {
		return (len == 0 || !(digits[0] & 1));
	}
	
	bool is_odd() const {
		return (len != 0 && (digits[0] & 1));
	}
	
private:
	// find max x: b * x <= cur_value
	static digit_type div_find_digit(const BigNum &b, const BigNum &cur_value) {
		operation_type x = 0;
		operation_type l = 0, r = BASE, m;
		while (l <= r) {
			m = (l + r) >> 1;
			if (b * m <= cur_value) {
				x = m;
				l = m + 1;
			} else {
				r = m - 1;
			}
		}
		assert(b * x <= cur_value);
		return x;
	}
	
public:
	// may be result === a
	static void div_static(BigNum &result, const BigNum &a, const BigNum &b, BigNum *remaind) {
		assert(b.len > 0);
		assert(b.len < MAX_LEN);
		if (a.len == 0) {
			remaind->len = 0;
			result.len = 0;
			return;
		}
		BigNum cur_value(0);
		operation_type x;
		len_type j = 0;
		for (len_type i = a.len-1;; i--) {
			cur_value.digits[0] = a.digits[i];
			if (cur_value.len == 0 && cur_value.digits[0] > 0) cur_value.len = 1;
			x = div_find_digit(b, cur_value);
			cur_value -= b * x;
			if (j == 0 && x > 0) j = i + 1;
			result.digits[i] = x;
			if (i == 0) break;
			cur_value.shift_left_assign(1);
		}
		result.len = j;
		*remaind = cur_value;
	}

	BigNum div(const BigNum &b, BigNum *remaind) const {
		BigNum result;
		div_static(result, *this, b, remaind);
		return result;
	}
	
	BigNum operator/(const BigNum &b) const {
		BigNum remaind;
		return div(b, &remaind);
	}
	
	BigNum& operator /=(const BigNum &b) {
		BigNum remaind;
		div_static(*this, *this, b, &remaind);
		return *this;
	}
	
	BigNum operator%(const BigNum &b) const {
		BigNum remaind;
		div(b, &remaind);
		return remaind;
	}
	
	BigNum& operator %=(const BigNum &b) {
		BigNum result = (*this) % b;
		(*this) = result;
		return *this;
	}
	
	// may be result === a
	static void shift_left_static(BigNum &result, const BigNum &a, const len_type exp) {
		assert(a.len <= a.len + exp); // detect overflow
		assert(a.len + exp <= MAX_LEN);
		if (a.len == 0) {result.len = 0; return;}
		if (exp == 0) return;
		len_type i;
		for (i = a.len+exp-1; i>=exp; --i) {
			result.digits[i] = a.digits[i-exp];
		}
		for (;; --i) {
			result.digits[i] = 0;
			if (i == 0) break;
		}
		result.len = a.len + exp;
	}
	
	BigNum shift_left(const len_type exp) const {
		BigNum result;
		shift_left_static(result, *this, exp);
		return shift_left;
	}
	
	void shift_left_assign(const len_type exp) {
		shift_left_static(*this, *this, exp);
	}
	
	// on 0^0 returns 1
	BigNum pow(const len_type exp) const {
		BigNum result(1);
		len_type mask = LEN_TYPE_MAX_MASK;
		while (mask != 0 && !(exp & mask)) mask >>= 1;
		while (mask > 0) {
			result *= result;
			if (mask & exp) {
				result *= (*this);
			}
			mask >>= 1;
		}
		return result;
	}
	
	void pow_assign(const len_type exp) {
		(*this) = pow(exp);
	}
	
	static BigNum min(const BigNum &a, const BigNum &b) {
		if (a <= b) return a;
		return b;
	}
	
	static BigNum max(const BigNum &a, const BigNum &b) {
		if (a >= b) return a;
		return b;
	}
	
	// floor(sqrt(n))
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
		
		while (l + 1 < r) {
			BigNum m((l + r).div2());
			BigNum m_sq(m * m);
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
		
		while (r0.is_even()) {
			r0.div2_assign();
			if (s0.is_even() && t0.is_even()) {
				s0.div2_assign();
				t0.div2_assign();
			} else {
				s0 += b; assert(s0.is_even()); s0.div2_assign();
				t0 += a; assert(t0.is_even()); t0.div2_assign();
			}
		}
		
		while (1) {
			while (r1.is_even()) {
				r1.div2_assign();
				if (s1.is_even() && t1.is_even()) {
					s1.div2_assign();
					t1.div2_assign();
				} else {
					s1 += b; assert(s1.is_even()); s1.div2_assign();
					t1 += a; assert(t1.is_even()); t1.div2_assign();
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
	
	// solves a*x - b*y = c
	// a, b > 0 and are not both even
	// returns: success
	static bool linear_diophantine(
		const BigNum &a, const BigNum &b, const BigNum &c,
		BigNum *x, BigNum *y
	) {
		BigNum xe, ye, gcd;
		extended_binary_euclidean(a, b, &xe, &ye, &gcd);
		// a * xe - b * ye = gcd
		BigNum ar, br, cr, crem;
		cr = c.div(gcd, &crem);
		if (crem != 0) return false;
		ar = a / gcd;
		br = b / gcd;
		// ar * xe - br * ye = 1    | * cr
		
		typedef BigNum<BASE, MAX_LEN*2> DoubleBigNum;
		DoubleBigNum ad = ar, bd = br, cd = cr;
		DoubleBigNum xd = xe, yd = ye;
		xd *= cd;
		yd *= cd;
		// ad * xd - bd * yd = cd
		DoubleBigNum pd = DoubleBigNum::min(xd / bd, yd / ad);
		xd -= bd * pd;
		yd -= ad * pd;
		
		BigNum xr = xd;
		BigNum yr = yd;
		*x = xr;
		*y = yr;
		return true;
	}
};

#endif/*BIGNUM_H*/

