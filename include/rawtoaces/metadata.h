// Copyright Contributors to the rawtoaces project.
// SPDX-License-Identifier: Apache-2.0
// https://github.com/AcademySoftwareFoundation/rawtoaces

#ifndef RTA_METADATA_H_
#define RTA_METADATA_H_

#include <vector>

namespace rta
{

struct Metadata
{
    // Colorimetry
    std::vector<double> cameraCalibration1;
    std::vector<double> cameraCalibration2;
    std::vector<double> xyz2rgbMatrix1;
    std::vector<double> xyz2rgbMatrix2;
    double              calibrationIlluminant1;
    double              calibrationIlluminant2;

    std::vector<double> analogBalance;
    std::vector<double> neutralRGB;

    double baselineExposure;

    friend bool operator==( const Metadata &m1, const Metadata &m2 )
    {
        if ( m1.calibrationIlluminant1 != m2.calibrationIlluminant1 )
            return false;
        if ( m1.calibrationIlluminant2 != m2.calibrationIlluminant2 )
            return false;
        if ( m1.baselineExposure != m2.baselineExposure )
            return false;
        if ( m1.analogBalance != m2.analogBalance )
            return false;
        if ( m1.neutralRGB != m2.neutralRGB )
            return false;
        if ( m1.cameraCalibration1 != m2.cameraCalibration1 )
            return false;
        if ( m1.cameraCalibration2 != m2.cameraCalibration2 )
            return false;
        if ( m1.xyz2rgbMatrix1 != m2.xyz2rgbMatrix1 )
            return false;
        if ( m1.xyz2rgbMatrix2 != m2.xyz2rgbMatrix2 )
            return false;

        return true;
    }

    friend bool operator!=( const Metadata &m1, const Metadata &m2 )
    {
        return !( m1 == m2 );
    }
};

} // namespace rta

#endif // RTA_METADATA_H_
