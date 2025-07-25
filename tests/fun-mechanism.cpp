/* this project is part of the vrt project; licensed under the MIT license. see LICENSE for more info */

#include <string>
#include <string_view>
#include <vrt>
#include <gtest/gtest.h>

class FunMechanismTest : public ::testing::Test {};

TEST_F(FunMechanismTest, ExactTypeMatch)
{
    vrt::variant<int, double> v(42);
    EXPECT_EQ(v.index(), 0);
    EXPECT_EQ(vrt::get<int>(v), 42);
}

TEST_F(FunMechanismTest, DirectConstruction)
{
    struct FromInt
    {
        int value;
        FromInt(int x) : value(x) {}
    };

    vrt::variant<FromInt, double> v(FromInt(42));
    EXPECT_EQ(v.index(), 0);
    EXPECT_EQ(vrt::get<FromInt>(v).value, 42);
}

TEST_F(FunMechanismTest, UnambiguousConversion)
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

TEST_F(FunMechanismTest, StringLiteralHandling)
{
    vrt::variant<std::string_view, std::string, const char*> v(std::string_view("hello"));
    EXPECT_EQ(v.index(), 0);
    EXPECT_EQ(vrt::get<std::string_view>(v), "hello");
}

TEST_F(FunMechanismTest, StringLiteralToConstCharPtr)
{
    vrt::variant<const char*, std::string> v("hello");
    EXPECT_EQ(v.index(), 0);
    EXPECT_STREQ(vrt::get<const char*>(v), "hello");
}

TEST_F(FunMechanismTest, ExplicitVsImplicitConstructor)
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

TEST_F(FunMechanismTest, MovePreference)
{
    struct FromRValueRef
    {
        std::string value;
        FromRValueRef(std::string &&s) : value(std::move(s)) {}
    };

    vrt::variant<FromRValueRef, int> v(std::string("test"));
    EXPECT_EQ(v.index(), 0);
    EXPECT_EQ(vrt::get<FromRValueRef>(v).value, "test");
}

TEST_F(FunMechanismTest, ConversionSequenceLength)
{
    struct FromShort
    {
        short value;
        FromShort(short x) : value(x) {}
    };

    vrt::variant<FromShort, double> v(static_cast<short>(42));
    EXPECT_EQ(v.index(), 0);
    EXPECT_EQ(vrt::get<FromShort>(v).value, 42);
}

TEST_F(FunMechanismTest, InPlaceConstruction)
{
    struct MultiArg
    {
        int a, b;
        MultiArg(int x, int y) : a(x), b(y) {}
    };

    vrt::variant<MultiArg, double> v(std::in_place_type<MultiArg>, 10, 20);
    EXPECT_EQ(v.index(), 0);
    EXPECT_EQ(vrt::get<MultiArg>(v).a, 10);
    EXPECT_EQ(vrt::get<MultiArg>(v).b, 20);
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

TEST_F(FunMechanismTest, AssignmentUnambiguous)
{
    struct FromInt
    {
        int value = 0;
        FromInt() = default;
        FromInt(int x) : value(x) {}
        FromInt& operator=(int x) { value = x; return *this; }
    };

    vrt::variant<FromInt, std::string> v;
    v = 42;
    EXPECT_EQ(v.index(), 0);
    EXPECT_EQ(vrt::get<FromInt>(v).value, 42);

    v = std::string("hello");
    EXPECT_EQ(v.index(), 1);
    EXPECT_EQ(vrt::get<std::string>(v), "hello");
}

TEST_F(FunMechanismTest, AmbiguousConstructionPrevented)
{
    struct A { A(int) {} };
    struct B { B(int) {} };

    static_assert(!std::is_constructible_v<vrt::variant<A, B>, int>);
}

TEST_F(FunMechanismTest, AmbiguousStringConstructionPrevented)
{
    static_assert(!std::is_constructible_v<vrt::variant<std::string, std::string_view>, const char*>);
}

TEST_F(FunMechanismTest, BoolSpecialHandling)
{
    vrt::variant<bool, int> v1(true);
    EXPECT_EQ(v1.index(), 0);
    EXPECT_TRUE(vrt::get<bool>(v1));

    vrt::variant<bool, int> v2(42);
    EXPECT_EQ(v2.index(), 1);
    EXPECT_EQ(vrt::get<int>(v2), 42);
}

TEST_F(FunMechanismTest, WideningConversionAllowed)
{
    struct FromDouble { FromDouble(double) {} };

    static_assert(std::is_constructible_v<vrt::variant<FromDouble>, int>);

    vrt::variant<FromDouble> v(42);
    EXPECT_EQ(v.index(), 0);
}
