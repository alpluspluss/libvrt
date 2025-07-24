/* this project is part of the vrt project; licensed under the MIT license. see LICENSE for more info */

#include <string>
#include <vrt>
#include <gtest/gtest.h>

class FunMechanismTest : public ::testing::Test {};

TEST_F(FunMechanismTest, ExactTypeMatch)
{
	vrt::variant<int, double> v(42);
	EXPECT_EQ(v.index(), 0);
	EXPECT_EQ(vrt::get<int>(v), 42);
}

TEST_F(FunMechanismTest, BestMatchSelection)
{
	struct FromInt
	{
		int value;
		FromInt(int x) : value(x) {}
	};

	struct FromLong
	{
		long value;
		FromLong(long x) : value(x) {}
	};

	vrt::variant<FromLong, FromInt> v(42);
	EXPECT_EQ(v.index(), 1);
	EXPECT_EQ(vrt::get<FromInt>(v).value, 42);
}

TEST_F(FunMechanismTest, StringLiteralHandling)
{
	vrt::variant<std::string_view, std::string> v("hello");
	EXPECT_EQ(v.index(), 0);
	EXPECT_EQ(vrt::get<std::string_view>(v), "hello");
}

TEST_F(FunMechanismTest, ImplicitConversion)
{
	struct FromDouble
	{
		double value;
		FromDouble(double x) : value(x) {}
	};

	vrt::variant<FromDouble, int> v(3.14);
	EXPECT_EQ(v.index(), 0);
	EXPECT_DOUBLE_EQ(vrt::get<FromDouble>(v).value, 3.14);
}

TEST_F(FunMechanismTest, PreferExactMatch)
{
	struct FromInt
	{
		FromInt(int) {}
	};
	struct FromDouble
	{
		FromDouble(double) {}
	};

	vrt::variant<FromDouble, FromInt> v1(42);
	EXPECT_EQ(v1.index(), 1);

	vrt::variant<FromDouble, FromInt> v2(3.14);
	EXPECT_EQ(v2.index(), 0);
}

TEST_F(FunMechanismTest, ConstructorPriority)
{
	struct MultiConstructor
	{
		std::string value;
		MultiConstructor(int x) : value("int: " + std::to_string(x)) {}
		MultiConstructor(double x) : value("double: " + std::to_string(x)) {}
		MultiConstructor(const std::string &x) : value("string: " + x) {}
	};

	vrt::variant<MultiConstructor> v1(42);
	EXPECT_EQ(vrt::get<MultiConstructor>(v1).value, "int: 42");

	vrt::variant<MultiConstructor> v2(3.14);
	EXPECT_EQ(vrt::get<MultiConstructor>(v2).value, "double: 3.140000");

	vrt::variant<MultiConstructor> v3(std::string("test"));
	EXPECT_EQ(vrt::get<MultiConstructor>(v3).value, "string: test");
}

TEST_F(FunMechanismTest, AssignmentUsesCorrectAlternative)
{
	struct FromInt
	{
		int value;
		FromInt() = default;
		FromInt(int x) : value(x) {}

		FromInt & operator=(int x)
		{
			value = x;
			return *this;
		}
	};

	struct FromDouble
	{
		double value;
		FromDouble() = default;
		FromDouble(double x) : value(x) {}

		FromDouble & operator=(double x)
		{
			value = x;
			return *this;
		}
	};

	vrt::variant<FromDouble, FromInt> v;

	v = 42;
	EXPECT_EQ(v.index(), 1);
	EXPECT_EQ(vrt::get<FromInt>(v).value, 42);

	v = 3.14;
	EXPECT_EQ(v.index(), 0);
	EXPECT_DOUBLE_EQ(vrt::get<FromDouble>(v).value, 3.14);
}

/*
TEST_F(FunMechanismTest, NoAmbiguousConstruction)
{
	struct A
	{
		A(int) {}
	};
	struct B
	{
		B(int) {}
	};
	static_assert(!std::is_constructible_v<vrt::variant<A, B>, int>);
}

TEST_F(FunMechanismTest, NoAmbiguousAssignment)
{
	struct A
	{
		A(int) {}

		A & operator=(int)
		{
			return *this;
		}
	};
	struct B
	{
		B(int) {}

		B & operator=(int)
		{
			return *this;
		}
	};

	vrt::variant<A, B> v { A { 42 } };
	static_assert(!std::is_assignable_v<vrt::variant<A, B> &, int>);
}
*/

TEST_F(FunMechanismTest, ReferencesAndConst)
{
	struct FromConstRef
	{
		std::string value;
		FromConstRef(const std::string &s) : value(s) {}
	};

	struct FromRValueRef
	{
		std::string value;
		FromRValueRef(std::string &&s) : value(std::move(s)) {}
	};

	std::string s = "test";
	vrt::variant<FromConstRef, FromRValueRef> v1(s);
	EXPECT_EQ(v1.index(), 0);

	vrt::variant<FromConstRef, FromRValueRef> v2(std::move(s));
	EXPECT_EQ(v2.index(), 1);
}

TEST_F(FunMechanismTest, ArrayInitializationTest)
{
	struct ExplicitFromInt
	{
		int value;
		explicit ExplicitFromInt(int x) : value(x) {}
	};

	struct ImplicitFromInt
	{
		int value;
		ImplicitFromInt(int x) : value(x) {}
	};

	vrt::variant<ExplicitFromInt, ImplicitFromInt> v(42);
	EXPECT_EQ(v.index(), 1);
	EXPECT_EQ(vrt::get<ImplicitFromInt>(v).value, 42);
}

TEST_F(FunMechanismTest, NestedVariantConstruction)
{
	using inner_variant = vrt::variant<int, std::string>;
	using outer_variant = vrt::variant<inner_variant, double>;

	outer_variant v(42);
	EXPECT_EQ(v.index(), 0);

	const auto &inner = vrt::get<inner_variant>(v);
	EXPECT_EQ(inner.index(), 0);
	EXPECT_EQ(vrt::get<int>(inner), 42);
}

TEST_F(FunMechanismTest, ConversionSequenceLength)
{
	struct FromShort
	{
		short value;
		FromShort(short x) : value(x) {}
	};

	struct FromLong
	{
		long value;
		FromLong(long x) : value(x) {}
	};

	vrt::variant<FromLong, FromShort> v { static_cast<short>(42) };
	EXPECT_EQ(v.index(), 1);
	EXPECT_EQ(vrt::get<FromShort>(v).value, 42);
}

TEST_F(FunMechanismTest, UserDefinedConversions)
{
	struct Convertible
	{
		int value;
		Convertible(int x) : value(x) {}

		operator double() const
		{
			return static_cast<double>(value);
		}
	};

	struct FromDouble
	{
		double value;
		FromDouble(double x) : value(x) {}
	};

	Convertible c { 42 };
	vrt::variant<FromDouble> v(c);
	EXPECT_EQ(v.index(), 0);
	EXPECT_DOUBLE_EQ(vrt::get<FromDouble>(v).value, 42.0);
}
