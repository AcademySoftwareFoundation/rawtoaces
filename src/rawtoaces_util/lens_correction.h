// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the rawtoaces Project.

#pragma once

#include <string>
#include <vector>

#include <OpenImageIO/imagebuf.h>

namespace rta
{
namespace util
{

std::pair<bool, const OIIO::ImageBuf &> fetch_vignette_map(
    const OIIO::ImageSpec &spec,
    int                    verbosity,
    bool                   disable_cache,
    std::string           &error_message );

std::pair<bool, const OIIO::ImageBuf &> fetch_distortion_map(
    const OIIO::ImageSpec &spec,
    int                    verbosity,
    bool                   disable_cache,
    std::string           &error_message );

std::pair<bool, const OIIO::ImageBuf &> fetch_aberration_map(
    const OIIO::ImageSpec &spec,
    int                    verbosity,
    bool                   disable_cache,
    std::string           &error_message );

// _____________________________________________________________________________
// These are exposed for using in unit tests, may want to split then into a
// separate private header, but given that this one is not public, may as well
// leave them here for now.

OIIO::ImageSpec init_lens_map_spec(
    const OIIO::ImageSpec &src_spec,
    int                    channels,
    const std::string     &camera_make,
    const std::string     &camera_model,
    const std::string     &lens_make,
    const std::string     &lens_model,
    float                  focal_length,
    float                  aperture       = 0.0f,
    float                  focus_distance = 0.0f );

bool apply_vignette_map(
    OIIO::ImageBuf       &dst_buffer,
    const OIIO::ImageBuf &src_buffer,
    const std::string    &camera_make,
    const std::string    &camera_model,
    const std::string    &lens_make,
    const std::string    &lens_model,
    float                 focal_length,
    float                 aperture,
    float                 focus_distance,
    int                   verbosity,
    bool                  disable_cache,
    std::string          &error_message );

bool apply_distortion_map(
    OIIO::ImageBuf       &dst_buffer,
    const OIIO::ImageBuf &src_buffer,
    const std::string    &camera_make,
    const std::string    &camera_model,
    const std::string    &lens_make,
    const std::string    &lens_model,
    float                 focal_length,
    int                   verbosity,
    bool                  disable_cache,
    std::string          &error_message );

bool apply_aberration_map(
    OIIO::ImageBuf       &dst_buffer,
    const OIIO::ImageBuf &src_buffer,
    const std::string    &camera_make,
    const std::string    &camera_model,
    const std::string    &lens_make,
    const std::string    &lens_model,
    float                 focal_length,
    int                   verbosity,
    bool                  disable_cache,
    std::string          &error_message );

bool solve_vignette_map(
    const OIIO::ImageSpec &spec,
    bool                   inverse,
    OIIO::ImageBuf        &buffer,
    std::string           &error_message );

bool solve_distortion_map(
    const OIIO::ImageSpec &spec,
    bool                   inverse,
    OIIO::ImageBuf        &buffer,
    std::string           &error_message );

bool solve_aberration_map(
    const OIIO::ImageSpec &spec,
    bool                   inverse,
    OIIO::ImageBuf        &cache_data,
    std::string           &error_message );

void reset_database();

} // namespace util
} // namespace rta
