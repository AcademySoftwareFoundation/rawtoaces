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
inline double invertD( double val )
{
    assert( fabs( val - 0.0 ) >= DBL_EPSILON );

    return 1.0 / val;
};

template <typename T> T clip( T val, T target )
{
    return std::min( val, target );
};

template <typename T> int isSquare( const vector<vector<T>> &vm )
{
    FORI( vm.size() )
    {
        if ( vm[i].size() != vm.size() )
            return 0;
    }

    return 1;
};

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
};

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
};

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
};

template <typename T>
vector<vector<T>> invertVM( const vector<vector<T>> &vMtx )
{
    assert( isSquare( vMtx ) );

    Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic> m;
    m.resize( vMtx.size(), vMtx[0].size() );
    FORIJ( m.rows(), m.cols() ) m( i, j ) = vMtx[i][j];

    //    Map < Eigen::Matrix < T, Eigen::Dynamic, Eigen::Dynamic, RowMajor > > m (vMtx[0]);
    //    m.resize(vMtx.size(), vMtx[0].size());

    m = m.inverse();

    vector<vector<T>> vMtxR( m.rows(), vector<T>( m.cols() ) );
    FORIJ( m.rows(), m.cols() ) vMtxR[i][j] = m( i, j );

    return vMtxR;
};

template <typename T> vector<T> invertV( const vector<T> &vMtx )
{
    int               size = std::sqrt( static_cast<int>( vMtx.size() ) );
    vector<vector<T>> tmp( size, vector<T>( size ) );

    FORIJ( size, size )
    tmp[i][j] = vMtx[i * size + j];

    tmp = invertVM( tmp );
    vector<T> result( vMtx.size() );

    FORIJ( size, size ) result[i * size + j] = tmp[i][j];

    return result;
};

template <typename T> vector<vector<T>> diagVM( const vector<T> &vct )
{
    assert( vct.size() != 0 );
    vector<vector<T>> vctdiag( vct.size(), vector<T>( vct.size(), T( 0.0 ) ) );

    FORI( vct.size() ) vctdiag[i][i] = vct[i];

    return vctdiag;
};

template <typename T> vector<T> diagV( const vector<T> &vct )
{
    assert( vct.size() != 0 );

    int       length = static_cast<int>( vct.size() );
    vector<T> vctdiag( length * length, T( 0.0 ) );

    FORI( length )
    {
        vctdiag[i * length + i] = vct[i];
    }

    return vctdiag;
};

template <typename T>
vector<vector<T>> transposeVec( const vector<vector<T>> &vMtx )
{
    assert( vMtx.size() != 0 && vMtx[0].size() != 0 );

    Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic> m;
    m.resize( vMtx.size(), vMtx[0].size() );

    FORIJ( m.rows(), m.cols() ) m( i, j ) = vMtx[i][j];
    m.transposeInPlace();

    vector<vector<T>> vTran( m.rows(), vector<T>( m.cols() ) );
    FORIJ( m.rows(), m.cols() ) vTran[i][j] = m( i, j );

    return vTran;
};

template <typename T> T sumVector( const vector<T> &vct )
{
    Eigen::Matrix<T, Eigen::Dynamic, 1> v;
    v.resize( vct.size(), 1 );
    FORI( v.rows() ) v( i, 0 ) = vct[i];

    return v.sum();
};

template <typename T> T sumVectorM( const vector<vector<T>> &vct )
{
    size_t row = vct.size();
    size_t col = vct[0].size();

    T                                   sum = T( 0 );
    Eigen::Matrix<T, Eigen::Dynamic, 1> v;
    v.resize( row * col, 1 );

    FORIJ( row, col )
    v( i * col + j ) = vct[i][j];

    return v.sum();
};

template <typename T> void scaleVector( vector<T> &vct, const T scale )
{
    Eigen::Matrix<T, Eigen::Dynamic, 1> v;
    v.resize( vct.size(), 1 );

    FORI( vct.size() ) v( i, 0 ) = vct[i];
    v *= scale;

    FORI( vct.size() ) vct[i] = v( i, 0 );

    return;
};

template <typename T> void scale_vector_max( vector<T> &vector )
{
    Eigen::Matrix<T, Eigen::Dynamic, 1> column_vector;
    column_vector.resize( vector.size(), 1 );

    FORI( vector.size() )
    {
        column_vector( i, 0 ) = vector[i];
    }
    column_vector *= ( 1.0 / column_vector.maxCoeff() );

    FORI( vector.size() )
    {
        vector[i] = column_vector( i, 0 );
    }

    return;
};

template <typename T> void scale_vector_min( vector<T> &vector )
{
    Eigen::Matrix<T, Eigen::Dynamic, 1> column_vector;
    column_vector.resize( vector.size(), 1 );

    FORI( vector.size() )
    {
        column_vector( i, 0 ) = vector[i];
    }
    column_vector *= ( 1.0 / column_vector.minCoeff() );

    FORI( vector.size() )
    {
        vector[i] = column_vector( i, 0 );
    }

    return;
};

template <typename T> void scaleVectorD( vector<T> &vct )
{
    Eigen::Matrix<T, Eigen::Dynamic, 1> v;
    v.resize( vct.size(), 1 );

    FORI( v.rows() ) v( i, 0 ) = vct[i];
    FORI( v.rows() ) vct[i]    = v.maxCoeff() / vct[i];

    return;
};

template <typename T>
vector<T> mulVectorElement( const vector<T> &vct1, const vector<T> &vct2 )
{
    assert( vct1.size() == vct2.size() );

    Eigen::Array<T, Eigen::Dynamic, 1> a1, a2;
    a1.resize( vct1.size(), 1 );
    a2.resize( vct1.size(), 1 );

    FORI( a1.rows() )
    {
        a1( i, 0 ) = vct1[i];
        a2( i, 0 ) = vct2[i];
    }
    a1 *= a2;

    vector<T> vct3( a1.data(), a1.data() + a1.rows() * a1.cols() );

    return vct3;
};

template <typename T>
vector<T> divVectorElement( const vector<T> &vct1, const vector<T> &vct2 )
{
    assert( vct1.size() == vct2.size() );

    vector<T> vct2D( vct2.size(), T( 1.0 ) );
    FORI( vct2.size() )
    {
        assert( vct2[i] != T( 0.0 ) );
        vct2D[i] = T( 1.0 ) / vct2[i];
    }

    return mulVectorElement( vct1, vct2D );
};

template <typename T>
vector<T> mulVector( vector<T> vct1, vector<T> vct2, int k = 3 )
{
    int rows = ( static_cast<int>( vct1.size() ) ) / k;
    int cols = ( static_cast<int>( vct2.size() ) ) / k;

    assert( rows * k == vct1.size() );
    assert( k * cols == vct2.size() );

    vector<T> vct3( rows * cols );
    T        *pA = &vct1[0];
    T        *pB = &vct2[0];
    T        *pC = &vct3[0];

    for ( int r = 0; r < rows; r++ )
    {
        for ( int cArB = 0; cArB < k; cArB++ )
        {
            for ( int c = 0; c < cols; c++ )
            {
                pC[r * cols + c] += pA[r * k + cArB] * pB[cArB * cols + c];
            }
        }
    }

    return vct3;
};

template <typename T>
vector<vector<T>>
mulVector( const vector<vector<T>> &vct1, const vector<vector<T>> &vct2 )
{
    assert( vct1.size() != 0 && vct2.size() != 0 );

    Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic> m1, m2, m3;
    m1.resize( vct1.size(), vct1[0].size() );
    m2.resize( vct2[0].size(), vct2.size() );

    FORIJ( m1.rows(), m1.cols() )
    m1( i, j ) = vct1[i][j];
    FORIJ( m2.rows(), m2.cols() )
    m2( i, j ) = vct2[j][i];

    m3 = m1 * m2;

    vector<vector<T>> vct3( m3.rows(), vector<T>( m3.cols() ) );
    FORIJ( m3.rows(), m3.cols() ) vct3[i][j] = m3( i, j );

    return vct3;
};

template <typename T>
vector<T> mulVector( const vector<vector<T>> &vct1, const vector<T> &vct2 )
{
    assert( vct1.size() != 0 && ( vct1[0] ).size() == vct2.size() );

    Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic> m1, m2, m3;
    m1.resize( vct1.size(), vct1[0].size() );
    m2.resize( vct2.size(), 1 );

    FORIJ( m1.rows(), m1.cols() )
    m1( i, j ) = vct1[i][j];
    FORI( m2.rows() )
    m2( i, 0 ) = vct2[i];

    m3 = m1 * m2;

    vector<T> vct3( m3.data(), m3.data() + m3.rows() * m3.cols() );

    return vct3;
};

template <typename T>
vector<T> mulVector( const vector<T> &vct1, const vector<vector<T>> &vct2 )
{
    return mulVector( vct2, vct1 );
};

template <typename T>
T *mulVectorArray(
    T                            *data,
    const uint32_t                total,
    const uint8_t                 dim,
    const vector<vector<double>> &vct )
{
    assert( vct.size() == dim && isSquare( vct ) );

    /**
    // new implementation based on Eigen::Eigen::Matrix (Slow...)
     
    Eigen::Matrix <T, Eigen::Dynamic, Eigen::Dynamic> MI, mvct;
    MI.resize(total/dim, dim);
    mvct.resize(dim, dim);
    FORIJ(MI.rows(), MI.cols()) MI(i,j) = data[i*dim+j];
    FORIJ(dim, dim) mvct(i,j) = static_cast<T>(vct[i][j]);
    
    Eigen::Matrix<T,Eigen::Dynamic,Eigen::Dynamic,RowMajor> MR(MI * (mvct.transpose()));
    FORI(total) data[i] = MR(i);
    */

    if ( dim == 3 || dim == 4 )
    {
        for ( uint32_t i = 0; i < total; i += dim )
        {
            T temp[4];

            for ( uint8_t j = 0; j < dim; j++ )
            {
                temp[j] = 0;

                for ( uint8_t k = 0; k < dim; k++ )
                    temp[j] += vct[j][k] * data[i + k];
            }

            for ( uint8_t j = 0; j < dim; j++ )
                data[i + j] = temp[j];
        }
    }

    return data;
};

template <typename T>
vector<vector<T>>
solveVM( const vector<vector<T>> &vct1, const vector<vector<T>> &vct2 )
{

    Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic> m1, m2, m3;
    m1.resize( vct1.size(), vct1[0].size() );
    m2.resize( vct2.size(), vct2[0].size() );

    FORIJ( vct1.size(), vct1[0].size() )
    m1( i, j ) = vct1[i][j];
    FORIJ( vct2.size(), vct2[0].size() )
    m2( i, j ) = vct2[i][j];

    // colPivHouseholderQr()
    m3 = m1.jacobiSvd( Eigen::ComputeThinU | Eigen::ComputeThinV ).solve( m2 );

    vector<vector<T>> vct3( m3.rows(), vector<T>( m3.cols() ) );
    FORIJ( m3.rows(), m3.cols() ) vct3[i][j] = m3( i, j );

    return vct3;
};

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
    FORI( tcp.size() )
    sum += std::pow( ( tcp[i] / src[i] - 1.0 ), T( 2.0 ) );

    return sum;
};

template <typename T>
int findIndexInterp1( T val, const vector<T> &x, int size )
{
    T   dist  = T( 1e9 );
    int index = -1;

    FORI( size )
    {
        T tmp = val - x[i];
        if ( tmp < dist && tmp >= T( 0 ) )
        {
            dist  = tmp;
            index = i;
        }
    }

    return index;
};

template <typename T>
vector<T> interp1DLinear(
    const vector<int> &X0, const vector<int> &X1, const vector<T> &Y0 )
{
    assert( X0.size() == Y0.size() );

    vector<T> slope, intercept, Y1;

    FORI( X0.size() - 1 )
    {
        slope.push_back( ( Y0[i + 1] - Y0[i] ) / ( X0[i + 1] - X0[i] ) );
        intercept.push_back( Y0[i] - X0[i] * slope[i] );
    }

    slope.push_back( slope[slope.size() - 1] );
    intercept.push_back( intercept[intercept.size() - 1] );

    FORI( X1.size() )
    {
        int index = findIndexInterp1( X1[i], X0, int( X0.size() ) );
        if ( index != -1 )
            Y1.push_back( slope[index] * X1[i] + intercept[index] );
        else
            Y1.push_back( slope[0] * X1[i] + intercept[0] );
    }

    return Y1;
};

template <typename T> vector<T> xy_to_XYZ( const vector<T> &xy )
{
    vector<T> XYZ( 3 );
    XYZ[0] = xy[0];
    XYZ[1] = xy[1];
    XYZ[2] = 1 - xy[0] - xy[1];

    return XYZ;
};

template <typename T> vector<T> uv_to_xy( const vector<T> &uv )
{
    T         xyS[] = { 3.0, 2.0 };
    vector<T> xyScale( xyS, xyS + sizeof( xyS ) / sizeof( T ) );
    xyScale = mulVectorElement( xyScale, uv );

    T scale = 2 * uv[0] - 8 * uv[1] + 4;
    scaleVector( xyScale, 1.0 / scale );

    return xyScale;
};

template <typename T> vector<T> uv_to_XYZ( const vector<T> &uv )
{
    return xy_to_XYZ( uv_to_xy( uv ) );
};

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
};

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
    FORIJ( XYZ.size(), 3 )
    {
        tmpXYZ[i][j] = XYZ[i][j] / ACES_white_point_XYZ[j];
        if ( tmpXYZ[i][j] > T( e ) )
            tmpXYZ[i][j] = ceres::pow( tmpXYZ[i][j], T( 1.0 / 3.0 ) );
        else
            tmpXYZ[i][j] = T( k ) * tmpXYZ[i][j] + add;
    }

    vector<vector<T>> outCalcLab( XYZ.size(), vector<T>( 3 ) );
    FORI( XYZ.size() )
    {
        outCalcLab[i][0] = T( 116.0 ) * tmpXYZ[i][1] - T( 16.0 );
        outCalcLab[i][1] = T( 500.0 ) * ( tmpXYZ[i][0] - tmpXYZ[i][1] );
        outCalcLab[i][2] = T( 200.0 ) * ( tmpXYZ[i][1] - tmpXYZ[i][2] );
    }

    return outCalcLab;
};

template <typename T>
vector<vector<T>>
getCalcXYZt( const vector<vector<T>> &RGB, const T beta_params[6] )
{
    assert( RGB.size() == 190 );

    vector<vector<T>> BV( 3, vector<T>( 3 ) );
    vector<vector<T>> M( 3, vector<T>( 3 ) );

    FORIJ( 3, 3 ) M[i][j] = T( acesrgb_XYZ_3[i][j] );

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
};

} // namespace core
} // namespace rta

#endif
