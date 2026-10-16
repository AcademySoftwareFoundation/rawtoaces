<!-- SPDX-License-Identifier: Apache-2.0 -->
<!-- Copyright Contributors to the rawtoaces project. -->

<p align="center">
  <img src="ASWF/logos/rawtoaces-horizontal-color.png" alt="rawtoaces">
</p>

[![CI](https://github.com/AcademySoftwareFoundation/rawtoaces/actions/workflows/ci.yml/badge.svg)](https://github.com/AcademySoftwareFoundation/rawtoaces/actions/workflows/ci.yml)
[![Code scanning – CodeQL](https://github.com/AcademySoftwareFoundation/rawtoaces/actions/workflows/github-code-scanning/codeql/badge.svg)](https://github.com/AcademySoftwareFoundation/rawtoaces/actions/workflows/github-code-scanning/codeql)
[![OpenSSF Best Practices](https://www.bestpractices.dev/projects/11185/badge)](https://www.bestpractices.dev/projects/11185)
[![codecov](https://codecov.io/gh/AcademySoftwareFoundation/rawtoaces/branch/main/graph/badge.svg)](https://codecov.io/gh/AcademySoftwareFoundation/rawtoaces)
[![Documentation Status](https://readthedocs.org/projects/rawtoaces/badge/?version=latest)](https://rawtoaces.readthedocs.io/en/latest/)

## Introduction

The RAW to ACES Utility, `rawtoaces`, converts digital camera RAW files to ACES AP0 image data in ACES container files. It provides a command line tool, C++ libraries, and Python bindings.

Conversion can use measured camera spectral sensitivities and illuminant data, matrices in the RAW file's metadata, or the Adobe camera coefficients supplied by LibRaw through OpenImageIO. The output follows the Academy Color Encoding Specification ([SMPTE ST 2065-1](https://pub.smpte.org/pub/st2065-1/st2065-1-2021.pdf)) and ACES Image Container specification ([SMPTE ST 2065-4](https://pub.smpte.org/pub/st2065-4/st2065-4-2023.pdf)).

## Quick start

Install with Homebrew on macOS or Linux:

```sh
brew install rawtoaces
```

For building from source and other installation options, see the [installation guide](src/docs/getting_started.rst).

Usage example:

```sh
rawtoaces --output-dir "$PWD/output" --create-dirs input.dng
```

The default CLI conversion uses metadata white balance and selects the spectral matrix method when camera data is available, otherwise the metadata matrix method.

## Documentation and source

- [Full documentation](https://rawtoaces.readthedocs.io/en/latest/)
- [Installation, data setup, and v1 CLI migration](src/docs/getting_started.rst)
- [Docker installation and conversion](src/docs/getting_started.rst#docker)
- [CLI usage guide](src/docs/user_guide.rst)
- [C++ API guide](src/docs/api/cpp/index.rst) and [Python API guide](src/docs/api/python/index.rst)
- [Spectral datasets and schema](https://github.com/AcademySoftwareFoundation/rawtoaces-data)
- [Public headers](include/rawtoaces), [source](src), and [tests](tests)
- [Changes](docs/CHANGES.md) and [known issues](https://github.com/AcademySoftwareFoundation/rawtoaces/issues)

## Community

rawtoaces is a project of the [Academy Software Foundation](https://www.aswf.io/).

- [Contributing](CONTRIBUTING.md)
- [AI policy](docs/AI_Policy.md)
- [Security policy](SECURITY.md)
- [Code of conduct](CODE_OF_CONDUCT.md)
- [License](LICENSE)
- [rawtoaces-discussion mailing list](https://lists.aswf.io/g/rawtoaces-discussion)
- `#rawtoaces` channel on [ASWF Slack](https://slack.aswf.io/)
