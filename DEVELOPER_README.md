# Naming conventions
## Prefix
Heap-allocated values referenced by variable names having these prefixes in a
function's declaration are treated as follows in the function:
* "cp" copied
* "cps" shallow-copied
* "cpi" top item won't be copied, but referred-to items will
* otherwise: not copied, directly or otherwise.

Such variables mainly used in construction ("mk") functions, such as
```
const struct fmla_t * mk_fmla_not(const struct fmla_t * cp_subfmla);
```
except when the construction is from an equivalent value, e.g.,
```
const struct fmla_t * mk_abstract_vars(const struct fmla_t * at, struct sym_gen_t * vg, struct valuation_t ** v);
```
