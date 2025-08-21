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

        /// The leftmost sample's wavelength in nanometers.
        float last = 0;

        /// The sampling step in nanometers.
        float step = 0;

        /// Comparison operator, mostly required for storing the `Spectrum`
        /// data in containers.
        /// - parameter shape: another `Shape` object to compare `this` with.
        /// - returns: `true` if the objects are equal.
        bool operator==( const Shape &shape ) const;
    } shape;

    /// The reference shape to use with all `Spectrum` objects by default.
    /// These are the values used by rawtoaces internally.
    inline static Shape ReferenceShape = { 380, 780, 5 };

    /// An empty shape. Useful for creatin a `Spectrum` objects without
    /// allocating any samples.
    inline static Shape EmptyShape = { 0, 0, 0 };

    /// The spectral samples storage.
    std::vector<double> values;

    /// The `Spectrum` object constructor. Allocates as many spectral samples
    /// as required for the `shape` parameter, and initialises them with
    /// `value`.
    /// - parameter value: the value to initialise the spectral samples with.
    /// - parameter shape: the shape of the spectral data to create. Pass a shape
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
    /// - returns: the sum of all elements in `values`.
    double integrate();

    /// Find the maximum element in `values`
    /// - returns: the maximum element in `values`.
    double max() const;
};

/// A data-class for storing spectral data, based on the file format used in
/// [rawtoaces-data](https://github.com/AcademySoftwareFoundation/rawtoaces-data).
struct SpectralData
{
    /// Header data
    std::string manufacturer;
    std::string model;
    std::string illuminant;
    std::string catalog_number;
    std::string description;
    std::string document_creator;
    std::string unique_identifier;
    std::string measurement_equipment;
    std::string laboratory;
    std::string creation_date;
    std::string comments;
    std::string license;

    // Spectral data
    std::string units;
    std::string reflection_geometry;
    std::string transmission_geometry;
    std::string bandwidth_FWHM;
    std::string bandwidth_corrected;

    /// A spectral channel, contains a channel name and the corresponding
    /// `Spectrum` object. Can represent a single curve in an RGB or XYZ triplet.
    typedef std::pair<std::string, Spectrum> SpectralChannel;

    /// A spectral set, may contain one or multiple spectral channels, like an
    /// RGB or XYZ triplet.
    typedef std::vector<SpectralChannel> SpectralSet;

    /// The spectral data storage.
    std::map<std::string, SpectralSet> data;

    bool load( const std::string &path, bool reshape = true );

    /// A convenience operator returning the `Spectrum` of a given channel name
    /// in the "main" data set.
    /// - parameter name: the channel name in the "main" data set to return.
    /// - returns: the `Spectrum` object corresponding to the given channel
    /// name.
    /// - throws: if the requested channel is not found.
    Spectrum       &operator[]( std::string name );
    const Spectrum &operator[]( std::string name ) const;

    /// A convenience method returning the `Spectrum` of a given channel name
    /// in the given data set.
    /// - parameter set_name: the set name to search for.
    /// - parameter channel_name: the channel name to search for.
    /// - returns: the `Spectrum` object reference if found.
    /// name.
    /// - throws: if the requested channel is not found.
    Spectrum       &get( std::string set_name, std::string channel_name );
    const Spectrum &get( std::string set_name, std::string channel_name ) const;
};

} // namespace core
} // namespace rta
