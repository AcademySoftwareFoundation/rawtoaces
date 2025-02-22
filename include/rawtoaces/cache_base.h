// Copyright Contributors to the rawtoaces project.
// SPDX-License-Identifier: Apache-2.0
// https://github.com/AcademySoftwareFoundation/rawtoaces

#include <iostream>
#include <filesystem>

#include <string>
#include <list>
#include <variant>

#include <rawtoaces/metadata.h>

namespace rta
{
namespace cache
{

template <class CacheEntryData> class CacheEntryDescriptor
{
public:
    virtual size_t map_index() const                                      = 0;
    virtual bool   fetch( CacheEntryData &data, int verbosity = 0 ) const = 0;
    virtual bool   operator==( const CacheEntryDescriptor &other ) const  = 0;

    // Each subclass must also implement the folowing 2 methods.
    // Not sure how to declare them here.

    // friend std::ostream& operator<< (std::ostream& stream, const DescriptorBase& descriptor);
    // std::tuple<Types && ...> construct_entry() const
};

template <class Descriptor, class CacheEntryData, size_t size> class CacheBase
{
public:
    const CacheEntryData &fetch( const Descriptor &descriptor )
    {
        size_t map_index = descriptor.map_index();

        std::list<std::pair<Descriptor, CacheEntryData>> &map =
            _maps[map_index];

        if ( disabled )
        {
            if ( verbosity > 0 )
            {
                std::cerr << name << ": disabled " << std::endl;
            }
            map.clear();
        }
        else
        {

            if ( verbosity > 0 )
            {
                std::cerr << name << ": searching for a " << descriptor;
            }

            for ( auto iter = map.begin(); iter != map.end(); ++iter )
            {
                if ( iter->first == descriptor )
                {
                    if ( iter != map.begin() )
                    {
                        map.splice( map.begin(), map, iter, std::next( iter ) );
                    }

                    if ( verbosity > 0 )
                    {
                        std::cerr << name << ": found in cache!" << std::endl;
                    }
                    return map.front().second;
                }
            }

            if ( map.size() == capacity )
            {
                map.pop_back();
            }

            if ( verbosity > 0 )
            {
                std::cerr << name << ": not found. Calculating a new entry."
                          << std::endl;
            }
        }

        map.emplace_front(
            std::piecewise_construct,
            std::forward_as_tuple( descriptor ),
            descriptor.construct_entry() );

        CacheEntryData &data = map.front().second;
        descriptor.fetch( data, verbosity );
        return data;
    };

    bool        disabled  = false;
    int         capacity  = 10;
    int         verbosity = 0;
    std::string name      = "Cache";

private:
    std::list<std::pair<Descriptor, CacheEntryData>> _maps[size];
};

} // namespace cache
} // namespace rta
