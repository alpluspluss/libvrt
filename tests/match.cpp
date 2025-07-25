/* this project is part of the vrt project; licensed under the MIT license. see LICENSE for more info */

#include <string>
#include <vector>
#include <vrt>
#include <gtest/gtest.h>

class PatternMatchingTest : public ::testing::Test {};

TEST_F(PatternMatchingTest, BasicMatch)
{
	vrt::variant<int, std::string, double> v { 42 };

	auto result = vrt::match(v) | [](auto &&var)
	{
		return vrt::visit([](auto &&value) -> std::string
		{
			using T = std::decay_t<decltype(value)>;
			if constexpr (std::is_same_v<T, int>)
			{
				return "int: " + std::to_string(value);
			}
			else if constexpr (std::is_same_v<T, std::string>)
			{
				return "string: " + value;
			}
			else if constexpr (std::is_same_v<T, double>)
			{
				return "double: " + std::to_string(value);
			}
		}, var);
	};

	EXPECT_EQ(result, "int: 42");
}

TEST_F(PatternMatchingTest, MatchWithDifferentTypes)
{
	vrt::variant<int, std::string, double> v1 { 42 };
	vrt::variant<int, std::string, double> v2 { std::string("hello") };
	vrt::variant<int, std::string, double> v3 { 3.14 };

	auto matcher = [](auto &&var)
	{
		return vrt::visit([](auto &&value) -> std::string
		{
			using T = std::decay_t<decltype(value)>;
			if constexpr (std::is_same_v<T, int>)
			{
				return "matched_int";
			}
			else if constexpr (std::is_same_v<T, std::string>)
			{
				return "matched_string";
			}
			else if constexpr (std::is_same_v<T, double>)
			{
				return "matched_double";
			}
		}, var);
	};

	EXPECT_EQ(vrt::match(v1) | matcher, "matched_int");
	EXPECT_EQ(vrt::match(v2) | matcher, "matched_string");
	EXPECT_EQ(vrt::match(v3) | matcher, "matched_double");
}

TEST_F(PatternMatchingTest, MatchBuilderForwarding)
{
	vrt::variant<int, std::string> v { 42 };

	auto result = vrt::match(std::move(v)) | [](auto &&var)
	{
		return vrt::visit([](auto &&value) -> int
		{
			using T = std::decay_t<decltype(value)>;
			if constexpr (std::is_same_v<T, int>)
			{
				return value * 2;
			}
			else
			{
				return 0;
			}
		}, var);
	};

	EXPECT_EQ(result, 84);
}

TEST_F(PatternMatchingTest, MatchWithSideEffects)
{
	vrt::variant<int, std::string> v { std::string("test") };

	int counter = 0;
	auto result = vrt::match(v) | [&counter](auto &&var)
	{
		return vrt::visit([&counter](auto &&value) -> bool
		{
			counter++;
			using T = std::decay_t<decltype(value)>;
			return std::is_same_v<T, std::string>;
		}, var);
	};

	EXPECT_TRUE(result);
	EXPECT_EQ(counter, 1);
}

TEST_F(PatternMatchingTest, MatchDeductionGuide)
{
	vrt::variant<int, std::string> v { 42 };

	auto match_builder = vrt::match(v);
	auto result = match_builder | [](auto &&var)
	{
		return var.index();
	};

	EXPECT_EQ(result, 0);
}

TEST_F(PatternMatchingTest, MatchWithValuelessVariant)
{
	vrt::variant<std::string> v { "hello" };
	auto moved = std::move(v);

	EXPECT_TRUE(v.valueless_by_exception());

	EXPECT_THROW(
		vrt::match(v) | [](auto&& var) {
		return vrt::visit([](auto&&) { return 42; }, var);
		},
		std::bad_variant_access
	);
}

TEST_F(PatternMatchingTest, ChainedMatching)
{
	vrt::variant<int, std::string, double> v1 { 42 };
	vrt::variant<int, std::string, double> v2 { std::string("hello") };

	auto process = [](auto &&var)
	{
		return vrt::visit([](auto &&value) -> std::string
		{
			using T = std::decay_t<decltype(value)>;
			if constexpr (std::is_same_v<T, int>)
			{
				return std::to_string(value * 2);
			}
			else if constexpr (std::is_same_v<T, std::string>)
			{
				return value + "_processed";
			}
			else
			{
				return "unknown";
			}
		}, var);
	};

	auto result1 = vrt::match(v1) | process;
	auto result2 = vrt::match(v2) | process;

	EXPECT_EQ(result1, "84");
	EXPECT_EQ(result2, "hello_processed");
}

TEST_F(PatternMatchingTest, MatchWithComplexTypes)
{
	struct Point
	{
		int x, y;
		Point(int x, int y) : x(x), y(y) {}
	};

	struct Circle
	{
		int radius;
		Circle(int r) : radius(r) {}
	};

	vrt::variant<Point, Circle, std::string> v { Point { 3, 4 } };

	auto result = vrt::match(v) | [](auto &&var)
	{
		return vrt::visit([](auto &&value) -> std::string
		{
			using T = std::decay_t<decltype(value)>;
			if constexpr (std::is_same_v<T, Point>)
			{
				return "point(" + std::to_string(value.x) + "," + std::to_string(value.y) + ")";
			}
			else if constexpr (std::is_same_v<T, Circle>)
			{
				return "circle(r=" + std::to_string(value.radius) + ")";
			}
			else if constexpr (std::is_same_v<T, std::string>)
			{
				return "string:" + value;
			}
			else
			{
				return "unknown";
			}
		}, var);
	};

	EXPECT_EQ(result, "point(3,4)");
}

TEST_F(PatternMatchingTest, MatchInLoop)
{
	using test_variant = vrt::variant<int, std::string, bool>;

	std::vector<test_variant> variants = {
		42,
		std::string("hello"),
		true,
		99,
		std::string("world"),
		false
	};

	std::vector<std::string> results;

	for (const auto &v: variants)
	{
		auto result = vrt::match(v) | [](auto &&var)
		{
			return vrt::visit([](auto &&value) -> std::string
			{
				using T = std::decay_t<decltype(value)>;
				if constexpr (std::is_same_v<T, int>)
				{
					return "int:" + std::to_string(value);
				}
				else if constexpr (std::is_same_v<T, std::string>)
				{
					return "string:" + value;
				}
				else if constexpr (std::is_same_v<T, bool>)
				{
					return value ? "bool:true" : "bool:false";
				}
			}, var);
		};
		results.push_back(result);
	}

	EXPECT_EQ(results.size(), 6);
	EXPECT_EQ(results[0], "int:42");
	EXPECT_EQ(results[1], "string:hello");
	EXPECT_EQ(results[2], "bool:true");
	EXPECT_EQ(results[3], "int:99");
	EXPECT_EQ(results[4], "string:world");
	EXPECT_EQ(results[5], "bool:false");
}

TEST_F(PatternMatchingTest, MatchWithReturnTypeDeduction)
{
	vrt::variant<int, double> v { 42 };

	auto int_result = vrt::match(v) | [](auto &&var)
	{
		return vrt::visit([](auto &&value)
		{
			using T = std::decay_t<decltype(value)>;
			if constexpr (std::is_same_v<T, int>)
			{
				return value;
			}
			else
			{
				return static_cast<int>(value);
			}
		}, var);
	};

	EXPECT_EQ(int_result, 42);

	v = 3.14;
	auto converted_result = vrt::match(v) | [](auto &&var)
	{
		return vrt::visit([](auto &&value)
		{
			using T = std::decay_t<decltype(value)>;
			if constexpr (std::is_same_v<T, int>)
			{
				return value;
			}
			else
			{
				return static_cast<int>(value);
			}
		}, var);
	};

	EXPECT_EQ(converted_result, 3);
}

TEST_F(PatternMatchingTest, MatchBuilderCopySemantics)
{
	vrt::variant<std::string> v { std::string("original") };

	auto match_builder = vrt::match(v);

	auto result1 = match_builder | [](auto &&var)
	{
		return vrt::visit([](auto &&value) -> std::string
		{
			return value + "_first";
		}, var);
	};

	auto result2 = match_builder | [](auto &&var)
	{
		return vrt::visit([](auto &&value) -> std::string
		{
			return value + "_second";
		}, var);
	};

	EXPECT_EQ(result1, "original_first");
	EXPECT_EQ(result2, "original_second");
	EXPECT_EQ(vrt::get<std::string>(v), "original");
}

TEST_F(PatternMatchingTest, MatchWithMultipleVisitors)
{
	vrt::variant<int, std::string, double> v { 42 };

	auto get_type_name = [](auto &&var)
	{
		return vrt::visit([](auto &&value) -> std::string
		{
			using T = std::decay_t<decltype(value)>;
			if constexpr (std::is_same_v<T, int>)
			{
				return "integer";
			}
			else if constexpr (std::is_same_v<T, std::string>)
			{
				return "text";
			}
			else if constexpr (std::is_same_v<T, double>)
			{
				return "floating";
			}
		}, var);
	};

	auto get_value_string = [](auto &&var)
	{
		return vrt::visit([](auto &&value) -> std::string
		{
			using T = std::decay_t<decltype(value)>;
			if constexpr (std::is_same_v<T, int>)
			{
				return std::to_string(value);
			}
			else if constexpr (std::is_same_v<T, std::string>)
			{
				return value;
			}
			else if constexpr (std::is_same_v<T, double>)
			{
				return std::to_string(value);
			}
		}, var);
	};

	auto type_name = vrt::match(v) | get_type_name;
	auto value_str = vrt::match(v) | get_value_string;

	EXPECT_EQ(type_name, "integer");
	EXPECT_EQ(value_str, "42");
}
