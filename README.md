# TYM Datalog
Version 1.0

# Building
`make`

# Running
`./out/tym`

e.g., `./out/tym -i tests/4.test`

The `-v` flag activates verbose output.

## Backend solver
To build with backend solver support (to use [Z3](https://github.com/Z3Prover/z3))
define `TYM_Z3_PATH` to point to Z3's installation directory when calling `make`:
`TYM_Z3_PATH=${Z3_PATH} make`.
When running the resulting binary, remember to indicate where to find Z3's dynamically-liked library (libz3.dylib on macOS -- or the .so analogue on Linux).
For example, `DYLD_LIBRARY_PATH=z3-4.5.0-x64-osx-10.11.6/bin/ ./out/tym -f smt_solve -m fact -i tests/4.test -q "e(X)."`

## Stand-alone binaries from Datalog programs
Use `-f c_output` to translate a Datalog program to C, then use `tymc.sh`
to compile and link it with Tym, to produce a standalone executable from your
original program. Use the `CFLAGS` environment variable to instruct tymc on
`-L` and `-I` parameters to find Tym's binaries and headers.

# License
LGPL v3 or any later version (see [LICENSE](LICENSE)).

# Author
Nik Sultana
