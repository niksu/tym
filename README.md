# TYM Datalog
This is a simple implementation of Datalog.

# Building
`make`

# Running
`./out/tym`

e.g., `./out/tym -i tests/4.test`

The `-v` flag activates verbose output.

# Tests
* `make test_modules` tests internal APIs (for forming and destroying expressions, etc)
* `make test_regression` tests parser + printer + internal API
* `MEM_CHECK=1 make test_regression` checks for memory-safety during regression tests.
* for debug output build with `CFLAGS=-DDEBUG`

# License
LGPL v3 (see [LICENSE](LICENSE)).

# Author
Nik Sultana
