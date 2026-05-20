// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the rawtoaces Project.

#pragma once

#include "define.h"

#include <Eigen/Core>
#include <ceres/ceres.h>

namespace rta
{
namespace core
{

// Non-class functions

template <typename T>
bool inverse(
    const std::vector<std::vector<T>> &src_mat,
    std::vector<std::vector<T>>       &dst_mat )
{
    size_t rows = src_mat.size();
    if ( rows == 0 )
        return false;

    size_t cols = src_mat[0].size();
    if ( cols == 0 )
        return false;
if (rows != cols) return false;
    Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic> m;
    m.resize( rows, cols );
    for ( Eigen::Index i = 0; i < m.rows(); i++ )
    {
        if ( src_mat[i].size() != cols )
            return false;

        for ( Eigen::Index j = 0; j < m.cols(); j++ )
            m( i, j ) = src_mat[i][j];
    }

    if ( std::fabs( m.determinant() ) < 1e-9 )
        return false;

    m = m.inverse();

    dst_mat.resize( rows );
    for ( Eigen::Index i = 0; i < m.rows(); i++ )
    {
        dst_mat[i].resize( cols );
        for ( Eigen::Index j = 0; j < m.cols(); j++ )
            dst_mat[i][j] = m( i, j );
    }

    return true;
}

template <typename T>
std::vector<std::vector<T>>
transposeVec( const std::vector<std::vector<T>> &vMtx )
{
    assert( vMtx.size() != 0 && vMtx[0].size() != 0 );

    Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic> m;
    m.resize( vMtx.size(), vMtx[0].size() );

    for ( Eigen::Index i = 0; i < m.rows(); i++ )
        for ( Eigen::Index j = 0; j < m.cols(); j++ )
            m( i, j ) = vMtx[i][j];
    m.transposeInPlace();

    std::vector<std::vector<T>> vTran( m.rows(), std::vector<T>( m.cols() ) );
    for ( Eigen::Index i = 0; i < m.rows(); i++ )
        for ( Eigen::Index j = 0; j < m.cols(); j++ )
            vTran[i][j] = m( i, j );

    return vTran;
}

template <typename T> T sumVector( const std::vector<T> &vct )
{
    Eigen::Matrix<T, Eigen::Dynamic, 1> v;
    v.resize( vct.size(), 1 );
    for ( Eigen::Index i = 0; i < v.rows(); i++ )
        v( i, 0 ) = vct[i];

    return v.sum();
}

template <typename T> T sumVectorM( const std::vector<std::vector<T>> &vct )
{
    size_t row = vct.size();
    size_t col = vct[0].size();

    Eigen::Matrix<T, Eigen::Dynamic, 1> v;
    v.resize( row * col, 1 );

    for ( size_t i = 0; i < row; i++ )
        for ( size_t j = 0; j < col; j++ )
            v( i * col + j ) = vct[i][j];

    return v.sum();
}

template <typename T> void scaleVector( std::vector<T> &vct, const T scale )
{
    Eigen::Matrix<T, Eigen::Dynamic, 1> v;
    v.resize( vct.size(), 1 );

    for ( size_t i = 0; i < vct.size(); i++ )
        v( i, 0 ) = vct[i];
    v *= scale;

    for ( size_t i = 0; i < vct.size(); i++ )
        vct[i] = v( i, 0 );

    return;
}

/// Calculate matrix-matrix multiplication.
///
/// @param vct1 left side multiplicant matrix
/// @param vct2 right side multiplicant matrix
/// @return the matrix product
template <typename T>
std::vector<std::vector<T>> product(
    const std::vector<std::vector<T>> &vct1,
    const std::vector<std::vector<T>> &vct2 )
{
    assert( vct1.size() != 0 && vct2.size() != 0 );

    Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic> m1, m2, m3;
    m1.resize( vct1.size(), vct1[0].size() );
    m2.resize( vct2[0].size(), vct2.size() );

    for ( Eigen::Index i = 0; i < m1.rows(); i++ )
        for ( Eigen::Index j = 0; j < m1.cols(); j++ )
            m1( i, j ) = vct1[i][j];
    for ( Eigen::Index i = 0; i < m2.rows(); i++ )
        for ( Eigen::Index j = 0; j < m2.cols(); j++ )
            m2( i, j ) = vct2[i][j];

    m3 = m1 * m2;

    std::vector<std::vector<T>> vct3( m3.rows(), std::vector<T>( m3.cols() ) );
    for ( Eigen::Index i = 0; i < m3.rows(); i++ )
        for ( Eigen::Index j = 0; j < m3.cols(); j++ )
            vct3[i][j] = m3( i, j );

    return vct3;
}

/// Calculate matrix-vector multiplication.
///
/// @param vct1 left side multiplicant matrix
/// @param vct2 right side multiplicant vector
/// @return the matrix product
template <typename T>
std::vector<T>
product( const std::vector<std::vector<T>> &vct1, const std::vector<T> &vct2 )
{
    assert( vct1.size() != 0 && ( vct1[0] ).size() == vct2.size() );

    Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic> m1, m2, m3;
    m1.resize( vct1.size(), vct1[0].size() );
    m2.resize( vct2.size(), 1 );

    for ( Eigen::Index i = 0; i < m1.rows(); i++ )
        for ( Eigen::Index j = 0; j < m1.cols(); j++ )
            m1( i, j ) = vct1[i][j];
    for ( Eigen::Index i = 0; i < m2.rows(); i++ )
        m2( i, 0 ) = vct2[i];

    m3 = m1 * m2;

    std::vector<T> vct3( m3.data(), m3.data() + m3.rows() * m3.cols() );

    return vct3;
}

/// Calculate the Sum of Squared Errors (SSE) between two vectors.
/// The SSE measures how well the calculated values (tcp) match the reference values (src).
/// Formula: Σ((tcp[i] / src[i] - 1)²)
///
/// @param tcp The calculated/target values to compare
/// @param src The reference/source values to compare against
/// @return The sum of squared relative errors
/// @pre tcp.size() == src.size()
template <typename T>
T calculate_SSE( const std::vector<T> &tcp, const std::vector<T> &src )
{
    assert( tcp.size() == src.size() );
    std::vector<T> tmp( src.size() );

    T sum = T( 0.0 );
    for ( size_t i = 0; i < tcp.size(); i++ )
        sum += std::pow( ( tcp[i] / src[i] - 1.0 ), T( 2.0 ) );

    return sum;
}

template <typename T> std::vector<T> xy_to_XYZ( const std::vector<T> &xy )
{
    std::vector<T> XYZ( 3 );
    XYZ[0] = xy[0];
    XYZ[1] = xy[1];
    XYZ[2] = 1 - xy[0] - xy[1];

    return XYZ;
}

template <typename T> std::vector<T> uv_to_xy( const std::vector<T> &uv )
{
    T              scale  = 2 * uv[0] - 8 * uv[1] + 4;
    std::vector<T> result = { 3.0 * uv[0] / scale, 2.0 * uv[1] / scale };
    return result;
}

template <typename T> std::vector<T> uv_to_XYZ( const std::vector<T> &uv )
{
    return xy_to_XYZ( uv_to_xy( uv ) );
} // LCOV_EXCL_LINE - bug in coverage tool

template <typename T> std::vector<T> XYZ_to_uv( const std::vector<T> &XYZ )
{
    T              scale  = XYZ[0] + 15 * XYZ[1] + 3 * XYZ[2];
    std::vector<T> result = { 4.0 * XYZ[0] / scale, 6.0 * XYZ[1] / scale };
    return result;
}

template <typename T>
std::vector<std::vector<T>> calculate_CAT(
    const std::vector<T> &src_white_XYZ,
    const std::vector<T> &dst_white_XYZ,
    bool                  use_bradford )
{
    assert( src_white_XYZ.size() == 3 );
    assert( dst_white_XYZ.size() == 3 );

    // clang-format off
    
    //  Color Adaptation Matrices - Bradford
    static const std::vector<std::vector<double>> Bradford = {
        {  0.8951,  0.2664, -0.1614 },
        { -0.7502,  1.7135,  0.0367 },
        {  0.0389, -0.0685,  1.0296 }
    };
    
    static const std::vector<std::vector<double>> Bradford_inv = {
        {  0.98699290546671225,  -0.14705425642099007,  0.15996265166373122  },
        {  0.43230526972339445,   0.51836027153677744,  0.049291228212855594 },
        { -0.0085286645751773294, 0.040042821654084869, 0.96848669578754998  }
    };

    // Color Adaptation Matrices - CAT02
    static const std::vector<std::vector<double>> CAT02 = {
        {  0.7328, 0.4296, -0.1624 },
        { -0.7036, 1.6975,  0.0061 },
        {  0.0030, 0.0136,  0.9834 }
    };

    static const std::vector<std::vector<double>> CAT02_inv = {
        {  1.0961238208355142,    -0.27886900021828726,   0.18274517938277304  },
        {  0.45436904197535921,    0.47353315430741177,   0.072097803717229125 },
        { -0.0096276087384293551, -0.0056980312161134198, 1.0153256399545427   }
    };
    // clang-format on

    const auto &M1 = use_bradford ? Bradford : CAT02;
    const auto &M2 = use_bradford ? Bradford_inv : CAT02_inv;

    std::vector<double> src_white_LMS = product( M1, src_white_XYZ );
    std::vector<double> dst_white_LMS = product( M1, dst_white_XYZ );

    std::vector<std::vector<double>> mat( 3, std::vector<double>( 3, 0 ) );
    for ( size_t i = 0; i < 3; i++ )
        mat[i][i] = dst_white_LMS[i] / src_white_LMS[i];

    mat = product( mat, M1 );
    mat = product( M2, mat );
    return mat;
}

template <typename T>
std::vector<std::vector<T>> XYZ_to_LAB( const std::vector<std::vector<T>> &XYZ )
{
    assert( !XYZ.empty() );
    assert( XYZ[0].size() == 3 );
    T add = T( 16.0 / 116.0 );

    std::vector<std::vector<T>> tmpXYZ(
        XYZ.size(), std::vector<T>( 3, T( 1.0 ) ) );
    for ( size_t i = 0; i < XYZ.size(); i++ )
        for ( size_t j = 0; j < 3; j++ )
        {
            tmpXYZ[i][j] = XYZ[i][j] / ACES_white_point_XYZ[j];
            if ( tmpXYZ[i][j] > T( e ) )
                tmpXYZ[i][j] = ceres::pow( tmpXYZ[i][j], T( 1.0 / 3.0 ) );
            else
                tmpXYZ[i][j] = T( k ) * tmpXYZ[i][j] + add;
        }

    std::vector<std::vector<T>> outCalcLab( XYZ.size(), std::vector<T>( 3 ) );
    for ( size_t i = 0; i < XYZ.size(); i++ )
    {
        outCalcLab[i][0] = T( 116.0 ) * tmpXYZ[i][1] - T( 16.0 );
        outCalcLab[i][1] = T( 500.0 ) * ( tmpXYZ[i][0] - tmpXYZ[i][1] );
        outCalcLab[i][2] = T( 200.0 ) * ( tmpXYZ[i][1] - tmpXYZ[i][2] );
    }

    return outCalcLab;
}

template <typename T>
std::vector<std::vector<T>>
getCalcXYZt( const std::vector<std::vector<T>> &RGB, const T beta_params[6] )
{
    assert( !RGB.empty() );

    std::vector<std::vector<T>> camera_to_ACES_transposed(
        3, std::vector<T>( 3 ) );
    std::vector<std::vector<T>> ACES_to_XYZ_transposed(
        3, std::vector<T>( 3 ) );

    for ( size_t i = 0; i < 3; i++ )
        for ( size_t j = 0; j < 3; j++ )
            ACES_to_XYZ_transposed[j][i] = T( acesrgb_XYZ_3[i][j] );

    camera_to_ACES_transposed[0][0] = beta_params[0];
    camera_to_ACES_transposed[1][0] = beta_params[1];
    camera_to_ACES_transposed[2][0] = 1.0 - beta_params[0] - beta_params[1];
    camera_to_ACES_transposed[0][1] = beta_params[2];
    camera_to_ACES_transposed[1][1] = beta_params[3];
    camera_to_ACES_transposed[2][1] = 1.0 - beta_params[2] - beta_params[3];
    camera_to_ACES_transposed[0][2] = beta_params[4];
    camera_to_ACES_transposed[1][2] = beta_params[5];
    camera_to_ACES_transposed[2][2] = 1.0 - beta_params[4] - beta_params[5];

    auto calc_ACES = product( RGB, camera_to_ACES_transposed );
    auto calc_XYZ  = product( calc_ACES, ACES_to_XYZ_transposed );

    return calc_XYZ;
}

} // namespace core
} // namespace rta
