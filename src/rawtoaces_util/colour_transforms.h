// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the rawtoaces Project.

#pragma once

#include <string>
#include <vector>

#include <rawtoaces/rawtoaces_core.h>

namespace rta
{
namespace util
{

bool fetch_illuminant_from_multipliers(
    const std::string         &camera_make,
    const std::string         &camera_model,
    const std::vector<double> &wb_multipliers,
    core::SpectralSolver      &solver,
    int                        verbosity,
    bool                       disable_cache,
    std::string               &out_illuminant,
    std::string               &error_message );

bool fetch_multipliers_from_illuminant(
    const std::string    &camera_make,
    const std::string    &camera_model,
    const std::string    &in_illuminant,
    core::SpectralSolver &solver,
    int                   verbosity,
    bool                  disable_cache,
    std::vector<double>  &out_multipliers,
    std::string          &error_message );

bool fetch_matrix_from_illuminant(
    const std::string                &camera_make,
    const std::string                &camera_model,
    const std::string                &in_illuminant,
    core::SpectralSolver             &solver,
    int                               verbosity,
    bool                              disable_cache,
    std::vector<std::vector<double>> &out_matrix,
    std::string                       &error_message );

void fetch_matrix_from_metadata(
    const core::Metadata             &metadata,
    int                               verbosity,
    bool                              disable_cache,
    std::vector<std::vector<double>> &out_matrix );

} // namespace util
} // namespace rta
