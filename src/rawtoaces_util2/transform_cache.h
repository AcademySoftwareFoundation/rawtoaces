// Copyright Contributors to the rawtoaces project.
// SPDX-License-Identifier: Apache-2.0
// https://github.com/AcademySoftwareFoundation/rawtoaces

#include <rawtoaces/cache_base.h>

namespace rta
{
namespace cache
{

enum TransformEntryType
{
    Illum_from_WB,
    WB_from_Illum,
    Mat_from_Illum,
    Mat_from_DNG,
    Mat_from_nonDNG
};

struct Vector3
{
    double value[3];

    friend bool operator==( const Vector3 &v1, const Vector3 &v2 )
    {
        for ( size_t i = 0; i < 3; i++ )
            if ( v1.value[i] != v2.value[i] )
                return false;
        return true;
    }

    friend bool operator!=( const Vector3 &v1, const Vector3 &v2 )
    {
        return !( v1 == v2 );
    }
};

struct Matrix33
{
    double value[3][3];

    friend bool operator==( const Matrix33 &m1, const Matrix33 &m2 )
    {
        for ( size_t i = 0; i < 3; i++ )
            for ( size_t j = 0; j < 3; j++ )
                if ( m1.value[i][j] != m2.value[i][j] )
                    return false;
        return true;
    }

    friend bool operator!=( const Matrix33 &m1, const Matrix33 &m2 )
    {
        return !( m1 == m2 );
    }
};

using WB         = Vector3;
using Illuminant = std::string;

class TransformCacheEntryData
{
public:
    std::variant<std::pair<WB, Illuminant>, Matrix33> value;
};

class TransformDescriptor : public CacheEntryDescriptor<TransformCacheEntryData>
{
public:
    TransformEntryType type;
    std::string        camera_make;
    std::string        camera_model;
    std::variant<
        WB,                     // for Illum_from_WB
        Illuminant,             // for WB_from_Illum and Mat_from_Illum
        Metadata,               // for Mat_from_DNG
        std::pair<WB, Matrix33> // for Mat_from_nonDNG
        >
        value;

    bool   operator==( const TransformDescriptor &other ) const;
    bool   operator==( const CacheEntryDescriptor &other ) const override;
    size_t map_index() const override;
    bool
    fetch( TransformCacheEntryData &data, int verbosity = 0 ) const override;

    friend std::ostream &
    operator<<( std::ostream &stream, const TransformDescriptor &descriptor );

    std::tuple<> construct_entry() const { return std::tuple<>(); };
};

class TransformCache
    : public CacheBase<TransformDescriptor, TransformCacheEntryData, 5>
{
public:
    TransformCache() : CacheBase() { name = "RTA Transform cache"; }
};

std::vector<std::string> collectDataFiles( const std::string &type );
std::string              findFile( const std::string &filename );

} // namespace cache
} // namespace rta
