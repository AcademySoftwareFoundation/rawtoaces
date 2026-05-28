// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the rawtoaces Project.

#include "test_data_helpers.h"

#include <nlohmann/json.hpp>
#include <fstream>

// Parametric spectral curve in the form of Gaussian function with asymmetric
// shape: f(x) = a * exp( 1 / ((x - b)/2c)^2 ) )
// Where:
//    a - amplitude
//    b - central wavelength
//    c - shape, c = c1 to the left of the central wavelength, c2 otherwise.
struct SpectralCurve
{
    const char *name;
    float       a;
    float       b;
    float       c1;
    float       c2;
};

struct SpectralCurve HypotheticalCamera[] = {
    { "R", 0.7f, 600.0f, 50.0f, 40.0f },
    { "G", 1.0f, 520.0f, 40.0f, 50.0f },
    { "B", 0.9f, 450.0f, 20.0f, 60.0f }
};

struct SpectralCurve HypotheticalIlluminant[] = {
    { "power", 102.17f, 479.5f, 69.2f, 333.8f }
};

struct SpectralCurve HypotheticalObserver[] = {
    { "X", 1.06f, 599.0f, 36.0f, 31.4f },
    { "Y", 1.02f, 553.0f, 36.4f, 47.1f },
    { "Z", 1.86f, 444.1f, 16.9f, 28.6f }
};

struct SpectralCurve HypotheticalTrainingData[] = {
    { "dark skin", 0.23f, 853.2f, 243.3f, 868.5f },
    { "light skin", 0.84f, 876.7f, 254.5f, 935.0f },
    { "blue sky", 0.34f, 152.7f, 686.5f, 409.3f },
    { "foliage", 0.12f, 564.2f, 108.6f, 2765.7f },
    { "blue flower", 0.86f, -179871.6f, 281557.4f, 127679.8f },
    { "bluish green", 0.48f, 472.6f, 58.8f, 176.6f },
    { "orange", 0.64f, 636.9f, 63.0f, 25075.8f },
    { "purplish blue", 0.21f, -78.6f, 1605.9f, 6790.9f },
    { "moderate red", 0.61f, 646.9f, 48.2f, 8427.9f },
    { "purple", 2.67f, -2277037.9f, 3551360.2f, 991385.8f },
    { "yellow green", 0.44f, 531.5f, 37.1f, 323.0f },
    { "orange yellow", 0.67f, 617.9f, 67.3f, 28865.4f },
    { "blue", 0.30f, 450.5f, 35.5f, 42.1f },
    { "green", 0.25f, 512.2f, 36.0f, 137.9f },
    { "red", 0.72f, 655.4f, 35.3f, 17634.9f },
    { "yellow", 0.77f, 580.8f, 55.0f, 27054.0f },
    { "magenta", 0.54f, -597175.3f, 1237915.0f, 989153.3f },
    { "cyan", 0.42f, 474.6f, 57.0f, 67.8f },
    { "white 9.5 (.05 D)", 0.92f, 434.8f, 28.9f, 67276.6f },
    { "neutral 8 (.23 D)", 0.59f, 427.0f, 28.0f, 1079.5f },
    { "neutral 6.5 (.44 D)", 0.36f, 424.8f, 31.9f, 760.2f },
    { "neutral 5 (.70 D)", 0.19f, 426.6f, 42.1f, 645.7f },
    { "neutral 3.5 (1.05 D)", 0.09f, 532.9f, 337.3f, 431.0f },
    { "black 2 (1.5 D)", 0.03f, 1025.5f, 8281.3f, 10129.4f }
};

template <size_t size>
rta::core::SpectralData generate_spectral_data(
    const struct SpectralCurve ( &curves )[size],
    const std::string &manufacturer = "",
    const std::string &model        = "",
    const std::string &type         = "" )
{
    rta::core::SpectralData result;
    result.manufacturer     = manufacturer;
    result.model            = model;
    result.type             = type;
    result.license          = "Apache-2.0";
    result.document_creator = "rawtoaces";
    result.creation_date    = "2026-01-01T00:00:00Z";
    result.description = "Hypothetical spectral curves for testing purposes";

    result.data.emplace( "main", rta::core::SpectralData::SpectralSet() );

    rta::core::Spectrum::Shape shape = rta::core::Spectrum::ReferenceShape;

    for ( size_t i = 0; i < size; i++ )
    {
        auto  channel_name = curves[i].name;
        auto &channel      = result.data["main"].emplace_back(
            rta::core::SpectralData::SpectralChannel(
                channel_name, rta::core::Spectrum( 0, shape ) ) );

        float a  = curves[i].a;
        float b  = curves[i].b;
        float c1 = curves[i].c1;
        float c2 = curves[i].c2;

        size_t index = 0;
        for ( float wl = shape.first; wl <= shape.last; wl += shape.step )
        {
            float c     = wl < b ? c1 : c2;
            float value = a * expf( -powf( wl - b, 2.0f ) / 2.0f / c / c );
            channel.second.values[index] = value;
            index++;
        }
    }

    return result;
}

rta::core::SpectralData
create_hypothetical_camera( const std::string &make, const std::string &model )
{
    return generate_spectral_data( HypotheticalCamera, make, model, "" );
}

rta::core::SpectralData
create_hypothetical_illuminant( const std::string &type )
{
    return generate_spectral_data( HypotheticalIlluminant, "", "", type );
}

rta::core::SpectralData create_hypothetical_observer()
{
    return generate_spectral_data( HypotheticalObserver );
}

rta::core::SpectralData create_hypothetical_training_data()
{
    return generate_spectral_data( HypotheticalTrainingData );
}

void save_spectral_json(
    const rta::core::SpectralData &data, const std::string &filename )
{
    nlohmann::json json_header;

    json_header["schema_version"]         = "1.0.0";
    json_header["description"]            = data.description;
    json_header["document_creator"]       = data.document_creator;
    json_header["document_creation_date"] = data.creation_date;
    json_header["license"]                = data.license;

    if ( !data.manufacturer.empty() )
        json_header["manufacturer"] = data.manufacturer;

    if ( !data.model.empty() )
        json_header["model"] = data.model;

    if ( !data.type.empty() )
        json_header["type"] = data.type;

    nlohmann::json json_data;
    json_data["header"] = json_header;

    for ( const auto &set_item: data.data )
    {
        const auto &set_name = set_item.first;
        const auto &set_data = set_item.second;

        std::vector<std::string> channels;
        for ( const auto &item: set_data )
        {
            channels.push_back( item.first );
        }

        assert( !set_data.empty() );

        nlohmann::json json_spectral_data;

        const auto &shape = set_data[0].second.shape;

        size_t index = 0;
        for ( float wl = shape.first; wl <= shape.last; wl += shape.step )
        {
            std::vector<double> values( channels.size() );

            for ( size_t channel = 0; channel < channels.size(); channel++ )
            {
                values[channel] = set_data[channel].second.values[index];
            }

            json_spectral_data[std::to_string( int( wl ) )] = values;
            index++;
        }
        json_data["spectral_data"]["data"][set_name]  = json_spectral_data;
        json_data["spectral_data"]["index"][set_name] = channels;
    }

    std::ofstream file( filename );
    file << json_data.dump( 4 );
}
