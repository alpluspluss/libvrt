/* this project is part of the vrt project; licensed under the MIT license. see LICENSE for more info */

#include <string>
#include <vrt>
#include <gtest/gtest.h>

class SwitchCaseTest : public ::testing::Test {};

TEST_F(SwitchCaseTest, StaticTypeIndices)
{
	using test_variant = vrt::variant<int, std::string, double>;

	static_assert(test_variant::of<int> == 0);
	static_assert(test_variant::of<std::string> == 1);
	static_assert(test_variant::of<double> == 2);

	EXPECT_EQ(test_variant::of<int>, 0);
	EXPECT_EQ(test_variant::of<std::string>, 1);
	EXPECT_EQ(test_variant::of<double>, 2);
}

TEST_F(SwitchCaseTest, SwitchOnVariantIndex)
{
	vrt::variant<int, std::string, double> v;

	v = 42;
	switch (v.index())
	{
		case decltype(v)::of<int>:
			EXPECT_EQ(vrt::get<int>(v), 42);
			break;
		case decltype(v)::of<std::string>:
			FAIL() << "should not reach string case";
			break;
		case decltype(v)::of<double>:
			FAIL() << "should not reach double case";
			break;
		default:
			FAIL() << "unknown variant index";
	}

	v = std::string("hello");
	switch (v.index())
	{
		case decltype(v)::of<int>:
			FAIL() << "should not reach int case";
			break;
		case decltype(v)::of<std::string>:
			EXPECT_EQ(vrt::get<std::string>(v), "hello");
			break;
		case decltype(v)::of<double>:
			FAIL() << "should not reach double case";
			break;
		default:
			FAIL() << "unknown variant index";
	}

	v = 3.14;
	switch (v.index())
	{
		case decltype(v)::of<int>:
			FAIL() << "should not reach int case";
			break;
		case decltype(v)::of<std::string>:
			FAIL() << "should not reach string case";
			break;
		case decltype(v)::of<double>:
			EXPECT_DOUBLE_EQ(vrt::get<double>(v), 3.14);
			break;
		default:
			FAIL() << "unknown variant index";
	}
}

TEST_F(SwitchCaseTest, SwitchWithComplexTypes)
{
	struct CustomType
	{
		int value;
		CustomType(int v) : value(v) {}
	};

	using complex_variant = vrt::variant<CustomType, std::string, bool>;

	static_assert(complex_variant::of<CustomType> == 0);
	static_assert(complex_variant::of<std::string> == 1);
	static_assert(complex_variant::of<bool> == 2);

	complex_variant v { CustomType { 99 } };

	std::string result;
	switch (v.index())
	{
		case complex_variant::of<CustomType>:
			result = "custom(" + std::to_string(vrt::get<CustomType>(v).value) + ")";
			break;
		case complex_variant::of<std::string>:
			result = "string";
			break;
		case complex_variant::of<bool>:
			result = "bool";
			break;
	}

	EXPECT_EQ(result, "custom(99)");
}

TEST_F(SwitchCaseTest, SwitchWithValuelessVariant)
{
	vrt::variant<std::string> v { "hello" };
	auto moved = std::move(v);

	EXPECT_TRUE(v.valueless_by_exception());
	EXPECT_EQ(v.index(), vrt::variant_npos);

	bool handled_valueless = false;
	switch (v.index())
	{
		case decltype(v)::of<std::string>:
			FAIL() << "should not reach string case for valueless variant";
			break;
		case vrt::variant_npos:
			handled_valueless = true;
			break;
		default:
			FAIL() << "unexpected variant index";
	}

	EXPECT_TRUE(handled_valueless);
}

TEST_F(SwitchCaseTest, CompileTimeConstantExpression)
{
	using test_variant = vrt::variant<int, char, bool>;

	constexpr auto int_index = test_variant::of<int>;
	constexpr auto char_index = test_variant::of<char>;
	constexpr auto bool_index = test_variant::of<bool>;

	static_assert(int_index == 0);
	static_assert(char_index == 1);
	static_assert(bool_index == 2);

	constexpr const char *type_names[] = { "int", "char", "bool" };

	test_variant v { true };
	EXPECT_STREQ(type_names[v.index()], "bool");

	v = 'x';
	EXPECT_STREQ(type_names[v.index()], "char");

	v = 42;
	EXPECT_STREQ(type_names[v.index()], "int");
}

TEST_F(SwitchCaseTest, SwitchWithLargeVariant)
{
	using large_variant = vrt::variant<int, char, bool, double, float, std::string, long>;

	static_assert(large_variant::of<int> == 0);
	static_assert(large_variant::of<char> == 1);
	static_assert(large_variant::of<bool> == 2);
	static_assert(large_variant::of<double> == 3);
	static_assert(large_variant::of<float> == 4);
	static_assert(large_variant::of<std::string> == 5);
	static_assert(large_variant::of<long> == 6);

	large_variant v { std::string("test") };

	int case_hit = -1;
	switch (v.index())
	{
		case large_variant::of<int>:
			case_hit = 0;
			break;
		case large_variant::of<char>:
			case_hit = 1;
			break;
		case large_variant::of<bool>:
			case_hit = 2;
			break;
		case large_variant::of<double>:
			case_hit = 3;
			break;
		case large_variant::of<float>:
			case_hit = 4;
			break;
		case large_variant::of<std::string>:
			case_hit = 5;
			break;
		case large_variant::of<long>:
			case_hit = 6;
			break;
		default:
			case_hit = -1;
			break;
	}

	EXPECT_EQ(case_hit, 5);
	EXPECT_EQ(vrt::get<std::string>(v), "test");
}

TEST_F(SwitchCaseTest, SwitchInLoop)
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
		switch (v.index())
		{
			case test_variant::of<int>:
				results.push_back("int:" + std::to_string(vrt::get<int>(v)));
				break;
			case test_variant::of<std::string>:
				results.push_back("string:" + vrt::get<std::string>(v));
				break;
			case test_variant::of<bool>:
				results.push_back(vrt::get<bool>(v) ? "bool:true" : "bool:false");
				break;
		}
	}

	EXPECT_EQ(results.size(), 6);
	EXPECT_EQ(results[0], "int:42");
	EXPECT_EQ(results[1], "string:hello");
	EXPECT_EQ(results[2], "bool:true");
	EXPECT_EQ(results[3], "int:99");
	EXPECT_EQ(results[4], "string:world");
	EXPECT_EQ(results[5], "bool:false");
}
