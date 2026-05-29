// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the rawtoaces Project.

#pragma once

#include <rawtoaces/spectral_data.h>

// The make and model parameters only affect the metadata, not the curves shape.
rta::core::SpectralData create_hypothetical_camera(
    const std::string &make  = "TEST_CAMERA_MAKE",
    const std::string &model = "TEST_CAMERA_MODEL" );

// The type parameter only affects the metadata, not the curve shape.
rta::core::SpectralData
create_hypothetical_illuminant( const std::string &type = "TEST_ILLUMINANT" );

rta::core::SpectralData create_hypothetical_observer();

rta::core::SpectralData create_hypothetical_training_data();

void save_spectral_json(
    const rta::core::SpectralData &data, const std::string &filename );
