// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the rawtoaces Project.

#include <rawtoaces/spectral_data.h>

#include <assert.h>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

namespace rta
{
namespace core
{

bool Spectrum::Shape::operator==( const Spectrum::Shape &shape ) const
{
    return first == shape.first && last == shape.last && step == shape.step;
}

Spectrum::Spectrum( double value, const Shape &reference_shape )
    : shape( reference_shape )
{
    if ( shape.step > 0 )
        values.resize(
            static_cast<size_t>(
                ( shape.last - shape.first + shape.step ) / shape.step ),
            value );
}

template <typename Val_or_Ref, typename F>
static Val_or_Ref op( Val_or_Ref lhs, const Spectrum &rhs, F func )
{
    assert( lhs.shape == rhs.shape );
    assert( lhs.values.size() == rhs.values.size() );

    auto l = lhs.values.begin();
    auto r = rhs.values.begin();
    while ( l != lhs.values.end() )
        *l++ = func( *l, *r++ );
    return lhs;
}

Spectrum operator+( Spectrum lhs, const Spectrum &rhs )
{
    return op<Spectrum>( lhs, rhs, std::plus<double>() );
}

Spectrum operator-( Spectrum lhs, const Spectrum &rhs )
{
    return op<Spectrum>( lhs, rhs, std::minus<double>() );
}

Spectrum operator*( Spectrum lhs, const Spectrum &rhs )
{
    return op<Spectrum>( lhs, rhs, std::multiplies<double>() );
}

Spectrum operator/( Spectrum lhs, const Spectrum &rhs )
{
    return op<Spectrum>( lhs, rhs, std::divides<double>() );
}

Spectrum &Spectrum::operator+=( const Spectrum &rhs )
{
    return op<Spectrum &>( *this, rhs, std::plus<double>() );
}

Spectrum &Spectrum::operator-=( const Spectrum &rhs )
{
    return op<Spectrum &>( *this, rhs, std::minus<double>() );
}

Spectrum &Spectrum::operator*=( const Spectrum &rhs )
{
    return op<Spectrum &>( *this, rhs, std::multiplies<double>() );
}

Spectrum &Spectrum::operator/=( const Spectrum &rhs )
{
    return op<Spectrum &>( *this, rhs, std::divides<double>() );
}

void Spectrum::reshape()
{
    if ( shape == ReferenceShape )
        return;

    std::vector<double> temp;
    size_t              src    = 0;
    float               wl_src = shape.first;
    float               wl_dst = ReferenceShape.first;

    while ( wl_dst <= ReferenceShape.last )
    {
        if ( wl_src < wl_dst )
        {
            if ( src < values.size() - 1 )
            {
                float next_wl_src = shape.first + shape.step * ( src + 1 );
                if ( next_wl_src <= wl_dst )
                {
                    // The next source wavelength is still not big enough,
                    // advancing.
                    src++;
                    wl_src = next_wl_src;
                }
                else
                {
                    // The target wavelength is between two source samples,
                    // linearly interpolating.
                    double ratio =
                        ( wl_dst - wl_src ) / ( next_wl_src - wl_src );
                    double vv =
                        values[src] * ( 1.0 - ratio ) + values[src + 1] * ratio;
                    temp.push_back( vv );
                    wl_dst = ReferenceShape.first +
                             ReferenceShape.step * temp.size();
                }
            }
            else
            {
                // We have passed all available source samples,
                // copying the last sample.
                temp.push_back( values[src] );
                wl_dst =
                    ReferenceShape.first + ReferenceShape.step * temp.size();
            }
        }
        else if ( wl_src == wl_dst )
        {
            // Found an exact match, just copy it over.
            temp.push_back( values[src] );
            wl_dst = ReferenceShape.first + ReferenceShape.step * temp.size();
        }
        else
        {
            // Haven't reached the available source range yet, advancing.
            temp.push_back( values[src] );
            wl_dst = ReferenceShape.first + ReferenceShape.step * temp.size();
        }
    }

    values = temp;
    shape  = ReferenceShape;
}

double Spectrum::integrate()
{
    double result = 0;
    for ( auto &v: values )
        result += v;
    return result;
}

double Spectrum::max() const
{
    if ( values.empty() )
        return 0;
    return *std::max_element( values.begin(), values.end() );
}

inline void
parse_string( nlohmann::json &j, std::string &dst, const std::string &key )
{
    assert( !key.empty() );

    auto &v = j[key];
    if ( v.is_null() )
        dst = "";
    else
        dst = v;
}

bool SpectralData::load( const std::string &path, bool reshape )
{
    // Reset all in case the object has been initialised before.
    manufacturer.erase();
    model.erase();
    illuminant.erase();
    catalog_number.erase();
    description.erase();
    document_creator.erase();
    unique_identifier.erase();
    measurement_equipment.erase();
    laboratory.erase();
    creation_date.erase();
    comments.erase();
    license.erase();
    units.erase();
    reflection_geometry.erase();
    transmission_geometry.erase();
    bandwidth_FWHM.erase();
    bandwidth_corrected.erase();
    data.clear();

    core::Spectrum::Shape shape;

    try
    {
        std::ifstream i( path );
        if ( !i.is_open() )
        {
            std::cerr << "Error: Failed to open file " << path << "."
                      << std::endl;
            return false;
        }
        nlohmann::json file_data = nlohmann::json::parse( i );

        nlohmann::json &h = file_data["header"];
        parse_string( h, manufacturer, "manufacturer" );
        parse_string( h, model, "model" );
        parse_string( h, illuminant, "illuminant" );
        parse_string( h, catalog_number, "catalog_number" );
        parse_string( h, description, "description" );
        parse_string( h, document_creator, "document_creator" );
        parse_string( h, unique_identifier, "unique_identifier" );
        parse_string( h, measurement_equipment, "measurement_equipment" );
        parse_string( h, laboratory, "laboratory" );
        parse_string( h, creation_date, "document_creation_date" );
        parse_string( h, comments, "comments" );
        parse_string( h, license, "license" );

        nlohmann::json &d = file_data["spectral_data"];
        parse_string( d, units, "units" );
        parse_string( d, reflection_geometry, "reflection_geometry" );
        parse_string( d, transmission_geometry, "transmission_geometry" );
        parse_string( d, bandwidth_FWHM, "bandwidth_FWHM" );
        parse_string( d, bandwidth_corrected, "bandwidth_corrected" );

        float prev_wavelength = -1;

        std::vector<float>       wavelengths;
        std::vector<std::string> keys;

        nlohmann::json spectral_index = file_data["spectral_data"]["index"];

        for ( auto &[set_name, set_channels]: spectral_index.items() )
        {
            auto set_entry =
                data.emplace( set_name, SpectralData::SpectralSet() ).first;

            for ( auto channel_name: set_channels )
            {
                set_entry->second.emplace_back( SpectralData::SpectralChannel(
                    channel_name, Spectrum( 0, Spectrum::EmptyShape ) ) );
            }
        }

        nlohmann::json spectral_data = file_data["spectral_data"]["data"];

        for ( auto &[set_name, set_values]: spectral_data.items() )
        {
            auto  &set_entry = data[set_name];
            size_t count     = set_entry.size();

            for ( auto &[bin_wavelength, bin_values]:
                  spectral_data[set_name].items() )
            {
                float this_wavelength = std::stof( bin_wavelength );

                if ( prev_wavelength != -1 )
                {
                    float new_step = this_wavelength - prev_wavelength;

                    if ( shape.step != 0 && new_step != shape.step )
                    {
                        std::cerr << "Error: Inconsistent wavelength step "
                                  << "detected in " << path
                                  << ". Expected: " << shape.step
                                  << ", got: " << new_step << "." << std::endl;
                        return false;
                    }

                    shape.step = new_step;
                }
                else
                {
                    shape.first = this_wavelength;
                }

                prev_wavelength = this_wavelength;

                for ( size_t j = 0; j < count; j++ )
                {
                    set_entry[j].second.values.push_back( bin_values[j] );
                }
            }
        }

        shape.last = prev_wavelength;

        for ( auto &[k, v]: data )
        {
            for ( auto &vv: v )
            {
                vv.second.shape = shape;
                if ( reshape )
                {
                    vv.second.reshape();
                }
            }
        }
    }
    catch ( nlohmann::detail::parse_error &error )
    {
        std::cerr << "Error: JSON parsing of " << path
                  << " failed with error: " << error.what() << std::endl;
        return false;
    }
    catch ( const std::exception &error )
    {
        std::cerr << "Error: JSON parsing of " << path
                  << " failed with error: " << error.what() << std::endl;
        return false;
    }

    return true;
}

Spectrum &SpectralData::get( std::string set_name, std::string channel_name )
{
    if ( data.count( set_name ) != 1 )
    {
        throw std::invalid_argument(
            "The requested data set '" + set_name +
            "' not found in spectral data." );
    }

    auto &set_data = data.at( set_name );
    auto  it       = std::find_if(
        set_data.begin(),
        set_data.end(),
        [&]( const SpectralData::SpectralChannel &x ) {
            return x.first == channel_name;
        } );
    if ( it == set_data.end() )
    {
        throw std::invalid_argument(
            "The requested channel '" + channel_name +
            "' not found in the data set '" + set_name +
            "' of spectral data." );
    }
    return it->second;
}

const Spectrum &
SpectralData::get( std::string set_name, std::string channel_name ) const
{
    assert( data.count( set_name ) == 1 );

    const auto &set_data = data.at( set_name );
    auto        it       = std::find_if(
        set_data.begin(),
        set_data.end(),
        [&]( const SpectralData::SpectralChannel &x ) {
            return x.first == channel_name;
        } );
    return it->second;
}

Spectrum &SpectralData::operator[]( std::string name )
{
    return get( "main", name );
}

const Spectrum &SpectralData::operator[]( std::string name ) const
{
    return get( "main", name );
}

} // namespace core
} // namespace rta
