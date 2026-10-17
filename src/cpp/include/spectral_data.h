// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the rawtoaces Project.

#pragma once

#include <string>
#include <vector>
#include <map>

namespace rta
{
namespace core
{

/// A data class for storing a spectral curve. Implements a few arithmetic
/// operations and simple reshaping via linear interpolation.
struct Spectrum
{
    /// The spectral data sampling information. Only regular step data sets are
    /// currently supported.  All values are wavelength in nanometers.
    struct Shape
    {
        /// The leftmost sample's wavelength in nanometers.
        float first = 0;

        /// The rightmost sample's wavelength in nanometers.
        float last = 0;

        /// The sampling step in nanometers.
        float step = 0;

        /// Comparison operator, mostly required for storing the `Spectrum`
        /// data in containers.
        /// @param shape another `Shape` object to compare `this` with.
        /// @return `true` if the objects are equal.
        bool operator==( const Shape &shape ) const;
    } shape;

    /// The reference shape to use with all `Spectrum` objects by default.
    /// These are the values used by rawtoaces internally.
    inline static Shape ReferenceShape = { 380, 780, 5 };

    /// An empty shape. Useful for creating `Spectrum` objects without
    /// allocating any samples.
    inline static Shape EmptyShape = { 0, 0, 0 };

    /// The spectral samples storage.
    std::vector<double> values;

    /// The `Spectrum` object constructor. Allocates as many spectral samples
    /// as required for the `shape` parameter, and initialises them with
    /// `value`.
    /// @param value the value to initialise the spectral samples with.
    /// @param shape the shape of the spectral data to create. Pass a shape
    /// with zero step, like `rta::core::Spectrum::EmptyShape` to avoid
    /// allocating any samples.
    Spectrum( double value = 0, const Shape &shape = ReferenceShape );

    /// Per-element addition operator.
    friend Spectrum operator+( Spectrum lhs, const Spectrum &rhs );

    /// Per-element subtraction operator.
    friend Spectrum operator-( Spectrum lhs, const Spectrum &rhs );

    /// Per-element multiplication operator.
    friend Spectrum operator*( Spectrum lhs, const Spectrum &rhs );

    /// Per-element division operator.
    friend Spectrum operator/( Spectrum lhs, const Spectrum &rhs );

    /// Per-element addition operator.
    Spectrum &operator+=( const Spectrum &rhs );

    /// Per-element subtraction operator.
    Spectrum &operator-=( const Spectrum &rhs );

    /// Per-element multiplication operator.
    Spectrum &operator*=( const Spectrum &rhs );

    /// Per-element division operator.
    Spectrum &operator/=( const Spectrum &rhs );

    /// Reshape the `Spectrum` object to the reference shape
    /// (`rta::core::Spectrum::ReferenceShape`).
    void reshape();

    /// Integrate the spectral curve.
    /// @return the sum of all elements in `values`.
    double integrate() const;

    /// Find the maximum element in `values`
    /// @return the maximum element in `values`.
    double max() const;
};

/// A data-class for storing spectral data, based on the file format used in
/// [rawtoaces-data](https://github.com/AcademySoftwareFoundation/rawtoaces-data).
struct SpectralData
{
    /// Manufacturer of the device being tested.
    std::string manufacturer;
    /// Model of the device being tested.
    std::string model;
    /// Type of the spectral dataset.
    std::string type;
    /// Description of the spectral dataset.
    std::string description;
    /// Creator of the document, e.g. company, individual, laboratory, etc.
    std::string document_creator;
    /// Generated unique identifier for the document, e.g. SHA256.
    std::string unique_identifier;
    /// Measurement equipment used to test the device.
    std::string measurement_equipment;
    /// Laboratory or company that performed the measurements.
    std::string laboratory;
    /// Document creation date expressed as per RFC 3339 - Date and Time on the
    /// Internet: Timestamps, e.g. `2017-01-01T12:00:00Z`.
    std::string creation_date;
    /// Additional information for the spectral dataset.
    std::string comments;
    /// Usage license of the document, e.g. "CC-BY-NC-ND".
    std::string license;

    /// Unit or quantity of measurement for the spectral dataset.
    std::string units;
    /// Reflection geometry attributes as per CIE 15:2004.
    /// Required if units is reflectance.
    std::string reflection_geometry;
    /// Transmission geometry attributes as per CIE 15:2004.
    /// Required if units is transmittance.
    std::string transmission_geometry;
    /// Spectro-radiometer full-width at half-maximum bandwidth in nm.
    std::string bandwidth_FWHM;
    /// Whether bandwidth correction has been applied to the spectral data.
    std::string bandwidth_corrected;

    /// A spectral channel, contains a channel name and the corresponding
    /// `Spectrum` object. Can represent a single curve in an RGB or XYZ triplet.
    typedef std::pair<std::string, Spectrum> SpectralChannel;

    /// A spectral set, may contain one or multiple spectral channels, like an
    /// RGB or XYZ triplet.
    typedef std::vector<SpectralChannel> SpectralSet;

    /// The spectral data storage.
    std::map<std::string, SpectralSet> data;

    /// Loads spectral data from a given file path.
    /// @param path a path to the file to load data from
    /// @param reshape if set to `true`, the data will be reshaped to the
    /// reference shape (`rta::core::Spectrum::ReferenceShape`).
    /// @param error_message an optional destination for any error
    /// message that occurred during loading.
    /// @return `true` if loaded successfully.
    bool load(
        const std::string &path,
        bool               reshape       = true,
        std::string       *error_message = nullptr );

    /// A convenience operator returning the `Spectrum` of a given channel name
    /// in the "main" data set.
    /// @param name the channel name in the "main" data set to return.
    /// @return the `Spectrum` object corresponding to the given channel
    /// name.
    /// @throws std::invalid_argument if the requested channel is not found
    /// (non-const overload; the const overload requires the channel to exist).
    Spectrum       &operator[]( std::string name );
    const Spectrum &operator[]( std::string name ) const;

    /// A convenience method returning the `Spectrum` of a given channel name
    /// in the given data set.
    /// @param set_name the set name to search for.
    /// @param channel_name the channel name to search for.
    /// @return the `Spectrum` object reference if found.
    /// @throws std::invalid_argument if the set or channel is not found
    /// (non-const overload).
    Spectrum       &get( std::string set_name, std::string channel_name );
    const Spectrum &get( std::string set_name, std::string channel_name ) const;
};

} // namespace core
} // namespace rta
