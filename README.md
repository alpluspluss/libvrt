<div align="center">
    <img src="assets/vrt-logo.png" alt="Logo" width="35%">
    <h1>vrt</h1>
    <p>Make variants switchable again</p>
</div>

## Why `vrt`?

`std::variant` is a powerful type introduced in C++17, however, it can be cumbersome to use as
it requires verbose `std::visit` calls and can lead to boilerplate code. `vrt` solves this with
natural switch case support while maintaining 100% API compatibility with `std::variant`.

### Comparison

#### `std::variant`

```cpp
std::variant<int, double, std::string> v = 42;

std::visit([](auto&& arg) 
{
    using T = std::decay_t<decltype(arg)>;
    if constexpr (std::is_same_v<T, int>) 
    {
        std::cout << "int: " << arg << '\n';
    } 
    else if constexpr (std::is_same_v<T, double>) 
    {
        std::cout << "double: " << arg << '\n';
    } 
    else if constexpr (std::is_same_v<T, std::string>) 
    {
        std::cout << "string: " << arg << '\n';
    }
}, v);
```

#### `vrt::variant`

```cpp
using Value = vrt::variant<int, double, std::string>;

Value v = 42;
switch (v.index()) 
{
    case Value::of<int>:
        std::cout << "int: " << vrt::get<int>(v) << '\n';
        break;
    case Value::of<double>:
        std::cout << "double: " << vrt::get<double>(v) << '\n';
        break;
    case Value::of<std::string>:
        std::cout << "string: " << vrt::get<std::string>(v) << '\n';
        break;
}
```

### Performance

**`vrt::variant` delivers up to 4.6x faster performance than `std::visit`**, with switch
cases reaching over 860 million operations per second. Both `vrt::visit` and switch 
cases significantly outperform the standard library.

| Operation         | std::visit | vrt::visit     | vrt::switch    |
|-------------------|------------|----------------|----------------|
| Small variants    | 142M ops/s | **651M ops/s** | **704M ops/s** |
| Mixed variants    | 300M ops/s | **862M ops/s** | **724M ops/s** |
| Many variants     | 262M ops/s | **430M ops/s** | **417M ops/s** |
| Single operations | 1.55ns     | **0.52ns**     | **0.51ns**     |

See the [benchmark results](assets/bench-results.json) for raw performance analysis.

## Quick Start

### Pre-requisites

- C++20 or later
- Concepts support

### Installation

#### Using CMake

```shell
git clone https://github.com/alpluspluss/libvrt.git
cd libvrt
mkdir build && cd build
cmake .. -DCMAKE_INSTALL_PREFIX=/usr/local -DLIBVRT_BUILD_TESTS=OFF
make install
```

And then in your project's `CMakeLists.txt`:

```cmake
find_package(vrt REQUIRED)
target_link_libraries(target PRIVATE vrt::vrt)
```

#### Header-only

Simply copy the `include/vrt` directory to your project's include path then do

```cpp
#include <vrt>
```

### Usage

```cpp
#include <iostream>
#include <vrt>

int main()
{
    /* create a variant that can hold int, double, and std::string */
    vrt::variant<int, double, std::string> v;
    
    v = 42; /* assign an int value */
    std::cout << "holds int?: " << vrt::holds_alternative<int>(v) << std::endl;
    std::cout << "value: " << vrt::get<int>(v) << std::endl;
    
    /* the switch case */
    switch (v.index())
    {
    case decltype(v)::of<int>:
        std::cout << "int: " << vrt::get<int>(v) << '\n';
        break;
        
    case decltype(v)::of<double>:
        std::cout << "double: " << vrt::get<double>(v) << '\n';
        break;
        
    case decltype(v)::of<std::string>:
        std::cout << "string: " << vrt::get<std::string>(v) << '\n';
        break;
    }
    
    return 0;
}
```

## API Reference

`vrt::variant` provides the exact same API as `std::variant`, including non-member functions
like `holds_alternative`, `get`, and `visit`. The key difference is that you can
use `switch` statements with `vrt::variant`.

### Construction and Assignment

```cpp
vrt::variant<int, std::string> v1;                      /* default construction */
vrt::variant<int, std::string> v2 = 42;                 /* direct initialization */
vrt::variant<int, std::string> v3 = v1;                 /* copy construction */
vrt::variant<int, std::string> v4 = std::move(v1);      /* move construction */

v2 = v3;                                                /* copy assignment */
v2 = std::move(v3);                                     /* move assignment */
v2 = "hello";                                           /* assignment with a different type */
```

### Accessing Values

```cpp
/* check what type does the variant object is holding currently */
bool has_int = vrt::holds_alternative<int>(v);
bool has_int2 = v.holds_alternative<int>();

/* safe access; this throws `std::bad_variant_access` if the type does not match */
int value = vrt::get<int>(v);
int value2 = v.get<int>();
int value3 = vrt::get<0>(v); /* by index */

/* safe access; returns `nullptr` on wrong type */
int* ptr = vrt::get_if<int>(&v);
int* ptr2 = v.get_if<int>();
int* ptr3 = vrt::get_if<0>(&v); /* by index */

/* state inspection */
std::size_t idx = v.index();
bool empty = v.valueless_by_exception();
```

### Visitor Pattern

```cpp
/* std::visit compatibility - same syntax as std::variant */
auto result = vrt::visit([](auto&& arg) -> std::string 
{
    using T = std::decay_t<decltype(arg)>;
    if constexpr (std::is_same_v<T, int>)
        return "integer: " + std::to_string(arg);
    else if constexpr (std::is_same_v<T, double>)
        return "double: " + std::to_string(arg);
    else if constexpr (std::is_same_v<T, std::string>)
        return "string: " + arg;
}, v);
```

### In-place Construction

```cpp
vrt::variant<int, std::string, std::vector<int>> v;

v.emplace<std::string>("constructed");

/* constructing a vector in-place by index */
v.emplace<2>(5, 42);  /* vector with 5 elements of value 42 */
```

### Comparison

```cpp
vrt::variant<int, std::string> v1 = 42;
vrt::variant<int, std::string> v2 = 42;

bool equal = (v1 == v2);        /* true */
bool not_equal = (v1 != v2);    /* false */
auto ordering = (v1 <=> v2);    /* `std::strong_ordering::equal` */
```

### Type Traits

```cpp
using variant_t = vrt::variant<int, double, std::string>;

/* number of alternatives */
constexpr std::size_t n = vrt::variant_size_v<variant_t>;               /* 3 */

/* type of I-th alternative */
using first_type = vrt::variant_alternative_t<0, variant_t>;            /* int */
using second_type = vrt::variant_alternative_t<1, variant_t>;           /* double */

/* index of type `T` in the variant - two equivalent forms */
constexpr std::size_t int_index = variant_t::of<int>;                   /* 0 (short form) */ 
constexpr std::size_t string_index = variant_t::index_of<std::string>;  /* 2 (long form) */
```

### Switch Case Support

```cpp
using variant_t = vrt::variant<int, double, std::string>;

auto process = [](const variant_t& v) 
{
    switch (v.index()) 
    {
        case variant_t::of<int>:
            return "got integer: " + std::to_string(vrt::get<int>(v));
        case variant_t::of<double>:
            return "got double: " + std::to_string(vrt::get<double>(v));
        case variant_t::of<std::string>:
            return "got string: " + vrt::get<std::string>(v);
        default:
            return "empty variant";
    }
};
```

> [!NOTE]
> `vrt::variant` uses small storage optimization (SSO) to minimize heap allocations. Types
> up to 32 bytes are stored inline within the variant object. When a variant contains
> types larger than 32 bytes, only those oversized types are heap-allocated while smaller
> types remain inline.
>
> Operations are noexcept whenever possible and heap objects transfer ownership to the variant without
> any copy operations or deallocation.

### CMake Build Options

- `LIBVRT_BUILD_TESTS` - Build unit tests; defaults to `ON`
- `LIBVRT_BUILD_BENCHMARKS` - Build benchmarks; defaults to `ON`
- `LIBVRT_INSTALL` - Enable installation; defaults to `ON`

## Migration from `std::variant`

vrt is a drop-in replacement. Simply change `std::variant<T...>` to `vrt::variant<T...>` and optionally:

1. **Keep using `std::visit`** - Replace with `vrt::visit` for full compatibility
2. **Migrate to switch cases** - Replace visitor patterns with clean switch statements

**Migration example:**

```cpp
/* from */
std::variant<int, std::string> v = 42;
std::visit([](auto&& arg) { /* handle */ }, v);

/* to */
vrt::variant<int, std::string> v = 42;
vrt::visit([](auto&& arg) { /* handle */ }, v);

/* final */
vrt::variant<int, std::string> v = 42;
switch (v.index()) 
{
    case decltype(v)::of<int>: /* handle int */ break;
    case decltype(v)::of<std::string>: /* handle string */ break;
}
```

## License

This project is licensed under the MIT License. See [LICENSE](LICENSE.txt) for details.
