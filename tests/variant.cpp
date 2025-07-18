/* this project is part of the vrt project; licensed under the MIT license. see LICENSE for more info */

#include <string>
#include <vector>
#include <vrt>
#include <gtest/gtest.h>

class VariantTest : public ::testing::Test
{
protected:
	using test_variant = vrt::variant<int, double, std::string>;
};

TEST_F(VariantTest, DefaultConstruction)
{
	test_variant v;

	EXPECT_EQ(v.index(), 0);
	EXPECT_TRUE(vrt::holds_alternative<int>(v));
	EXPECT_EQ(v.get<int>(), 0);
	EXPECT_FALSE(v.valueless_by_exception());
}

TEST_F(VariantTest, ConvertingConstruction)
{
	test_variant v1 { 42 };
	EXPECT_EQ(v1.index(), 0);
	EXPECT_EQ(vrt::get<int>(v1), 42);

	test_variant v2 { 3.14 };
	EXPECT_EQ(v2.index(), 1);
	EXPECT_DOUBLE_EQ(vrt::get<double>(v2), 3.14);

	test_variant v3 { std::string("hello") };
	EXPECT_EQ(v3.index(), 2);
	EXPECT_EQ(vrt::get<std::string>(v3), "hello");
}

TEST_F(VariantTest, Assignment)
{
	test_variant v;

	v = 42;
	EXPECT_EQ(v.index(), 0);
	EXPECT_EQ(vrt::get<int>(v), 42);

	v = 3.14;
	EXPECT_EQ(v.index(), 1);
	EXPECT_DOUBLE_EQ(vrt::get<double>(v), 3.14);

	v = std::string("world");
	EXPECT_EQ(v.index(), 2);
	EXPECT_EQ(vrt::get<std::string>(v), "world");
}

TEST_F(VariantTest, SwitchSupport)
{
	auto test_switch = [](const test_variant &v) -> std::string
	{
		switch (v.index())
		{
			case test_variant::of<int>:
				return "int: " + std::to_string(v.get<int>());
			case test_variant::of<double>:
				return "double: " + std::to_string(v.get<double>());
			case test_variant::of<std::string>:
				return "string: " + v.get<std::string>();
			default:
				return "unknown";
		}
	};

	EXPECT_EQ(test_switch(test_variant{42}), "int: 42");
	EXPECT_EQ(test_switch(test_variant{3.14}), "double: 3.140000");
	EXPECT_EQ(test_switch(test_variant{std::string("test")}), "string: test");
}

TEST_F(VariantTest, IndexOfConstants)
{
	EXPECT_EQ(test_variant::of<int>, 0);
	EXPECT_EQ(test_variant::of<double>, 1);
	EXPECT_EQ(test_variant::of<std::string>, 2);
}

TEST_F(VariantTest, CopyConstruction)
{
	test_variant v1 { std::string("original") };
	test_variant v2 { v1 };

	EXPECT_EQ(vrt::get<std::string>(v2), "original");
	EXPECT_EQ(vrt::get<std::string>(v1), "original");
	EXPECT_EQ(v1.index(), v2.index());
}

TEST_F(VariantTest, MoveConstruction)
{
	test_variant v1 { std::string("original") };
	test_variant v2 { std::move(v1) };

	EXPECT_EQ(vrt::get<std::string>(v2), "original");
	EXPECT_TRUE(v1.valueless_by_exception());
}

TEST_F(VariantTest, CopyAssignment)
{
	test_variant v1 { std::string("original") };
	test_variant v2 { 42 };

	v2 = v1;
	EXPECT_EQ(vrt::get<std::string>(v2), "original");
	EXPECT_EQ(vrt::get<std::string>(v1), "original");
}

TEST_F(VariantTest, MoveAssignment)
{
	test_variant v1 { std::string("original") };
	test_variant v2 { 42 };

	v2 = std::move(v1);
	EXPECT_EQ(vrt::get<std::string>(v2), "original");
	EXPECT_TRUE(v1.valueless_by_exception());
}

TEST_F(VariantTest, SelfAssignment)
{
	test_variant v(std::string("test"));
	v = v;
	EXPECT_EQ(vrt::get<std::string>(v), "test");
	v = std::move(v);
}

TEST_F(VariantTest, EmplaceByType)
{
	vrt::variant<int, std::string, std::vector<int> > v;

	auto &str_ref = v.emplace<std::string>("constructed");
	EXPECT_EQ(vrt::get<std::string>(v), "constructed");
	EXPECT_EQ(&str_ref, &vrt::get<std::string>(v));
}

TEST_F(VariantTest, EmplaceByIndex)
{
	vrt::variant<int, std::string, std::vector<int> > v;

	auto &vec_ref = v.emplace<2>(3, 42);
	EXPECT_EQ(vrt::get<std::vector<int>>(v).size(), 3);
	EXPECT_EQ(vrt::get<std::vector<int>>(v)[0], 42);
	EXPECT_EQ(&vec_ref, &vrt::get<std::vector<int>>(v));
}

TEST_F(VariantTest, GetIfPointer)
{
	test_variant v { 42 };

	auto *int_ptr = vrt::get_if<int>(&v);
	ASSERT_NE(int_ptr, nullptr);
	EXPECT_EQ(*int_ptr, 42);

	auto *str_ptr = vrt::get_if<std::string>(&v);
	EXPECT_EQ(str_ptr, nullptr);

	const auto &cv = v;
	const auto *const_int_ptr = vrt::get_if<int>(&cv);
	ASSERT_NE(const_int_ptr, nullptr);
	EXPECT_EQ(*const_int_ptr, 42);
}

TEST_F(VariantTest, GetIfByIndex)
{
	test_variant v { 42 };

	auto *int_ptr = vrt::get_if<0>(&v);
	ASSERT_NE(int_ptr, nullptr);
	EXPECT_EQ(*int_ptr, 42);

	auto *double_ptr = vrt::get_if<1>(&v);
	EXPECT_EQ(double_ptr, nullptr);
}

TEST_F(VariantTest, GetIfNullptr)
{
	test_variant *null_variant = nullptr;
	EXPECT_EQ(vrt::get_if<int>(null_variant), nullptr);
}

TEST_F(VariantTest, GetThrowsOnWrongType)
{
	test_variant v { 42 };

	EXPECT_THROW(vrt::get<std::string>(v), std::bad_variant_access);
	EXPECT_THROW(v.get<std::string>(), std::bad_variant_access);
}

TEST_F(VariantTest, GetByIndex)
{
	test_variant v { 42 };

	EXPECT_EQ(vrt::get<0>(v), 42);
	EXPECT_THROW(vrt::get<1>(v), std::bad_variant_access);
}

TEST_F(VariantTest, HoldsAlternative)
{
	test_variant v { 42 };

	EXPECT_TRUE(vrt::holds_alternative<int>(v));
	EXPECT_FALSE(vrt::holds_alternative<double>(v));
	EXPECT_FALSE(vrt::holds_alternative<std::string>(v));

	EXPECT_TRUE(v.holds_alternative<int>());
	EXPECT_FALSE(v.holds_alternative<double>());
}

TEST_F(VariantTest, EqualityComparison)
{
	test_variant v1 { 42 };
	test_variant v2 { 42 };
	test_variant v3 { 43 };
	test_variant v4 { std::string("hello") };

	EXPECT_EQ(v1, v2);
	EXPECT_NE(v1, v3);
	EXPECT_NE(v1, v4);
	EXPECT_NE(v3, v4);
}

TEST_F(VariantTest, InequalityComparison)
{
	test_variant v1 { 42 };
	test_variant v2 { 43 };

	EXPECT_NE(v1, v2);
	EXPECT_FALSE(v1 != v1);
}

TEST_F(VariantTest, ThreeWayComparison)
{
	test_variant v1 { 42 };
	test_variant v2 { 42 };
	test_variant v3 { 43 };

	auto cmp1 = v1 <=> v2;
	auto cmp2 = v1 <=> v3;
	auto cmp3 = v3 <=> v1;

	EXPECT_TRUE(cmp1 == 0);
	EXPECT_TRUE(cmp2 < 0);
	EXPECT_TRUE(cmp3 > 0);
}

TEST_F(VariantTest, ComparisonWithDifferentTypes)
{
	test_variant v1 { 42 };
	test_variant v2 { std::string("a") };

	EXPECT_LT(v1, v2);
}

TEST_F(VariantTest, Swap)
{
	test_variant v1 { 42 };
	test_variant v2 { std::string("hello") };

	v1.swap(v2);
	EXPECT_EQ(vrt::get<std::string>(v1), "hello");
	EXPECT_EQ(vrt::get<int>(v2), 42);
}

TEST_F(VariantTest, ADLSwap)
{
	test_variant v1 { 42 };
	test_variant v2 { std::string("hello") };

	using vrt::swap;
	swap(v1, v2);
	EXPECT_EQ(vrt::get<std::string>(v1), "hello");
	EXPECT_EQ(vrt::get<int>(v2), 42);
}

TEST_F(VariantTest, TypeTraits)
{
	EXPECT_EQ(vrt::variant_size_v<test_variant>, 3);

	static_assert(std::is_same_v<vrt::variant_alternative_t<0, test_variant>, int>);
	static_assert(std::is_same_v<vrt::variant_alternative_t<1, test_variant>, double>);
	static_assert(std::is_same_v<vrt::variant_alternative_t<2, test_variant>, std::string>);
}

TEST_F(VariantTest, RValueGetOverloads)
{
	auto get_rvalue_string = []() -> test_variant
	{
		return test_variant { std::string("rvalue") };
	};

	std::string moved = vrt::get<std::string>(get_rvalue_string());
	EXPECT_EQ(moved, "rvalue");

	std::string moved2 = vrt::get<2>(get_rvalue_string());
	EXPECT_EQ(moved2, "rvalue");
}

TEST_F(VariantTest, SmallObjectsUseStackStorage)
{
	vrt::variant<int, double> small_variant { 42 };

	EXPECT_LE(sizeof(small_variant), 64);
}

TEST_F(VariantTest, LargeObjectsUseHeapStorage)
{
	struct large_type
	{
		char data[1000];
	};
	vrt::variant<int, large_type> large_variant { 42 };

	EXPECT_LE(sizeof(large_variant), 64);
}

TEST_F(VariantTest, MixedSizeVariant)
{
	struct medium_type
	{
		char data[16];
	};
	struct huge_type
	{
		char data[10000];
	};

	vrt::variant<int, medium_type, huge_type> mixed_variant { 42 };

	EXPECT_LE(sizeof(mixed_variant), 64);

	mixed_variant = medium_type {};
	EXPECT_EQ(mixed_variant.index(), 1);

	mixed_variant = huge_type {};
	EXPECT_EQ(mixed_variant.index(), 2);
}

TEST_F(VariantTest, EmptyStateAfterMove)
{
	vrt::variant<int, std::string> v { std::string("test") };
	auto v2 = std::move(v);

	EXPECT_TRUE(v.valueless_by_exception());
	EXPECT_EQ(v.index(), vrt::variant_npos);
	EXPECT_FALSE(v2.valueless_by_exception());
}

TEST_F(VariantTest, SameTypeMultipleTimes)
{
	vrt::variant<int, int, double> v { 42 };
	EXPECT_EQ(v.index(), 0);

	EXPECT_EQ(decltype(v)::of<int>, 0);
	EXPECT_EQ(decltype(v)::of<double>, 2);
}

TEST_F(VariantTest, VisitFunction)
{
	test_variant v = 42;

	auto visitor = [](auto &&arg) -> std::string
	{
		using T = std::decay_t<decltype(arg)>;
		if constexpr (std::is_same_v<T, int>)
			return "int: " + std::to_string(arg);
		else if constexpr (std::is_same_v<T, double>)
			return "double: " + std::to_string(arg);
		else if constexpr (std::is_same_v<T, std::string>)
			return "string: " + arg;
	};

	EXPECT_EQ(vrt::visit(visitor, v), "int: 42");

	v = 3.14;
	EXPECT_EQ(vrt::visit(visitor, v), "double: 3.140000");

	v = std::string("test");
	EXPECT_EQ(vrt::visit(visitor, v), "string: test");
}

TEST_F(VariantTest, VisitWithVoidReturn)
{
	test_variant v = 42;
	std::string result;

	vrt::visit([&]<typename T0>([[maybe_unused]] T0 &&arg)
	{
		using T = std::decay_t<T0>;
		if constexpr (std::is_same_v<T, int>)
			result = "visited int";
		else if constexpr (std::is_same_v<T, double>)
			result = "visited double";
		else if constexpr (std::is_same_v<T, std::string>)
			result = "visited string";
	}, v);

	EXPECT_EQ(result, "visited int");
}

TEST_F(VariantTest, VisitWithMutableLambda)
{
	test_variant v = 42;

	auto counter = 0;
	auto counting_visitor = [counter](auto &&arg) mutable -> int
	{
		++counter;
		return counter;
	};

	EXPECT_EQ(vrt::visit(counting_visitor, v), 1);
	EXPECT_EQ(vrt::visit(counting_visitor, v), 2);
}

TEST_F(VariantTest, VisitThrowsOnValueless)
{
	test_variant v = 42;
	auto v2 = std::move(v);

	EXPECT_THROW(vrt::visit([](auto&&) { return 0; }, v), std::bad_variant_access);
}

TEST_F(VariantTest, LargeVariantWithManyTypes)
{
	using big_variant = vrt::variant<
		int, double, float, char, short, long,
		std::string, std::vector<int>, bool,
		std::unique_ptr<int>
	>;

	big_variant v = 42;
	EXPECT_EQ(v.index(), big_variant::of<int>);

	v = std::make_unique<int>(123);
	EXPECT_EQ(v.index(), big_variant::of<std::unique_ptr<int>>);
	EXPECT_EQ(*vrt::get<std::unique_ptr<int>>(v), 123);
}

TEST_F(VariantTest, MoveOnlyTypes)
{
	using move_only_variant = vrt::variant<std::unique_ptr<int>, std::string>;

	move_only_variant v = std::make_unique<int>(42);
	EXPECT_EQ(*vrt::get<std::unique_ptr<int>>(v), 42);

	auto v2 = std::move(v);
	EXPECT_EQ(*vrt::get<std::unique_ptr<int>>(v2), 42);
	EXPECT_TRUE(v.valueless_by_exception());

	v2 = std::string("hello");
	switch (v2.index())
	{
		case move_only_variant::of<std::unique_ptr<int> >:
			FAIL() << "Should not be unique_ptr";
			break;
		case move_only_variant::of<std::string>:
			EXPECT_EQ(vrt::get<std::string>(v2), "hello");
			break;
	}
}

TEST_F(VariantTest, NonDefaultConstructibleFirstType)
{
	struct non_default
	{
		int value;

		non_default() = delete;

		explicit non_default(int v) : value(v) {}
	};

	using variant_t = vrt::variant<non_default, int>;

	variant_t v { non_default { 42 } };
	EXPECT_EQ(vrt::get<non_default>(v).value, 42);

	v = 123;
	EXPECT_EQ(vrt::get<int>(v), 123);
}

TEST_F(VariantTest, PerfectForwarding)
{
	struct move_tracker
	{
		bool was_moved_from = false;
		bool was_moved_to = false;

		move_tracker() = default;

		move_tracker(const move_tracker &) = default;

		move_tracker(move_tracker &&other) : was_moved_to(true)
		{
			other.was_moved_from = true;
		}
	};

	using variant_t = vrt::variant<move_tracker, int>;

	move_tracker tracker;
	variant_t v { std::move(tracker) };

	EXPECT_TRUE(tracker.was_moved_from);
	EXPECT_TRUE(vrt::get<move_tracker>(v).was_moved_to);
}

TEST_F(VariantTest, ComplexSwitchPatterns)
{
	using complex_variant = vrt::variant<
		int, double, std::string,
		std::vector<int>, std::unique_ptr<int>
	>;

	auto process = [](const complex_variant &v) -> std::string
	{
		switch (v.index())
		{
			case complex_variant::of<int>:
				return "number: " + std::to_string(vrt::get<int>(v));
			case complex_variant::of<double>:
				return "decimal: " + std::to_string(vrt::get<double>(v));
			case complex_variant::of<std::string>:
				return "text: " + vrt::get<std::string>(v);
			case complex_variant::of<std::vector<int> >:
			{
				const auto &vec = vrt::get<std::vector<int> >(v);
				return "vector of size: " + std::to_string(vec.size());
			}
			case complex_variant::of<std::unique_ptr<int> >:
			{
				const auto &ptr = vrt::get<std::unique_ptr<int> >(v);
				return ptr ? "pointer: " + std::to_string(*ptr) : "null pointer";
			}
			default:
				return "unknown";
		}
	};

	EXPECT_EQ(process(complex_variant{42}), "number: 42");
	EXPECT_EQ(process(complex_variant{3.14}), "decimal: 3.140000");
	EXPECT_EQ(process(complex_variant{std::string("test")}), "text: test");
	EXPECT_EQ(process(complex_variant{std::vector<int>{1,2,3}}), "vector of size: 3");
	EXPECT_EQ(process(complex_variant{std::make_unique<int>(99)}), "pointer: 99");
}

TEST_F(VariantTest, NestedVariants)
{
	using inner_variant = vrt::variant<int, std::string>;
	using outer_variant = vrt::variant<inner_variant, double>;

	outer_variant v { inner_variant { 42 } };

	switch (v.index())
	{
		case outer_variant::of<inner_variant>:
		{
			const auto &inner = vrt::get<inner_variant>(v);
			switch (inner.index())
			{
				case inner_variant::of<int>:
					EXPECT_EQ(vrt::get<int>(inner), 42);
					break;
				case inner_variant::of<std::string>:
					FAIL() << "Should be int";
					break;
			}
			break;
		}
		case outer_variant::of<double>:
			FAIL() << "Should be inner_variant";
			break;
		default: __builtin_unreachable();
	}
}

int main(int argc, char **argv)
{
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
