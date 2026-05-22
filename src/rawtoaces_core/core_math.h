// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the rawtoaces Project.

#pragma once

#include <vector>

namespace rta
{
namespace core
{

bool inverse(
    const std::vector<std::vector<double>> &src_mat,
    std::vector<std::vector<double>>       &dst_mat );

std::vector<std::vector<double>>
transposeVec( const std::vector<std::vector<double>> &vMtx );

double sumVector( const std::vector<double> &vct );

double sumVectorM( const std::vector<std::vector<double>> &vct );

void scaleVector( std::vector<double> &vct, const double scale );

/// Calculate matrix-matrix multiplication.
///
/// @param vct1 left side multiplicant matrix
/// @param vct2 right side multiplicant matrix
/// @return the matrix product
template <typename T>
std::vector<std::vector<T>> product(
    const std::vector<std::vector<T>> &vct1,
    const std::vector<std::vector<T>> &vct2 );

/// Calculate matrix-vector multiplication.
///
/// @param vct1 left side multiplicant matrix
/// @param vct2 right side multiplicant vector
/// @return the matrix product
template <typename T>
std::vector<T>
product( const std::vector<std::vector<T>> &vct1, const std::vector<T> &vct2 );

/// Calculate the Sum of Squared Errors (SSE) between two vectors.
/// The SSE measures how well the calculated values (tcp) match the reference values (src).
/// Formula: Σ((tcp[i] / src[i] - 1)²)
///
/// @param tcp The calculated/target values to compare
/// @param src The reference/source values to compare against
/// @return The sum of squared relative errors
/// @pre tcp.size() == src.size()
double
calculate_SSE( const std::vector<double> &tcp, const std::vector<double> &src );

std::vector<double> xy_to_XYZ( const std::vector<double> &xy );

std::vector<double> uv_to_xy( const std::vector<double> &uv );

std::vector<double> uv_to_XYZ( const std::vector<double> &uv );

std::vector<double> XYZ_to_uv( const std::vector<double> &XYZ );

std::vector<std::vector<double>> calculate_CAT(
    const std::vector<double> &src_white_XYZ,
    const std::vector<double> &dst_white_XYZ,
    bool                       use_bradford );

template <typename T>
std::vector<std::vector<T>>
XYZ_to_LAB( const std::vector<std::vector<T>> &XYZ );

template <typename T>
std::vector<std::vector<T>>
getCalcXYZt( const std::vector<std::vector<T>> &RGB, const T beta_params[6] );

/// Perform curve fitting optimization to find optimal IDT matrix parameters.
/// This function uses the Ceres optimization library to find the best
/// 6-parameter IDT matrix that minimizes the difference between camera RGB
/// responses and target XYZ values across all training patches.
/// The optimization process iteratively adjusts the beta_params parameters
/// to achieve the best color transformation.
///
/// @param source_RGB Camera RGB responses for training patches
/// @param target_XYZ Target XYZ values for training patches
/// @param verbosity Verbosity level for optimization output:
/// - 0-2: No output from solver
/// - 3: Ceres solver full report
/// - 4: Additionally enables Ceres minimizer progress to stdout
/// @param out_IDT_matrix Output IDT matrix computed from optimized parameters
/// @return true if optimization succeeded, false otherwise
bool solve_spectral_transform(
    const std::vector<std::vector<double>> &source_RGB,
    const std::vector<std::vector<double>> &target_XYZ,
    int                                     verbosity,
    std::vector<std::vector<double>>       &out_IDT_matrix );

} // namespace core
} // namespace rta
