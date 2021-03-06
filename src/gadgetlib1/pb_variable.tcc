/** @file
 ********************************************************************************
 * @authors    Eli Ben-Sasson, Alessandro Chiesa, Daniel Genkin,
 *             Shaul Kfir, Eran Tromer, Madars Virza.
 * This file is part of libsnark, developed by SCIPR Lab <http://scipr-lab.org>.
 * @copyright  MIT license (see LICENSE file)
 *******************************************************************************/

#ifndef PB_VARIABLE_TCC_
#define PB_VARIABLE_TCC_
#include <cassert>
#include "gadgetlib1/protoboard.hpp"
#include "common/utils.hpp"

namespace libsnark {

template<typename FieldT>
void pb_variable<FieldT>::allocate(protoboard<FieldT> &pb, const std::string &annotation)
{
    this->index = pb.allocate_var_index(annotation);
}

/* allocates pb_variable<FieldT> array in MSB->LSB order */
template<typename FieldT>
void pb_variable_array<FieldT>::allocate(protoboard<FieldT> &pb, const size_t n, const std::string &annotation_prefix)
{
#ifdef DEBUG
    assert(annotation_prefix != "");
#endif
    (*this).resize(n);

    for (size_t i = 0; i < n; ++i)
    {
        (*this)[i].allocate(pb, FMT(annotation_prefix, "_%zu", i));
    }
}

template<typename FieldT>
void pb_variable_array<FieldT>::fill_with_field_elements(protoboard<FieldT> &pb, const std::vector<FieldT>& vals) const
{
    assert(this->size() == vals.size());
    for (size_t i = 0; i < vals.size(); ++i)
    {
        pb.val((*this)[i]) = vals[i];
    }
}

template<typename FieldT>
void pb_variable_array<FieldT>::fill_with_bits(protoboard<FieldT> &pb, const bit_vector& bits) const
{
    assert(this->size() == bits.size());
    for (size_t i = 0; i < bits.size(); ++i)
    {
        pb.val((*this)[i]) = (bits[i] ? FieldT::one() : FieldT::zero());
    }
}

template<typename FieldT>
void pb_variable_array<FieldT>::fill_with_bits_of_field_element(protoboard<FieldT> &pb, const FieldT &r) const
{
    const bigint<FieldT::num_limbs> rint = r.as_bigint();
    for (size_t i = 0; i < this->size(); ++i)
    {
        pb.val((*this)[i]) = rint.test_bit(i) ? FieldT::one() : FieldT::zero();
    }
}

template<typename FieldT>
void pb_variable_array<FieldT>::fill_with_bits_of_ulong(protoboard<FieldT> &pb, const unsigned long i) const
{
    this->fill_with_bits_of_field_element(pb, FieldT(i));
}

template<typename FieldT>
std::vector<FieldT> pb_variable_array<FieldT>::get_vals(const protoboard<FieldT> &pb) const
{
    std::vector<FieldT> result(this->size());
    for (size_t i = 0; i < this->size(); ++i)
    {
        result[i] = pb.val((*this)[i]);
    }
    return result;
}

template<typename FieldT>
bit_vector pb_variable_array<FieldT>::get_bits(const protoboard<FieldT> &pb) const
{
    bit_vector result;
    for (size_t i = 0; i < this->size(); ++i)
    {
        const FieldT v = pb.val((*this)[i]);
        assert(v == FieldT::zero() || v == FieldT::one());
        result.push_back(v == FieldT::one());
    }
    return result;
}

template<typename FieldT>
FieldT pb_variable_array<FieldT>::get_field_element_from_bits(protoboard<FieldT> &pb) const
{
    FieldT result = FieldT::zero();

    for (size_t i = 0; i < this->size(); ++i)
    {
        /* push in the new bit */
        const FieldT v = pb.val((*this)[this->size()-1-i]);
        assert(v == FieldT::zero() || v == FieldT::one());
        result += result + v;
    }

    return result;
}

template<typename FieldT>
pb_linear_combination<FieldT>::pb_linear_combination()
{
    this->is_variable = false;
}

template<typename FieldT>
pb_linear_combination<FieldT>::pb_linear_combination(const pb_variable<FieldT> &var)
{
    this->is_variable = true;
    this->index = var.index;
    this->terms.emplace_back(linear_term<FieldT>(var));
}

template<typename FieldT>
void pb_linear_combination<FieldT>::assign(protoboard<FieldT> &pb, const linear_combination<FieldT> &lc)
{
    assert(this->is_variable == false);
    this->index = pb.allocate_lc_index();
    this->terms = lc.terms;
}

} // libsnark
#endif // PB_VARIABLE_TCC
