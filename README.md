# TYM Datalog

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

# Tests
* `make test_modules` tests internal APIs (for forming and destroying expressions, etc)
* `make test_regression` tests parser + printer + internal API. (Remember to set the `TYM_Z3_PATH` variable if the binary was previously compiled to use Z3.)
* `MEM_CHECK=1 make test_regression` checks for memory-safety during regression tests.
* for debug output, build with `CFLAGS=-DTYM_DEBUG`.
* `CC=gcc-7 make` to compile with that specific compiler.

# License
LGPL v3 (see [LICENSE](LICENSE)).

# Author
Nik Sultana
