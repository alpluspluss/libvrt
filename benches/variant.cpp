#include <array>
#include <random>
#include <string>
#include <variant>
#include <vector>
#include <vrt>
#include <benchmark/benchmark.h>

#ifdef HAS_BOOST
#include <boost/variant.hpp>
#include <boost/variant/apply_visitor.hpp>
#endif

struct trivial_small
{
	int value;

	bool operator==(const trivial_small& other) const = default;
};

struct non_trivial_small
{
	std::unique_ptr<int> ptr;

	non_trivial_small() : ptr(std::make_unique<int>(0)) {}
	non_trivial_small(int v) : ptr(std::make_unique<int>(v)) {}
	non_trivial_small(const non_trivial_small& other) : ptr(std::make_unique<int>(*other.ptr)) {}
	non_trivial_small& operator=(const non_trivial_small& other)
	{
		if (this != &other)
			ptr = std::make_unique<int>(*other.ptr);
		return *this;
	}
	non_trivial_small(non_trivial_small&&) = default;
	non_trivial_small& operator=(non_trivial_small&&) = default;

	bool operator==(const non_trivial_small& other) const
	{
		return ptr && other.ptr && *ptr == *other.ptr;
	}
};

struct medium_object
{
	std::array<int, 32> data;
	std::string name;

	medium_object() : data{}, name("default") {}
	medium_object(int val, const std::string& n) : name(n)
	{
		data.fill(val);
	}

	bool operator==(const medium_object& other) const = default;
};

struct large_object
{
	std::array<int, 256> data;
	std::vector<std::string> strings;

	large_object() : data{}, strings(8, "test") {}
	large_object(int val) : strings(8, "test")
	{
		data.fill(val);
	}

	bool operator==(const large_object& other) const = default;
};

using std_simple_t = std::variant<int, double, std::string>;
using vrt_simple_t = vrt::variant<int, double, std::string>;

#ifdef HAS_BOOST
using boost_simple_t = boost::variant<int, double, std::string>;
#endif

using std_complex_t = std::variant<trivial_small, non_trivial_small, medium_object>;
using vrt_complex_t = vrt::variant<trivial_small, non_trivial_small, medium_object>;

#ifdef HAS_BOOST
using boost_complex_t = boost::variant<trivial_small, non_trivial_small, medium_object>;
#endif

using std_large_t = std::variant<
	int, double, float, char, short, long, long long, bool,
	std::string, std::vector<int>, std::array<int, 4>, medium_object, large_object
>;
using vrt_large_t = vrt::variant<
	int, double, float, char, short, long, long long, bool,
	std::string, std::vector<int>, std::array<int, 4>, medium_object, large_object
>;

#ifdef HAS_BOOST
using boost_large_t = boost::variant<
	int, double, float, char, short, long, long long, bool,
	std::string, std::vector<int>, std::array<int, 4>, medium_object, large_object
>;
#endif

struct cvisitor
{
	template<typename T>
		requires std::is_arithmetic_v<T>
	int operator()(T value) const
	{
		return static_cast<int>(value) * 2;
	}

	int operator()(const std::string& str) const
	{
		return static_cast<int>(str.length());
	}

	int operator()(const std::vector<int>& vec) const
	{
		return static_cast<int>(vec.size());
	}

	template<std::size_t N>
	int operator()(const std::array<int, N>& arr) const
	{
		return static_cast<int>(N);
	}

	int operator()(const trivial_small& obj) const
	{
		return obj.value * 2;
	}

	int operator()(const non_trivial_small& obj) const
	{
		return *obj.ptr * 2;
	}

	int operator()(const medium_object& obj) const
	{
		return obj.data[0] * 2;
	}

	int operator()(const large_object& obj) const
	{
		return obj.data[0] * 2;
	}
};

#ifdef HAS_BOOST
struct boost_visitor : public boost::static_visitor<int>
{
	template<typename T>
		requires std::is_arithmetic_v<T>
	int operator()(T value) const
	{
		return static_cast<int>(value) * 2;
	}

	int operator()(const std::string& str) const
	{
		return static_cast<int>(str.length());
	}

	int operator()(const std::vector<int>& vec) const
	{
		return static_cast<int>(vec.size());
	}

	template<std::size_t N>
	int operator()(const std::array<int, N>& arr) const
	{
		return static_cast<int>(N);
	}

	int operator()(const trivial_small& obj) const
	{
		return obj.value * 2;
	}

	int operator()(const non_trivial_small& obj) const
	{
		return *obj.ptr * 2;
	}

	int operator()(const medium_object& obj) const
	{
		return obj.data[0] * 2;
	}

	int operator()(const large_object& obj) const
	{
		return obj.data[0] * 2;
	}
};
#endif

template<typename Variant>
int switch_visit_impl(const Variant& v)
{
	if constexpr (std::is_same_v<Variant, vrt_simple_t>)
	{
		switch (v.index())
		{
			case Variant::template of<int>:
				return v.template get<int>() * 2;
			case Variant::template of<double>:
				return static_cast<int>(v.template get<double>() * 2.0);
			case Variant::template of<std::string>:
				return static_cast<int>(v.template get<std::string>().length());
			default:
				return 0;
		}
	}
	else if constexpr (std::is_same_v<Variant, vrt_complex_t>)
	{
		switch (v.index())
		{
			case Variant::template of<trivial_small>:
				return v.template get<trivial_small>().value * 2;
			case Variant::template of<non_trivial_small>:
				return *v.template get<non_trivial_small>().ptr * 2;
			case Variant::template of<medium_object>:
				return v.template get<medium_object>().data[0] * 2;
			default:
				return 0;
		}
	}
	else if constexpr (std::is_same_v<Variant, vrt_large_t>)
	{
		switch (v.index())
		{
			case Variant::template of<int>:
				return v.template get<int>() * 2;
			case Variant::template of<double>:
				return static_cast<int>(v.template get<double>() * 2.0);
			case Variant::template of<float>:
				return static_cast<int>(v.template get<float>() * 2.0f);
			case Variant::template of<char>:
				return static_cast<int>(v.template get<char>()) * 2;
			case Variant::template of<short>:
				return static_cast<int>(v.template get<short>()) * 2;
			case Variant::template of<long>:
				return static_cast<int>(v.template get<long>()) * 2;
			case Variant::template of<long long>:
				return static_cast<int>(v.template get<long long>()) * 2;
			case Variant::template of<bool>:
				return v.template get<bool>() ? 1 : 0;
			case Variant::template of<std::string>:
				return static_cast<int>(v.template get<std::string>().length());
			case Variant::template of<std::vector<int>>:
				return static_cast<int>(v.template get<std::vector<int>>().size());
			case Variant::template of<std::array<int, 4>>:
				return 4;
			case Variant::template of<medium_object>:
				return v.template get<medium_object>().data[0] * 2;
			case Variant::template of<large_object>:
				return v.template get<large_object>().data[0] * 2;
			default:
				return 0;
		}
	}

	return 0;
}

template<typename Variant>
std::vector<Variant> generate_test_variants(std::size_t count, unsigned seed = 42)
{
	std::vector<Variant> variants;
	variants.reserve(count);
	std::mt19937 gen(seed);

	if constexpr (std::is_same_v<Variant, std_simple_t> ||
	              std::is_same_v<Variant, vrt_simple_t>
#ifdef HAS_BOOST
	              || std::is_same_v<Variant, boost_simple_t>
#endif
	)
	{
		std::uniform_int_distribution<> dis(0, 2);
		for (std::size_t i = 0; i < count; ++i)
		{
			switch (dis(gen))
			{
				case 0:
					variants.emplace_back(static_cast<int>(i));
					break;
				case 1:
					variants.emplace_back(static_cast<double>(i) * 1.5);
					break;
				case 2:
					variants.emplace_back(std::string("test_") + std::to_string(i));
					break;
			}
		}
	}
	else if constexpr (std::is_same_v<Variant, std_complex_t> ||
	                   std::is_same_v<Variant, vrt_complex_t>
#ifdef HAS_BOOST
	                   || std::is_same_v<Variant, boost_complex_t>
#endif
	)
	{
		std::uniform_int_distribution<> dis(0, 2);
		for (std::size_t i = 0; i < count; ++i)
		{
			switch (dis(gen))
			{
				case 0:
					variants.emplace_back(trivial_small{static_cast<int>(i)});
					break;
				case 1:
					variants.emplace_back(non_trivial_small{static_cast<int>(i)});
					break;
				case 2:
					variants.emplace_back(medium_object{static_cast<int>(i), "test"});
					break;
			}
		}
	}
	else if constexpr (std::is_same_v<Variant, std_large_t> ||
	                   std::is_same_v<Variant, vrt_large_t>
#ifdef HAS_BOOST
	                   || std::is_same_v<Variant, boost_large_t>
#endif
	)
	{
		std::uniform_int_distribution<> dis(0, 5);
		for (std::size_t i = 0; i < count; ++i)
		{
			switch (dis(gen))
			{
				case 0:
					variants.emplace_back(static_cast<int>(i));
					break;
				case 1:
					variants.emplace_back(static_cast<double>(i) * 1.5);
					break;
				case 2:
					variants.emplace_back(std::string("test_") + std::to_string(i));
					break;
				case 3:
					variants.emplace_back(std::vector<int>(i % 10, static_cast<int>(i)));
					break;
				case 4:
					variants.emplace_back(medium_object{static_cast<int>(i), "test"});
					break;
				case 5:
					variants.emplace_back(large_object{static_cast<int>(i)});
					break;
			}
		}
	}

	return variants;
}

template<typename Variant>
static void BM_StdVisit_Batch(benchmark::State& state)
{
	constexpr std::size_t VARIANT_COUNT = 1000;
	auto variants = generate_test_variants<Variant>(VARIANT_COUNT);
	cvisitor visitor;

	for (auto _ : state)
	{
		std::size_t sum = 0;
		for (const auto& v : variants)
		{
			sum += std::visit(visitor, v);
		}
		benchmark::DoNotOptimize(sum);
	}
	state.SetItemsProcessed(state.iterations() * VARIANT_COUNT);
}

template<typename Variant>
static void BM_VrtVisit_Batch(benchmark::State& state)
{
	constexpr std::size_t VARIANT_COUNT = 1000;
	auto variants = generate_test_variants<Variant>(VARIANT_COUNT);
	cvisitor visitor;

	for (auto _ : state)
	{
		std::size_t sum = 0;
		for (const auto& v : variants)
		{
			sum += vrt::visit(visitor, v);
		}
		benchmark::DoNotOptimize(sum);
	}
	state.SetItemsProcessed(state.iterations() * VARIANT_COUNT);
}

template<typename Variant>
static void BM_VrtSwitch_Batch(benchmark::State& state)
{
	constexpr std::size_t VARIANT_COUNT = 1000;
	auto variants = generate_test_variants<Variant>(VARIANT_COUNT);

	for (auto _ : state)
	{
		std::size_t sum = 0;
		for (const auto& v : variants)
		{
			sum += switch_visit_impl(v);
		}
		benchmark::DoNotOptimize(sum);
	}
	state.SetItemsProcessed(state.iterations() * VARIANT_COUNT);
}

#ifdef HAS_BOOST
template<typename Variant>
static void BM_BoostVisit_Batch(benchmark::State& state)
{
	constexpr std::size_t VARIANT_COUNT = 1000;
	auto variants = generate_test_variants<Variant>(VARIANT_COUNT);
	boost_visitor visitor;

	for (auto _ : state)
	{
		std::size_t sum = 0;
		for (const auto& v : variants)
		{
			sum += boost::apply_visitor(visitor, v);
		}
		benchmark::DoNotOptimize(sum);
	}
	state.SetItemsProcessed(state.iterations() * VARIANT_COUNT);
}
#endif

template<typename Variant>
static void BM_StdVisit_Single(benchmark::State& state)
{
	Variant v;

	if constexpr (std::is_same_v<Variant, std_simple_t>
#ifdef HAS_BOOST
	              || std::is_same_v<Variant, boost_simple_t>
#endif
	)
	{
		v = 42;
	}
	else if constexpr (std::is_same_v<Variant, std_complex_t>
#ifdef HAS_BOOST
	                   || std::is_same_v<Variant, boost_complex_t>
#endif
	)
	{
		v = trivial_small{42};
	}
	else if constexpr (std::is_same_v<Variant, std_large_t>
#ifdef HAS_BOOST
	                   || std::is_same_v<Variant, boost_large_t>
#endif
	)
	{
		v = 42;
	}

	cvisitor visitor;

	for (auto _ : state)
	{
		benchmark::DoNotOptimize(std::visit(visitor, v));
	}
}

template<typename Variant>
static void BM_VrtVisit_Single(benchmark::State& state)
{
	Variant v;

	if constexpr (std::is_same_v<Variant, vrt_simple_t>)
	{
		v = 42;
	}
	else if constexpr (std::is_same_v<Variant, vrt_complex_t>)
	{
		v = trivial_small{42};
	}
	else if constexpr (std::is_same_v<Variant, vrt_large_t>)
	{
		v = 42;
	}

	cvisitor visitor;

	for (auto _ : state)
	{
		benchmark::DoNotOptimize(vrt::visit(visitor, v));
	}
}

template<typename Variant>
static void BM_VrtSwitch_Single(benchmark::State& state)
{
	Variant v;

	if constexpr (std::is_same_v<Variant, vrt_simple_t>)
	{
		v = 42;
	}
	else if constexpr (std::is_same_v<Variant, vrt_complex_t>)
	{
		v = trivial_small{42};
	}
	else if constexpr (std::is_same_v<Variant, vrt_large_t>)
	{
		v = 42;
	}

	for (auto _ : state)
	{
		benchmark::DoNotOptimize(switch_visit_impl(v));
	}
}

#ifdef HAS_BOOST
template<typename Variant>
static void BM_BoostVisit_Single(benchmark::State& state)
{
	Variant v;

	if constexpr (std::is_same_v<Variant, boost_simple_t>)
	{
		v = 42;
	}
	else if constexpr (std::is_same_v<Variant, boost_complex_t>)
	{
		v = trivial_small{42};
	}
	else if constexpr (std::is_same_v<Variant, boost_large_t>)
	{
		v = 42;
	}

	boost_visitor visitor;

	for (auto _ : state)
	{
		benchmark::DoNotOptimize(boost::apply_visitor(visitor, v));
	}
}
#endif

template<typename Variant>
static void BM_Construction(benchmark::State& state)
{
	for (auto _ : state)
	{
		if constexpr (requires { Variant{42}; })
		{
			Variant v1{42};
			benchmark::DoNotOptimize(v1);
		}
		if constexpr (requires { Variant{3.14}; })
		{
			Variant v2{3.14};
			benchmark::DoNotOptimize(v2);
		}
		if constexpr (requires { Variant{std::string("test")}; })
		{
			Variant v3{std::string("test")};
			benchmark::DoNotOptimize(v3);
		}
		if constexpr (requires { Variant{trivial_small{123}}; })
		{
			Variant v4{trivial_small{123}};
			benchmark::DoNotOptimize(v4);
		}
	}
}

template<typename Variant>
static void BM_Assignment(benchmark::State& state)
{
	Variant v;

	for (auto _ : state)
	{
		if constexpr (requires { v = 42; })
		{
			v = 42;
			benchmark::DoNotOptimize(v);
		}
		if constexpr (requires { v = 3.14; })
		{
			v = 3.14;
			benchmark::DoNotOptimize(v);
		}
		if constexpr (requires { v = std::string("test"); })
		{
			v = std::string("test");
			benchmark::DoNotOptimize(v);
		}
	}
}

template<typename Variant>
static void BM_Copy(benchmark::State& state)
{
	auto variants = generate_test_variants<Variant>(100);

	for (auto _ : state)
	{
		for (const auto& original : variants)
		{
			Variant copy = original;
			benchmark::DoNotOptimize(copy);
		}
	}
	state.SetItemsProcessed(state.iterations() * 100);
}

template<typename Variant>
static void BM_Move(benchmark::State& state)
{
	for (auto _ : state)
	{
		auto variants = generate_test_variants<Variant>(100);
		for (auto&& original : variants)
		{
			Variant moved = std::move(original);
			benchmark::DoNotOptimize(moved);
		}
	}
	state.SetItemsProcessed(state.iterations() * 100);
}

#define REGISTER_VARIANT_BENCHMARKS(category, std_type, vrt_type) \
	BENCHMARK(BM_StdVisit_Batch<std_type>)->Name("std::variant/" category "/Visit/Batch"); \
	BENCHMARK(BM_VrtVisit_Batch<vrt_type>)->Name("vrt::variant/" category "/Visit/Batch"); \
	BENCHMARK(BM_VrtSwitch_Batch<vrt_type>)->Name("vrt::variant/" category "/Switch/Batch"); \
	BENCHMARK(BM_StdVisit_Single<std_type>)->Name("std::variant/" category "/Visit/Single"); \
	BENCHMARK(BM_VrtVisit_Single<vrt_type>)->Name("vrt::variant/" category "/Visit/Single"); \
	BENCHMARK(BM_VrtSwitch_Single<vrt_type>)->Name("vrt::variant/" category "/Switch/Single"); \
	BENCHMARK(BM_Construction<std_type>)->Name("std::variant/" category "/Construction"); \
	BENCHMARK(BM_Construction<vrt_type>)->Name("vrt::variant/" category "/Construction"); \
	BENCHMARK(BM_Assignment<std_type>)->Name("std::variant/" category "/Assignment"); \
	BENCHMARK(BM_Assignment<vrt_type>)->Name("vrt::variant/" category "/Assignment"); \
	BENCHMARK(BM_Copy<std_type>)->Name("std::variant/" category "/Copy"); \
	BENCHMARK(BM_Copy<vrt_type>)->Name("vrt::variant/" category "/Copy"); \
	BENCHMARK(BM_Move<std_type>)->Name("std::variant/" category "/Move"); \
	BENCHMARK(BM_Move<vrt_type>)->Name("vrt::variant/" category "/Move");

#ifdef HAS_BOOST
#define REGISTER_BOOST_BENCHMARKS(category, boost_type) \
	BENCHMARK(BM_BoostVisit_Batch<boost_type>)->Name("boost::variant/" category "/Visit/Batch"); \
	BENCHMARK(BM_BoostVisit_Single<boost_type>)->Name("boost::variant/" category "/Visit/Single"); \
	BENCHMARK(BM_Construction<boost_type>)->Name("boost::variant/" category "/Construction"); \
	BENCHMARK(BM_Assignment<boost_type>)->Name("boost::variant/" category "/Assignment"); \
	BENCHMARK(BM_Copy<boost_type>)->Name("boost::variant/" category "/Copy"); \
	BENCHMARK(BM_Move<boost_type>)->Name("boost::variant/" category "/Move");
#else
#define REGISTER_BOOST_BENCHMARKS(category, boost_type)
#endif

REGISTER_VARIANT_BENCHMARKS("Simple", std_simple_t, vrt_simple_t)
REGISTER_BOOST_BENCHMARKS("Simple", boost_simple_t)

REGISTER_VARIANT_BENCHMARKS("Complex", std_complex_t, vrt_complex_t)
REGISTER_BOOST_BENCHMARKS("Complex", boost_complex_t)

REGISTER_VARIANT_BENCHMARKS("Large", std_large_t, vrt_large_t)
REGISTER_BOOST_BENCHMARKS("Large", boost_large_t)

BENCHMARK_MAIN();
