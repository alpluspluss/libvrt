/* this project is part of the vrt project; licensed under the MIT license. see LICENSE for more info */

#include <gtest/gtest.h>
#include <vrt>
#include <string>

class VisitationTest : public ::testing::Test
{
protected:
	using test_variant = vrt::variant<int, double, std::string>;
};

TEST_F(VisitationTest, BasicVisit)
{
	test_variant v { 42 };

	auto result = vrt::visit([](auto &&arg) -> std::string
	{
		using T = std::decay_t<decltype(arg)>;
		if constexpr (std::is_same_v<T, int>)
			return "int: " + std::to_string(arg);
		else if constexpr (std::is_same_v<T, double>)
			return "double: " + std::to_string(arg);
		else if constexpr (std::is_same_v<T, std::string>)
			return "string: " + arg;
	}, v);

	EXPECT_EQ(result, "int: 42");
}

TEST_F(VisitationTest, VisitDouble)
{
	test_variant v { 3.14 };

	auto result = vrt::visit([](auto &&arg) -> std::string
	{
		using T = std::decay_t<decltype(arg)>;
		if constexpr (std::is_same_v<T, int>)
			return "int: " + std::to_string(arg);
		else if constexpr (std::is_same_v<T, double>)
			return "double: " + std::to_string(arg);
		else if constexpr (std::is_same_v<T, std::string>)
			return "string: " + arg;
	}, v);

	EXPECT_EQ(result, "double: 3.140000");
}

TEST_F(VisitationTest, VisitString)
{
	test_variant v { std::string("hello") };

	auto result = vrt::visit([](auto &&arg) -> std::string
	{
		using T = std::decay_t<decltype(arg)>;
		if constexpr (std::is_same_v<T, int>)
			return "int: " + std::to_string(arg);
		else if constexpr (std::is_same_v<T, double>)
			return "double: " + std::to_string(arg);
		else if constexpr (std::is_same_v<T, std::string>)
			return "string: " + arg;
	}, v);

	EXPECT_EQ(result, "string: hello");
}

TEST_F(VisitationTest, VisitWithVoidReturn)
{
	test_variant v { 42 };
	std::string result;

	vrt::visit([&](auto &&arg)
	{
		using T = std::decay_t<decltype(arg)>;
		if constexpr (std::is_same_v<T, int>)
			result = "visited int";
		else if constexpr (std::is_same_v<T, double>)
			result = "visited double";
		else if constexpr (std::is_same_v<T, std::string>)
			result = "visited string";
	}, v);

	EXPECT_EQ(result, "visited int");
}

TEST_F(VisitationTest, VisitWithMutableLambda)
{
	test_variant v { 42 };

	auto counter = 0;
	auto result1 = vrt::visit([counter](auto &&) mutable -> int
	{
		return ++counter;
	}, v);

	auto result2 = vrt::visit([counter](auto &&) mutable -> int
	{
		return ++counter;
	}, v);

	EXPECT_EQ(result1, 1);
	EXPECT_EQ(result2, 1);
}

TEST_F(VisitationTest, VisitConst)
{
	const test_variant v { 42 };

	auto result = vrt::visit([](auto &&arg) -> std::string
	{
		using T = std::decay_t<decltype(arg)>;
		if constexpr (std::is_same_v<T, int>)
			return "const int: " + std::to_string(arg);
		else if constexpr (std::is_same_v<T, double>)
			return "const double: " + std::to_string(arg);
		else if constexpr (std::is_same_v<T, std::string>)
			return "const string: " + arg;
	}, v);

	EXPECT_EQ(result, "const int: 42");
}

TEST_F(VisitationTest, VisitRValue)
{
	auto make_variant = []() -> test_variant
	{
		return test_variant { std::string("rvalue") };
	};

	auto result = vrt::visit([](auto &&arg) -> std::string
	{
		using T = std::decay_t<decltype(arg)>;
		if constexpr (std::is_same_v<T, int>)
			return "int: " + std::to_string(arg);
		else if constexpr (std::is_same_v<T, double>)
			return "double: " + std::to_string(arg);
		else if constexpr (std::is_same_v<T, std::string>)
			return "string: " + std::move(arg);
	}, make_variant());

	EXPECT_EQ(result, "string: rvalue");
}

TEST_F(VisitationTest, VisitThrowsOnValueless)
{
	test_variant v { std::string("hello") };
	auto moved = std::move(v);

	EXPECT_TRUE(v.valueless_by_exception());
	EXPECT_THROW(vrt::visit([](auto&&) { return 0; }, v), std::bad_variant_access);
}

TEST_F(VisitationTest, VisitExceptionPropagation)
{
	test_variant v { 42 };

	EXPECT_THROW(vrt::visit([](auto&&) -> int {
		             throw std::runtime_error("test exception");
		             }, v), std::runtime_error);
}

TEST_F(VisitationTest, SpecializedVisitor)
{
	struct Visitor
	{
		std::string operator()(int i) const
		{
			return "int: " + std::to_string(i);
		}

		std::string operator()(double d) const
		{
			return "double: " + std::to_string(d);
		}

		std::string operator()(const std::string &s) const
		{
			return "string: " + s;
		}
	};

	test_variant v { 42 };
	auto result = vrt::visit(Visitor {}, v);
	EXPECT_EQ(result, "int: 42");

	v = 3.14;
	result = vrt::visit(Visitor {}, v);
	EXPECT_EQ(result, "double: 3.140000");

	v = std::string("test");
	result = vrt::visit(Visitor {}, v);
	EXPECT_EQ(result, "string: test");
}
