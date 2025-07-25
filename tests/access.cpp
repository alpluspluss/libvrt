/* this project is part of the vrt project; licensed under the MIT license. see LICENSE for more info */

#include <string>
#include <vrt>
#include <gtest/gtest.h>

class AccessTest : public ::testing::Test
{
protected:
	using test_variant = vrt::variant<int, double, std::string>;
};

TEST_F(AccessTest, GetByType)
{
	test_variant v { 42 };

	EXPECT_EQ(vrt::get<int>(v), 42);
	EXPECT_EQ(v.get<int>(), 42);

	const auto &cv = v;
	EXPECT_EQ(vrt::get<int>(cv), 42);
	EXPECT_EQ(cv.get<int>(), 42);
}

TEST_F(AccessTest, GetByIndex)
{
	test_variant v { 42 };

	EXPECT_EQ(vrt::get<0>(v), 42);

	const auto &cv = v;
	EXPECT_EQ(vrt::get<0>(cv), 42);
}

TEST_F(AccessTest, GetRValueReference)
{
	auto make_variant = []() -> test_variant
	{
		return test_variant { std::string("rvalue") };
	};

	std::string moved = vrt::get<std::string>(make_variant());
	EXPECT_EQ(moved, "rvalue");

	std::string moved2 = vrt::get<2>(make_variant());
	EXPECT_EQ(moved2, "rvalue");
}

TEST_F(AccessTest, GetConstRValueReference)
{
	auto make_variant = []() -> const test_variant
	{
		return test_variant { std::string("const_rvalue") };
	};

	const std::string moved = vrt::get<std::string>(make_variant());
	EXPECT_EQ(moved, "const_rvalue");
}

TEST_F(AccessTest, GetThrowsOnWrongType)
{
	test_variant v { 42 };

	EXPECT_THROW(vrt::get<std::string>(v), std::bad_variant_access);
	EXPECT_THROW(v.get<std::string>(), std::bad_variant_access);
	EXPECT_THROW(vrt::get<double>(v), std::bad_variant_access);
}

TEST_F(AccessTest, GetThrowsOnWrongIndex)
{
	test_variant v { 42 };

	EXPECT_THROW(vrt::get<1>(v), std::bad_variant_access);
	EXPECT_THROW(vrt::get<2>(v), std::bad_variant_access);
}

TEST_F(AccessTest, GetThrowsOnValueless)
{
	test_variant v { std::string("hello") };
	auto moved = std::move(v);

	EXPECT_TRUE(v.valueless_by_exception());
	EXPECT_THROW(vrt::get<int>(v), std::bad_variant_access);
	EXPECT_THROW(v.get<int>(), std::bad_variant_access);
}

TEST_F(AccessTest, GetIfByType)
{
	test_variant v { 42 };

	auto *int_ptr = vrt::get_if<int>(&v);
	ASSERT_NE(int_ptr, nullptr);
	EXPECT_EQ(*int_ptr, 42);

	auto *int_ptr2 = v.get_if<int>();
	ASSERT_NE(int_ptr2, nullptr);
	EXPECT_EQ(*int_ptr2, 42);

	auto *str_ptr = vrt::get_if<std::string>(&v);
	EXPECT_EQ(str_ptr, nullptr);

	auto *str_ptr2 = v.get_if<std::string>();
	EXPECT_EQ(str_ptr2, nullptr);
}

TEST_F(AccessTest, GetIfByIndex)
{
	test_variant v { 42 };

	auto *int_ptr = vrt::get_if<0>(&v);
	ASSERT_NE(int_ptr, nullptr);
	EXPECT_EQ(*int_ptr, 42);

	auto *double_ptr = vrt::get_if<1>(&v);
	EXPECT_EQ(double_ptr, nullptr);
}

TEST_F(AccessTest, GetIfConst)
{
	const test_variant v { 42 };

	const auto *int_ptr = vrt::get_if<int>(&v);
	ASSERT_NE(int_ptr, nullptr);
	EXPECT_EQ(*int_ptr, 42);

	const auto *str_ptr = vrt::get_if<std::string>(&v);
	EXPECT_EQ(str_ptr, nullptr);
}

TEST_F(AccessTest, GetIfNullptr)
{
	test_variant *null_variant = nullptr;
	EXPECT_EQ(vrt::get_if<int>(null_variant), nullptr);
	EXPECT_EQ(vrt::get_if<0>(null_variant), nullptr);
}

TEST_F(AccessTest, GetIfValueless)
{
	test_variant v { std::string("hello") };
	auto moved = std::move(v);

	EXPECT_TRUE(v.valueless_by_exception());
	EXPECT_EQ(vrt::get_if<int>(&v), nullptr);
	EXPECT_EQ(v.get_if<int>(), nullptr);
	EXPECT_EQ(vrt::get_if<std::string>(&v), nullptr);
}

TEST_F(AccessTest, HoldsAlternative)
{
	test_variant v { 42 };

	EXPECT_TRUE(vrt::holds_alternative<int>(v));
	EXPECT_TRUE(v.holds_alternative<int>());
	EXPECT_FALSE(vrt::holds_alternative<double>(v));
	EXPECT_FALSE(v.holds_alternative<double>());
	EXPECT_FALSE(vrt::holds_alternative<std::string>(v));
	EXPECT_FALSE(v.holds_alternative<std::string>());
}

TEST_F(AccessTest, HoldsAlternativeValueless)
{
	test_variant v { std::string("hello") };
	auto moved = std::move(v);

	EXPECT_TRUE(v.valueless_by_exception());
	EXPECT_FALSE(vrt::holds_alternative<int>(v));
	EXPECT_FALSE(v.holds_alternative<int>());
	EXPECT_FALSE(vrt::holds_alternative<std::string>(v));
	EXPECT_FALSE(v.holds_alternative<std::string>());
}

TEST_F(AccessTest, Index)
{
	test_variant v1 { 42 };
	EXPECT_EQ(v1.index(), 0);

	test_variant v2 { 3.14 };
	EXPECT_EQ(v2.index(), 1);

	test_variant v3 { std::string("hello") };
	EXPECT_EQ(v3.index(), 2);
}

TEST_F(AccessTest, IndexValueless)
{
	test_variant v { std::string("hello") };
	auto moved = std::move(v);

	EXPECT_TRUE(v.valueless_by_exception());
	EXPECT_EQ(v.index(), vrt::variant_npos);
}

TEST_F(AccessTest, ValuelessByException)
{
	test_variant v { 42 };
	EXPECT_FALSE(v.valueless_by_exception());

	auto moved = std::move(v);
	EXPECT_TRUE(v.valueless_by_exception());
	EXPECT_FALSE(moved.valueless_by_exception());
}
