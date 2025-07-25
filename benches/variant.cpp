
#pragma GCC diagnostic ignored "-Wall"
#pragma GCC diagnostic ignored "-Wextra"
#pragma GCC diagnostic ignored "-Wpedantic"

#include <array>
#include <memory>
#include <random>
#include <sstream>
#include <string>
#include <variant>
#include <vector>
#include <vrt>
#include <benchmark/benchmark.h>

#ifdef HAS_BOOST_VARIANT
#include <boost/variant.hpp>
#include <boost/variant/apply_visitor.hpp>
#endif

#ifdef HAS_BOOST_VARIANT2
#include <boost/variant2/variant.hpp>
#endif

struct simple_pod
{
	int x;
	double y;
};

struct medium_object
{
	std::array<int, 16> data;
	std::string label;
	medium_object() : data {}, label("default") {}

	medium_object(int v, std::string l) : label(std::move(l))
	{
		data.fill(v);
	}
};

struct complex_object
{
	std::array<double, 32> matrix;
	std::vector<std::string> tags;
	std::unique_ptr<int> ptr;
	complex_object() : matrix {}, tags(4, "tag"), ptr(std::make_unique<int>(42)) {}

	complex_object(const complex_object &other) : matrix(other.matrix), tags(other.tags),
	                                              ptr(std::make_unique<int>(*other.ptr)) {}

	complex_object &operator=(const complex_object &other)
	{
		if (this != &other)
		{
			matrix = other.matrix;
			tags = other.tags;
			ptr = std::make_unique<int>(*other.ptr);
		}
		return *this;
	}

	complex_object(complex_object &&) = default;

	complex_object &operator=(complex_object &&) = default;
};

using test_variant_std = std::variant<int, std::string, simple_pod, medium_object, complex_object>;
using test_variant_vrt = vrt::variant<int, std::string, simple_pod, medium_object, complex_object>;

#ifdef HAS_BOOST_VARIANT
using test_variant_boost = boost::variant<int, std::string, simple_pod, medium_object, complex_object>;
#endif

#ifdef HAS_BOOST_VARIANT2
using test_variant_boost2 = boost::variant2::variant<int, std::string, simple_pod, medium_object, complex_object>;
#endif

struct simple_visitor
{
	int operator()(int i) const
	{
		return i * 2;
	}

	int operator()(const std::string &s) const
	{
		return static_cast<int>(s.length());
	}

	int operator()(const simple_pod &p) const
	{
		return p.x;
	}

	int operator()(const medium_object &m) const
	{
		return m.data[0];
	}

	int operator()(const complex_object &c) const
	{
		return *c.ptr;
	}
};

struct complex_visitor
{
	std::string operator()(int i) const
	{
		std::string result;
		for (int j = 0; j < 10; ++j)
		{
			result += std::to_string(i + j);
			if (j < 9)
				result += ",";
		}
		std::vector<int> temp(i % 100);
		std::iota(temp.begin(), temp.end(), 0);
		std::sort(temp.begin(), temp.end(), std::greater<int>());
		for (size_t k = 0; k < std::min(size_t(5), temp.size()); ++k)
		{
			result += ":" + std::to_string(temp[k]);
		}
		return result;
	}

	std::string operator()(const std::string &s) const
	{
		std::string result = s;
		std::reverse(result.begin(), result.end());
		std::transform(result.begin(), result.end(), result.begin(), ::toupper);
		std::vector<char> chars(result.begin(), result.end());
		std::sort(chars.begin(), chars.end());
		std::string sorted(chars.begin(), chars.end());
		return s + "_processed_" + sorted;
	}

	std::string operator()(const simple_pod &p) const
	{
		std::ostringstream oss;
		oss << "pod{x=" << p.x << ",y=" << p.y << "}";
		std::string base = oss.str();
		for (int i = 0; i < 5; ++i)
		{
			base += "_layer" + std::to_string(i);
		}
		return base;
	}

	std::string operator()(const medium_object &m) const
	{
		std::string result = m.label + "_analysis:";
		int sum = 0;
		for (const auto &val: m.data)
		{
			sum += val;
		}
		result += "sum=" + std::to_string(sum);
		std::vector<int> data_copy(m.data.begin(), m.data.end());
		std::sort(data_copy.begin(), data_copy.end());
		result += ",median=" + std::to_string(data_copy[data_copy.size() / 2]);
		return result;
	}

	std::string operator()(const complex_object &c) const
	{
		std::string result = "complex_analysis:";
		double matrix_sum = 0;
		for (const auto &val: c.matrix)
		{
			matrix_sum += val;
		}
		result += "matrix_avg=" + std::to_string(matrix_sum / c.matrix.size());
		result += ",tags=" + std::to_string(c.tags.size());
		result += ",ptr_val=" + std::to_string(*c.ptr);
		std::vector<std::string> tags_copy = c.tags;
		std::sort(tags_copy.begin(), tags_copy.end());
		for (const auto &tag: tags_copy)
		{
			result += "," + tag;
		}
		return result;
	}
};

#ifdef HAS_BOOST_VARIANT
struct boost_simple_visitor : boost::static_visitor<int>
{
public:
	boost_simple_visitor() = default;

	template<typename T>
	int operator()(const T &value) const
	{
		return simple_visitor {}(value);
	}
};

struct boost_complex_visitor : boost::static_visitor<std::string>
{
public:
	boost_complex_visitor() = default;

	template<typename T>
	std::string operator()(const T &value) const
	{
		return complex_visitor {}(value);
	}
};
#endif

template<typename Variant>
static void BM_DefaultConstruction(benchmark::State &state)
{
	for (auto _: state)
	{
		Variant v;
		benchmark::DoNotOptimize(v);
	}
}

template<typename Variant>
static void BM_ValueConstruction_Int(benchmark::State &state)
{
	for (auto _: state)
	{
		Variant v { 42 };
		benchmark::DoNotOptimize(v);
	}
}

template<typename Variant>
static void BM_ValueConstruction_String(benchmark::State &state)
{
	for (auto _: state)
	{
		Variant v { std::string("benchmark_test_string") };
		benchmark::DoNotOptimize(v);
	}
}

template<typename Variant>
static void BM_ValueConstruction_Complex(benchmark::State &state)
{
	for (auto _: state)
	{
		Variant v { complex_object {} };
		benchmark::DoNotOptimize(v);
	}
}

template<typename Variant>
static void BM_CopyConstruction(benchmark::State &state)
{
	Variant source { medium_object { 123, "test_object" } };
	for (auto _: state)
	{
		Variant v { source };
		benchmark::DoNotOptimize(v);
	}
}

template<typename Variant>
static void BM_MoveConstruction(benchmark::State &state)
{
	for (auto _: state)
	{
		Variant source { complex_object {} };
		Variant v { std::move(source) };
		benchmark::DoNotOptimize(v);
	}
}

template<typename Variant>
static void BM_CopyAssignment_SameType(benchmark::State &state)
{
	Variant v1 { 42 };
	Variant v2 { 84 };
	for (auto _: state)
	{
		v1 = v2;
		benchmark::DoNotOptimize(v1);
	}
}

template<typename Variant>
static void BM_CopyAssignment_DifferentType(benchmark::State &state)
{
	Variant v1 { 42 };
	Variant v2 { std::string("different_type") };
	for (auto _: state)
	{
		v1 = v2;
		benchmark::DoNotOptimize(v1);
		std::swap(v1, v2);
	}
}

template<typename Variant>
static void BM_MoveAssignment_SameType(benchmark::State &state)
{
	for (auto _: state)
	{
		Variant v1 { std::string("original") };
		Variant v2 { std::string("moved") };
		v1 = std::move(v2);
		benchmark::DoNotOptimize(v1);
	}
}

template<typename Variant>
static void BM_MoveAssignment_DifferentType(benchmark::State &state)
{
	for (auto _: state)
	{
		Variant v1 { 42 };
		Variant v2 { complex_object {} };
		v1 = std::move(v2);
		benchmark::DoNotOptimize(v1);
	}
}

template<typename Variant>
static void BM_Emplace_Int(benchmark::State &state)
{
	Variant v;
	for (auto _: state)
	{
		if constexpr (std::is_same_v<Variant, test_variant_vrt>)
		{
			v.template emplace<int>(state.iterations() % 1000);
		}
#ifdef HAS_BOOST_VARIANT
		else if constexpr (std::is_same_v<Variant, test_variant_boost>)
		{
			v = static_cast<int>(state.iterations() % 1000);
		}
#endif
#ifdef HAS_BOOST_VARIANT2
		else if constexpr (std::is_same_v<Variant, test_variant_boost2>)
		{
			v.template emplace<int>(state.iterations() % 1000);
		}
#endif
		else
		{
			v.template emplace<int>(state.iterations() % 1000);
		}
		benchmark::DoNotOptimize(v);
	}
}

template<typename Variant>
static void BM_Emplace_String(benchmark::State &state)
{
	Variant v;
	for (auto _: state)
	{
		if constexpr (std::is_same_v<Variant, test_variant_vrt>)
		{
			v.template emplace<std::string>("test_string");
		}
#ifdef HAS_BOOST_VARIANT
		else if constexpr (std::is_same_v<Variant, test_variant_boost>)
		{
			v = std::string("test_string");
		}
#endif
#ifdef HAS_BOOST_VARIANT2
		else if constexpr (std::is_same_v<Variant, test_variant_boost2>)
		{
			v.template emplace<std::string>("test_string");
		}
#endif
		else
		{
			v.template emplace<std::string>("test_string");
		}
		benchmark::DoNotOptimize(v);
	}
}

template<typename Variant>
static void BM_Emplace_Complex(benchmark::State &state)
{
	Variant v;
	for (auto _: state)
	{
		if constexpr (std::is_same_v<Variant, test_variant_vrt>)
		{
			v.template emplace<complex_object>();
		}
#ifdef HAS_BOOST_VARIANT
		else if constexpr (std::is_same_v<Variant, test_variant_boost>)
		{
			v = complex_object {};
		}
#endif
#ifdef HAS_BOOST_VARIANT2
		else if constexpr (std::is_same_v<Variant, test_variant_boost2>)
		{
			v.template emplace<complex_object>();
		}
#endif
		else
		{
			v.template emplace<complex_object>();
		}
		benchmark::DoNotOptimize(v);
	}
}

template<typename Variant>
static void BM_SimpleVisitor(benchmark::State &state)
{
	Variant v { medium_object { 42, "test" } };
	simple_visitor visitor;
	for (auto _: state)
	{
		int result;
		if constexpr (std::is_same_v<Variant, test_variant_vrt>)
		{
			result = vrt::visit(visitor, v);
		}
#ifdef HAS_BOOST_VARIANT
		else if constexpr (std::is_same_v<Variant, test_variant_boost>)
		{
			result = boost::apply_visitor(boost_simple_visitor {}, v);
		}
#endif
#ifdef HAS_BOOST_VARIANT2
		else if constexpr (std::is_same_v<Variant, test_variant_boost2>)
		{
			result = boost::variant2::visit(visitor, v);
		}
#endif
		else
		{
			result = std::visit(visitor, v);
		}
		benchmark::DoNotOptimize(result);
	}
}

template<typename Variant>
static void BM_ComplexVisitor(benchmark::State &state)
{
	Variant v { complex_object {} };
	complex_visitor visitor;
	for (auto _: state)
	{
		std::string result;
		if constexpr (std::is_same_v<Variant, test_variant_vrt>)
		{
			result = vrt::visit(visitor, v);
		}
#ifdef HAS_BOOST_VARIANT
		else if constexpr (std::is_same_v<Variant, test_variant_boost>)
		{
			result = boost::apply_visitor(boost_complex_visitor {}, v);
		}
#endif
#ifdef HAS_BOOST_VARIANT2
		else if constexpr (std::is_same_v<Variant, test_variant_boost2>)
		{
			result = boost::variant2::visit(visitor, v);
		}
#endif
		else
		{
			result = std::visit(visitor, v);
		}
		benchmark::DoNotOptimize(result);
	}
}

template<typename Variant>
static void BM_VrtSwitch(benchmark::State &state)
{
	if constexpr (std::is_same_v<Variant, test_variant_vrt>)
	{
		Variant v { medium_object { 42, "test" } };
		for (auto _: state)
		{
			int result;
			switch (v.index())
			{
				case Variant::template of<int>:
					result = vrt::get<int>(v) * 2;
					break;
				case Variant::template of<std::string>:
					result = static_cast<int>(vrt::get<std::string>(v).length());
					break;
				case Variant::template of<simple_pod>:
					result = vrt::get<simple_pod>(v).x;
					break;
				case Variant::template of<medium_object>:
					result = vrt::get<medium_object>(v).data[0];
					break;
				case Variant::template of<complex_object>:
					result = *vrt::get<complex_object>(v).ptr;
					break;
				default:
					result = 0;
			}
			benchmark::DoNotOptimize(result);
		}
	}
	else
	{
		for (auto _: state)
		{
			benchmark::DoNotOptimize(42);
		}
	}
}

template<typename Variant>
static void BM_VrtMatch(benchmark::State &state)
{
	if constexpr (std::is_same_v<Variant, test_variant_vrt>)
	{
		Variant v { medium_object { 42, "test" } };
		for (auto _: state)
		{
			auto result = vrt::match(v) | [](const auto &value) -> int
			{
				if constexpr (std::is_same_v<std::decay_t<decltype(value)>, int>)
				{
					return value * 2;
				}
				else if constexpr (std::is_same_v<std::decay_t<decltype(value)>, std::string>)
				{
					return static_cast<int>(value.length());
				}
				else if constexpr (std::is_same_v<std::decay_t<decltype(value)>, simple_pod>)
				{
					return value.x;
				}
				else if constexpr (std::is_same_v<std::decay_t<decltype(value)>, medium_object>)
				{
					return value.data[0];
				}
				else if constexpr (std::is_same_v<std::decay_t<decltype(value)>, complex_object>)
				{
					return *value.ptr;
				}
				else
				{
					return 0;
				}
			};
			benchmark::DoNotOptimize(result);
		}
	}
	else
	{
		for (auto _: state)
		{
			benchmark::DoNotOptimize(42);
		}
	}
}

template<typename Variant>
static void BM_TypeQuery_Index(benchmark::State &state)
{
	Variant v { std::string("test") };
	for (auto _: state)
	{
		auto idx = [&]()
		{
			if constexpr (std::is_same_v<Variant, test_variant_vrt>)
			{
				return v.index();
			}
#ifdef HAS_BOOST_VARIANT
			else if constexpr (std::is_same_v<Variant, test_variant_boost>)
			{
				return v.which();
			}
#endif
#ifdef HAS_BOOST_VARIANT2
			else if constexpr (std::is_same_v<Variant, test_variant_boost2>)
			{
				return v.index();
			}
#endif
			else
			{
				return v.index();
			}
		}();
		benchmark::DoNotOptimize(idx);
	}
}

template<typename Variant>
static void BM_TypeQuery_HoldsAlternative(benchmark::State &state)
{
	Variant v { std::string("test") };
	for (auto _: state)
	{
		bool result = [&]()
		{
			if constexpr (std::is_same_v<Variant, test_variant_vrt>)
			{
				return v.template holds_alternative<std::string>();
			}
#ifdef HAS_BOOST_VARIANT
			else if constexpr (std::is_same_v<Variant, test_variant_boost>)
			{
				return boost::get<std::string>(&v) != nullptr;
			}
#endif
#ifdef HAS_BOOST_VARIANT2
			else if constexpr (std::is_same_v<Variant, test_variant_boost2>)
			{
				return boost::variant2::holds_alternative<std::string>(v);
			}
#endif
			else
			{
				return std::holds_alternative<std::string>(v);
			}
		}();
		benchmark::DoNotOptimize(result);
	}
}

template<typename Variant>
static void BM_ValueAccess_Get(benchmark::State &state)
{
	Variant v { std::string("benchmark_test") };
	for (auto _: state)
	{
		const auto &result = [&]() -> const std::string &{
			if constexpr (std::is_same_v<Variant, test_variant_vrt>)
			{
				return vrt::get<std::string>(v);
			}
#ifdef HAS_BOOST_VARIANT
			else if constexpr (std::is_same_v<Variant, test_variant_boost>)
			{
				return boost::get<std::string>(v);
			}
#endif
#ifdef HAS_BOOST_VARIANT2
			else if constexpr (std::is_same_v<Variant, test_variant_boost2>)
			{
				return boost::variant2::get<std::string>(v);
			}
#endif
			else
			{
				return std::get<std::string>(v);
			}
		}();
		benchmark::DoNotOptimize(result);
	}
}

template<typename Variant>
static void BM_ValueAccess_GetIf(benchmark::State &state)
{
	Variant v { std::string("benchmark_test") };
	for (auto _: state)
	{
		const auto *result = [&]() -> const std::string *{
			if constexpr (std::is_same_v<Variant, test_variant_vrt>)
			{
				return vrt::get_if<std::string>(&v);
			}
#ifdef HAS_BOOST_VARIANT
			else if constexpr (std::is_same_v<Variant, test_variant_boost>)
			{
				return boost::get<std::string>(&v);
			}
#endif
#ifdef HAS_BOOST_VARIANT2
			else if constexpr (std::is_same_v<Variant, test_variant_boost2>)
			{
				return boost::variant2::get_if<std::string>(&v);
			}
#endif
			else
			{
				return std::get_if<std::string>(&v);
			}
		}();
		benchmark::DoNotOptimize(result);
	}
}

#define REGISTER_BENCHMARKS(lib, variant_type) \
    BENCHMARK(BM_DefaultConstruction<variant_type>)->Name(lib "/DefaultConstruction"); \
    BENCHMARK(BM_ValueConstruction_Int<variant_type>)->Name(lib "/ValueConstruction/Int"); \
    BENCHMARK(BM_ValueConstruction_String<variant_type>)->Name(lib "/ValueConstruction/String"); \
    BENCHMARK(BM_ValueConstruction_Complex<variant_type>)->Name(lib "/ValueConstruction/Complex"); \
    BENCHMARK(BM_CopyConstruction<variant_type>)->Name(lib "/CopyConstruction"); \
    BENCHMARK(BM_MoveConstruction<variant_type>)->Name(lib "/MoveConstruction"); \
    BENCHMARK(BM_CopyAssignment_SameType<variant_type>)->Name(lib "/CopyAssignment/SameType"); \
    BENCHMARK(BM_CopyAssignment_DifferentType<variant_type>)->Name(lib "/CopyAssignment/DifferentType"); \
    BENCHMARK(BM_MoveAssignment_SameType<variant_type>)->Name(lib "/MoveAssignment/SameType"); \
    BENCHMARK(BM_MoveAssignment_DifferentType<variant_type>)->Name(lib "/MoveAssignment/DifferentType"); \
    BENCHMARK(BM_Emplace_Int<variant_type>)->Name(lib "/Emplace/Int"); \
    BENCHMARK(BM_Emplace_String<variant_type>)->Name(lib "/Emplace/String"); \
    BENCHMARK(BM_Emplace_Complex<variant_type>)->Name(lib "/Emplace/Complex"); \
    BENCHMARK(BM_SimpleVisitor<variant_type>)->Name(lib "/SimpleVisitor"); \
    BENCHMARK(BM_ComplexVisitor<variant_type>)->Name(lib "/ComplexVisitor"); \
    BENCHMARK(BM_TypeQuery_Index<variant_type>)->Name(lib "/TypeQuery/Index"); \
    BENCHMARK(BM_TypeQuery_HoldsAlternative<variant_type>)->Name(lib "/TypeQuery/HoldsAlternative"); \
    BENCHMARK(BM_ValueAccess_Get<variant_type>)->Name(lib "/ValueAccess/Get"); \
    BENCHMARK(BM_ValueAccess_GetIf<variant_type>)->Name(lib "/ValueAccess/GetIf");

REGISTER_BENCHMARKS("std", test_variant_std)
REGISTER_BENCHMARKS("vrt", test_variant_vrt)

BENCHMARK(BM_VrtSwitch<test_variant_vrt>)->Name("vrt/VrtSwitch");
BENCHMARK(BM_VrtMatch<test_variant_vrt>)->Name("vrt/VrtMatch");

#ifdef HAS_BOOST_VARIANT
REGISTER_BENCHMARKS("boost", test_variant_boost)
#endif

#ifdef HAS_BOOST_VARIANT2
REGISTER_BENCHMARKS("boost2", test_variant_boost2)
#endif

BENCHMARK_MAIN();
