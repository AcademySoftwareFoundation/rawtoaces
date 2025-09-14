// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the rawtoaces Project.

#ifdef WIN32
#    define WIN32_LEAN_AND_MEAN
#    include <windows.h>
#endif

#include <filesystem>
#include <OpenImageIO/unittest.h>

#include "../src/rawtoaces_core/mathOps.h"
#include <rawtoaces/rawtoaces_core.h>

#define DATA_PATH "../_deps/rawtoaces_data-src/data/"

void init_Spectrum( rta::core::Spectrum &spectrum )
{
    for ( size_t i = 0; i < spectrum.values.size(); i++ )
        spectrum.values[i] = static_cast<double>( i );
}

void check_Spectrum(
    const rta::core::Spectrum       &spectrum,
    const rta::core::Spectrum::Shape shape =
        rta::core::Spectrum::ReferenceShape )
{
    OIIO_CHECK_EQUAL( spectrum.shape.first, shape.first );
    OIIO_CHECK_EQUAL( spectrum.shape.last, shape.last );
    OIIO_CHECK_EQUAL( spectrum.shape.step, shape.step );
    OIIO_CHECK_EQUAL(
        spectrum.values.size(),
        ( shape.last - shape.first + shape.step ) / shape.step );

    for ( size_t i = 0; i < spectrum.values.size(); i++ )
        OIIO_CHECK_EQUAL( spectrum.values[i], i );
}

void testSpectralData_Spectrum()
{
    rta::core::Spectrum spectrum1;
    init_Spectrum( spectrum1 );
    check_Spectrum( spectrum1 );

    rta::core::Spectrum spectrum2 = spectrum1;
    check_Spectrum( spectrum2 );

    rta::core::Spectrum::Shape shape = { 20, 50, 10 };
    rta::core::Spectrum        spectrum3( 0, shape );
    init_Spectrum( spectrum3 );
    check_Spectrum( spectrum3, shape );
}

void init_SpectralData( rta::core::SpectralData &data )
{
    data.manufacturer          = "manufacturer";
    data.model                 = "model";
    data.illuminant            = "illuminant";
    data.catalog_number        = "catalog_number";
    data.description           = "description";
    data.document_creator      = "document_creator";
    data.unique_identifier     = "unique_identifier";
    data.measurement_equipment = "measurement_equipment";
    data.laboratory            = "laboratory";
    data.creation_date         = "creation_date";
    data.comments              = "comments";
    data.license               = "license";
    data.units                 = "units";
    data.reflection_geometry   = "reflection_geometry";
    data.transmission_geometry = "transmission_geometry";
    data.bandwidth_FWHM        = "bandwidth_FWHM";
    data.bandwidth_corrected   = "bandwidth_corrected";

    auto &entry = data.data["main"];
    entry.emplace_back( std::pair<std::string, rta::core::Spectrum>(
        "channel1", rta::core::Spectrum() ) );
    entry.emplace_back( std::pair<std::string, rta::core::Spectrum>(
        "channel2", rta::core::Spectrum() ) );
    init_Spectrum( data["channel1"] );
    init_Spectrum( data["channel2"] );
}

void check_SpectralData( const rta::core::SpectralData &data )
{
    OIIO_CHECK_EQUAL( data.manufacturer, "manufacturer" );
    OIIO_CHECK_EQUAL( data.model, "model" );
    OIIO_CHECK_EQUAL( data.illuminant, "illuminant" );
    OIIO_CHECK_EQUAL( data.catalog_number, "catalog_number" );
    OIIO_CHECK_EQUAL( data.description, "description" );
    OIIO_CHECK_EQUAL( data.document_creator, "document_creator" );
    OIIO_CHECK_EQUAL( data.unique_identifier, "unique_identifier" );
    OIIO_CHECK_EQUAL( data.measurement_equipment, "measurement_equipment" );
    OIIO_CHECK_EQUAL( data.laboratory, "laboratory" );
    OIIO_CHECK_EQUAL( data.creation_date, "creation_date" );
    OIIO_CHECK_EQUAL( data.comments, "comments" );
    OIIO_CHECK_EQUAL( data.license, "license" );
    OIIO_CHECK_EQUAL( data.units, "units" );
    OIIO_CHECK_EQUAL( data.reflection_geometry, "reflection_geometry" );
    OIIO_CHECK_EQUAL( data.transmission_geometry, "transmission_geometry" );
    OIIO_CHECK_EQUAL( data.bandwidth_FWHM, "bandwidth_FWHM" );
    OIIO_CHECK_EQUAL( data.bandwidth_corrected, "bandwidth_corrected" );

    OIIO_CHECK_EQUAL( data.data.size(), 1 );
    OIIO_CHECK_EQUAL( data.data.count( "main" ), 1 );
    OIIO_CHECK_EQUAL( data.data.at( "main" ).size(), 2 );
    OIIO_CHECK_EQUAL( data.data.at( "main" )[0].first, "channel1" );
    OIIO_CHECK_EQUAL( data.data.at( "main" )[1].first, "channel2" );

    check_Spectrum( data["channel1"] );
    check_Spectrum( data["channel2"] );
}

void testSpectralData_Properties()
{
    rta::core::SpectralData data1;
    init_SpectralData( data1 );
    check_SpectralData( data1 );

    rta::core::SpectralData data2 = data1;
    check_SpectralData( data2 );

    rta::core::SpectralData data3( data1 );
    check_SpectralData( data3 );
}

void testSpectralData_LoadSpst()
{
    std::filesystem::path absolutePath =
        std::filesystem::absolute( DATA_PATH "camera/arri_d21_380_780_5.json" );

    rta::core::SpectralData camera;
    bool                    result;

    result = camera.load( absolutePath.string() );
    OIIO_CHECK_ASSERT( result );
    OIIO_CHECK_EQUAL( camera.manufacturer, "arri" );
    OIIO_CHECK_EQUAL( camera.model, "d21" );
    OIIO_CHECK_EQUAL( camera.data.size(), 1 );
    OIIO_CHECK_EQUAL( camera.data.count( "main" ), 1 );
    OIIO_CHECK_EQUAL( camera.data.at( "main" ).size(), 3 );
    OIIO_CHECK_EQUAL( camera.data.at( "main" )[0].first, "R" );
    OIIO_CHECK_EQUAL( camera.data.at( "main" )[1].first, "G" );
    OIIO_CHECK_EQUAL( camera.data.at( "main" )[2].first, "B" );

    double rgb[81][3] = { { 0.000188205, 8.59E-05, 9.58E-05 },
                          { 0.000440222, 0.000166118, 0.000258734 },
                          { 0.001561591, 0.00046321, 0.001181466 },
                          { 0.006218858, 0.001314864, 0.006881015 },
                          { 0.022246734, 0.003696276, 0.031937733 },
                          { 0.049120511, 0.00805609, 0.087988515 },
                          { 0.102812947, 0.017241631, 0.216210301 },
                          { 0.105467801, 0.021953991, 0.276918236 },
                          { 0.117352663, 0.028731455, 0.384008295 },
                          { 0.108489774, 0.036438901, 0.498308108 },
                          { 0.078494347, 0.037473311, 0.485933057 },
                          { 0.06542927, 0.047763009, 0.618489235 },
                          { 0.05126662, 0.057989658, 0.696558624 },
                          { 0.038300854, 0.063272391, 0.711794157 },
                          { 0.036088371, 0.078451972, 0.821540625 },
                          { 0.038076306, 0.099730024, 0.918286066 },
                          { 0.036894365, 0.112097767, 0.818615612 },
                          { 0.044395944, 0.156013174, 0.907103055 },
                          { 0.055918682, 0.217501304, 1 },
                          { 0.060307176, 0.238434493, 0.86480047 },
                          { 0.066779015, 0.269670797, 0.878082723 },
                          { 0.074505107, 0.300101812, 0.874303769 },
                          { 0.07562978, 0.290737255, 0.704674036 },
                          { 0.085791103, 0.328330642, 0.628143997 },
                          { 0.108943209, 0.424666004, 0.588816784 },
                          { 0.138099867, 0.523135173, 0.513082855 },
                          { 0.168736396, 0.591697868, 0.436252915 },
                          { 0.220667659, 0.742521719, 0.392230422 },
                          { 0.268662105, 0.832207187, 0.343540362 },
                          { 0.321560163, 0.912162297, 0.312675861 },
                          { 0.37671682, 0.976493082, 0.304109232 },
                          { 0.410777194, 0.973507973, 0.292240658 },
                          { 0.421878401, 1, 0.291164917 },
                          { 0.388993508, 0.931244461, 0.269598208 },
                          { 0.354154608, 0.889356652, 0.248312101 },
                          { 0.34283344, 0.762661473, 0.213286579 },
                          { 0.380725719, 0.693921344, 0.194295275 },
                          { 0.469885563, 0.5991218, 0.170597248 },
                          { 0.599407862, 0.530315531, 0.155055826 },
                          { 0.713821326, 0.418038191, 0.1317383 },
                          { 0.80813316, 0.340043294, 0.116047887 },
                          { 0.939975954, 0.27676007, 0.104954578 },
                          { 1, 0.217867885, 0.093258038 },
                          { 0.956064245, 0.155062572, 0.076556466 },
                          { 0.894704087, 0.11537981, 0.064641572 },
                          { 0.767742902, 0.089103008, 0.053623886 },
                          { 0.798777151, 0.083004112, 0.052099277 },
                          { 0.763111509, 0.075973825, 0.04909842 },
                          { 0.682557924, 0.067551041, 0.044677337 },
                          { 0.56116663, 0.056571832, 0.0382092 },
                          { 0.436680781, 0.045437665, 0.031713716 },
                          { 0.414781937, 0.042487508, 0.030781211 },
                          { 0.380963428, 0.03912278, 0.029786697 },
                          { 0.305406639, 0.032338965, 0.026385578 },
                          { 0.260012751, 0.028342775, 0.02448327 },
                          { 0.191033296, 0.022001542, 0.020646569 },
                          { 0.141171909, 0.017151907, 0.017480635 },
                          { 0.122396106, 0.01528005, 0.015881482 },
                          { 0.102299712, 0.013443924, 0.01414462 },
                          { 0.07855096, 0.011348793, 0.011965207 },
                          { 0.060474144, 0.009399874, 0.009474274 },
                          { 0.041685047, 0.007185144, 0.006997807 },
                          { 0.028123563, 0.005351653, 0.005182991 },
                          { 0.02203961, 0.004473424, 0.004168945 },
                          { 0.017482165, 0.003764279, 0.003387594 },
                          { 0.012357413, 0.002865598, 0.002507749 },
                          { 0.008721969, 0.001999441, 0.001714727 },
                          { 0.006462905, 0.001438107, 0.001233306 },
                          { 0.00454705, 0.001049424, 0.000918575 },
                          { 0.002933579, 0.000695583, 0.000587696 },
                          { 0.00211892, 0.000533403, 0.000436494 },
                          { 0.001499002, 0.000394215, 0.000315097 },
                          { 0.001022687, 0.000293059, 0.000238467 },
                          { 0.000681853, 0.000211926, 0.000168269 },
                          { 0.000561613, 0.000202539, 0.000170632 },
                          { 0.000384839, 0.000125687, 8.94E-05 },
                          { 0.000286597, 0.000104774, 6.92E-05 },
                          { 0.000269169, 0.000138887, 0.000126057 },
                          { 0.000163058, 6.47E-05, 4.57E-05 },
                          { 0.000149065, 7.26E-05, 5.84E-05 },
                          { 3.71E-05, 0.0, 2.70E-06 } };

    const std::string channels[3] = { "R", "G", "B" };

    for ( size_t i = 0; i < 3; i++ )
    {
        const rta::core::Spectrum &spectrum = camera[channels[i]];
        OIIO_CHECK_EQUAL( spectrum.shape.first, 380 );
        OIIO_CHECK_EQUAL( spectrum.shape.last, 780 );
        OIIO_CHECK_EQUAL( spectrum.shape.step, 5 );
        OIIO_CHECK_EQUAL( spectrum.values.size(), 81 );

        for ( size_t j = 0; j < 3; j++ )
        {
            OIIO_CHECK_EQUAL_THRESH( spectrum.values[j], rgb[j][i], 1e-5 );
        }
    }
}

int main( int, char ** )
{
    testSpectralData_Spectrum();
    testSpectralData_Properties();
    testSpectralData_LoadSpst();

    return unit_test_failures;
}
