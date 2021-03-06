/** @file
 ********************************************************************************
 * @authors    Eli Ben-Sasson, Alessandro Chiesa, Daniel Genkin,
 *             Shaul Kfir, Eran Tromer, Madars Virza.
 * This file is part of libsnark, developed by SCIPR Lab <http://scipr-lab.org>.
 * @copyright  MIT license (see LICENSE file)
 *******************************************************************************/

#ifndef EDWARDS_G1_HPP_
#define EDWARDS_G1_HPP_
#include <vector>
#include "algebra/curves/edwards/edwards_init.hpp"
#include "common/field_utils.hpp"

namespace libsnark {

class edwards_G1;
std::ostream& operator<<(std::ostream &, const edwards_G1&);
std::istream& operator>>(std::istream &, edwards_G1&);

class edwards_G1 {
public:
#ifdef PROFILE_OP_COUNTS
    static long long add_cnt;
    static long long dbl_cnt;
#endif
    static std::vector<size_t> wnaf_window_table;
    static std::vector<size_t> fixed_base_exp_window_table;
    static edwards_G1 G1_zero;
    static edwards_G1 G1_one;

    edwards_Fq X, Y, Z;
    edwards_G1();
private:
    edwards_G1(const edwards_Fq& X, const edwards_Fq& Y, const edwards_Fq& Z) : X(X), Y(Y), Z(Z) {};

public:
    typedef edwards_Fq base_field;
    typedef edwards_Fr scalar_field;
    // using inverted coordinates
    edwards_G1(const edwards_Fq& X, const edwards_Fq& Y) : X(Y), Y(X), Z(X*Y) {};

    void print() const;
    void print_coordinates() const;

    void to_affine_coordinates();
    void to_special();
    bool is_special() const;

    bool is_zero() const;

    bool operator==(const edwards_G1 &other) const;
    bool operator!=(const edwards_G1 &other) const;

    edwards_G1 operator+(const edwards_G1 &other) const;
    edwards_G1 operator-() const;
    edwards_G1 operator-(const edwards_G1 &other) const;

    edwards_G1 add(const edwards_G1 &other) const;
    edwards_G1 add_special(const edwards_G1 &other) const;
    edwards_G1 dbl() const;

    bool is_well_formed() const;

    static edwards_G1 zero();
    static edwards_G1 one();
    static edwards_G1 random_element();

    static size_t size_in_bits() { return edwards_Fq::size_in_bits() + 1; }
    static bigint<base_field::num_limbs> base_field_char() { return base_field::field_char(); }
    static bigint<scalar_field::num_limbs> order() { return scalar_field::field_char(); }

    friend std::ostream& operator<<(std::ostream &out, const edwards_G1 &g);
    friend std::istream& operator>>(std::istream &in, edwards_G1 &g);
};

template<mp_size_t m>
edwards_G1 operator*(const bigint<m> &lhs, const edwards_G1 &rhs)
{
    edwards_G1 res;

    bool found_one = false;
    for (long i = m-1; i >= 0; --i)
    {
        for (long j = GMP_NUMB_BITS - 1; j >= 0; --j)
        {
            if (found_one)
            {
                res = res.dbl();
            }

            if (lhs.data[i] & (1ul<<j))
            {
                found_one = true;
                res = res + rhs;
            }
        }
    }

    return res;
}

template<mp_size_t m, const bigint<m>& modulus_p>
edwards_G1 operator*(const Fp_model<m,modulus_p> &lhs, const edwards_G1 &rhs)
{
    return (lhs.as_bigint()) * rhs;
}

std::ostream& operator<<(std::ostream& out, const std::vector<edwards_G1> &v);
std::istream& operator>>(std::istream& in, std::vector<edwards_G1> &v);

template<typename T>
void batch_to_special_all_non_zeros(std::vector<T> &vec);
template<>
void batch_to_special_all_non_zeros<edwards_G1>(std::vector<edwards_G1> &vec);

} // libsnark
#endif // EDWARDS_G1_HPP_
