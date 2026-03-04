// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the rawtoaces Project.

#pragma once

#include <iostream>
#include <list>
#include <array>
#include <functional>
#include <OpenImageIO/imagebuf.h>
#include <rawtoaces/rawtoaces_core.h>

namespace rta
{

template <typename T, size_t S>
std::ostream &operator<<( std::ostream &os, const std::array<T, S> &array )
{
    os << "(";
    for ( size_t i = 0; i < S; i++ )
    {
        os << array[i] << ( i != S - 1 ? ", " : "" );
    }
    os << ")";
    return os;
}

template <typename... Ts, size_t... Is>
std::ostream &println_tuple_impl(
    std::ostream &os, std::tuple<Ts...> tuple, std::index_sequence<Is...> )
{
    static_assert(
        sizeof...( Is ) == sizeof...( Ts ),
        "Indices must have same number of elements as tuple types." );
    static_assert(
        sizeof...( Ts ) > 0, "Cannot insert empty tuple into stream." );
    auto last = sizeof...( Ts ) - 1; // assuming index sequence 0,...,N-1

    ( ( os << std::get<Is>( tuple ) << ( Is != last ? ", " : "" ) ), ... );
    return os;
}

template <typename... Ts>
std::ostream &operator<<( std::ostream &os, const std::tuple<Ts...> &tuple )
{
    return println_tuple_impl( os, tuple, std::index_sequence_for<Ts...>{} );
} // LCOV_EXCL_LINE - bug in coverage tool

std::ostream &operator<<( std::ostream &os, const rta::core::Metadata &data );

bool operator==(
    const rta::core::Metadata &data1, const rta::core::Metadata &data2 );

std::ostream &operator<<( std::ostream &os, const OIIO::ImageSpec &data );

bool operator==( const OIIO::ImageSpec &data1, const OIIO::ImageSpec &data2 );

namespace cache
{

template <class Descriptor, class Data> class Cache
{
public:
    Cache( const std::string &cache_name = "default" ) : name( cache_name ) {}

    const std::pair<bool, Data> &fetch(
        const Descriptor                        &descriptor,
        const std::function<bool( Data &data )> &func )
    {
        if ( disabled )
        {
            if ( verbosity > 0 )
            {
                std::cerr << "Cache (" << name << "): disabled." << std::endl;
            }
            _map.clear();
        }
        else
        {

            if ( verbosity > 0 )
            {
                std::cerr << "Cache (" << name << "): searching for an entry ["
                          << descriptor << "]." << std::endl;
            }

            for ( auto iter = _map.begin(); iter != _map.end(); ++iter )
            {
                if ( std::get<0>( *iter ) == descriptor )
                {
                    if ( iter != _map.begin() )
                    {
                        _map.splice(
                            _map.begin(), _map, iter, std::next( iter ) );
                    }

                    if ( verbosity > 0 )
                    {
                        std::cerr << "Cache (" << name << "): found in cache!"
                                  << std::endl;
                    }
                    return _map.front().second;
                }
            }

            if ( _map.size() == capacity )
            {
                _map.pop_back();
            }

            if ( verbosity > 0 )
            {
                std::cerr << "Cache (" << name
                          << "): not found. Calculating a new entry."
                          << std::endl;
            }
        }

        auto &entry        = _map.emplace_front();
        entry.first        = descriptor;
        entry.second.first = func( entry.second.second );
        return entry.second;
    };

    bool        disabled  = false;
    size_t      capacity  = 10;
    int         verbosity = 0;
    std::string name      = "default";

private:
    std::list<std::pair<Descriptor, std::pair<bool, Data>>> _map;
};

} // namespace cache
} // namespace rta
