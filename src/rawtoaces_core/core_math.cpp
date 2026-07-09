// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the rawtoaces Project.

#include "core_math.h"
#include "define.h"
#include <assert.h>
#include <cmath>

#if defined( RTA_ENABLE_EIGEN ) && RTA_ENABLE_EIGEN
#    include <Eigen/Core>
#    include <Eigen/Dense>
#    include <Eigen/QR>
#    define RTA_HAS_EIGEN 1
#else
#    define RTA_HAS_EIGEN 0
#endif

#if defined( RTA_ENABLE_CERES ) && RTA_ENABLE_CERES
#    include <ceres/ceres.h>
#    define RTA_HAS_CERES 1
#else
#    define RTA_HAS_CERES 0
#endif

namespace rta
{
namespace core
{
namespace math
{

// Enabled by default if Eigen is available.
bool use_eigen = RTA_HAS_EIGEN;

// Enabled by default if Ceres is available.
bool use_ceres = RTA_HAS_CERES;

bool has_eigen()
{
    return RTA_HAS_EIGEN;
}

bool has_ceres()
{
    return RTA_HAS_CERES;
}

void check_eigen()
{
#if !RTA_HAS_EIGEN
    if ( use_eigen )
    {
        static bool reported = false;
        if ( !reported )
        {
            std::cerr << "The library was built without Eigen support. "
                      << "The built-in linear algebra implementation "
                      << "will be used instead. " << std::endl;
            reported = true;
        }
        use_eigen = false;
    }
#endif // !RTA_HAS_EIGEN
}

void check_ceres()
{
#if !RTA_HAS_CERES
    if ( use_ceres )
    {
        static bool reported = false;
        if ( !reported )
        {
            std::cerr << "The library was built without Ceres-solver support. "
                      << "The build-in solver will be used instead."
                      << std::endl;
            reported = true;
        }
        use_ceres = false;
    }
#endif // !RTA_HAS_CERES
}

#if RTA_HAS_EIGEN

/// Create eigen vector from std::vector
template <typename T>
Eigen::Matrix<T, Eigen::Dynamic, 1> vec_from_std( const std::vector<T> &vec )
{
    size_t n = vec.size();
    assert( n > 0 );

    Eigen::Matrix<T, Eigen::Dynamic, 1> result( vec.size() );
    for ( size_t i = 0; i < n; i++ )
        result( i ) = vec[i];
    return result;
}

/// Create std::vector from eigen vector
template <typename T>
std::vector<T> vec_to_std( const Eigen::Matrix<T, Eigen::Dynamic, 1> &vec )
{
    size_t n = vec.rows();
    assert( n > 0 );

    std::vector<T> result( n );

    for ( size_t i = 0; i < n; i++ )
        result[i] = vec( i );
    return result;
}

/// Create eigen matrix from std::vector
template <typename T>
Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic>
mat_from_std( const std::vector<std::vector<T>> &mat )
{
    size_t n = mat.size();
    assert( n > 0 );

    size_t m = mat[0].size();
    assert( m > 0 );

    Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic> result( n, m );

    for ( size_t i = 0; i < n; i++ )
    {
        assert( mat[i].size() == m );
        for ( size_t j = 0; j < m; j++ )
        {
            result( i, j ) = mat[i][j];
        }
    }
    return result;
}

/// Create std::vector from eigen matrix
template <typename T>
std::vector<std::vector<T>>
mat_to_std( const Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic> &mat )
{
    size_t n = mat.rows();
    assert( n > 0 );

    size_t m = mat.cols();
    assert( m > 0 );

    std::vector<std::vector<T>> result( n, std::vector<T>( m ) );

    for ( size_t i = 0; i < n; i++ )
        for ( size_t j = 0; j < m; j++ )
            result[i][j] = mat( i, j );
    return result;
}

#endif // RTA_HAS_EIGEN

bool inverse(
    const std::vector<std::vector<double>> &src_mat,
    std::vector<std::vector<double>>       &dst_mat )
{
    size_t rows = src_mat.size();
    if ( rows == 0 )
        return false;

    size_t cols = src_mat[0].size();
    if ( rows != cols )
        return false;

#if RTA_HAS_EIGEN
    if ( use_eigen )
    {
        auto m = mat_from_std( src_mat );
        if ( std::fabs( m.determinant() ) < 1e-9 )
            return false;

        m       = m.inverse();
        dst_mat = mat_to_std( m );
    }
    else
#endif // RTA_HAS_EIGEN
    {

        check_eigen();

        // Currently only supporting 3x3 matrices.
        if ( rows != 3 )
            return false;

        double a = src_mat[0][0];
        double b = src_mat[0][1];
        double c = src_mat[0][2];
        double d = src_mat[1][0];
        double e = src_mat[1][1];
        double f = src_mat[1][2];
        double g = src_mat[2][0];
        double h = src_mat[2][1];
        double i = src_mat[2][2];

        double A = e * i - f * h;
        double B = f * g - d * i;
        double C = d * h - e * g;

        double determinant = A * a + B * b + C * c;
        if ( std::fabs( determinant ) < 1e-9 )
            return false;

        double scale = 1.0 / determinant;

        double D = c * h - b * i;
        double E = a * i - c * g;
        double F = b * g - a * h;
        double G = b * f - c * e;
        double H = c * d - a * f;
        double I = a * e - b * d;

        dst_mat.resize( 3 );

        dst_mat[0].resize( 3 );
        dst_mat[0][0] = A * scale;
        dst_mat[0][1] = D * scale;
        dst_mat[0][2] = G * scale;

        dst_mat[1].resize( 3 );
        dst_mat[1][0] = B * scale;
        dst_mat[1][1] = E * scale;
        dst_mat[1][2] = H * scale;

        dst_mat[2].resize( 3 );
        dst_mat[2][0] = C * scale;
        dst_mat[2][1] = F * scale;
        dst_mat[2][2] = I * scale;
    }

    return true;
}

std::vector<std::vector<double>>
transposed( const std::vector<std::vector<double>> &src_mat )
{
    size_t rows = src_mat.size();
    assert( rows > 0 );

    size_t cols = src_mat[0].size();
    assert( cols > 0 );

    std::vector<std::vector<double>> result;

#if RTA_HAS_EIGEN
    if ( use_eigen )
    {
        auto m = mat_from_std( src_mat );
        m.transposeInPlace();
        result = mat_to_std( m );
    }
    else
#endif // RTA_HAS_EIGEN
    {
        check_eigen();

        result.resize( cols );
        for ( size_t i = 0; i < cols; i++ )
        {
            result[i].resize( rows );
            for ( size_t j = 0; j < rows; j++ )
                result[i][j] = src_mat[j][i];
        }
    }

    return result;
}

template <typename T>
std::vector<std::vector<T>> product(
    const std::vector<std::vector<T>> &vct1,
    const std::vector<std::vector<T>> &vct2 )
{
    size_t rows1 = vct1.size();
    assert( rows1 > 0 );

    size_t cols1 = vct1[0].size();
    assert( rows1 > 0 );

    size_t rows2 [[maybe_unused]] = vct2.size();
    assert( rows2 > 0 );

    size_t cols2 = vct2[0].size();
    assert( cols2 > 0 );

    assert( cols1 == rows2 );

    std::vector<std::vector<T>> result;

#if RTA_HAS_EIGEN
    if ( use_eigen )
    {
        auto m1 = mat_from_std( vct1 );
        auto m2 = mat_from_std( vct2 );

        Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic> m3 = m1 * m2;

        result = mat_to_std( m3 );
    }
    else
#endif // RTA_HAS_EIGEN
    {
        check_eigen();

        result.resize( rows1 );

        for ( size_t i = 0; i < rows1; i++ )
        {
            result[i].resize( cols2 );

            for ( size_t j = 0; j < cols2; j++ )
            {
                result[i][j] = T( 0.0 );

                for ( size_t k = 0; k < cols1; k++ )
                    result[i][j] += vct1[i][k] * vct2[k][j];
            }
        }
    }

    return result;
}

template <typename T>
std::vector<T>
product( const std::vector<std::vector<T>> &vct1, const std::vector<T> &vct2 )
{
    size_t rows1 = vct1.size();
    assert( rows1 > 0 );

    size_t cols1 = vct1[0].size();
    assert( rows1 > 0 );

    size_t rows2 [[maybe_unused]] = vct2.size();
    assert( rows2 > 0 );

    assert( cols1 == rows2 );

    std::vector<T> result;

#if RTA_HAS_EIGEN
    if ( use_eigen )
    {
        auto m1 = mat_from_std( vct1 );
        auto m2 = vec_from_std( vct2 );

        Eigen::Matrix<T, Eigen::Dynamic, 1> m3 = m1 * m2;

        result = vec_to_std( m3 );
    }
    else
#endif // RTA_HAS_EIGEN
    {
        check_eigen();

        result.resize( rows1 );

        for ( size_t i = 0; i < rows1; i++ )
        {
            result[i] = T( 0.0 );

            for ( size_t j = 0; j < cols1; j++ )
                result[i] += vct1[i][j] * vct2[j];
        }
    }

    return result;
}

double
calculate_SSE( const std::vector<double> &tcp, const std::vector<double> &src )
{
    assert( tcp.size() == src.size() );

    double sum = 0.0;
    for ( size_t i = 0; i < tcp.size(); i++ )
        sum += std::pow( ( tcp[i] / src[i] - 1.0 ), 2.0 );

    return sum;
}

std::vector<double> xy_to_XYZ( const std::vector<double> &xy )
{
    std::vector<double> XYZ( 3 );
    XYZ[0] = xy[0];
    XYZ[1] = xy[1];
    XYZ[2] = 1 - xy[0] - xy[1];

    return XYZ;
}

std::vector<double> uv_to_xy( const std::vector<double> &uv )
{
    double              scale  = 2 * uv[0] - 8 * uv[1] + 4;
    std::vector<double> result = { 3.0 * uv[0] / scale, 2.0 * uv[1] / scale };
    return result;
}

std::vector<double> uv_to_XYZ( const std::vector<double> &uv )
{
    return xy_to_XYZ( uv_to_xy( uv ) );
} // LCOV_EXCL_LINE - bug in coverage tool

std::vector<double> XYZ_to_uv( const std::vector<double> &XYZ )
{
    double              scale  = XYZ[0] + 15 * XYZ[1] + 3 * XYZ[2];
    std::vector<double> result = { 4.0 * XYZ[0] / scale, 6.0 * XYZ[1] / scale };
    return result;
}

std::vector<std::vector<double>> calculate_CAT(
    const std::vector<double> &src_white_XYZ,
    const std::vector<double> &dst_white_XYZ,
    bool                       use_bradford )
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
            if ( tmpXYZ[i][j] > T( k_e ) )
            {
#if RTA_HAS_CERES
                if ( use_ceres )
                {
                    tmpXYZ[i][j] = ceres::pow( tmpXYZ[i][j], T( 1.0 / 3.0 ) );
                }
                else
#endif // RTA_HAS_CERES
                {
                    tmpXYZ[i][j] = std::pow( tmpXYZ[i][j], T( 1.0 / 3.0 ) );
                }
            }
            else
                tmpXYZ[i][j] = T( k_k ) * tmpXYZ[i][j] + add;
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

// Change the base type of a vector of numbers.
template <typename T_dst, typename T_src>
std::vector<T_dst> change_type( const std::vector<T_src> &vec )
{
    std::vector<T_dst> result( vec.size() );
    std::transform(
        vec.begin(), vec.end(), result.begin(), []( const T_src &v ) {
            return T_dst( v );
        } );
    return result;
}

// Change the base type of a matrix of numbers.
template <typename T_dst, typename T_src>
std::vector<std::vector<T_dst>>
change_type( const std::vector<std::vector<T_src>> &mat )
{
    std::vector<std::vector<T_dst>> result( mat.size() );
    std::transform(
        mat.begin(),
        mat.end(),
        result.begin(),
        []( const std::vector<T_src> &v ) { return change_type<T_dst>( v ); } );
    return result;
}

// Explicit no-op specialisation when double->double conversion requested.
template <> std::vector<double> change_type( const std::vector<double> &vec )
{
    return vec;
}

// Explicit no-op specialisation when double->double conversion requested.
template <>
std::vector<std::vector<double>>
change_type( const std::vector<std::vector<double>> &mat )
{
    return mat;
}

template <typename T, typename U> struct is_same
{
    static const bool value = false;
};

template <typename T> struct is_same<T, T>
{
    static const bool value = true;
};

// (6/29)^3, used in lab_F() and lab_F_prime().
constexpr double v_6_29_3 = 6.0 * 6.0 * 6.0 / 29.0 / 29.0 / 29.0;

// (29/6)^2, used in lab_F() and lab_F_prime().
constexpr double v_29_6_2 = 29.0 * 29.0 / 6.0 / 6.0;

// The non-linear perceptual function of the XYZ to LAB conversion.
template <typename T> T lab_F( T arg )
{
    if ( arg > v_6_29_3 )
    {
#if RTA_HAS_CERES
        constexpr bool is_ceres = !is_same<T, double>::value;
        if constexpr ( is_ceres )
        {
            return ceres::pow( arg, T( 1.0 / 3.0 ) );
        }
        else
#endif // RTA_HAS_CERES
        {
            return std::pow( arg, T( 1.0 / 3.0 ) );
        }
    }
    else
        return arg * v_29_6_2 / 3.0 + 4.0 / 29.0;
}

// The derivative of `lab_F`, see above.
// (lab_F(arg))' = lab_F'(arg) * arg'
template <typename T> T lab_F_prime( T arg, T arg_prime )
{
    if ( arg > v_6_29_3 )
    {
#if RTA_HAS_CERES
        constexpr bool is_ceres = !is_same<T, double>::value;
        if constexpr ( is_ceres )
        {
            // LCOV_EXCL_START
            // This code is never executed, as Ceres calculates its own
            // derivatives. We can not remove this branching
            // completely, the compiler will complain about this function
            // called in Ceres mode. Could have put `return 0` here, but having
            // the correct code looks tidier.
            return T( 1.0 / 3.0 ) * ceres::pow( arg, -2.0 / 3.0 ) * arg_prime;
            // LCOV_EXCL_STOP
        }
        else
#endif // RTA_HAS_CERES
        {
            return T( 1.0 / 3.0 ) * std::pow( arg, -2.0 / 3.0 ) * arg_prime;
        }
    }
    else
        return arg_prime * v_29_6_2 / 3.0;
}

// XYZ to LAB conversion.
// The input XYZ value must be pre-normalised to the white point.
template <typename T> void XYZ_to_LAB( const T ( &in )[3], T ( &out )[3] )
{
    out[0] = T( 116.0 ) * lab_F( in[1] ) - T( 16.0 );
    out[1] = T( 500.0 ) * ( lab_F( in[0] ) - lab_F( in[1] ) );
    out[2] = T( 200.0 ) * ( lab_F( in[1] ) - lab_F( in[2] ) );
}

// The derivative of the `XYZ_to_LAB` function, see above.
template <typename T>
void XYZ_to_LAB_prime(
    const T ( &arg )[3], const T ( &arg_prime )[3], T ( &out )[3] )
{
    out[0] = T( 116.0 ) * lab_F_prime( arg[1], arg_prime[1] );
    out[1] = T( 500.0 ) * ( lab_F_prime( arg[0], arg_prime[0] ) -
                            lab_F_prime( arg[1], arg_prime[1] ) );
    out[2] = T( 200.0 ) * ( lab_F_prime( arg[1], arg_prime[1] ) -
                            lab_F_prime( arg[2], arg_prime[2] ) );
}

/// Solve a system of linear equations using Gauss elimination.
bool solve_linear( std::vector<std::vector<double>> &a, std::vector<double> &b )
{
    size_t n = a.size();
    assert( n > 0 );
    assert( a[0].size() == n );
    assert( b.size() == n );

    for ( size_t i = 0; i < n; i++ )
    {
        double max_val = a[i][i];
        size_t pivot   = i;

        for ( size_t j = i; j < n; j++ )
        {
            double v = std::abs( a[j][i] );
            if ( v > max_val )
            {
                max_val = v;
                pivot   = j;
            }
        }

        if ( max_val == 0 )
            return false;

        if ( pivot != i )
        {
            std::swap( a[i], a[pivot] );
            std::swap( b[i], b[pivot] );
        }

        double scale = 1.0 / a[i][i];
        for ( size_t k = i + 1; k < n; k++ )
        {
            a[i][k] *= scale;
        }
        a[i][i] = 1.0;
        b[i] *= scale;

        for ( size_t j = i + 1; j < n; j++ )
        {
            if ( a[j][i] != 0 )
            {
                double local_scale = -a[j][i];
                for ( size_t k = i; k < n; k++ )
                {
                    a[j][k] += a[i][k] * local_scale;
                }
                b[j] += b[i] * local_scale;
            }
        }
    }

    for ( size_t i = n - 1; i > 0; i-- )
    {
        for ( size_t j = 0; j < i; j++ )
        {
            double scale = -a[j][i];
            b[j] += b[i] * scale;
        }
    }

    return true;
}

/// Cost function object for IDT matrix optimization using Ceres solver.
/// This struct implements the objective function for curve fitting between camera RGB
/// responses and target XYZ values. It's used to find the optimal 6-parameter IDT
/// matrix that minimizes the difference between predicted and actual color values
/// across all training patches.
struct IDTOptimizationCost
{
    IDTOptimizationCost(
        const std::vector<std::vector<double>> &source_RGB,
        const std::vector<std::vector<double>> &target_XYZ )
        : _source_RGB( source_RGB ), _target_LAB( XYZ_to_LAB( target_XYZ ) )
    {}

    template <typename T>
    bool operator()(
        const T                     *beta_params,
        T                           *residuals,
        std::vector<std::vector<T>> *jacobian = nullptr ) const;

    const std::vector<std::vector<double>> _source_RGB;
    const std::vector<std::vector<double>> _target_LAB;
};

/// Cost function operator for Ceres optimization of IDT matrix parameters.
/// This function computes the residual errors between target LAB values and
/// calculated LAB values from camera RGB responses transformed by candidate
/// IDT matrix parameters. It's used by the Ceres optimization library to
/// iteratively find the optimal 6-parameter IDT matrix that minimizes
/// color differences across all training patches.
///
/// The function transforms camera RGB values using candidate IDT parameters beta_params,
/// converts the result to XYZ using ACES RGB primaries, then to LAB color space,
/// and computes the difference from target LAB values as residuals.
///
/// @param beta_params 6-element array of IDT matrix parameters [b00, b01, b02, b10, b11, b12]
/// @param residuals Output array of LAB differences
/// @return true (required by Ceres interface)
/// @pre _source_RGB must contain camera RGB responses
/// @pre _target_LAB must contain target LAB values
template <typename T>
bool IDTOptimizationCost::operator()(
    const T                     *beta_params,
    T                           *residuals,
    std::vector<std::vector<T>> *jacobian ) const
{
    size_t n = _source_RGB.size();
    assert( n > 0 );

    std::vector<std::vector<T>> source_rgb =
        change_type<T, double>( _source_RGB );
    std::vector<std::vector<T>> target_lab =
        change_type<T, double>( _target_LAB );

    const T                    *B = beta_params;
    const T                     T1( 1.0 );
    std::vector<std::vector<T>> mat_idt_transposed = {
        { B[0], B[2], B[4] },
        { B[1], B[3], B[5] },
        { T1 - B[0] - B[1], T1 - B[2] - B[3], T1 - B[4] - B[5] }
    };

    std::vector<std::vector<T>> mat_out_to_xyz_transposed(
        3, std::vector<T>( 3 ) );
    for ( size_t i = 0; i < 3; i++ )
    {
        for ( size_t j = 0; j < 3; j++ )
        {
            mat_out_to_xyz_transposed[i][j] = T( acesrgb_XYZ_3[j][i] );
        }
    }

    std::vector<std::vector<T>> rgb_W = { { T1, T1, T1 } };

    std::vector<std::vector<T>> out_rgb =
        product( source_rgb, mat_idt_transposed );
    std::vector<std::vector<T>> out_xyz =
        product( out_rgb, mat_out_to_xyz_transposed );
    std::vector<std::vector<T>> xyz_W =
        product( rgb_W, mat_out_to_xyz_transposed );

    for ( size_t i = 0; i < n; i++ )
    {
        T xyz1[3] = { out_xyz[i][0], out_xyz[i][1], out_xyz[i][2] };

        for ( size_t j = 0; j < 3; j++ )
        {
            xyz1[j] /= xyz_W[0][j];
        }

        T lab1[3];
        T lab2[3] = { T( target_lab[i][0] ),
                      T( target_lab[i][1] ),
                      T( target_lab[i][2] ) };

        XYZ_to_LAB( xyz1, lab1 );

        residuals[i * 3 + 0] = lab1[0] - lab2[0];
        residuals[i * 3 + 1] = lab1[1] - lab2[1];
        residuals[i * 3 + 2] = lab1[2] - lab2[2];

        if ( jacobian != nullptr )
        {
            // Partial derivatives of the out_rgb with regards to the IDT
            // matrix variables (x)

            // Ro = x0*(Rs-Bs) + x1*(Gs-Bs) + Rw*Bs
            // Go = x2*(Rs-Bs) + x3*(Gs-Bs) + Gw*Bs
            // Bo = x4*(Rs-Bs) + x5*(Gs-Bs) + Bw*Bs

            // Ro_ = (Rs-Bs) when j == 0, 0 otherwise
            // Ro_ = (Gs-Bs) when j == 1, 0 otherwise
            // Go_ = (Rs-Bs) when j == 2, 0 otherwise
            // Go_ = (Gs-Bs) when j == 3, 0 otherwise
            // Bo_ = (Rs-Bs) when j == 4, 0 otherwise
            // Bo_ = (Gs-Bs) when j == 5, 0 otherwise

            T &Rs = source_rgb[i][0];
            T &Gs = source_rgb[i][1];
            T &Bs = source_rgb[i][2];

            std::vector<std::vector<T>> out_rgb_ = {
                { Rs - Bs, T( 0 ), T( 0 ) }, { Gs - Bs, T( 0 ), T( 0 ) },
                { T( 0 ), Rs - Bs, T( 0 ) }, { T( 0 ), Gs - Bs, T( 0 ) },
                { T( 0 ), T( 0 ), Rs - Bs }, { T( 0 ), T( 0 ), Gs - Bs },
            };

            // Partial derivatives of the out_xyz with regards to the IDT
            // matrix variables (x), which is just the out_to_xyz matrix
            // applied to out_rgb_.
            std::vector<std::vector<T>> out_xyz_ =
                product( out_rgb_, mat_out_to_xyz_transposed );

            for ( size_t j = 0; j < 6; j++ )
            {
                T xyz_[3] = { out_xyz_[j][0] / xyz_W[0][0],
                              out_xyz_[j][1] / xyz_W[0][1],
                              out_xyz_[j][2] / xyz_W[0][2] };

                T lab_[3];
                XYZ_to_LAB_prime( xyz1, xyz_, lab_ );

                for ( size_t c = 0; c < 3; c++ )
                {
                    ( *jacobian )[i * 3 + c][j] = T( lab_[c] );
                }
            }
        }
    }

    return true;
}

#if RTA_HAS_CERES
/// Solve a non-linear optimisation problem by minimising the cost function
/// provided in `cost_function`.
///
/// @param cost_function The cost function to minimise.
/// @param beta_params Parameters to solve. Must be initialised with the
/// initial values, modified in-place.
/// @param verbosity Verbosity level for optimization output:
/// - 0-2: No output from solver
/// - 3: Ceres solver full report to stderr
/// - 4: Additionally enables Ceres minimizer progress to stdout
/// @return true if optimization succeeded, false otherwise
inline bool minimise_ceres(
    IDTOptimizationCost *cost_function,
    std::vector<double> &beta_params,
    size_t               size,
    double               error_threshold,
    size_t               max_steps,
    int                  verbosity )
{
    ceres::Problem problem;

    ceres::CostFunction *ceres_cost_function =
        new ceres::AutoDiffCostFunction<IDTOptimizationCost, ceres::DYNAMIC, 6>(
            cost_function, int( size ) );

    problem.AddResidualBlock( ceres_cost_function, NULL, beta_params.data() );

    ceres::Solver::Options options;
    options.linear_solver_type        = ceres::DENSE_QR;
    options.parameter_tolerance       = error_threshold;
    options.function_tolerance        = error_threshold;
    options.min_line_search_step_size = error_threshold;
    options.max_num_iterations        = (int)max_steps;

    if ( verbosity > 3 )
        options.minimizer_progress_to_stdout = true;

    ceres::Solver::Summary summary;
    ceres::Solve( options, &problem, &summary );

    if ( verbosity > 2 )
        std::cerr << summary.FullReport() << std::endl;

    return summary.num_successful_steps > 0;
}
#endif // RTA_HAS_CERES

void print_summary(
    const std::string &status,
    double             initial_cost,
    double             final_cost,
    size_t             steps )
{
    std::cerr << std::scientific;
    std::cerr << "Solver Summary (built-in solver)" << std::endl;
    std::cerr << std::endl;
    std::cerr << "Cost:" << std::endl;
    std::cerr << "Initial " << std::right << std::setw( 37 ) << initial_cost
              << std::endl;
    std::cerr << "Final   " << std::right << std::setw( 37 ) << final_cost
              << std::endl;
    std::cerr << "Change  " << std::right << std::setw( 37 )
              << initial_cost - final_cost << std::endl;
    std::cerr << std::endl;
    std::cerr << "Minimizer iterations " << std::right << std::setw( 24 )
              << steps << std::endl;
    std::cerr << std::endl;
    std::cerr << "Termination:     " << status << std::endl;
    std::cerr << std::endl;
}

inline bool minimise_builtin(
    IDTOptimizationCost *cost_function,
    std::vector<double> &beta_params,
    size_t               size,
    double               error_threshold,
    size_t               max_steps,
    int                  verbosity )
{
    assert( cost_function != nullptr );

    bool        result = true;
    std::string status;

    size_t step         = 0;
    double prev_cost    = 0;
    double initial_cost = 0;

    std::vector<std::vector<double>> jacobian(
        size, std::vector<double>( 6, 0 ) );
    std::vector<double> residuals( size, 0 );

    while ( true )
    {
        if ( step == max_steps )
        {
            status = "NO_CONVERGENCE (Maximum number of iterations reached.)";
            break;
        }

        double *B = beta_params.data();

        // The cost function currently always returns true.
        bool cost_result = ( *cost_function )( B, residuals.data(), &jacobian );
        assert( cost_result );
        (void)cost_result;

        double new_cost = 0;
        for ( auto &i: residuals )
        {
            new_cost += i * i;
        }

        if ( std::isnan( new_cost ) )
        {
            result = false;
            status =
                "FAILURE "
                "(Initial residual and Jacobian evaluation failed.)";
            break;
        }

        new_cost *= 0.5;

        if ( step == 0 )
        {
            prev_cost    = new_cost;
            initial_cost = new_cost;
        }

        auto J_t = transposed( jacobian );
        auto A   = product( J_t, jacobian );
        auto b   = product( J_t, residuals );

        // Solve A * x = b to find the gradient

#if RTA_HAS_EIGEN
        if ( use_eigen )
        {
            Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic> A_mat =
                mat_from_std( A );
            Eigen::Matrix<double, Eigen::Dynamic, 1> b_vec = vec_from_std( b );
            Eigen::Matrix<double, Eigen::Dynamic, 1> x =
                A_mat.colPivHouseholderQr().solve( b_vec );
            b = vec_to_std( x );
        }
        else
#endif //RTA_HAS_EIGEN
        {
            if ( !solve_linear( A, b ) )
            {
                for ( auto &v: b )
                {
                    v = 0.0;
                }
            }
        }

        double grad = 0;
        for ( const auto &v: b )
        {
            grad = std::max( grad, std::fabs( v ) );
        }

        if ( grad < error_threshold )
        {
            status = "CONVERGENCE (Gradient tolerance reached.)";
            break;
        }

        for ( size_t i = 0; i < 6; i++ )
        {
            beta_params[i] -= b[i];
        }

        if ( verbosity > 3 )
        {
            if ( step == 0 )
            {
                std::cout << "iter      cost      cost_change" << std::endl;
            }

            std::cout << std::setw( 4 ) << step << std::setw( 0 ) << " "
                      << " " << std::right << std::setw( 12 )
                      << std::setprecision( 6 ) << std::scientific << new_cost
                      << " " << std::right << std::setw( 11 )
                      << std::setprecision( 2 ) << std::scientific
                      << prev_cost - new_cost << std::endl;
        }

        if ( step > 0 && ( std::abs( prev_cost - new_cost ) <
                           prev_cost * error_threshold ) )
        {
            status = "CONVERGENCE (Function tolerance reached.)";
            break;
        }
        prev_cost = new_cost;

        step++;
    }

    delete cost_function;
    if ( verbosity > 2 )
    {
        print_summary( status, initial_cost, prev_cost, step );
    }
    return result;
}

bool solve_spectral_transform(
    const std::vector<std::vector<double>> &source_RGB,
    const std::vector<std::vector<double>> &target_XYZ,
    double                                  error_threshold,
    size_t                                  max_steps,
    int                                     verbosity,
    std::vector<std::vector<double>>       &out_IDT_matrix )
{
    auto cost_function = new IDTOptimizationCost( source_RGB, target_XYZ );
    auto size          = source_RGB.size() * source_RGB[0].size();

    std::vector<double> beta_params = { 1.0, 0.0, 0.0, 1.0, 0.0, 0.0 };

    bool success = false;

#if RTA_HAS_CERES
    if ( use_ceres )
    {
        success = minimise_ceres(
            cost_function,
            beta_params,
            size,
            error_threshold,
            max_steps,
            verbosity );
    }
    else
#endif // RTA_HAS_CERES
    {
        check_ceres();
        success = minimise_builtin(
            cost_function,
            beta_params,
            size,
            error_threshold,
            max_steps,
            verbosity );
    }

    if ( success )
    {
        out_IDT_matrix.resize( 3 );
        out_IDT_matrix[0].resize( 3 );
        out_IDT_matrix[1].resize( 3 );
        out_IDT_matrix[2].resize( 3 );

        out_IDT_matrix[0][0] = beta_params[0];
        out_IDT_matrix[0][1] = beta_params[1];
        out_IDT_matrix[0][2] = 1.0 - beta_params[0] - beta_params[1];
        out_IDT_matrix[1][0] = beta_params[2];
        out_IDT_matrix[1][1] = beta_params[3];
        out_IDT_matrix[1][2] = 1.0 - beta_params[2] - beta_params[3];
        out_IDT_matrix[2][0] = beta_params[4];
        out_IDT_matrix[2][1] = beta_params[5];
        out_IDT_matrix[2][2] = 1.0 - beta_params[4] - beta_params[5];

        return true;
    }

    return false;
}

bool solve_spectral_transform(
    const std::vector<std::vector<double>> &source_RGB,
    const std::vector<std::vector<double>> &target_XYZ,
    int                                     verbosity,
    std::vector<std::vector<double>>       &out_IDT_matrix )
{
    return solve_spectral_transform(
        source_RGB, target_XYZ, 1e-17, 300, verbosity, out_IDT_matrix );
}

// Explicit instantiation
template std::vector<std::vector<double>> product<double>(
    const std::vector<std::vector<double>> &vct1,
    const std::vector<std::vector<double>> &vct2 );

// Explicit instantiation
template std::vector<double> product<double>(
    const std::vector<std::vector<double>> &vct1,
    const std::vector<double>              &vct2 );

// Explicit instantiation
template std::vector<std::vector<double>>
XYZ_to_LAB( const std::vector<std::vector<double>> &XYZ );

// Explicit instantiation
template std::vector<std::vector<double>> getCalcXYZt(
    const std::vector<std::vector<double>> &RGB, const double beta_params[6] );

// Explicit instantiation
template void XYZ_to_LAB( const double ( &in )[3], double ( &out )[3] );

// Explicit instantiation
template void XYZ_to_LAB_prime(
    const double ( &arg )[3],
    const double ( &arg_prime )[3],
    double ( &out )[3] );

} // namespace math
} // namespace core
} // namespace rta
