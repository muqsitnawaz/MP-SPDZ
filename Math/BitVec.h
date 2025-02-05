/*
 * BitVec.h
 *
 */

#ifndef MATH_BITVEC_H_
#define MATH_BITVEC_H_

#include "Integer.h"
#include "field_types.h"

class BitDiagonal;

template<class T>
class BitVec_ : public IntBase<T>
{
public:
    typedef IntBase<T> super;

    typedef BitVec_ Scalar;

    typedef BitVec_ next;
    typedef BitDiagonal Square;

    static const int n_bits = sizeof(T) * 8;

    static char type_char() { return 'B'; }
    static DataFieldType field_type() { return DATA_GF2; }

    static bool allows(Dtype dtype) { return dtype == DATA_TRIPLE or dtype == DATA_BIT; }

    BitVec_() {}
    BitVec_(long a) : super(a) {}
    BitVec_(const super& a) : super(a) {}
    template<int K>
    BitVec_(const Z2<K>& a) : super(a.get_limb(0)) {}

    BitVec_ operator+(const BitVec_& other) const { return *this ^ other; }
    BitVec_ operator-(const BitVec_& other) const { return *this ^ other; }
    BitVec_ operator*(const BitVec_& other) const { return *this & other; }

    BitVec_ operator/(const BitVec_& other) const { (void) other; throw not_implemented(); }

    BitVec_& operator+=(const BitVec_& other) { *this ^= other; return *this; }
    BitVec_& operator-=(const BitVec_& other) { *this ^= other; return *this; }

    BitVec_ extend_bit() const { return -(this->a & 1); }
    BitVec_ mask(int n) const { return n < n_bits ? *this & ((1L << n) - 1) : *this; }

    template<int t>
    void add(octetStream& os) { *this += os.get<BitVec_>(); }

    void mul(const BitVec_& a, const BitVec_& b) { *this = a * b; }

    void randomize(PRNG& G, int n = n_bits) { super::randomize(G); *this = mask(n); }

    void pack(octetStream& os, int n = n_bits) const { os.store_int(this->a, DIV_CEIL(n, 8)); }
    void unpack(octetStream& os, int n = n_bits) { this->a = os.get_int(DIV_CEIL(n, 8)); }

    static BitVec_ unpack_new(octetStream& os, int n = n_bits)
    {
        BitVec_ res;
        res.unpack(os, n);
        return res;
    }
};

typedef BitVec_<long> BitVec;

#endif /* MATH_BITVEC_H_ */
