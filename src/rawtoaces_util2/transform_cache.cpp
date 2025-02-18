// Copyright Contributors to the rawtoaces project.
// SPDX-License-Identifier: Apache-2.0
// https://github.com/AcademySoftwareFoundation/rawtoaces

#include "transform_cache.h"

#include <rawtoaces/mathOps.h>
#include <rawtoaces/rta.h>

namespace rta
{
namespace cache
{

std::ostream &
operator<<( std::ostream &stream, const TransformDescriptor &descriptor )
{
    switch ( descriptor.type )
    {
        case TransformEntryType::Illum_from_WB:
            stream << "illuminant from WB weights:";
            break;
        case TransformEntryType::WB_from_Illum:
            stream << "WB weights from illuminant:";
            break;
        case TransformEntryType::Mat_from_Illum:
            stream << "IDT matrix from illuminant:";
            break;
        case TransformEntryType::Mat_from_DNG:
            stream << "IDT matrix from DNG metadata:";
            break;
        case TransformEntryType::Mat_from_nonDNG:
            stream << "IDT matrix from non-DNG metadata:";
            break;
    }

    stream << std::endl;
    stream << "  camera make: " << descriptor.camera_make << std::endl;
    stream << "  camera model: " << descriptor.camera_model << std::endl;

    switch ( descriptor.type )
    {
        case TransformEntryType::Illum_from_WB: {
            auto &wb = std::get<WB>( descriptor.value );

            stream << "  white balance: (";
            stream << wb.value[0] << ", ";
            stream << wb.value[1] << ", ";
            stream << wb.value[2] << ")";
            break;
        }
        case TransformEntryType::WB_from_Illum:
        case TransformEntryType::Mat_from_Illum: {
            auto &illum = std::get<Illuminant>( descriptor.value );
            stream << "  illuminant: " << illum;
        }
        case TransformEntryType::Mat_from_DNG:
        case TransformEntryType::Mat_from_nonDNG: break;
    }

    stream << std::endl;
    return stream;
}

bool TransformDescriptor::operator==( const TransformDescriptor &other ) const
{
    if ( camera_make != other.camera_make )
        return false;
    if ( camera_model != other.camera_model )
        return false;
    if ( value != other.value )
        return false;
    return true;
}

bool TransformDescriptor::operator==( const CacheEntryDescriptor &other ) const
{
    return false;
}

size_t TransformDescriptor::map_index() const
{
    return (size_t)type;
}

std::vector<std::string> collectDataFiles( const std::string &type )
{
    std::vector<std::string> result;

    auto paths = pathsFinder();

    for ( auto &path: paths.paths )
    {
        if ( std::filesystem::is_directory( path ) )
        {
            auto type_path = path + "/" + type;
            if ( std::filesystem::exists( type_path ) )
            {
                auto it = std::filesystem::directory_iterator( type_path );
                for ( auto filename2: it )
                {
                    auto p = filename2.path();
                    if ( filename2.path().extension() == ".json" )
                    {
                        result.push_back( filename2.path().string() );
                    }
                }
            }
        }
    }
    return result;
}

std::string findFile( const std::string &filename )
{
    auto paths = pathsFinder();

    for ( auto &path: paths.paths )
    {
        std::string full_path = path + "/" + filename;
        if ( std::filesystem::exists( full_path ) )
            return full_path;
    }
    return "";
}

void prepare_solver(
    Idt              &idt,
    const std::string camera_make,
    const std::string camera_model,
    const std::string illuminant_name,
    int               verbosity )
{
    idt.setVerbosity( verbosity );

    bool found_camera = false;
    auto camera_paths = collectDataFiles( "camera" );

    for ( auto &path: camera_paths )
    {
        if ( idt.loadCameraSpst(
                 path, camera_make.c_str(), camera_model.c_str() ) )
        {
            found_camera = true;
            break;
        }
    }

    if ( !found_camera )
    {
        std::cerr << "Camera spectral sensitivity data not found for "
                  << camera_make << " " << camera_model << ". "
                  << "Please check that the data is available "
                  << "at the location(s) specified in RAWTOACES_DATA_PATH"
                  << std::endl;
        exit( 1 );
    }

    auto training = findFile( "training/training_spectral.json" );
    if ( training.length() )
    {
        idt.loadTrainingData( training );
    }

    auto cmf = findFile( "cmf/cmf_1931.json" );
    if ( cmf.length() )
    {
        idt.loadCMF( cmf );
    }

    auto illum_paths = collectDataFiles( "illuminant" );
    int  res         = idt.loadIlluminant( illum_paths, illuminant_name );
}

bool TransformDescriptor::fetch(
    TransformCacheEntryData &data, int verbosity ) const
{
    switch ( type )
    {
        case TransformEntryType::Illum_from_WB: {
            auto               &wb1 = std::get<WB>( value );
            std::vector<double> wb2( 3 );
            wb2[0] = wb1.value[0];
            wb2[1] = wb1.value[1];
            wb2[2] = wb1.value[2];

            Idt idt;
            prepare_solver( idt, camera_make, camera_model, "na", verbosity );
            idt.chooseIllumSrc( wb2, 0 );

            WB   wb;
            auto wb3    = idt.getWB();
            wb.value[0] = wb3[0];
            wb.value[1] = wb3[1];
            wb.value[2] = wb3[2];
            auto illum  = idt.getBestIllum();

            data.value = std::pair<WB, std::string>( wb, illum.getIllumType() );
            break;
        }
        case TransformEntryType::WB_from_Illum: {
            auto &illum = std::get<Illuminant>( value );

            Idt idt;
            prepare_solver( idt, camera_make, camera_model, illum, verbosity );
            idt.chooseIllumType( illum.c_str(), 0 );

            WB   wb;
            auto wb2    = idt.getWB();
            wb.value[0] = wb2[0];
            wb.value[1] = wb2[1];
            wb.value[2] = wb2[2];
            auto illum2 = idt.getBestIllum();

            data.value =
                std::pair<WB, std::string>( wb, illum2.getIllumType() );
            break;
        }
        case TransformEntryType::Mat_from_Illum: {
            auto &illum = std::get<Illuminant>( value );

            Idt idt;
            prepare_solver( idt, camera_make, camera_model, illum, verbosity );

            // TODO: this also calculates the WB mults, which are not needed
            // here. This creates a bit of overhead, we may want to decouple
            // the selection of illuminant and WB calculation in the core
            // library. But this is still faster than running the matrix solver
            // everytime.
            idt.chooseIllumType( illum.c_str(), 0 );

            if ( idt.calIDT() )
            {
                auto      mat1 = idt.getIDT();
                Matrix33 &mat2 = data.value.emplace<Matrix33>();

                for ( size_t i = 0; i < 3; i++ )
                    for ( size_t j = 0; j < 3; j++ )
                        mat2.value[i][j] = mat1[i][j];
            }
            else
                return false;

            break;
        }
        case TransformEntryType::Mat_from_DNG: {
            auto &metadata = std::get<Metadata>( value );

            DNGIdt   *dng  = new DNGIdt( metadata );
            auto      mat1 = dng->getDNGIDTMatrix();
            Matrix33 &mat2 = data.value.emplace<Matrix33>();

            for ( size_t i = 0; i < 3; i++ )
                for ( size_t j = 0; j < 3; j++ )
                    mat2.value[i][j] = mat1[i][j];
            break;
        }
        case TransformEntryType::Mat_from_nonDNG: {
            auto p = std::get<std::pair<WB, Matrix33>>( value );

            Vector3                         &wb   = p.first;
            Matrix33                        &mat1 = p.second;
            std::vector<std::vector<double>> xyz2cam;

            xyz2cam.resize( 3 );
            for ( size_t row = 0; row < 3; row++ )
            {
                xyz2cam[row].resize( 3 );

                for ( size_t col = 0; col < 3; col++ )
                {
                    xyz2cam[row][col] = mat1.value[row][col];
                }
            }

            auto cam2xyz = invertVM( xyz2cam );

            for ( size_t row = 0; row < 3; row++ )
            {
                for ( size_t col = 0; col < 3; col++ )
                {
                    cam2xyz[row][col] /= wb.value[row];
                }
            }

            Matrix33 &mat2 = data.value.emplace<Matrix33>();

            for ( size_t i = 0; i < 3; i++ )
                for ( size_t j = 0; j < 3; j++ )
                    mat2.value[i][j] = cam2xyz[i][j];

            break;
        }
    }

    return true;
}

template class cache::
    CacheBase<TransformDescriptor, TransformCacheEntryData, 5>;

} // namespace cache
} // namespace rta
