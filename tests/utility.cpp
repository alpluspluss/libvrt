/* this project is part of the vrt project; licensed under the MIT license. see LICENSE for more info */

#include <memory>
#include <string>
#include <vector>
#include <vrt>
#include <gtest/gtest.h>

class UtilityTest : public ::testing::Test
{
protected:
	using test_variant = vrt::variant<int, double, std::string>;
};

TEST_F(UtilityTest, EmplaceByType)
{
	vrt::variant<int, std::string, std::vector<int> > v;

	auto &str_ref = v.emplace<std::string>("constructed");

	EXPECT_EQ(v.index(), 1);
	EXPECT_EQ(vrt::get<std::string>(v), "constructed");
	EXPECT_EQ(&str_ref, &vrt::get<std::string>(v));
	EXPECT_FALSE(v.valueless_by_exception());
}

TEST_F(UtilityTest, EmplaceByIndex)
{
	vrt::variant<int, std::string, std::vector<int> > v;

	auto &vec_ref = v.emplace<2>(3, 42);

	EXPECT_EQ(v.index(), 2);
	EXPECT_EQ(vrt::get<std::vector<int>>(v).size(), 3);
	EXPECT_EQ(vrt::get<std::vector<int>>(v)[0], 42);
	EXPECT_EQ(&vec_ref, &vrt::get<std::vector<int>>(v));
	EXPECT_FALSE(v.valueless_by_exception());
}

TEST_F(UtilityTest, EmplaceMultipleArgs)
{
	vrt::variant<int, std::string, std::vector<int> > v;

	v.emplace<std::vector<int> >(5, 99);

	EXPECT_EQ(v.index(), 2);
	EXPECT_EQ(vrt::get<std::vector<int>>(v).size(), 5);
	EXPECT_EQ(vrt::get<std::vector<int>>(v)[0], 99);
	EXPECT_EQ(vrt::get<std::vector<int>>(v)[4], 99);
}

TEST_F(UtilityTest, EmplaceReplacesCurrent)
{
	test_variant v { 42 };

	v.emplace<std::string>("replaced");

	EXPECT_EQ(v.index(), 2);
	EXPECT_EQ(vrt::get<std::string>(v), "replaced");
	EXPECT_FALSE(v.valueless_by_exception());
}

TEST_F(UtilityTest, EmplaceMoveOnlyType)
{
	vrt::variant<std::unique_ptr<int>, std::string> v;

	v.emplace<std::unique_ptr<int> >(std::make_unique<int>(42));

	EXPECT_EQ(v.index(), 0);
	EXPECT_EQ(*vrt::get<std::unique_ptr<int>>(v), 42);
	EXPECT_FALSE(v.valueless_by_exception());
}

TEST_F(UtilityTest, SwapSameType)
{
	test_variant v1 { 42 };
	test_variant v2 { 99 };

	v1.swap(v2);

	EXPECT_EQ(vrt::get<int>(v1), 99);
	EXPECT_EQ(vrt::get<int>(v2), 42);
	EXPECT_FALSE(v1.valueless_by_exception());
	EXPECT_FALSE(v2.valueless_by_exception());
}

TEST_F(UtilityTest, SwapDifferentTypes)
{
	test_variant v1 { 42 };
	test_variant v2 { std::string("hello") };

	v1.swap(v2);

	EXPECT_EQ(v1.index(), 2);
	EXPECT_EQ(v2.index(), 0);
	EXPECT_EQ(vrt::get<std::string>(v1), "hello");
	EXPECT_EQ(vrt::get<int>(v2), 42);
	EXPECT_FALSE(v1.valueless_by_exception());
	EXPECT_FALSE(v2.valueless_by_exception());
}

TEST_F(UtilityTest, SwapWithValueless)
{
	test_variant v1 { 42 };
	test_variant v2 { std::string("hello") };

	auto moved = std::move(v2);
	EXPECT_TRUE(v2.valueless_by_exception());

	v1.swap(v2);

	EXPECT_TRUE(v1.valueless_by_exception());
	EXPECT_EQ(v2.index(), 0);
	EXPECT_EQ(vrt::get<int>(v2), 42);
	EXPECT_FALSE(v2.valueless_by_exception());
}

TEST_F(UtilityTest, SwapBothValueless)
{
	test_variant v1 { std::string("hello") };
	test_variant v2 { std::string("world") };

	auto moved1 = std::move(v1);
	auto moved2 = std::move(v2);

	EXPECT_TRUE(v1.valueless_by_exception());
	EXPECT_TRUE(v2.valueless_by_exception());

	v1.swap(v2);

	EXPECT_TRUE(v1.valueless_by_exception());
	EXPECT_TRUE(v2.valueless_by_exception());
}

TEST_F(UtilityTest, SwapSelfAssignment)
{
	test_variant v { 42 };

	v.swap(v);

	EXPECT_EQ(v.index(), 0);
	EXPECT_EQ(vrt::get<int>(v), 42);
	EXPECT_FALSE(v.valueless_by_exception());
}

TEST_F(UtilityTest, ADLSwap)
{
	test_variant v1 { 42 };
	test_variant v2 { std::string("hello") };

	using vrt::swap;
	swap(v1, v2);

	EXPECT_EQ(v1.index(), 2);
	EXPECT_EQ(v2.index(), 0);
	EXPECT_EQ(vrt::get<std::string>(v1), "hello");
	EXPECT_EQ(vrt::get<int>(v2), 42);
}

TEST_F(UtilityTest, VariantSize)
{
	EXPECT_EQ(vrt::variant_size_v<test_variant>, 3);
	EXPECT_EQ(vrt::variant_size_v<vrt::variant<int>>, 1);

	/* dev-note: keep the parentheses around the template argument to avoid macro parsing issues
	   due to commas inside the template parameter list */
	EXPECT_EQ((vrt::variant_size_v<vrt::variant<int, double, std::string, float>>), 4);
}

TEST_F(UtilityTest, VariantAlternative)
{
	static_assert(std::is_same_v<vrt::variant_alternative_t<0, test_variant>, int>);
	static_assert(std::is_same_v<vrt::variant_alternative_t<1, test_variant>, double>);
	static_assert(std::is_same_v<vrt::variant_alternative_t<2, test_variant>, std::string>);
}

TEST_F(UtilityTest, VariantAlternativeConst)
{
	using const_variant = const test_variant;
	static_assert(std::is_same_v<vrt::variant_alternative_t<0, const_variant>, const int>, "Type mismatch for index 0");
	static_assert(std::is_same_v<vrt::variant_alternative_t<1, const_variant>, const double>, "Type mismatch for index 1");
	static_assert(std::is_same_v<vrt::variant_alternative_t<2, const_variant>, const std::string>, "Type mismatch for index 2");
}

TEST_F(UtilityTest, TypeIndexConstants)
{
	EXPECT_EQ(test_variant::of<int>, 0);
	EXPECT_EQ(test_variant::of<double>, 1);
	EXPECT_EQ(test_variant::of<std::string>, 2);
}

TEST_F(UtilityTest, TypeIndexSameTypeMultipleTimes)
{
	using variant_t = vrt::variant<int, int, double>;
	EXPECT_EQ(variant_t::of<int>, 0);
	EXPECT_EQ(variant_t::of<double>, 2);
}

TEST_F(UtilityTest, StdVariantSizeCompatibility)
{
	EXPECT_EQ(std::variant_size_v<test_variant>, 3);
	static_assert(std::variant_size_v<test_variant> == 3);
}

TEST_F(UtilityTest, StdVariantAlternativeCompatibility)
{
	static_assert(std::is_same_v<std::variant_alternative_t<0, test_variant>, int>);
	static_assert(std::is_same_v<std::variant_alternative_t<1, test_variant>, double>);
	static_assert(std::is_same_v<std::variant_alternative_t<2, test_variant>, std::string>);
}
