# LibPSFC.jl

**DEPRECATED:**  Use <https://github.com/ma-laforge/LibPSF.jl>.

## Description

The LibPSFC.jl module provides a Julia interface for Henrik Johansson's .psf reader.

## Sample Usage

Examples on how to use the LibPSFC.jl capabilities can be found under the [test directory](test/).

<a name="Installation"></a>
## Installation

		julia> Pkg.clone("https://github.com/ma-laforge/LibPSFC.jl.git")

### Compiling C/C++ Core (Ubuntu Systems)

The core C++ library (libpsf) requires the "boost" library.  Boost is installed with the following command:

		$ sudo apt-get install libboost-all-dev

The libpsf + C-wrapper code is then compiled as follows:

		$ cd [JULIA_LIB_PATH]/LibPSFC/core
		$ make all

NOTE:

 - `[JULIA_LIB_PATH]` is typically found under: `~/.julia/[VERSION]`.
 - The libpsf library currently generates many warnings with `-Wall`.

### Compiling C/C++ Core (Other Systems)

Unclear at the moment.

## Resources/Acknowledgments

### libpsf

The core of LibPSFC.jl makes use of Henrik Johansson's libpsf library:

 - **libpsf** (LGPL v3): <https://github.com/henjo/libpsf>.

The relevant portions of the libpsf library are provided in the [core subdirectory](core/).

## Known Limitations

### Missing Features

LibPSFC.jl does not currently support all the functionnality of the original libpsf library.  A few features known to be missing are listed below:

 - Does not support reading "structures" (ex: DC operating point results).
 - Does not support `StructVector`, nor `VectorStruct` (`m_invertstruct`).

### Compatibility

Extensive compatibility testing of LibPSFC.jl has not been performed.  The module has been tested using the following environment(s):

 - Linux / Julia-0.7.0 (64-bit) / Ubuntu

#### Repository versions:

This libpsf code provided in this module might not be the most recent:

 - **libpsf**: Sat Nov 29 10:53:38 2014 +0100

## Disclaimer

This software is provided "as is", with no guarantee of correctness.  Use at own risk.
