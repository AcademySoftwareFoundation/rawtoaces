// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the rawtoaces Project.

#ifndef _MATHOPS_h__
#define _MATHOPS_h__

#include "define.h"

#include <cfloat>

#include <Eigen/Core>
#include <ceres/ceres.h>

using namespace std;
using namespace Eigen;

namespace rta
{
namespace core
{

// Non-class functions

template <typename T> int isSquare( const vector<vector<T>> &vm )
{
    for ( size_t i = 0; i < vm.size(); i++ )
    {
        if ( vm[i].size() != vm.size() )
            return 0;
    }

    return 1;
}

template <typename T>
vector<T> addVectors( const vector<T> &vectorA, const vector<T> &vectorB )
{
    assert( vectorA.size() == vectorB.size() );
    vector<T> sum;
    sum.reserve( vectorA.size() );
    std::transform(
        vectorA.begin(),
        vectorA.end(),
        vectorB.begin(),
        std::back_inserter( sum ),
        std::plus<T>() );
    return sum;
}

template <typename T>
vector<T> subVectors( const vector<T> &vectorA, const vector<T> &vectorB )
{
    assert( vectorA.size() == vectorB.size() );
    vector<T> diff;
    diff.reserve( vectorA.size() );
    std::transform(
        vectorA.begin(),
        vectorA.end(),
        vectorB.begin(),
        std::back_inserter( diff ),
        std::minus<T>() );
    return diff;
}

/// Calculate the 2D cross product (scalar) of two 2D vectors.
/// This function computes the cross product of two 2D vectors, which results in a scalar
/// value representing the signed area of the parallelogram formed by the vectors.
/// The cross product is positive when vectorB is counterclockwise from vectorA,
/// negative when clockwise, and zero when the vectors are collinear.
///
/// @param vectorA First 2D vector [x1, y1]
/// @param vectorB Second 2D vector [x2, y2]
/// @return Scalar cross product: x1*y2 - x2*y1
/// @pre vectorA.size() == 2 && vectorB.size() == 2
template <typename T>
T cross2d_scalar( const vector<T> &vectorA, const vector<T> &vectorB )
{
    assert( vectorA.size() == 2 && vectorB.size() == 2 );
    return vectorA[0] * vectorB[1] - vectorA[1] * vectorB[0];
}

template <typename T>
vector<vector<T>> invertVM( const vector<vector<T>> &vMtx )
{
    assert( isSquare( vMtx ) );

    Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic> m;
    m.resize( vMtx.size(), vMtx[0].size() );
    for ( Eigen::Index i = 0; i < m.rows(); i++ )
        for ( Eigen::Index j = 0; j < m.cols(); j++ )
            m( i, j ) = vMtx[i][j];

    //    Map < Eigen::Matrix < T, Eigen::Dynamic, Eigen::Dynamic, RowMajor > > m (vMtx[0]);
    //    m.resize(vMtx.size(), vMtx[0].size());

    m = m.inverse();

    vector<vector<T>> vMtxR( m.rows(), vector<T>( m.cols() ) );
    for ( Eigen::Index i = 0; i < m.rows(); i++ )
        for ( Eigen::Index j = 0; j < m.cols(); j++ )
            vMtxR[i][j] = m( i, j );

    return vMtxR;
}

template <typename T> vector<T> invertV( const vector<T> &vMtx )
{
    size_t            size = static_cast<size_t>( std::sqrt( vMtx.size() ) );
    vector<vector<T>> tmp( size, vector<T>( size ) );

    for ( size_t i = 0; i < size; i++ )
        for ( size_t j = 0; j < size; j++ )
            tmp[i][j] = vMtx[i * size + j];

    tmp = invertVM( tmp );
    vector<T> result( vMtx.size() );

    for ( size_t i = 0; i < size; i++ )
        for ( size_t j = 0; j < size; j++ )
            result[i * size + j] = tmp[i][j];

    return result;
}

template <typename T> vector<T> diagV( const vector<T> &vct )
{
    assert( vct.size() != 0 );

    size_t    length = vct.size();
    vector<T> vctdiag( length * length, T( 0.0 ) );

    for ( size_t i = 0; i < length; i++ )
    {
        vctdiag[i * length + i] = vct[i];
    }

    return vctdiag;
}

template <typename T>
vector<vector<T>> transposeVec( const vector<vector<T>> &vMtx )
{
    assert( vMtx.size() != 0 && vMtx[0].size() != 0 );

    Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic> m;
    m.resize( vMtx.size(), vMtx[0].size() );

    for ( Eigen::Index i = 0; i < m.rows(); i++ )
        for ( Eigen::Index j = 0; j < m.cols(); j++ )
            m( i, j ) = vMtx[i][j];
    m.transposeInPlace();

    vector<vector<T>> vTran( m.rows(), vector<T>( m.cols() ) );
    for ( Eigen::Index i = 0; i < m.rows(); i++ )
        for ( Eigen::Index j = 0; j < m.cols(); j++ )
            vTran[i][j] = m( i, j );

    return vTran;
}

template <typename T> T sumVector( const vector<T> &vct )
{
    Eigen::Matrix<T, Eigen::Dynamic, 1> v;
    v.resize( vct.size(), 1 );
    for ( Eigen::Index i = 0; i < v.rows(); i++ )
        v( i, 0 ) = vct[i];

    return v.sum();
}

template <typename T> T sumVectorM( const vector<vector<T>> &vct )
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

template <typename T> void scaleVector( vector<T> &vct, const T scale )
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

template <typename T>
vector<T> mulVectorElement( const vector<T> &vct1, const vector<T> &vct2 )
{
    assert( vct1.size() == vct2.size() );

    Eigen::Array<T, Eigen::Dynamic, 1> a1, a2;
    a1.resize( vct1.size(), 1 );
    a2.resize( vct1.size(), 1 );

    for ( Eigen::Index i = 0; i < a1.rows(); i++ )
    {
        a1( i, 0 ) = vct1[i];
        a2( i, 0 ) = vct2[i];
    }
    a1 *= a2;

    vector<T> vct3( a1.data(), a1.data() + a1.rows() * a1.cols() );

    return vct3;
}

template <typename T>
vector<T> mulVector( vector<T> vct1, vector<T> vct2, int k = 3 )
{
    size_t rows = vct1.size() / k;
    size_t cols = vct2.size() / k;

    assert( rows * k == vct1.size() );
    assert( k * cols == vct2.size() );

    vector<T> vct3( rows * cols );
    T        *pA = &vct1[0];
    T        *pB = &vct2[0];
    T        *pC = &vct3[0];

    for ( size_t r = 0; r < rows; r++ )
    {
        for ( int cArB = 0; cArB < k; cArB++ )
        {
            for ( size_t c = 0; c < cols; c++ )
            {
                pC[r * cols + c] += pA[r * k + cArB] * pB[cArB * cols + c];
            }
        }
    }

    return vct3;
}

template <typename T>
vector<vector<T>>
mulVector( const vector<vector<T>> &vct1, const vector<vector<T>> &vct2 )
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
            m2( i, j ) = vct2[j][i];

    m3 = m1 * m2;

    vector<vector<T>> vct3( m3.rows(), vector<T>( m3.cols() ) );
    for ( Eigen::Index i = 0; i < m3.rows(); i++ )
        for ( Eigen::Index j = 0; j < m3.cols(); j++ )
            vct3[i][j] = m3( i, j );

    return vct3;
}

template <typename T>
vector<T> mulVector( const vector<vector<T>> &vct1, const vector<T> &vct2 )
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

    vector<T> vct3( m3.data(), m3.data() + m3.rows() * m3.cols() );

    return vct3;
}

template <typename T>
vector<T> mulVector( const vector<T> &vct1, const vector<vector<T>> &vct2 )
{
    return mulVector( vct2, vct1 );
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
T calculate_SSE( const vector<T> &tcp, const vector<T> &src )
{
    assert( tcp.size() == src.size() );
    vector<T> tmp( src.size() );

    T sum = T( 0.0 );
    for ( size_t i = 0; i < tcp.size(); i++ )
        sum += std::pow( ( tcp[i] / src[i] - 1.0 ), T( 2.0 ) );

    return sum;
}

template <typename T>
int findIndexInterp1( T val, const vector<T> &x, size_t size )
{
    T   dist  = T( 1e9 );
    int index = -1;

    for ( size_t i = 0; i < size; i++ )
    {
        T tmp = val - x[i];
        if ( tmp < dist && tmp >= T( 0 ) )
        {
            dist  = tmp;
            index = static_cast<int>( i );
        }
    }

    return index;
}

template <typename T>
vector<T> interp1DLinear(
    const vector<int> &X0, const vector<int> &X1, const vector<T> &Y0 )
{
    assert( X0.size() == Y0.size() );

    vector<T> slope, intercept, Y1;

    for ( size_t i = 0; i < X0.size() - 1; i++ )
    {
        slope.push_back( ( Y0[i + 1] - Y0[i] ) / ( X0[i + 1] - X0[i] ) );
        intercept.push_back( Y0[i] - X0[i] * slope[i] );
    }

    slope.push_back( slope[slope.size() - 1] );
    intercept.push_back( intercept[intercept.size() - 1] );

    for ( size_t i = 0; i < X1.size(); i++ )
    {
        int index = findIndexInterp1( X1[i], X0, X0.size() );
        if ( index != -1 )
            Y1.push_back( slope[index] * X1[i] + intercept[index] );
        else
            Y1.push_back( slope[0] * X1[i] + intercept[0] );
    }

    return Y1;
}

template <typename T> vector<T> xy_to_XYZ( const vector<T> &xy )
{
    vector<T> XYZ( 3 );
    XYZ[0] = xy[0];
    XYZ[1] = xy[1];
    XYZ[2] = 1 - xy[0] - xy[1];

    return XYZ;
}

template <typename T> vector<T> uv_to_xy( const vector<T> &uv )
{
    T         xyS[] = { 3.0, 2.0 };
    vector<T> xyScale( xyS, xyS + sizeof( xyS ) / sizeof( T ) );
    xyScale = mulVectorElement( xyScale, uv );

    T scale = 2 * uv[0] - 8 * uv[1] + 4;
    scaleVector( xyScale, 1.0 / scale );

    return xyScale;
}

template <typename T> vector<T> uv_to_XYZ( const vector<T> &uv )
{
    return xy_to_XYZ( uv_to_xy( uv ) );
}

template <typename T> vector<T> XYZ_to_uv( const vector<T> &XYZ )
{
    T         uvS[]   = { 4.0, 6.0 };
    T         slice[] = { XYZ[0], XYZ[1] };
    vector<T> uvScale( uvS, uvS + sizeof( uvS ) / sizeof( T ) );
    vector<T> vSlice( slice, slice + sizeof( slice ) / sizeof( T ) );

    uvScale = mulVectorElement( uvScale, vSlice );

    T scale = XYZ[0] + 15 * XYZ[1] + 3 * XYZ[2];
    scaleVector( uvScale, 1.0 / scale );

    return uvScale;
}

template <typename T>
std::vector<std::vector<T>> calculate_CAT(
    const std::vector<T> &src_white_XYZ, const std::vector<T> &dst_white_XYZ )
{
    assert( src_white_XYZ.size() == 3 );
    assert( dst_white_XYZ.size() == 3 );

    std::vector<double> src_white_LMS = mulVector( src_white_XYZ, CAT02 );
    std::vector<double> dst_white_LMS = mulVector( dst_white_XYZ, CAT02 );

    std::vector<std::vector<double>> mat( 3, std::vector<double>( 3, 0 ) );
    for ( size_t i = 0; i < 3; i++ )
        mat[i][i] = dst_white_LMS[i] / src_white_LMS[i];

    mat = mulVector( mat, transposeVec( CAT02 ) );
    mat = mulVector( CAT02_inv, transposeVec( mat ) );
    return mat;
}

template <typename T>
vector<vector<T>> XYZ_to_LAB( const vector<vector<T>> &XYZ )
{
    assert( XYZ.size() == 190 );
    T add = T( 16.0 / 116.0 );

    vector<vector<T>> tmpXYZ( XYZ.size(), vector<T>( 3, T( 1.0 ) ) );
    for ( size_t i = 0; i < XYZ.size(); i++ )
        for ( size_t j = 0; j < 3; j++ )
        {
            tmpXYZ[i][j] = XYZ[i][j] / ACES_white_point_XYZ[j];
            if ( tmpXYZ[i][j] > T( e ) )
                tmpXYZ[i][j] = ceres::pow( tmpXYZ[i][j], T( 1.0 / 3.0 ) );
            else
                tmpXYZ[i][j] = T( k ) * tmpXYZ[i][j] + add;
        }

    vector<vector<T>> outCalcLab( XYZ.size(), vector<T>( 3 ) );
    for ( size_t i = 0; i < XYZ.size(); i++ )
    {
        outCalcLab[i][0] = T( 116.0 ) * tmpXYZ[i][1] - T( 16.0 );
        outCalcLab[i][1] = T( 500.0 ) * ( tmpXYZ[i][0] - tmpXYZ[i][1] );
        outCalcLab[i][2] = T( 200.0 ) * ( tmpXYZ[i][1] - tmpXYZ[i][2] );
    }

    return outCalcLab;
}

template <typename T>
vector<vector<T>>
getCalcXYZt( const vector<vector<T>> &RGB, const T beta_params[6] )
{
    assert( RGB.size() == 190 );

    vector<vector<T>> BV( 3, vector<T>( 3 ) );
    vector<vector<T>> M( 3, vector<T>( 3 ) );

    for ( size_t i = 0; i < 3; i++ )
        for ( size_t j = 0; j < 3; j++ )
            M[i][j] = T( acesrgb_XYZ_3[i][j] );

    BV[0][0] = beta_params[0];
    BV[0][1] = beta_params[1];
    BV[0][2] = 1.0 - beta_params[0] - beta_params[1];
    BV[1][0] = beta_params[2];
    BV[1][1] = beta_params[3];
    BV[1][2] = 1.0 - beta_params[2] - beta_params[3];
    BV[2][0] = beta_params[4];
    BV[2][1] = beta_params[5];
    BV[2][2] = 1.0 - beta_params[4] - beta_params[5];

    vector<vector<T>> outCalcXYZt = mulVector( mulVector( RGB, BV ), M );

    return outCalcXYZt;
}

} // namespace core
} // namespace rta

#endif
