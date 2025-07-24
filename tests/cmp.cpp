/* this project is part of the vrt project; licensed under the MIT license. see LICENSE for more info */

#include <string>
#include <vrt>
#include <gtest/gtest.h>

class ComparisonTest : public ::testing::Test
{
protected:
	using test_variant = vrt::variant<int, double, std::string>;
};

TEST_F(ComparisonTest, EqualityComparison)
{
	test_variant v1 { 42 };
	test_variant v2 { 42 };
	test_variant v3 { 43 };

	EXPECT_TRUE(v1 == v2);
	EXPECT_FALSE(v1 == v3);
	EXPECT_FALSE(v2 == v3);
}

TEST_F(ComparisonTest, EqualityDifferentTypes)
{
	test_variant v1 { 42 };
	test_variant v2 { std::string("hello") };

	EXPECT_FALSE(v1 == v2);
	EXPECT_FALSE(v2 == v1);
}

TEST_F(ComparisonTest, EqualityStrings)
{
	test_variant v1 { std::string("hello") };
	test_variant v2 { std::string("hello") };
	test_variant v3 { std::string("world") };

	EXPECT_TRUE(v1 == v2);
	EXPECT_FALSE(v1 == v3);
	EXPECT_FALSE(v2 == v3);
}

TEST_F(ComparisonTest, EqualityValueless)
{
	test_variant v1 { std::string("hello") };
	test_variant v2 { std::string("world") };

	auto moved1 = std::move(v1);
	auto moved2 = std::move(v2);

	EXPECT_TRUE(v1.valueless_by_exception());
	EXPECT_TRUE(v2.valueless_by_exception());
	EXPECT_TRUE(v1 == v2);
}

TEST_F(ComparisonTest, EqualityOneValueless)
{
	test_variant v1 { 42 };
	test_variant v2 { std::string("hello") };

	auto moved = std::move(v2);
	EXPECT_TRUE(v2.valueless_by_exception());

	EXPECT_FALSE(v1 == v2);
	EXPECT_FALSE(v2 == v1);
}

TEST_F(ComparisonTest, InequalityComparison)
{
	test_variant v1 { 42 };
	test_variant v2 { 42 };
	test_variant v3 { 43 };

	EXPECT_FALSE(v1 != v2);
	EXPECT_TRUE(v1 != v3);
	EXPECT_TRUE(v2 != v3);
}

TEST_F(ComparisonTest, ThreeWayComparisonSameType)
{
	test_variant v1 { 42 };
	test_variant v2 { 42 };
	test_variant v3 { 43 };
	test_variant v4 { 41 };

	auto cmp1 = v1 <=> v2;
	auto cmp2 = v1 <=> v3;
	auto cmp3 = v1 <=> v4;

	EXPECT_TRUE(cmp1 == 0);
	EXPECT_TRUE(cmp2 < 0);
	EXPECT_TRUE(cmp3 > 0);
}

TEST_F(ComparisonTest, ThreeWayComparisonDifferentTypes)
{
	test_variant v1 { 42 };
	test_variant v2 { std::string("hello") };

	auto cmp1 = v1 <=> v2;
	auto cmp2 = v2 <=> v1;

	EXPECT_TRUE(cmp1 < 0);
	EXPECT_TRUE(cmp2 > 0);
}

TEST_F(ComparisonTest, ThreeWayComparisonStrings)
{
	test_variant v1 { std::string("apple") };
	test_variant v2 { std::string("apple") };
	test_variant v3 { std::string("banana") };
	test_variant v4 { std::string("aardvark") };

	auto cmp1 = v1 <=> v2;
	auto cmp2 = v1 <=> v3;
	auto cmp3 = v1 <=> v4;

	EXPECT_TRUE(cmp1 == 0);
	EXPECT_TRUE(cmp2 < 0);
	EXPECT_TRUE(cmp3 > 0);
}

TEST_F(ComparisonTest, ThreeWayComparisonValueless)
{
	test_variant v1 { std::string("hello") };
	test_variant v2 { std::string("world") };

	auto moved1 = std::move(v1);
	auto moved2 = std::move(v2);

	EXPECT_TRUE(v1.valueless_by_exception());
	EXPECT_TRUE(v2.valueless_by_exception());

	auto cmp = v1 <=> v2;
	EXPECT_TRUE(cmp == 0);
}

TEST_F(ComparisonTest, ThreeWayComparisonOneValueless)
{
	test_variant v1 { 42 };
	test_variant v2 { std::string("hello") };

	auto moved = std::move(v2);
	EXPECT_TRUE(v2.valueless_by_exception());

	auto cmp1 = v1 <=> v2;
	auto cmp2 = v2 <=> v1;

	EXPECT_TRUE(cmp1 > 0);
	EXPECT_TRUE(cmp2 < 0);
}

TEST_F(ComparisonTest, LessThanComparison)
{
	test_variant v1 { 42 };
	test_variant v2 { 43 };
	test_variant v3 { std::string("hello") };

	EXPECT_TRUE(v1 < v2);
	EXPECT_FALSE(v2 < v1);
	EXPECT_TRUE(v1 < v3);
	EXPECT_FALSE(v3 < v1);
}

TEST_F(ComparisonTest, LessEqualComparison)
{
	test_variant v1 { 42 };
	test_variant v2 { 42 };
	test_variant v3 { 43 };

	EXPECT_TRUE(v1 <= v2);
	EXPECT_TRUE(v1 <= v3);
	EXPECT_FALSE(v3 <= v1);
}

TEST_F(ComparisonTest, GreaterThanComparison)
{
	test_variant v1 { 42 };
	test_variant v2 { 43 };
	test_variant v3 { std::string("hello") };

	EXPECT_FALSE(v1 > v2);
	EXPECT_TRUE(v2 > v1);
	EXPECT_FALSE(v1 > v3);
	EXPECT_TRUE(v3 > v1);
}

TEST_F(ComparisonTest, GreaterEqualComparison)
{
	test_variant v1 { 42 };
	test_variant v2 { 42 };
	test_variant v3 { 43 };

	EXPECT_TRUE(v1 >= v2);
	EXPECT_FALSE(v1 >= v3);
	EXPECT_TRUE(v3 >= v1);
}
