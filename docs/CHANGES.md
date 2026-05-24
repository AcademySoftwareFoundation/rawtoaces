Release 2.1.1 (June ?? 2026) -- compared to 2.1.0
--------------------------------------------------------


**This version is API-compatible and ABI-compatible with the previous version.**

### Changes:

- *fix*: fix colour tint when processing DNG images [#280](https://github.com/AcademySoftwareFoundation/rawtoaces/pull/280)
- *fix*: allow custom white-balancing weights in DNG mode [#272](https://github.com/AcademySoftwareFoundation/rawtoaces/pull/272)
- *fix*: fix default orientation and box white balance [#268](https://github.com/AcademySoftwareFoundation/rawtoaces/pull/268)
- *fix*: add homebrew location to default DB search path [#264](https://github.com/AcademySoftwareFoundation/rawtoaces/pull/264)

Release 2.1.0 (March 18 2026) -- compared to 2.0.0
--------------------------------------------------------


**This version is API-compatible but not ABI-compatible with the previous versions.**

### MAIN CHANGES:

* Lens correction. This version introduces lens correction functionality, utilising the lensfun library (adds compile time dependency on lensfun). The following types of lens phenomena are supported: chromatic aberration, distortion, vignetting.
* Adds functionality to fetch missing metadata using exiftool (adds runtime dependency on exiftool).
* Adds in-memory transform cache to speed up conversion of multiple images using the same camera settings.

### API changes:

* Adds new properties to the existing classes:
    * `rta::util::ImageConverter::Settings.lens_correction_types`
    * `rta::util::ImageConverter::Settings.require_lens_correction`
    * `rta::util::ImageConverter::Settings.custom_lens_make`
    * `rta::util::ImageConverter::Settings.custom_lens_model`
    * `rta::util::ImageConverter::Settings.custom_aperture`
    * `rta::util::ImageConverter::Settings.custom_focal_length`
    * `rta::util::ImageConverter::Settings.custom_focus_distance`
    * `rta::util::ImageConverter::Settings.disable_cache`
    * `rta::util::ImageConverter::Settings.disable_exiftool`
    * `rta::util::ImageConverter.status`
    * `rta::util::ImageConverter.last_error_message`
    * `rta::core::SpectralSolver.last_error_message`
* Adds new methods to the existing classes:
    * `rta::util::ImageConverter.get_supported_formats()`
    * `rta::util::ImageConverter.apply_lens_correction()`
* Adds an optional parameter `error_message` to the method `rta::core::SpectralData.load()`

### All changes:

- *feat*: lens correction [#255](https://github.com/AcademySoftwareFoundation/rawtoaces/pull/255)
- *feat*: implement transform cache [#236](https://github.com/AcademySoftwareFoundation/rawtoaces/pull/236)
- *feat*: fetch metadata using ExifTool [#242](https://github.com/AcademySoftwareFoundation/rawtoaces/pull/242)
- *improvement*: use OIIO RAW extensions for file validation [#231](https://github.com/AcademySoftwareFoundation/rawtoaces/pull/231)
- *refactor*: refactor error handling to separate library and CLI error messages [#237](https://github.com/AcademySoftwareFoundation/rawtoaces/pull/237)
- *refactor*: refactor logging, verbosity levels [#247](https://github.com/AcademySoftwareFoundation/rawtoaces/pull/247), [#249](https://github.com/AcademySoftwareFoundation/rawtoaces/pull/249), [#252](https://github.com/AcademySoftwareFoundation/rawtoaces/pull/252)
- *python*: more python bindings for ImageConverter [#223](https://github.com/AcademySoftwareFoundation/rawtoaces/pull/223)
- *python*: python bindings for core library [#248](https://github.com/AcademySoftwareFoundation/rawtoaces/pull/248)
- *doc*: implement developer documentation builds [#222](https://github.com/AcademySoftwareFoundation/rawtoaces/pull/222)
- *doc*: documentation updates [#224](https://github.com/AcademySoftwareFoundation/rawtoaces/pull/224)
- *tests*: spectral data tests [#220](https://github.com/AcademySoftwareFoundation/rawtoaces/pull/220)
- *tests*: improve unit test coverage [#226](https://github.com/AcademySoftwareFoundation/rawtoaces/pull/226), [#227](https://github.com/AcademySoftwareFoundation/rawtoaces/pull/227), [#228](https://github.com/AcademySoftwareFoundation/rawtoaces/pull/228), [#235](https://github.com/AcademySoftwareFoundation/rawtoaces/pull/235), [#241](https://github.com/AcademySoftwareFoundation/rawtoaces/pull/241)
- *analysis*: implement dynamic analysis [#233](https://github.com/AcademySoftwareFoundation/rawtoaces/pull/233)
- *analysis*: fix analysis warnings [#230](https://github.com/AcademySoftwareFoundation/rawtoaces/pull/230), [#239](https://github.com/AcademySoftwareFoundation/rawtoaces/pull/239), [#246](https://github.com/AcademySoftwareFoundation/rawtoaces/pull/246)
- *build*: add options to disable unittests and data installation [#244](https://github.com/AcademySoftwareFoundation/rawtoaces/pull/244)
- *build*: fix build issues [#234](https://github.com/AcademySoftwareFoundation/rawtoaces/pull/234)
- *ci*: improve caching [#225](https://github.com/AcademySoftwareFoundation/rawtoaces/pull/225), [#232](https://github.com/AcademySoftwareFoundation/rawtoaces/pull/232)
- *ci*: CI runners maintenance [#240](https://github.com/AcademySoftwareFoundation/rawtoaces/pull/240), [#243](https://github.com/AcademySoftwareFoundation/rawtoaces/pull/243), [#245](https://github.com/AcademySoftwareFoundation/rawtoaces/pull/245), [#253](https://github.com/AcademySoftwareFoundation/rawtoaces/pull/253), [#254](https://github.com/AcademySoftwareFoundation/rawtoaces/pull/254), [#256](https://github.com/AcademySoftwareFoundation/rawtoaces/pull/256)
- *admin*: add OpenSSF Best Practices badge [#238](https://github.com/AcademySoftwareFoundation/rawtoaces/pull/238)

Release 2.0.0 (December 8 2025) -- compared to 1.1.0
--------------------------------------------------------

**This version is not API- or ABI-compatible with the previous versions.**

### MAIN CHANGES:

#### The core library (rawtoaces-idt):

- The core library has been renamed from `rawtoaces_idt` to `rawtoaces_core`.
- The `Idt` class has been renamed to `rta::core::SpectralSolver`, the public interface of the class has been cleaned up. Refer to [core_usage.cpp](../tests/core_usage.cpp) for usage examples.
- The `DNGIdt` class has been renamed to `rta::core::MetadataSolver`, the public interface of the class has been cleaned up. Refer to [util_usage.cpp](../tests/util_usage.cpp) for usage examples.
- Reshaping of spectral data has been added, so camera curves with other than 380..780nm with 5nm step sampling can be used.
- The dependency on boost::json has been removed in favour of nlohmann-json.

#### The util library (rawtoaces-util):

- The `AcesRender` class has been renamed to `rta::util::ImageConverter`, the public interface of the class has been cleaned up.
- The dependency on Libraw has been removed in favour of OpenImageIO.
- The dependency on AcesContainer has been removed in favour of OpenImageIO.
- The proprietary command line parcer has been replaced with OpenImageIO.

#### The command line tool (rawtoaces):

- Because of the new command line parser, all command line parameters have been changed. Please refer to `rawtoaces --help`, or [README](../README.md) for more info.
- The switch to using OpenImageIO instead of Libraw directly required us to drop some optional command line parameters, since those options are not exposed in OpenImageIO yet. We had to drop the functionality, like providing black frames or dead pixel masks, etc. We will look at reintroducing those options on the OpenImageIO side in the future, if a need arises. Here is the list of dropped parameters:
  - `-P` - bad pixel mask
  - `-K` - dark frame
  - `-j` - fuji-rotate
  - `-m` - median filter
  - `-f` - four-colour RGB
  - `-T` - print Libraw-supported cameras
  - `-F` - use big file
  - `-s` - image index in the file
  - `-G` - green_matching() filter
- Functionality added: multiple crop modes supported via `--crop-mode`.
- Functionality added: specify output directories via `--output-dir`.
- Functionality added: automatically create missing output directories via `--create-dirs`.
- Functionality changed: `rawtoaces` does not overwrite existing files by default any more. Use `--overwrite` to override.

#### Other:
- Removed dependencies: boost, Libraw, AcesContainer.
- Added dependencies: OpenImageIO, nlohmann-json.
- The data files are now being installed into `/usr/local/share`, not `/usr/local/include`. The old path is still being resolved for backward compatibility.
- The database (external rawtoaces-data repo) dependency has been switched to v1.0.0, which changes the data schema version to v1.0.0 and adds multiple new camera measurements, see  

### All changes:

- *feat*: implement a data class for storing spectral curves, replace boost:json with nlohmann-json [#164](https://github.com/AcademySoftwareFoundation/rawtoaces/pull/164)
- *feat*: add `--data-dir` argument and `auto` matrix method [#189](https://github.com/AcademySoftwareFoundation/rawtoaces/pull/189)
- *feat*: Python bindings - WIP, not fully functional yet [#205](https://github.com/AcademySoftwareFoundation/rawtoaces/pull/205), [#208](https://github.com/AcademySoftwareFoundation/rawtoaces/pull/208)
- *api*: further tweaks to the public API [#178](https://github.com/AcademySoftwareFoundation/rawtoaces/pull/178), [#183](https://github.com/AcademySoftwareFoundation/rawtoaces/pull/183), [#215](https://github.com/AcademySoftwareFoundation/rawtoaces/pull/215), [#219](https://github.com/AcademySoftwareFoundation/rawtoaces/pull/219) 
- *api*: switch to rawtoaces-data v1.0.0 which also bumps the supported data schema version to v1.0.0 [#221](https://github.com/AcademySoftwareFoundation/rawtoaces/pull/221)
- *fix*: install to /usr/local/share, not /usr/local/include [#172](https://github.com/AcademySoftwareFoundation/rawtoaces/pull/172)
- *fix*: reading of data folders from env [#171](https://github.com/AcademySoftwareFoundation/rawtoaces/pull/171)
- *fix*: fix command line parsing error [186](https://github.com/AcademySoftwareFoundation/rawtoaces/pull/186)
- *refactor*: remove dependency on Libraw from the core library [#162](https://github.com/AcademySoftwareFoundation/rawtoaces/pull/162)
- *refactor*: rename rawtoaces_idt to rawtoaces_core [#160](https://github.com/AcademySoftwareFoundation/rawtoaces/pull/160)
- *refactor*: cleanup public interfaces [#159](https://github.com/AcademySoftwareFoundation/rawtoaces/pull/159), [#169](https://github.com/AcademySoftwareFoundation/rawtoaces/pull/169), [#173](https://github.com/AcademySoftwareFoundation/rawtoaces/pull/173)
- *refactor*: refactor the usage timer [#165](https://github.com/AcademySoftwareFoundation/rawtoaces/pull/165), [#166](https://github.com/AcademySoftwareFoundation/rawtoaces/pull/166)
- *deps*: replace libraw and aces_container with OIIO [#167](https://github.com/AcademySoftwareFoundation/rawtoaces/pull/167)
- *deps*: replace proprietary command line parser with OIIO [#168](https://github.com/AcademySoftwareFoundation/rawtoaces/pull/168)
- *deps*: replace boost::unittest with oiio::unittest [#170](https://github.com/AcademySoftwareFoundation/rawtoaces/pull/170)
- *deps*: replace boost::filesystem with std::filesystem [#161](https://github.com/AcademySoftwareFoundation/rawtoaces/pull/161)
- *ci*: fix CI broken on aswf-2024+ images [#163](https://github.com/AcademySoftwareFoundation/rawtoaces/pull/163)
- *ci*: various CI fixes [#179](https://github.com/AcademySoftwareFoundation/rawtoaces/pull/179), [#209](https://github.com/AcademySoftwareFoundation/rawtoaces/pull/209), [#210](https://github.com/AcademySoftwareFoundation/rawtoaces/pull/210), [#213](https://github.com/AcademySoftwareFoundation/rawtoaces/pull/213), [#214](https://github.com/AcademySoftwareFoundation/rawtoaces/pull/214), [#217](https://github.com/AcademySoftwareFoundation/rawtoaces/pull/217) 
- *ci*: improve Windows CI performance by caching [#218](https://github.com/AcademySoftwareFoundation/rawtoaces/pull/218)
- *tests*: improve unittest coverage [#190](https://github.com/AcademySoftwareFoundation/rawtoaces/pull/190), [#192](https://github.com/AcademySoftwareFoundation/rawtoaces/pull/192), [#193](https://github.com/AcademySoftwareFoundation/rawtoaces/pull/193), [#194](https://github.com/AcademySoftwareFoundation/rawtoaces/pull/194), [#200](https://github.com/AcademySoftwareFoundation/rawtoaces/pull/200), [#207](https://github.com/AcademySoftwareFoundation/rawtoaces/pull/207), [#212](https://github.com/AcademySoftwareFoundation/rawtoaces/pull/212), [#216](https://github.com/AcademySoftwareFoundation/rawtoaces/pull/216)
- *tests*: test coverage report with badge and logo by [#188](https://github.com/AcademySoftwareFoundation/rawtoaces/pull/188)
- *analysis*: add static analysis [#206](https://github.com/AcademySoftwareFoundation/rawtoaces/pull/206), [#211](https://github.com/AcademySoftwareFoundation/rawtoaces/pull/211)
- *build*: fix build issues caused by the order of OIIO includes [#198](https://github.com/AcademySoftwareFoundation/rawtoaces/pull/198)
- *cleanup*: code cleanup [#174](https://github.com/AcademySoftwareFoundation/rawtoaces/pull/174), [#182](https://github.com/AcademySoftwareFoundation/rawtoaces/pull/182), [#185](https://github.com/AcademySoftwareFoundation/rawtoaces/pull/185), [#195](https://github.com/AcademySoftwareFoundation/rawtoaces/pull/195), [#197](https://github.com/AcademySoftwareFoundation/rawtoaces/pull/197)
- *admin* Add CONTRIBUTING and other documents [#199](https://github.com/AcademySoftwareFoundation/rawtoaces/pull/199)

Release 1.1.0 (August 11 2025) -- compared to 1.0.0
--------------------------------------------------------
- *feat*: Implement custom matrix mode [#109](https://github.com/AcademySoftwareFoundation/rawtoaces/pull/109)
- *fix*: Incorrect XYZ to ACES matrix [#108](https://github.com/AcademySoftwareFoundation/rawtoaces/pull/108)
- *fix*: Build issues with libraw 0.20.0 [#120](https://github.com/AcademySoftwareFoundation/rawtoaces/pull/120), [#127](https://github.com/AcademySoftwareFoundation/rawtoaces/pull/127) 
- *fix*: Remove hardcoded path [#133](https://github.com/AcademySoftwareFoundation/rawtoaces/pull/133)
- *fix*: Fix math error in matrix multiplication [#135](https://github.com/AcademySoftwareFoundation/rawtoaces/pull/135)
- *admin*: The codebase has been re-licenced to the Apache licence v2.0 [#147](https://github.com/AcademySoftwareFoundation/rawtoaces/pull/147)
- *admin*: The data files have been split into a separate repository [#149](https://github.com/AcademySoftwareFoundation/rawtoaces/pull/149), [#113](https://github.com/AcademySoftwareFoundation/rawtoaces/pull/113)
- *admin*: Switch from AMPAS_DATA_PATH to RAWTOACES_DATA_PATH environment variable [#156](https://github.com/AcademySoftwareFoundation/rawtoaces/pull/156)
- *docs*: various changes to README.md [#107](https://github.com/AcademySoftwareFoundation/rawtoaces/pull/107), [#110](https://github.com/AcademySoftwareFoundation/rawtoaces/pull/110), [#119](https://github.com/AcademySoftwareFoundation/rawtoaces/pull/119), [#145](https://github.com/AcademySoftwareFoundation/rawtoaces/pull/145), [#148](https://github.com/AcademySoftwareFoundation/rawtoaces/pull/148), [#152](https://github.com/AcademySoftwareFoundation/rawtoaces/pull/152)
- *ci*: various changes and fixes to the CI runners [#129](https://github.com/AcademySoftwareFoundation/rawtoaces/pull/129), [#130](https://github.com/AcademySoftwareFoundation/rawtoaces/pull/130), [#132](https://github.com/AcademySoftwareFoundation/rawtoaces/pull/132), [#140](https://github.com/AcademySoftwareFoundation/rawtoaces/pull/140), [#146](https://github.com/AcademySoftwareFoundation/rawtoaces/pull/146)
- *build*: Add docker container [#118](https://github.com/AcademySoftwareFoundation/rawtoaces/pull/118), [#131](https://github.com/AcademySoftwareFoundation/rawtoaces/pull/131), [#142](https://github.com/AcademySoftwareFoundation/rawtoaces/pull/142)
- *build*: various changes and fixes to the cmake scripts [#96](https://github.com/AcademySoftwareFoundation/rawtoaces/pull/96), [#128](https://github.com/AcademySoftwareFoundation/rawtoaces/pull/128), [#151](https://github.com/AcademySoftwareFoundation/rawtoaces/pull/151), [#153](https://github.com/AcademySoftwareFoundation/rawtoaces/pull/153), [#154](https://github.com/AcademySoftwareFoundation/rawtoaces/pull/154), [#158](https://github.com/AcademySoftwareFoundation/rawtoaces/pull/158)
- *build*: clang-format [#126](https://github.com/AcademySoftwareFoundation/rawtoaces/pull/126)
- *build*: headers cleanup [#134](https://github.com/AcademySoftwareFoundation/rawtoaces/pull/134)

Release 1.0.0 (November 7 2017)
--------------------------------------------------------
- Initial release of rawtoaces
