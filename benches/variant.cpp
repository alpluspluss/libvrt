#include <random>
#include <string>
#include <variant>
#include <vector>
#include <vrt>
#include <benchmark/benchmark.h>

struct small_type
{
	int value;
};

struct medium_type
{
	char data[64];
	int value;
};

struct large_type
{
	char data[1024];
	int value;
};

using std_variant_small = std::variant<int, double, std::string>;
using vrt_variant_small = vrt::variant<int, double, std::string>;

using std_variant_mixed = std::variant<small_type, medium_type, large_type>;
using vrt_variant_mixed = vrt::variant<small_type, medium_type, large_type>;

using std_variant_many = std::variant<int, double, float, char, short, long, std::string, std::vector<int> >;
using vrt_variant_many = vrt::variant<int, double, float, char, short, long, std::string, std::vector<int> >;

struct visitor
{
	int operator()(int v) const
	{
		return v * 2;
	}

	int operator()(double v) const
	{
		return static_cast<int>(v * 2.0);
	}

	int operator()(const std::string &v) const
	{
		return static_cast<int>(v.length());
	}

	int operator()(float v) const
	{
		return static_cast<int>(v * 2.0f);
	}

	int operator()(char v) const
	{
		return static_cast<int>(v) * 2;
	}

	int operator()(short v) const
	{
		return static_cast<int>(v) * 2;
	}

	int operator()(long v) const
	{
		return static_cast<int>(v) * 2;
	}

	int operator()(const std::vector<int> &v) const
	{
		return static_cast<int>(v.size());
	}

	int operator()(const small_type &v) const
	{
		return v.value * 2;
	}

	int operator()(const medium_type &v) const
	{
		return v.value * 2;
	}

	int operator()(const large_type &v) const
	{
		return v.value * 2;
	}
};

template<typename Variant>
int switch_visit(const Variant &v)
{
	if constexpr (std::is_same_v<Variant, vrt_variant_small>)
	{
		switch (v)
		{
			case Variant::template index_of<int>:
				return v.template get<int>() * 2;
			case Variant::template index_of<double>:
				return static_cast<int>(v.template get<double>() * 2.0);
			case Variant::template index_of<std::string>:
				return static_cast<int>(v.template get<std::string>().length());
		}
	}
	else if constexpr (std::is_same_v<Variant, vrt_variant_mixed>)
	{
		switch (v)
		{
			case Variant::template index_of<small_type>:
				return v.template get<small_type>().value * 2;
			case Variant::template index_of<medium_type>:
				return v.template get<medium_type>().value * 2;
			case Variant::template index_of<large_type>:
				return v.template get<large_type>().value * 2;
		}
	}
	else if constexpr (std::is_same_v<Variant, vrt_variant_many>)
	{
		switch (v)
		{
			case Variant::template index_of<int>:
				return v.template get<int>() * 2;
			case Variant::template index_of<double>:
				return static_cast<int>(v.template get<double>() * 2.0);
			case Variant::template index_of<float>:
				return static_cast<int>(v.template get<float>() * 2.0f);
			case Variant::template index_of<char>:
				return static_cast<int>(v.template get<char>()) * 2;
			case Variant::template index_of<short>:
				return static_cast<int>(v.template get<short>()) * 2;
			case Variant::template index_of<long>:
				return static_cast<int>(v.template get<long>()) * 2;
			case Variant::template index_of<std::string>:
				return static_cast<int>(v.template get<std::string>().length());
			case Variant::template index_of<std::vector<int> >:
				return static_cast<int>(v.template get<std::vector<int> >().size());
		}
	}
	return 0;
}

template<typename StdVariant, typename VrtVariant>
std::pair<std::vector<StdVariant>, std::vector<VrtVariant> > generate_test_data(size_t count)
{
	std::vector<StdVariant> std_variants;
	std::vector<VrtVariant> vrt_variants;
	std_variants.reserve(count);
	vrt_variants.reserve(count);

	std::random_device rd;
	std::mt19937 gen(rd());

	if constexpr (std::is_same_v<StdVariant, std_variant_small>)
	{
		std::uniform_int_distribution<> dis(0, 2);
		for (size_t i = 0; i < count; ++i)
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
	else if constexpr (std::is_same_v<StdVariant, std_variant_mixed>)
	{
		std::uniform_int_distribution<> dis(0, 2);
		for (size_t i = 0; i < count; ++i)
		{
			switch (dis(gen))
			{
				case 0:
					std_variants.emplace_back(small_type { static_cast<int>(i) });
					vrt_variants.emplace_back(small_type { static_cast<int>(i) });
					break;
				case 1:
					std_variants.emplace_back(medium_type { {}, static_cast<int>(i) });
					vrt_variants.emplace_back(medium_type { {}, static_cast<int>(i) });
					break;
				case 2:
					std_variants.emplace_back(large_type { {}, static_cast<int>(i) });
					vrt_variants.emplace_back(large_type { {}, static_cast<int>(i) });
					break;
			}
		}
	}
	else if constexpr (std::is_same_v<StdVariant, std_variant_many>)
	{
		std::uniform_int_distribution<> dis(0, 7);
		for (size_t i = 0; i < count; ++i)
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
					std_variants.emplace_back(std::string("test_") + std::to_string(i));
					vrt_variants.emplace_back(std::string("test_") + std::to_string(i));
					break;
				case 7:
					std_variants.emplace_back(std::vector<int>(i % 10, static_cast<int>(i)));
					vrt_variants.emplace_back(std::vector<int>(i % 10, static_cast<int>(i)));
					break;
			}
		}
	}

	return { std_variants, vrt_variants };
}

static void BM_StdVisit_Small(benchmark::State &state)
{
	auto [std_variants, vrt_variants] = generate_test_data<std_variant_small, vrt_variant_small>(1000);
	visitor vis;

	for (auto _: state)
	{
		for (const auto &v: std_variants)
		{
			benchmark::DoNotOptimize(std::visit(vis, v));
		}
	}
	state.SetItemsProcessed(state.iterations() * std_variants.size());
}

static void BM_VrtSwitch_Small(benchmark::State &state)
{
	auto [std_variants, vrt_variants] = generate_test_data<std_variant_small, vrt_variant_small>(1000);

	for (auto _: state)
	{
		for (const auto &v: vrt_variants)
		{
			benchmark::DoNotOptimize(switch_visit(v));
		}
	}
	state.SetItemsProcessed(state.iterations() * vrt_variants.size());
}

static void BM_StdVisit_Mixed(benchmark::State &state)
{
	auto [std_variants, vrt_variants] = generate_test_data<std_variant_mixed, vrt_variant_mixed>(1000);
	visitor vis;

	for (auto _: state)
	{
		for (const auto &v: std_variants)
		{
			benchmark::DoNotOptimize(std::visit(vis, v));
		}
	}
	state.SetItemsProcessed(state.iterations() * std_variants.size());
}

static void BM_VrtSwitch_Mixed(benchmark::State &state)
{
	auto [std_variants, vrt_variants] = generate_test_data<std_variant_mixed, vrt_variant_mixed>(1000);

	for (auto _: state)
	{
		for (const auto &v: vrt_variants)
		{
			benchmark::DoNotOptimize(switch_visit(v));
		}
	}
	state.SetItemsProcessed(state.iterations() * vrt_variants.size());
}

static void BM_StdVisit_Many(benchmark::State &state)
{
	auto [std_variants, vrt_variants] = generate_test_data<std_variant_many, vrt_variant_many>(1000);
	visitor vis;

	for (auto _: state)
	{
		for (const auto &v: std_variants)
		{
			benchmark::DoNotOptimize(std::visit(vis, v));
		}
	}
	state.SetItemsProcessed(state.iterations() * std_variants.size());
}

static void BM_VrtSwitch_Many(benchmark::State &state)
{
	auto [std_variants, vrt_variants] = generate_test_data<std_variant_many, vrt_variant_many>(1000);

	for (auto _: state)
	{
		for (const auto &v: vrt_variants)
		{
			benchmark::DoNotOptimize(switch_visit(v));
		}
	}
	state.SetItemsProcessed(state.iterations() * vrt_variants.size());
}

static void BM_StdVisit_Single(benchmark::State &state)
{
	std_variant_small v = 42;
	visitor vis;

	for (auto _: state)
	{
		benchmark::DoNotOptimize(std::visit(vis, v));
	}
}

static void BM_VrtSwitch_Single(benchmark::State &state)
{
	vrt_variant_small v = 42;

	for (auto _: state)
	{
		benchmark::DoNotOptimize(switch_visit(v));
	}
}

static void BM_StdConstruction(benchmark::State &state)
{
	for (auto _: state)
	{
		std_variant_small v1 { 42 };
		std_variant_small v2 { 3.14 };
		std_variant_small v3 { std::string("test") };
		benchmark::DoNotOptimize(v1);
		benchmark::DoNotOptimize(v2);
		benchmark::DoNotOptimize(v3);
	}
}

static void BM_VrtConstruction(benchmark::State &state)
{
	for (auto _: state)
	{
		vrt_variant_small v1 { 42 };
		vrt_variant_small v2 { 3.14 };
		vrt_variant_small v3 { std::string("test") };
		benchmark::DoNotOptimize(v1);
		benchmark::DoNotOptimize(v2);
		benchmark::DoNotOptimize(v3);
	}
}

// Benchmark: Assignment performance
static void BM_StdAssignment(benchmark::State &state)
{
	std_variant_small v;

	for (auto _: state)
	{
		v = 42;
		benchmark::DoNotOptimize(v);
		v = 3.14;
		benchmark::DoNotOptimize(v);
		v = std::string("test");
		benchmark::DoNotOptimize(v);
	}
}

static void BM_VrtAssignment(benchmark::State &state)
{
	vrt_variant_small v;

	for (auto _: state)
	{
		v = 42;
		benchmark::DoNotOptimize(v);
		v = 3.14;
		benchmark::DoNotOptimize(v);
		v = std::string("test");
		benchmark::DoNotOptimize(v);
	}
}

static void BM_StdLargeObjects(benchmark::State &state)
{
	std::vector<std_variant_mixed> variants;
	variants.reserve(100);

	for (auto _: state)
	{
		variants.clear();
		for (int i = 0; i < 100; ++i)
		{
			variants.emplace_back(large_type { {}, i });
		}
		benchmark::DoNotOptimize(variants);
	}
}

static void BM_VrtLargeObjects(benchmark::State &state)
{
	std::vector<vrt_variant_mixed> variants;
	variants.reserve(100);

	for (auto _: state)
	{
		variants.clear();
		for (int i = 0; i < 100; ++i)
		{
			variants.emplace_back(large_type { {}, i });
		}
		benchmark::DoNotOptimize(variants);
	}
}

BENCHMARK(BM_StdVisit_Small)->Name("std::visit/Small");
BENCHMARK(BM_VrtSwitch_Small)->Name("vrt::switch/Small");

BENCHMARK(BM_StdVisit_Mixed)->Name("std::visit/Mixed");
BENCHMARK(BM_VrtSwitch_Mixed)->Name("vrt::switch/Mixed");

BENCHMARK(BM_StdVisit_Many)->Name("std::visit/Many");
BENCHMARK(BM_VrtSwitch_Many)->Name("vrt::switch/Many");

BENCHMARK(BM_StdVisit_Single)->Name("std::visit/Single");
BENCHMARK(BM_VrtSwitch_Single)->Name("vrt::switch/Single");

BENCHMARK(BM_StdConstruction)->Name("std::variant/Construction");
BENCHMARK(BM_VrtConstruction)->Name("vrt::variant/Construction");

BENCHMARK(BM_StdAssignment)->Name("std::variant/Assignment");
BENCHMARK(BM_VrtAssignment)->Name("vrt::variant/Assignment");

BENCHMARK(BM_StdLargeObjects)->Name("std::variant/LargeObjects");
BENCHMARK(BM_VrtLargeObjects)->Name("vrt::variant/LargeObjects");

BENCHMARK_MAIN();
