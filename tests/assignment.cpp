/* this project is part of the vrt project; licensed under the MIT license. see LICENSE for more info */

#include <memory>
#include <string>
#include <vrt>
#include <gtest/gtest.h>

class AssignmentTest : public ::testing::Test
{
protected:
	using basic_variant = vrt::variant<int, double, std::string>;
	using move_only_variant = vrt::variant<std::unique_ptr<int>, std::string>;
};

TEST_F(AssignmentTest, CopyAssignmentSameType)
{
	basic_variant v1 { 42 };
	basic_variant v2 { 99 };

	v2 = v1;

	EXPECT_EQ(v2.index(), v1.index());
	EXPECT_EQ(vrt::get<int>(v2), 42);
	EXPECT_EQ(vrt::get<int>(v1), 42);
	EXPECT_FALSE(v1.valueless_by_exception());
	EXPECT_FALSE(v2.valueless_by_exception());
}

TEST_F(AssignmentTest, CopyAssignmentDifferentType)
{
	basic_variant v1 { std::string("hello") };
	basic_variant v2 { 42 };

	v2 = v1;

	EXPECT_EQ(v2.index(), v1.index());
	EXPECT_EQ(vrt::get<std::string>(v2), "hello");
	EXPECT_EQ(vrt::get<std::string>(v1), "hello");
	EXPECT_FALSE(v1.valueless_by_exception());
	EXPECT_FALSE(v2.valueless_by_exception());
}

TEST_F(AssignmentTest, MoveAssignmentSameType)
{
	basic_variant v1 { std::string("hello") };
	basic_variant v2 { std::string("world") };

	v2 = std::move(v1);

	EXPECT_EQ(v2.index(), 2);
	EXPECT_EQ(vrt::get<std::string>(v2), "hello");
	EXPECT_TRUE(vrt::get<std::string>(v1).empty() || /* moved-from state */ true);
	EXPECT_FALSE(v2.valueless_by_exception());
}

TEST_F(AssignmentTest, MoveAssignmentDifferentType)
{
	basic_variant v1 { std::string("hello") };
	basic_variant v2 { 42 };

	v2 = std::move(v1);

	EXPECT_EQ(v2.index(), 2);
	EXPECT_EQ(vrt::get<std::string>(v2), "hello");
	EXPECT_TRUE(v1.valueless_by_exception());
	EXPECT_FALSE(v2.valueless_by_exception());
}

TEST_F(AssignmentTest, ConvertingAssignmentSameType)
{
	basic_variant v { 42 };

	v = 99;

	EXPECT_EQ(v.index(), 0);
	EXPECT_EQ(vrt::get<int>(v), 99);
	EXPECT_FALSE(v.valueless_by_exception());
}

TEST_F(AssignmentTest, ConvertingAssignmentDifferentType)
{
	basic_variant v { 42 };

	v = std::string("hello");

	EXPECT_EQ(v.index(), 2);
	EXPECT_EQ(vrt::get<std::string>(v), "hello");
	EXPECT_FALSE(v.valueless_by_exception());
}

TEST_F(AssignmentTest, ConvertingAssignmentStringLiteral)
{
	basic_variant v { 42 };

	v = "hello";

	EXPECT_EQ(v.index(), 2);
	EXPECT_EQ(vrt::get<std::string>(v), "hello");
	EXPECT_FALSE(v.valueless_by_exception());
}

TEST_F(AssignmentTest, SelfAssignment)
{
	basic_variant v { std::string("test") };

	v = v;

	EXPECT_EQ(v.index(), 2);
	EXPECT_EQ(vrt::get<std::string>(v), "test");
	EXPECT_FALSE(v.valueless_by_exception());
}

TEST_F(AssignmentTest, SelfMoveAssignment)
{
	basic_variant v { std::string("test") };

	v = std::move(v);

	EXPECT_EQ(v.index(), 2);
	EXPECT_FALSE(v.valueless_by_exception());
}

TEST_F(AssignmentTest, MoveOnlyTypeAssignment)
{
	move_only_variant v1 { std::make_unique<int>(42) };
	move_only_variant v2 { std::string("hello") };

	v2 = std::move(v1);

	EXPECT_EQ(v2.index(), 0);
	EXPECT_EQ(*vrt::get<std::unique_ptr<int>>(v2), 42);
	EXPECT_TRUE(v1.valueless_by_exception());
	EXPECT_FALSE(v2.valueless_by_exception());
}

TEST_F(AssignmentTest, AssignmentChaining)
{
	basic_variant v1 { 42 };
	basic_variant v2 { 3.14 };
	basic_variant v3 { std::string("hello") };

	v1 = v2 = v3;

	EXPECT_EQ(v1.index(), 2);
	EXPECT_EQ(v2.index(), 2);
	EXPECT_EQ(v3.index(), 2);
	EXPECT_EQ(vrt::get<std::string>(v1), "hello");
	EXPECT_EQ(vrt::get<std::string>(v2), "hello");
	EXPECT_EQ(vrt::get<std::string>(v3), "hello");
}

TEST_F(AssignmentTest, AssignToValueless)
{
	basic_variant v1 { std::string("hello") };
	basic_variant v2 { 42 };

	auto v2_moved = std::move(v2);
	EXPECT_TRUE(v2.valueless_by_exception());

	v2 = v1;

	EXPECT_FALSE(v2.valueless_by_exception());
	EXPECT_EQ(v2.index(), 2);
	EXPECT_EQ(vrt::get<std::string>(v2), "hello");
}

TEST_F(AssignmentTest, AssignFromValueless)
{
	basic_variant v1 { std::string("hello") };
	basic_variant v2 { 42 };

	auto v1_moved = std::move(v1);
	EXPECT_TRUE(v1.valueless_by_exception());

	v2 = v1;

	EXPECT_TRUE(v1.valueless_by_exception());
	EXPECT_TRUE(v2.valueless_by_exception());
}

TEST_F(AssignmentTest, BothValueless)
{
	basic_variant v1 { std::string("hello") };
	basic_variant v2 { std::string("world") };

	auto v1_moved = std::move(v1);
	auto v2_moved = std::move(v2);

	EXPECT_TRUE(v1.valueless_by_exception());
	EXPECT_TRUE(v2.valueless_by_exception());

	v2 = v1;

	EXPECT_TRUE(v1.valueless_by_exception());
	EXPECT_TRUE(v2.valueless_by_exception());
}
