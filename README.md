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
switch (v) 
{
    case Value::index_of<int>:
        std::cout << "int: " << i << '\n';
        break;
    case Value::index_of<double>:
        std::cout << "double: " << d << '\n';
        break;
    case Value::index_of<std::string>:
        std::cout << "string: " << s << '\n';
        break;
}
```

### Performance

**`vrt::variant` delivers up to 5.3x faster performance than `std::visit`**, with switch 
cases reaching over 1 billion operations per second. See the [benchmark results](assets/bench-results.json) 
for detailed performance analysis.

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
    switch (v)
    {
    case decltype(v)::index_of<int>:
        std::cout << "int: " << vrt::get<int>(v) << '\n';
        break;
        
    case decltype(v)::index_of<double>:
        std::cout << "double: " << vrt::get<double>(v) << '\n';
        break;
    case decltype(v)::index_of<std::string>:
        std::cout << "string: " << vrt::get<std::string>(v) << '\n';
        break;
    }
    
    return 0;
}
```

## API Reference

`vrt::variant` provides the exact same API as `std::variant`, including non-member functions 
like `holds_alternative` and `get`. The only difference is that you can 
use `switch` statements with `vrt::variant`.

### Construction and Assignment

```cpp
vrt::variant<int, std::string> v1;                      /* default construction */
vrt::variant<int, std::string> v2 = 42;                 /* direct initialization */
vrt::variant<int, std::string> v3 = v1};                /* copy construction */
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

/* index of type `T` in the variant */
constexpr std::size_t int_index = variant_t::index_of<int>;             /* 0 */ 
constexpr std::size_t string_index = variant_t::index_of<std::string>;  /* 2 */
```

### Switch Case Support

```cpp
using variant_t = vrt::variant<int, double, std::string>;

auto process = [](const variant_t& v) 
{
    switch (v) 
    {
        case variant_t::index_of<int>:
            return "got integer: " + std::to_string(v.get<int>());
        case variant_t::index_of<double>:
            return "got double: " + std::to_string(v.get<double>());
        case variant_t::index_of<std::string>:
            return "got string: " + v.get<std::string>();
        default:
            return "empty variant";
    }
};
```

> [!NOTE]
> `vrt::variant` uses small storage optimization (SSO) to minimize heap allocations. Types 
> up to 24 bytes are stored inline within the variant object. When a variant contains 
> types larger than 24 bytes, only those oversized types are heap-allocated while smaller 
> types remain inline.
> 
> Operations are noexcept whenever possible and heap objects transfer ownership to the variant without
> any copy operations or deallocation.

### CMake Build Options

- `LIBVRT_BUILD_TESTS` - Build unit tests; defaults to `ON`
- `LIBVRT_BUILD_BENCHMARKS` - Build benchmarks; defaults to `ON`
- `LIBVRT_INSTALL` - Enable installation; defaults to `ON`

## Migration from `std::variant`

vrt is a drop-in replacement. Simply change `std::variant<T...>` to `vrt::variant<T...>` and change
`std::visit` to switch-case.

## License

This project is licensed under the MIT License. See [LICENSE](LICENSE.txt) for details.
