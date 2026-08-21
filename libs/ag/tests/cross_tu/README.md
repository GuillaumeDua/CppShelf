# Cross-TU tests

Since ODR violations are invisible within a single translation unit,  
then each test/target here links one binary from two TUs that deliberately disagree,  
so a broken contract should surfaces as a link error or as non-deterministic output rather than passing unnoticed.

- `typeinfo_bridge_{enabled,disabled}.cpp` - only one TU includes `<csl/ag/formatting/typeinfo.hpp>`.  
  Each formats different types, so no specialization is instantiated from two definitions:  
  the binary must link and behave deterministically (like `fmt`'s `<fmt/ranges.h>` opt-in contract).  
- `ostream_iword{,_remote}.cpp` - `details::mode_index()` must resolve to the same `xalloc` slot in every TU,  
  so manipulator state applied in one TU is seen by prints in another.
