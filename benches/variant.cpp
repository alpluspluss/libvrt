#include <random>
#include <string>
#include <variant>
#include <vector>
#include <array>
#include <vrt>
#include <benchmark/benchmark.h>

struct trivial_small
{
	int value;
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
};

struct medium_object
{
	std::array<int, 16> data;
	std::string name;

	medium_object() : data{}, name("default") {}
	medium_object(int val, const std::string& n) : name(n)
	{
		data.fill(val);
	}
};

struct large_object
{
	std::array<int, 256> data;
	std::vector<std::string> strings;

	large_object() : data{}, strings(10, "test") {}
	large_object(int val) : strings(10, "test")
	{
		data.fill(val);
	}
};

using std_small_variant = std::variant<int, double, std::string>;
using vrt_small_variant = vrt::variant<int, double, std::string>;

using std_mixed_variant = std::variant<trivial_small, non_trivial_small, medium_object>;
using vrt_mixed_variant = vrt::variant<trivial_small, non_trivial_small, medium_object>;

using std_large_variant = std::variant<int, double, std::string, medium_object, large_object>;
using vrt_large_variant = vrt::variant<int, double, std::string, medium_object, large_object>;

using std_many_variant = std::variant<
	int, double, float, char, short, long, long long,
	std::string, std::vector<int>, std::array<int, 4>
>;
using vrt_many_variant = vrt::variant<
	int, double, float, char, short, long, long long,
	std::string, std::vector<int>, std::array<int, 4>
>;

struct vvvv
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

template<typename Variant>
int switch_visit_impl(const Variant& v)
{
	if constexpr (std::is_same_v<Variant, vrt_small_variant>)
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
	else if constexpr (std::is_same_v<Variant, vrt_mixed_variant>)
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
	else if constexpr (std::is_same_v<Variant, vrt_large_variant>)
	{
		switch (v.index())
		{
			case Variant::template of<int>:
				return v.template get<int>() * 2;
			case Variant::template of<double>:
				return static_cast<int>(v.template get<double>() * 2.0);
			case Variant::template of<std::string>:
				return static_cast<int>(v.template get<std::string>().length());
			case Variant::template of<medium_object>:
				return v.template get<medium_object>().data[0] * 2;
			case Variant::template of<large_object>:
				return v.template get<large_object>().data[0] * 2;
			default:
				return 0;
		}
	}
	else if constexpr (std::is_same_v<Variant, vrt_many_variant>)
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
			case Variant::template of<std::string>:
				return static_cast<int>(v.template get<std::string>().length());
			case Variant::template of<std::vector<int>>:
				return static_cast<int>(v.template get<std::vector<int>>().size());
			case Variant::template of<std::array<int, 4>>:
				return 4;
			default:
				return 0;
		}
	}

	return 0;
}

template<typename StdVariant, typename VrtVariant>
std::pair<std::vector<StdVariant>, std::vector<VrtVariant>>
generate_variants(std::size_t count, unsigned seed = 42)
{
	std::vector<StdVariant> std_variants;
	std::vector<VrtVariant> vrt_variants;
	std_variants.reserve(count);
	vrt_variants.reserve(count);

	std::mt19937 gen(seed);

	if constexpr (std::is_same_v<StdVariant, std_small_variant>)
	{
		std::uniform_int_distribution<> dis(0, 2);
		for (std::size_t i = 0; i < count; ++i)
		{
			switch (dis(gen))
			{
				case 0:
					std_variants.emplace_back(static_cast<int>(i));
					vrt_variants.emplace_back(static_cast<int>(i));
					break;
				case 1:
					std_variants.emplace_back(static_cast<double>(i) * 1.5);
					vrt_variants.emplace_back(static_cast<double>(i) * 1.5);
					break;
				case 2:
					std_variants.emplace_back(std::string("test_") + std::to_string(i));
					vrt_variants.emplace_back(std::string("test_") + std::to_string(i));
					break;
			}
		}
	}
	else if constexpr (std::is_same_v<StdVariant, std_mixed_variant>)
	{
		std::uniform_int_distribution<> dis(0, 2);
		for (std::size_t i = 0; i < count; ++i)
		{
			switch (dis(gen))
			{
				case 0:
					std_variants.emplace_back(trivial_small{static_cast<int>(i)});
					vrt_variants.emplace_back(trivial_small{static_cast<int>(i)});
					break;
				case 1:
					std_variants.emplace_back(non_trivial_small{static_cast<int>(i)});
					vrt_variants.emplace_back(non_trivial_small{static_cast<int>(i)});
					break;
				case 2:
					std_variants.emplace_back(medium_object{static_cast<int>(i), "test"});
					vrt_variants.emplace_back(medium_object{static_cast<int>(i), "test"});
					break;
			}
		}
	}
	else if constexpr (std::is_same_v<StdVariant, std_large_variant>)
	{
		std::uniform_int_distribution<> dis(0, 4);
		for (std::size_t i = 0; i < count; ++i)
		{
			switch (dis(gen))
			{
				case 0:
					std_variants.emplace_back(static_cast<int>(i));
					vrt_variants.emplace_back(static_cast<int>(i));
					break;
				case 1:
					std_variants.emplace_back(static_cast<double>(i) * 1.5);
					vrt_variants.emplace_back(static_cast<double>(i) * 1.5);
					break;
				case 2:
					std_variants.emplace_back(std::string("test_") + std::to_string(i));
					vrt_variants.emplace_back(std::string("test_") + std::to_string(i));
					break;
				case 3:
					std_variants.emplace_back(medium_object{static_cast<int>(i), "test"});
					vrt_variants.emplace_back(medium_object{static_cast<int>(i), "test"});
					break;
				case 4:
					std_variants.emplace_back(large_object{static_cast<int>(i)});
					vrt_variants.emplace_back(large_object{static_cast<int>(i)});
					break;
			}
		}
	}
	else if constexpr (std::is_same_v<StdVariant, std_many_variant>)
	{
		std::uniform_int_distribution<> dis(0, 9);
		for (std::size_t i = 0; i < count; ++i)
		{
			switch (dis(gen))
			{
				case 0:
					std_variants.emplace_back(static_cast<int>(i));
					vrt_variants.emplace_back(static_cast<int>(i));
					break;
				case 1:
					std_variants.emplace_back(static_cast<double>(i) * 1.5);
					vrt_variants.emplace_back(static_cast<double>(i) * 1.5);
					break;
				case 2:
					std_variants.emplace_back(static_cast<float>(i) * 1.5f);
					vrt_variants.emplace_back(static_cast<float>(i) * 1.5f);
					break;
				case 3:
					std_variants.emplace_back(static_cast<char>('A' + (i % 26)));
					vrt_variants.emplace_back(static_cast<char>('A' + (i % 26)));
					break;
				case 4:
					std_variants.emplace_back(static_cast<short>(i));
					vrt_variants.emplace_back(static_cast<short>(i));
					break;
				case 5:
					std_variants.emplace_back(static_cast<long>(i));
					vrt_variants.emplace_back(static_cast<long>(i));
					break;
				case 6:
					std_variants.emplace_back(static_cast<long long>(i));
					vrt_variants.emplace_back(static_cast<long long>(i));
					break;
				case 7:
					std_variants.emplace_back(std::string("test_") + std::to_string(i));
					vrt_variants.emplace_back(std::string("test_") + std::to_string(i));
					break;
				case 8:
					std_variants.emplace_back(std::vector<int>(i % 10, static_cast<int>(i)));
					vrt_variants.emplace_back(std::vector<int>(i % 10, static_cast<int>(i)));
					break;
				case 9:
				{
					std::array<int, 4> arr;
					arr.fill(static_cast<int>(i));
					std_variants.emplace_back(arr);
					vrt_variants.emplace_back(arr);
					break;
				}
			}
		}
	}

	return {std_variants, vrt_variants};
}

template<typename StdVariant, typename VrtVariant>
static void BM_StdVisit_Pattern(benchmark::State& state)
{
	constexpr std::size_t VARIANT_COUNT = 1000;
	auto [std_variants, vrt_variants] = generate_variants<StdVariant, VrtVariant>(VARIANT_COUNT);

	for (auto _ : state)
	{
		std::size_t sum = 0;
		for (const auto& v : std_variants)
		{
			vvvv visitor;
			sum += std::visit(visitor, v);
		}
		benchmark::DoNotOptimize(sum);
	}
	state.SetItemsProcessed(state.iterations() * VARIANT_COUNT);
}

template<typename StdVariant, typename VrtVariant>
static void BM_VrtVisit_Pattern(benchmark::State& state)
{
	constexpr std::size_t VARIANT_COUNT = 1000;
	auto [std_variants, vrt_variants] = generate_variants<StdVariant, VrtVariant>(VARIANT_COUNT);
	vvvv visitor;

	for (auto _ : state)
	{
		std::size_t sum = 0;
		for (const auto& v : vrt_variants)
		{
			sum += vrt::visit(visitor, v);
		}
		benchmark::DoNotOptimize(sum);
	}
	state.SetItemsProcessed(state.iterations() * VARIANT_COUNT);
}

template<typename StdVariant, typename VrtVariant>
static void BM_VrtSwitch_Pattern(benchmark::State& state)
{
	constexpr std::size_t VARIANT_COUNT = 1000;
	auto [std_variants, vrt_variants] = generate_variants<StdVariant, VrtVariant>(VARIANT_COUNT);

	for (auto _ : state)
	{
		std::size_t sum = 0;
		for (const auto& v : vrt_variants)
		{
			sum += switch_visit_impl(v);
		}
		benchmark::DoNotOptimize(sum);
	}
	state.SetItemsProcessed(state.iterations() * VARIANT_COUNT);
}

template<typename StdVariant>
static void BM_StdVisit_Single(benchmark::State& state)
{
	StdVariant v;
	if constexpr (std::is_same_v<StdVariant, std_small_variant>)
		v = 42;
	else if constexpr (std::is_same_v<StdVariant, std_mixed_variant>)
		v = trivial_small{42};
	else if constexpr (std::is_same_v<StdVariant, std_large_variant>)
		v = 42;
	else if constexpr (std::is_same_v<StdVariant, std_many_variant>)
		v = 42;

	vvvv visitor;

	for (auto _ : state)
	{
		benchmark::DoNotOptimize(std::visit(visitor, v));
	}
}

template<typename VrtVariant>
static void BM_VrtVisit_Single(benchmark::State& state)
{
	VrtVariant v;
	if constexpr (std::is_same_v<VrtVariant, vrt_small_variant>)
		v = 42;
	else if constexpr (std::is_same_v<VrtVariant, vrt_mixed_variant>)
		v = trivial_small{42};
	else if constexpr (std::is_same_v<VrtVariant, vrt_large_variant>)
		v = 42;
	else if constexpr (std::is_same_v<VrtVariant, vrt_many_variant>)
		v = 42;

	vvvv visitor;

	for (auto _ : state)
	{
		benchmark::DoNotOptimize(vrt::visit(visitor, v));
	}
}

template<typename VrtVariant>
static void BM_VrtSwitch_Single(benchmark::State& state)
{
	VrtVariant v;
	if constexpr (std::is_same_v<VrtVariant, vrt_small_variant>)
		v = 42;
	else if constexpr (std::is_same_v<VrtVariant, vrt_mixed_variant>)
		v = trivial_small{42};
	else if constexpr (std::is_same_v<VrtVariant, vrt_large_variant>)
		v = 42;
	else if constexpr (std::is_same_v<VrtVariant, vrt_many_variant>)
		v = 42;

	for (auto _ : state)
	{
		benchmark::DoNotOptimize(switch_visit_impl(v));
	}
}

template<typename StdVariant, typename VrtVariant>
static void BM_Construction_Comparison(benchmark::State& state)
{
	for (auto _ : state)
	{
		if constexpr (std::is_same_v<StdVariant, std_small_variant>)
		{
			StdVariant std_v1{42};
			VrtVariant vrt_v1{42};
			StdVariant std_v2{3.14};
			VrtVariant vrt_v2{3.14};
			StdVariant std_v3{std::string("test")};
			VrtVariant vrt_v3{std::string("test")};
			benchmark::DoNotOptimize(std_v1);
			benchmark::DoNotOptimize(vrt_v1);
			benchmark::DoNotOptimize(std_v2);
			benchmark::DoNotOptimize(vrt_v2);
			benchmark::DoNotOptimize(std_v3);
			benchmark::DoNotOptimize(vrt_v3);
		}
		else if constexpr (std::is_same_v<StdVariant, std_mixed_variant>)
		{
			StdVariant std_v1{trivial_small{42}};
			VrtVariant vrt_v1{trivial_small{42}};
			StdVariant std_v2{non_trivial_small{123}};
			VrtVariant vrt_v2{non_trivial_small{123}};
			StdVariant std_v3{medium_object{456, "test"}};
			VrtVariant vrt_v3{medium_object{456, "test"}};
			benchmark::DoNotOptimize(std_v1);
			benchmark::DoNotOptimize(vrt_v1);
			benchmark::DoNotOptimize(std_v2);
			benchmark::DoNotOptimize(vrt_v2);
			benchmark::DoNotOptimize(std_v3);
			benchmark::DoNotOptimize(vrt_v3);
		}
		else if constexpr (std::is_same_v<StdVariant, std_large_variant>)
		{
			StdVariant std_v1{42};
			VrtVariant vrt_v1{42};
			StdVariant std_v2{3.14};
			VrtVariant vrt_v2{3.14};
			StdVariant std_v3{std::string("test")};
			VrtVariant vrt_v3{std::string("test")};
			StdVariant std_v4{medium_object{456, "test"}};
			VrtVariant vrt_v4{medium_object{456, "test"}};
			StdVariant std_v5{large_object{789}};
			VrtVariant vrt_v5{large_object{789}};
			benchmark::DoNotOptimize(std_v1);
			benchmark::DoNotOptimize(vrt_v1);
			benchmark::DoNotOptimize(std_v2);
			benchmark::DoNotOptimize(vrt_v2);
			benchmark::DoNotOptimize(std_v3);
			benchmark::DoNotOptimize(vrt_v3);
			benchmark::DoNotOptimize(std_v4);
			benchmark::DoNotOptimize(vrt_v4);
			benchmark::DoNotOptimize(std_v5);
			benchmark::DoNotOptimize(vrt_v5);
		}
		else if constexpr (std::is_same_v<StdVariant, std_many_variant>)
		{
			StdVariant std_v1{42};
			VrtVariant vrt_v1{42};
			StdVariant std_v2{3.14};
			VrtVariant vrt_v2{3.14};
			StdVariant std_v3{std::string("test")};
			VrtVariant vrt_v3{std::string("test")};
			StdVariant std_v4{std::vector<int>{1, 2, 3}};
			VrtVariant vrt_v4{std::vector<int>{1, 2, 3}};
			benchmark::DoNotOptimize(std_v1);
			benchmark::DoNotOptimize(vrt_v1);
			benchmark::DoNotOptimize(std_v2);
			benchmark::DoNotOptimize(vrt_v2);
			benchmark::DoNotOptimize(std_v3);
			benchmark::DoNotOptimize(vrt_v3);
			benchmark::DoNotOptimize(std_v4);
			benchmark::DoNotOptimize(vrt_v4);
		}
	}
}

template<typename StdVariant, typename VrtVariant>
static void BM_Assignment_Comparison(benchmark::State& state)
{
	StdVariant std_v;
	VrtVariant vrt_v;

	for (auto _ : state)
	{
		if constexpr (std::is_same_v<StdVariant, std_small_variant>)
		{
			std_v = 42;
			vrt_v = 42;
			benchmark::DoNotOptimize(std_v);
			benchmark::DoNotOptimize(vrt_v);

			std_v = 3.14;
			vrt_v = 3.14;
			benchmark::DoNotOptimize(std_v);
			benchmark::DoNotOptimize(vrt_v);

			std_v = std::string("test");
			vrt_v = std::string("test");
			benchmark::DoNotOptimize(std_v);
			benchmark::DoNotOptimize(vrt_v);
		}
		else if constexpr (std::is_same_v<StdVariant, std_mixed_variant>)
		{
			std_v = trivial_small{42};
			vrt_v = trivial_small{42};
			benchmark::DoNotOptimize(std_v);
			benchmark::DoNotOptimize(vrt_v);

			std_v = non_trivial_small{123};
			vrt_v = non_trivial_small{123};
			benchmark::DoNotOptimize(std_v);
			benchmark::DoNotOptimize(vrt_v);

			std_v = medium_object{456, "test"};
			vrt_v = medium_object{456, "test"};
			benchmark::DoNotOptimize(std_v);
			benchmark::DoNotOptimize(vrt_v);
		}
		else if constexpr (std::is_same_v<StdVariant, std_large_variant>)
		{
			std_v = 42;
			vrt_v = 42;
			benchmark::DoNotOptimize(std_v);
			benchmark::DoNotOptimize(vrt_v);

			std_v = 3.14;
			vrt_v = 3.14;
			benchmark::DoNotOptimize(std_v);
			benchmark::DoNotOptimize(vrt_v);

			std_v = std::string("test");
			vrt_v = std::string("test");
			benchmark::DoNotOptimize(std_v);
			benchmark::DoNotOptimize(vrt_v);

			std_v = medium_object{456, "test"};
			vrt_v = medium_object{456, "test"};
			benchmark::DoNotOptimize(std_v);
			benchmark::DoNotOptimize(vrt_v);
		}
		else if constexpr (std::is_same_v<StdVariant, std_many_variant>)
		{
			std_v = 42;
			vrt_v = 42;
			benchmark::DoNotOptimize(std_v);
			benchmark::DoNotOptimize(vrt_v);

			std_v = 3.14;
			vrt_v = 3.14;
			benchmark::DoNotOptimize(std_v);
			benchmark::DoNotOptimize(vrt_v);

			std_v = std::string("test");
			vrt_v = std::string("test");
			benchmark::DoNotOptimize(std_v);
			benchmark::DoNotOptimize(vrt_v);
		}
	}
}

template<typename StdVariant, typename VrtVariant>
static void BM_CacheEfficiency(benchmark::State& state)
{
	constexpr std::size_t LARGE_COUNT = 10000;
	auto [std_variants, vrt_variants] = generate_variants<StdVariant, VrtVariant>(LARGE_COUNT);
	vvvv visitor;

	for (auto _ : state)
	{
		std::size_t std_sum = 0, vrt_sum = 0;
		for (std::size_t i = 0; i < LARGE_COUNT; i += 17)
		{
			std_sum += std::visit(visitor, std_variants[i]);
			vrt_sum += switch_visit_impl(vrt_variants[i]);
		}
		benchmark::DoNotOptimize(std_sum);
		benchmark::DoNotOptimize(vrt_sum);
	}
}

#define REGISTER_COMPARISON_BENCHMARKS(name, std_type, vrt_type) \
	BENCHMARK(BM_StdVisit_Pattern<std_type, vrt_type>)->Name("std::visit/" name); \
	BENCHMARK(BM_VrtVisit_Pattern<std_type, vrt_type>)->Name("vrt::visit/" name); \
	BENCHMARK(BM_VrtSwitch_Pattern<std_type, vrt_type>)->Name("vrt::switch/" name); \
	BENCHMARK(BM_StdVisit_Single<std_type>)->Name("std::visit/" name "/Single"); \
	BENCHMARK(BM_VrtVisit_Single<vrt_type>)->Name("vrt::visit/" name "/Single"); \
	BENCHMARK(BM_VrtSwitch_Single<vrt_type>)->Name("vrt::switch/" name "/Single"); \
	BENCHMARK(BM_Construction_Comparison<std_type, vrt_type>)->Name("Construction/" name); \
	BENCHMARK(BM_Assignment_Comparison<std_type, vrt_type>)->Name("Assignment/" name); \
	BENCHMARK(BM_CacheEfficiency<std_type, vrt_type>)->Name("CacheEfficiency/" name);

REGISTER_COMPARISON_BENCHMARKS("SmallTypes", std_small_variant, vrt_small_variant)
REGISTER_COMPARISON_BENCHMARKS("MixedTypes", std_mixed_variant, vrt_mixed_variant)
REGISTER_COMPARISON_BENCHMARKS("LargeTypes", std_large_variant, vrt_large_variant)
REGISTER_COMPARISON_BENCHMARKS("ManyTypes", std_many_variant, vrt_many_variant)

BENCHMARK_MAIN();
