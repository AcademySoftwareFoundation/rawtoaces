# Lens correction audit

Audited baseline: upstream `main` at `9883c55` (14 October 2026). The locations
below refer to that baseline, so this list remains useful without adopting the
refactor. The PR #354 finding is listed separately because its `process_buffer`
API has not landed on this baseline.

## Changes in this patch

- Resolve metadata and settings overrides once into a lens descriptor shared by
  all stages, replacing repeated positional parameter lists and buffer handling.
- Keep the stage order: vignetting, chromatic aberration, geometric distortion.
  Stage output remains private until every requested correction succeeds.
- Share conversion policy through `process_buffer`, exposed in C++ and Python.
  Optional failures warn and process the original source; mandatory failures
  stop. `apply_lens_correction` remains a strict operation that returns failure
  and preserves its inputs, regardless of the optional-conversion setting.
- Own Lensfun databases, lookup arrays, and modifiers automatically. Serialize
  database loading/lookup while allowing map calculation to run concurrently.
- Key cached maps by complete image windows and channel layout, retry failed
  calculations, and validate numeric parameters and supported image geometry.
- Use one-channel vignette gains on all supported OIIO versions, preserve alpha
  and depth during exposure correction, and preserve auxiliary channels during
  chromatic correction. Distortion continues to remap all channels together.
- Preserve source metadata and buffer names and update the usage/API docs.

## Bugs in the audited main branch

### 1. A later correction failure leaves earlier corrections applied

**Trigger:** Request vignetting and chromatic aberration for a lens with a
vignetting model but no TCA model. Vignetting changes the buffer, then TCA fails.
Optional file conversion warns and continues using the partially corrected
pixels. Mandatory conversion fails after already modifying an in-place input.

**Locations:** [stage handling](https://github.com/AcademySoftwareFoundation/rawtoaces/blob/9883c55/src/rawtoaces_util/image_converter.cpp#L1820),
[optional conversion policy](https://github.com/AcademySoftwareFoundation/rawtoaces/blob/9883c55/src/rawtoaces_util/image_converter.cpp#L2849).

**Minimal fix:** Run the correction chain in temporary storage and assign its
result only after all stages succeed. On optional failure use the original
source. Regression coverage includes a vignette-only profile and both buffer
aliasing modes.

### 2. Cached maps can belong to a different image window

**Trigger:** Process two images with equal data widths/heights and lens metadata
but different data origins, full-window origins, or full-window dimensions.
The cache considers them equal even though the map calculation uses those
coordinates and full sizes. Map channel counts are also omitted.

**Location:** [cache descriptor equality](https://github.com/AcademySoftwareFoundation/rawtoaces/blob/9883c55/src/rawtoaces_util/lens_correction_cache.cpp#L19).

**Minimal fix:** Compare the full data and display ROIs, including channels,
as well as the map format/layout. Tests compare cached and uncached results
while varying each window field.

### 3. Failed map calculations are cached without their diagnostics

**Trigger:** Repeat a request whose first map calculation fails. The cache
returns its stored null pointer without rerunning the calculation, so the next
call loses the underlying error. A transient dependency failure cannot recover
for that descriptor while the failed entry remains cached.

**Location:** [unconditional cache insertion](https://github.com/AcademySoftwareFoundation/rawtoaces/blob/9883c55/src/rawtoaces_util/cache_base.h#L142).

**Minimal fix:** Cache only non-null results, still clear pending work and notify
waiters after a failure. Tests verify failure followed by success for the same
key and repeated lens-map failures with useful diagnostics.

### 4. Lensfun database initialization and reset are unsynchronized

**Trigger:** Concurrent converters calculate different map types/descriptors,
or disable map caching. They can access the global database while another
thread creates, loads, or resets it. The per-map cache locks do not protect this
shared global object. Reset can invalidate objects used by a live modifier.

**Location:** [global database and initialization](https://github.com/AcademySoftwareFoundation/rawtoaces/blob/9883c55/src/rawtoaces_util/lens_correction.cpp#L17).

**Minimal fix:** Lock initialization, lookup, and reset; keep shared ownership
of the database in each modifier until map generation finishes. Concurrent
cache-disabled lookup is covered by a regression test; the unsafe original
lifetime is established by code inspection.

### 5. Camera and lens lookup result arrays leak

**Trigger:** Any successful camera lookup allocates a result array, and any
successful lens lookup allocates another. Neither array is freed, including
camera-success/lens-failure paths.

**Locations:** [camera lookup](https://github.com/AcademySoftwareFoundation/rawtoaces/blob/9883c55/src/rawtoaces_util/lens_correction.cpp#L87),
[lens lookup](https://github.com/AcademySoftwareFoundation/rawtoaces/blob/9883c55/src/rawtoaces_util/lens_correction.cpp#L100).

**Minimal fix:** Wrap each result array with an `lf_free` deleter. Free the
arrays only; the database owns the camera and lens objects. Ownership is
specified in the Lensfun headers.

### 6. Failed database loading leaves a falsely initialized global database

**Trigger:** No default/custom Lensfun database can be loaded. The global
pointer is assigned before loading succeeds. A second attempt skips loading
and reports a missing camera instead of the original database error. Adding a
valid database path cannot recover without resetting the global explicitly.

**Location:** [database loading](https://github.com/AcademySoftwareFoundation/rawtoaces/blob/9883c55/src/rawtoaces_util/lens_correction.cpp#L44).

**Minimal fix:** Load a local candidate and publish it only on success. This
failure-state defect is established by code inspection.

### 7. Pre-populated destinations change geometry and channel counts

**Trigger:** Pass a destination with different dimensions or channels to
geometric/chromatic correction. The helpers allocate their output from the
destination spec rather than the source spec, causing failed warps, cropped
results, or missing channels depending on the mismatch.

**Locations:** [distortion output spec](https://github.com/AcademySoftwareFoundation/rawtoaces/blob/9883c55/src/rawtoaces_util/lens_correction.cpp#L456),
[aberration output spec](https://github.com/AcademySoftwareFoundation/rawtoaces/blob/9883c55/src/rawtoaces_util/lens_correction.cpp#L599).

**Minimal fix:** Derive correction output from the source spec. Tests compare
empty, pre-populated, and in-place destinations.

### 8. Chromatic correction cycles RGB transforms across auxiliary channels

**Trigger:** Correct an RGBA or multi-channel image. The loop uses `(i * 2) % 6`
for every channel, so alpha receives the red-channel displacement and later
auxiliary channels receive other colour displacements.

**Location:** [per-channel aberration loop](https://github.com/AcademySoftwareFoundation/rawtoaces/blob/9883c55/src/rawtoaces_util/lens_correction.cpp#L608).

**Minimal fix:** Correct the three RGB channels and copy the others unchanged.
Reject inputs without the required RGB layout. A spatial alpha gradient is
checked for exact preservation.

### 9. Vignetting scales alpha/depth and assumes RGB on older OIIO

**Trigger:** Exposure correction multiplies all image channels, including alpha
and depth. Before OIIO 3.1, the gain image is hard-coded to three channels even
for grayscale or RGBA sources, making output layout depend on `mul`'s channel
matching behavior.

**Location:** [vignette application](https://github.com/AcademySoftwareFoundation/rawtoaces/blob/9883c55/src/rawtoaces_util/lens_correction.cpp#L294).

**Minimal fix:** Apply one scalar gain per pixel to colour channels only,
retaining the source layout and excluding alpha/depth. Tests cover RGB,
grayscale, and explicit alpha/depth gradients. The old-version branch is
identified by inspection; the new path has no OIIO-version branch.

### 10. Invalid numeric parameters and image layouts reach Lensfun

**Trigger:** Negative, NaN, or infinite focal/aperture/focus values pass the
zero-only metadata check. Nonpositive full sizes reach modifier creation and
normalization divisors. Deep/volume sources are converted to 2D map specs,
while the row solvers implement only flat 2D images.

**Locations:** [parameter lookup](https://github.com/AcademySoftwareFoundation/rawtoaces/blob/9883c55/src/rawtoaces_util/image_converter.cpp#L1690),
[modifier dimensions](https://github.com/AcademySoftwareFoundation/rawtoaces/blob/9883c55/src/rawtoaces_util/lens_correction.cpp#L127),
[coordinate normalization](https://github.com/AcademySoftwareFoundation/rawtoaces/blob/9883c55/src/rawtoaces_util/lens_correction.cpp#L365).

**Minimal fix:** Validate required numeric values as finite and positive;
accept zero only as the existing optional-focus sentinel. Validate a nonempty,
flat 2D source and positive full dimensions before generating maps. Tests
exercise invalid focal values, deep/volume inputs, and zero full width.

### 11. Copy failures and successful-call status are reported incorrectly

**Trigger:** Correction is disabled and copying an invalid/unreadable source
fails. Its return value is ignored and the method returns true. Successful
correction also never resets `status`, leaving a previous failure status active.

**Location:** [apply_lens_correction](https://github.com/AcademySoftwareFoundation/rawtoaces/blob/9883c55/src/rawtoaces_util/image_converter.cpp#L1714).

**Minimal fix:** Check copying and source initialization, stage the copy, and
set `status` from the actual outcome. Preserve the documented previous error
message on success. Tests cover invalid sources and failure followed by success.

### 12. Missing optional metadata writes an error despite success

**Trigger:** A file has no lens manufacturer or focus distance. The optional
lookup writes a "Missing..." message, its false result is ignored, and
correction may subsequently succeed while recording a new error message.

**Locations:** [optional lens manufacturer](https://github.com/AcademySoftwareFoundation/rawtoaces/blob/9883c55/src/rawtoaces_util/image_converter.cpp#L1760),
[optional focus](https://github.com/AcademySoftwareFoundation/rawtoaces/blob/9883c55/src/rawtoaces_util/image_converter.cpp#L1813).

**Minimal fix:** Resolve optional values without producing missing-field errors.
Tests deliberately omit both fields and check successful correction with no
new diagnostic.

### 13. Geometric correction loses the input buffer name

**Trigger:** Distortion creates an unnamed temporary buffer and assigns it over
the destination. Subsequent usage timers/logging no longer have the source
filename, especially for in-place file conversion.

**Location:** [distortion result construction](https://github.com/AcademySoftwareFoundation/rawtoaces/blob/9883c55/src/rawtoaces_util/lens_correction.cpp#L461).

**Minimal fix:** Preserve the input name when committing the correction result.
Tests check the resulting name for every correction type.

### 14. Python lens-correction examples contain invalid multiline assignments

**Trigger:** Copy either Python correction-flags example. The assignment breaks
across lines without enclosing parentheses, so Python raises a syntax error.

**Location:** [Python usage example](https://github.com/AcademySoftwareFoundation/rawtoaces/blob/9883c55/src/docs/lens_correction.rst#L68).

**Minimal fix:** Enclose each multiline flag expression in parentheses. Both
Python blocks are parsed during local validation.

## Separate finding from PR #354

At the reviewed head `ad1999a`, `process_buffer(dst, src)` continues after an
optional lens-correction failure without populating `dst`. Missing metadata
can trigger the failure before the destination is written, so separate-buffer
processing uses stale pixels or an empty destination.

**Location:** [PR #354 optional failure path](https://github.com/AcademySoftwareFoundation/rawtoaces/blob/ad1999ad7c9c0a14b22d3a2bacb112e3534ae15a/src/rawtoaces_util/image_converter.cpp#L2877).

**Minimal fix:** Copy the original source into the destination before continuing
on optional failure; also make correction transactional to prevent partial
results in the aliasing case. This patch adds the buffer API on current main
with that policy and regression tests for empty/stale destinations in C++ and
Python. It overlaps the API introduced by #354 and should be reconciled if that
PR lands first.

## Validation and observations

- macOS arm64, OIIO 3.2.1.1, Lensfun 0.3.4, Python 3.14.
- Full C++/Python CTest suite with Lensfun: 17/17 passed.
- Full C++/Python CTest suite without Lensfun: 16/16 passed.
- AddressSanitizer lens/caching regression targets: 2/2 passed.
- Doxygen/Sphinx documentation build passed; both Python examples parsed.
- Changed C++ files pass clang-format 16.0.6, matching CI.
- The existing combination tests cover all eight correction flag combinations.
- A converter test assumed no installed spectral database. It now explicitly
  supplies a missing directory, avoiding false failures on developer machines.
- The overloaded `operator&&` already performs a bitwise flag check; it was not
  a bug. The refactored implementation uses explicit flag comparisons.
- Windows/Linux, older OIIO, and newer Lensfun were not executed locally.
