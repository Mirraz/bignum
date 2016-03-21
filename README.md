# bignum
`BigNum` template class with long arithmetic operations and algorithms. Only positive numbers are supported.

## Description
| file | description |
| --- | --- |
| `bignum.h` | template class, uses `c++11` standard |
| `bignum_tests.cpp` | tests and usage examples, **compile** it by `make` command |

## bignum.h template class

### typedefs
| typedef | description |
| --- | --- |
| `len_type` | type of bignumber length (count of digits) |
| `digit_type` | type of digit < `BASE` |
| `operation_type` | type of `BASE` and for intermediate operations with carry, `(BASE)^2-1` must fit in it |
| `dec_len_type` | type of count of decimal digits in decimal representation of bignumber |
| `dec_digit_type` | type of decimal digit (0-9) |
| `pow_exp_type` | type which can store `log(BASE)/log(10)` |

### defines
| define | description |
| --- | --- |
| `LEN_TYPE_MAX` | maximum value of variable of type `len_type` |
| `DIGIT_TYPE_MAX` | maximum value of variable of type `digit_type` |
| `DEC_LEN_TYPE_MAX` | maximum value of variable of type `dec_len_type` |
| `LEN_TYPE_MAX_MASK` | most significant bit mask for `len_type` |

### Template parameters
| parameter | description | optional |
| --- | --- | --- |
| `BASE` |  long arithmetic base | |
| `MAX_LEN` | maximal bignumber length (count of digits) | |
| `IS_BASE_DECIMAL` | whether `BASE` is power of 10 | optional, will be calculated at compile-time if omitted |
| `MAX_DECIMAL_LEN` | maximal count of decimal digits in decimal representation of bignumber | optional, will be calculated at compile-time if omitted |
| `BASE_DECIMAL_LEN` | if `BASE` is power of 10 then `BASE_DECIMAL_LEN` is the exponent of this power i.e. length of decimal representation of `BASE`, else it must be `0` | optional, will be calculated at compile-time if omitted |

### Operations, operators and other methods
| name | description | limitations |
| --- | --- | --- |
| `BigNum`, `=` | constructors and assign operators from basic integer or other bignum | |
| `value` | get basic integer value | bignum value must fit in basic integer type |
| `fprintd`, `printd` | print decimal representation | |
| `<`, `<=`, `>`, `>=`, `==`, `!=` | comparison with basic integer or other bignum | |
| `+`, `+=` | addition with basic integer or other bignum | sum must fit in bignum |
| `-`, `-=` | subtraction of basic integer or other bignum | minuend >= subtrahend |
| `*`, `*=` | multiplication with basic integer or other bignum | product must fit in bignum |
| `div`, `/`, `/=`, `%`, `%=` | division with remainder by basic integer or other bignum (long division) | divisor is not zero |
| `div2`, `div2_assign`, `is_even`, `is_odd` | division by 2 and parity | |
| `pow`, `pow_assign` | fast exponentiation by squaring | power must fit in bignum |
| `min`, `min`, `swap` | min, max and swap utility methods | |

### Algorithms
| name | description | limitations |
| --- | --- | --- |
| `square_root` | finds `floor(sqrt(n))` | bignumber length <= `MAX_LEN-1` |
| `extended_binary_euclidean` | extended euclidean algorithm implemented by binary GCD algorithm | for now works only for not both even numbers |
| `linear_diophantine`| solves diophantine equation `a*x - b*y = c` | for now works only for not both even `a` and `b` |

