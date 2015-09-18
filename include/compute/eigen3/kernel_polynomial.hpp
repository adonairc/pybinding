#pragma once
#include "support/dense.hpp"
#include "support/sparse.hpp"

namespace compute {

template<class real_t>
inline void kpm_kernel_sum(real_t& result, real_t const& a, real_t const& b) {
    result += a * b;
}

template<class real_t>
inline void kpm_kernel_sum(std::complex<real_t>& result, std::complex<real_t> const& a,
                           std::complex<real_t> const& b) {
    result.real(result.real() + a.real() * b.real() - a.imag() * b.imag());
    result.imag(result.imag() + a.real() * b.imag() + a.imag() * b.real());
}

template<typename scalar_t>
inline void kpm_kernel(int size, SparseMatrixX<scalar_t> const& matrix,
                       VectorX<scalar_t> const& x, VectorX<scalar_t>& y) {
    const auto* const value = matrix.valuePtr();
    const auto* const row_start = matrix.outerIndexPtr();
    const auto* const column_index = matrix.innerIndexPtr();

    for (auto i = 0; i < size; ++i) {
        auto r = -y[i];
        for (auto j = row_start[i]; j < row_start[i + 1]; ++j)
            kpm_kernel_sum(r, value[j], x[column_index[j]]);
        y[i] = r;
    }
}

} // namespace compute
