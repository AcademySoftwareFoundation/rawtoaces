// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the rawtoaces Project.

#pragma once

#include <iostream>
#include <list>
#include <array>
#include <functional>

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
}

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

        _map.emplace_front();

        auto &entry        = _map.front();
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
