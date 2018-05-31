# Tests
* `make test_modules` tests internal APIs (for forming and destroying expressions, etc)
* `make test_regression` tests parser + printer + internal API. (Remember to set the `TYM_Z3_PATH` variable if the binary was previously compiled to use Z3.)
* `MEM_CHECK=1 make test_regression` checks for memory-safety during regression tests.
* for debug output, build with `CFLAGS=-DTYM_DEBUG`.
* `CC=gcc-7 make` to compile with that specific compiler.

# Memory Tests
* `valgrind --leak-check=full out/tym 2>&1`
* For regressiong tests: `valgrind --leak-check=full out/tym -i tests/1.test --test_parsing 2>&1`
* For solving: `./out/tym -i tests/4.test` and Valgrind of it.
* For solving with queries: `./out/tym -i tests/4.test -q "e(X)."` and Valgrind of it.
