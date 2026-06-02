# C++ to C Conversion Reference Table

Document version: `0.1`

Applies to: Ruckig Community `0.17.3` under `original/ruckig-main`

Priority: implementation reference for all phases. It does not define scope. If
this document conflicts with `docs/c_rewrite_execution_plan.md`, the execution
plan takes precedence.

## Project Defaults

Use these defaults for the Ruckig C rewrite:

1. Baseline language: C99.
2. C11 features are optional and must be guarded or avoided in public headers.
3. Public names use `ruckig_` for functions/types and `RUCKIG_` for constants.
4. For the first release, allocate during create/destroy paths only. Explicit
   resize functions may be added later, but they are not required by the
   first-release public API.
5. Do not allocate in `ruckig_calculate`, `ruckig_update`, root solvers, profile
   checks, or trajectory sampling.
6. Prefer hand-written loops for small hot arrays over generic library
   dispatch.
7. Do not enable unsafe fast math by default.
8. Keep debug formatting out of the real-time core path.
9. Preserve original numerical constants unless oracle tests justify a
   documented change.

## Containers and Optional Values

| C++ pattern | C rewrite pattern | Project note |
| --- | --- | --- |
| `std::array<double, 7>` | `double t[7]` | Use fixed arrays for profile internals. |
| `std::array<double, 8>` | `double p[8]` | Use fixed arrays for profile state. |
| `std::vector<double>` | `double* data` plus `size_t count` | Allocate during create; reuse during calculation. |
| `std::vector<Vector>` | flattened array plus row count/stride | Prefer contiguous storage for DoF/section data. |
| `std::optional<double>` | `bool has_value; double value;` | Keep flags explicit. |
| `std::optional<Vector>` | `bool has_vector; double* vector;` | Allocate optional vector storage during create when required. |
| `std::optional<Interval>` | `bool has_interval; interval value;` | Use for `Block::a` and `Block::b`. |
| `std::tuple<A, B, C>` | output pointer parameters | Example: `double* p, double* v, double* a`. |
| `std::string` / `std::stringstream` | debug-only formatting helpers | Not required in production core. |

## Algorithms and STL Helpers

| C++ helper | C rewrite pattern | Project note |
| --- | --- | --- |
| `std::upper_bound` | private binary-search helper | Use for cumulative time/profile segment lookup. |
| `std::sort` | insertion/selection sort for small hot arrays | Avoid `qsort` in hot paths; use `qsort` only in tests or non-hot utilities. |
| `std::min_element` | explicit loop | Keeps comparisons inline and predictable. |
| `std::all_of` | explicit loop with early return | Avoid callback overhead. |
| `std::iota` | explicit `for` loop | Simple and clear. |
| `std::swap` | temp variable helper or macro | Prefer typed `static inline` helpers where useful. |
| lambda with captures | direct private helper or callback + userdata | Do not expose callback in public API. |
| `std::function` | avoid in core | If needed internally, use function pointer + userdata. |

## Syntax and Object Model

| C++ feature | C rewrite pattern | Project note |
| --- | --- | --- |
| `namespace ruckig` | `ruckig_` / `RUCKIG_` prefixes | Required for public symbols. |
| `class` | opaque `struct` plus lifecycle functions | Public structs are opaque in the first release. |
| constructor | `ruckig_*_create` | Must allocate the handle and initialize defaults matching C++. |
| destructor | `ruckig_*_destroy` | Must release owned memory; `destroy(NULL)` is a no-op. |
| `const T&` | `const T*` | Use pointer validity checks where appropriate. |
| `T&` | `T*` | Used for output/mutable parameters. |
| `enum class` | `typedef enum` | Preserve numeric values for public result codes. |
| `operator==` / `operator!=` | explicit compare function | Needed for input cache comparison. |
| `if constexpr` | enum/switch or explicit family functions | Do not merge behavior until oracle tests pass. |
| template non-type parameter | runtime value or internal specialization | First release uses dynamic DoF. |

## Math and Numeric Functions

| C++ | C | Project note |
| --- | --- | --- |
| `std::numeric_limits<double>::epsilon()` | `DBL_EPSILON` from `<float.h>` | Use for machine epsilon. |
| `std::numeric_limits<double>::infinity()` | `INFINITY` from `<math.h>` | Preserve infinity semantics for order reduction. |
| `std::isinf(x)` | `isinf(x)` from `<math.h>` | Include `<math.h>`. |
| `std::isnan(x)` | `isnan(x)` from `<math.h>` | Include `<math.h>`. |
| `std::abs(double)` | `fabs(x)` | Avoid integer `abs`. |
| `std::sqrt(x)` | `sqrt(x)` | Link libm where required. |
| `std::cbrt(x)` | `cbrt(x)` | C99. |
| `std::fmod(x, y)` | `fmod(x, y)` | Used for discrete duration rounding. |
| `pow2(x)` helper | `x * x` | Prefer explicit multiplication. |

Do not use `-ffast-math` or equivalent unsafe math flags in release validation
or oracle tests.

## Header Replacements

| C++ header | C header/replacement | Project note |
| --- | --- | --- |
| `<algorithm>` | private helpers | Search, sort, min/max loops. |
| `<array>` | fixed C arrays | Profile and brake internals. |
| `<vector>` | allocated buffers | Allocate during create, not calculate/update. |
| `<optional>` | explicit flags | `bool has_*`. |
| `<tuple>` | output parameters | Pointer outputs. |
| `<string>` / `<sstream>` | `<stdio.h>` debug-only helpers | Not in hot path. |
| `<iostream>` | debug-only `stdio` | Avoid production logging in core. |
| `<cmath>` | `<math.h>` | C99 math. |
| `<cfloat>` / `<limits>` | `<float.h>` and `<math.h>` | `DBL_EPSILON`, `INFINITY`. |
| `<chrono>` | optional timing wrapper | Compile-time optional; not core algorithm. |
| `<cstring>` | `<string.h>` | `memcpy`, `memset`, comparisons. |
| `<memory>` | lifecycle functions | No smart pointers in C. |

## Memory Rules

Allowed allocation sites:

1. `ruckig_create`
2. `ruckig_destroy`
3. `ruckig_input_create`
4. `ruckig_input_destroy`
5. `ruckig_output_create`
6. `ruckig_output_destroy`
7. `ruckig_trajectory_create`
8. `ruckig_trajectory_destroy`

Optional explicit resize functions may be added later. If added, they are also
allowed allocation sites, but they must remain outside `calculate`, `update`,
profile solver, root solver, and trajectory sampling paths.

Forbidden allocation sites:

1. `ruckig_calculate`
2. `ruckig_update`
3. root solvers
4. profile checks
5. profile step solvers
6. trajectory sampling
7. input validation

## Template Conversion Patterns

| C++ template pattern | C rewrite pattern | Rule |
| --- | --- | --- |
| `template<Tag tag> f()` | `f(tag)` with enum dispatch | Good for simple behavior switches. |
| `template<signs, limits> check()` | distinct family function plus enum dispatch | Do not collapse families prematurely. |
| `template<size_t DOFs>` | runtime `dofs` | First release is dynamic DoF only. |
| `StandardVector<T, DOFs>` | `T*` plus `dofs` | Backed by preallocated storage. |
| `enable_if<D == 1>` overload | optional fast path | Future optimization only. |

## C11 Guard Pattern

C11 features are optional and must not be required by public headers. If a C11
feature is useful internally, guard it explicitly:

```c
#if defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 201112L)
/* C11-only internal helper, such as _Static_assert or alignas-style support. */
#endif
```

Public API behavior must remain available in C99 mode.

## Debug and Formatting

Debug helpers may be useful while porting, but they are not required public API.

Rules:

1. Do not put formatting in calculation hot paths.
2. Guard debug string helpers behind a build flag.
3. Do not use debug output for test assertions; assert numeric values directly.
4. Prefer oracle-diff reports in tests over production `to_string` functions.
5. Do not include or call `stdio` formatting helpers from production real-time
   core files unless the code is fully compiled out by a debug-only build flag.
