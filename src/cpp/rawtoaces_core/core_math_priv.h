// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the rawtoaces Project.

#pragma once

#include <vector>

namespace rta
{
namespace core
{
namespace math
{

// Contains the declarations of the private functions,
// exposed here for unit-testing.

// XYZ to LAB conversion.
template <typename T> void XYZ_to_LAB( const T ( &in )[3], T ( &out )[3] );

// The derivative of the `XYZ_to_LAB` function, see above.
template <typename T>
void XYZ_to_LAB_prime(
    const T ( &arg )[3], const T ( &arg_prime )[3], T ( &out )[3] );

template <typename T_dst, typename T_src>
std::vector<T_dst> change_type( const std::vector<T_src> &vec );

/// Solve a system of linear equations using Gauss elimination.
bool solve_linear(
    std::vector<std::vector<double>> &a, std::vector<double> &b );

bool solve_spectral_transform(
    const std::vector<std::vector<double>> &source_RGB,
    const std::vector<std::vector<double>> &target_XYZ,
    double                                  error_threshold,
    size_t                                  max_steps,
    int                                     verbosity,
    std::vector<std::vector<double>>       &out_IDT_matrix );

} // namespace math
} // namespace core
} // namespace rta
