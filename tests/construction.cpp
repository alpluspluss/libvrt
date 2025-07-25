/* this project is part of the vrt project; licensed under the MIT license. see LICENSE for more info */

#include <memory>
#include <string>
#include <vector>
#include <vrt>
#include <gtest/gtest.h>

class ConstructionTest : public ::testing::Test
{
protected:
	using basic_variant = vrt::variant<int, double, std::string>;
	using move_only_variant = vrt::variant<std::unique_ptr<int>, std::string>;
};

TEST_F(ConstructionTest, DefaultConstruction)
{
	basic_variant v;
	EXPECT_EQ(v.index(), 0);
	EXPECT_TRUE(vrt::holds_alternative<int>(v));
	EXPECT_EQ(vrt::get<int>(v), 0);
	EXPECT_FALSE(v.valueless_by_exception());
}

TEST_F(ConstructionTest, ConvertingConstructionInt)
{
	basic_variant v { 42 };
	EXPECT_EQ(v.index(), 0);
	EXPECT_EQ(vrt::get<int>(v), 42);
	EXPECT_FALSE(v.valueless_by_exception());
}

TEST_F(ConstructionTest, ConvertingConstructionDouble)
{
	basic_variant v { 3.14 };
	EXPECT_EQ(v.index(), 1);
	EXPECT_DOUBLE_EQ(vrt::get<double>(v), 3.14);
	EXPECT_FALSE(v.valueless_by_exception());
}

TEST_F(ConstructionTest, ConvertingConstructionString)
{
	basic_variant v { std::string("hello") };
	EXPECT_EQ(v.index(), 2);
	EXPECT_EQ(vrt::get<std::string>(v), "hello");
	EXPECT_FALSE(v.valueless_by_exception());
}

TEST_F(ConstructionTest, ConvertingConstructionStringLiteral)
{
	basic_variant v { "hello" };
	EXPECT_EQ(v.index(), 2);
	EXPECT_EQ(vrt::get<std::string>(v), "hello");
	EXPECT_FALSE(v.valueless_by_exception());
}

TEST_F(ConstructionTest, InPlaceConstructionByType)
{
	vrt::variant<int, std::string, std::vector<int> > v { std::in_place_type<std::string>, "constructed" };
	EXPECT_EQ(v.index(), 1);
	EXPECT_EQ(vrt::get<std::string>(v), "constructed");
	EXPECT_FALSE(v.valueless_by_exception());
}

TEST_F(ConstructionTest, InPlaceConstructionByIndex)
{
	vrt::variant<int, std::string, std::vector<int> > v { std::in_place_index<2>, 3, 42 };
	EXPECT_EQ(v.index(), 2);
	EXPECT_EQ(vrt::get<std::vector<int>>(v).size(), 3);
	EXPECT_EQ(vrt::get<std::vector<int>>(v)[0], 42);
	EXPECT_FALSE(v.valueless_by_exception());
}

TEST_F(ConstructionTest, CopyConstruction)
{
	basic_variant v1 { std::string("original") };
	basic_variant v2 { v1 };

	EXPECT_EQ(v2.index(), v1.index());
	EXPECT_EQ(vrt::get<std::string>(v2), "original");
	EXPECT_EQ(vrt::get<std::string>(v1), "original");
	EXPECT_FALSE(v1.valueless_by_exception());
	EXPECT_FALSE(v2.valueless_by_exception());
}

TEST_F(ConstructionTest, MoveConstruction)
{
	basic_variant v1 { std::string("original") };
	basic_variant v2 { std::move(v1) };

	EXPECT_EQ(v2.index(), 2);
	EXPECT_EQ(vrt::get<std::string>(v2), "original");
	EXPECT_TRUE(v1.valueless_by_exception());
	EXPECT_FALSE(v2.valueless_by_exception());
}

TEST_F(ConstructionTest, MoveOnlyTypeConstruction)
{
	move_only_variant v { std::make_unique<int>(42) };
	EXPECT_EQ(v.index(), 0);
	EXPECT_EQ(*vrt::get<std::unique_ptr<int>>(v), 42);
	EXPECT_FALSE(v.valueless_by_exception());
}

TEST_F(ConstructionTest, NonDefaultConstructibleFirstType)
{
	struct NonDefault
	{
		int value;

		NonDefault() = delete;

		explicit NonDefault(int v) : value(v) {}
	};

	using variant_t = vrt::variant<NonDefault, int>;
	variant_t v { NonDefault { 42 } };

	EXPECT_EQ(v.index(), 0);
	EXPECT_EQ(vrt::get<NonDefault>(v).value, 42);
	EXPECT_FALSE(v.valueless_by_exception());
}

TEST_F(ConstructionTest, MonostateDefaultConstruction)
{
	struct NonDefault
	{
		int value;

		NonDefault() = delete;

		explicit NonDefault(int v) : value(v) {}
	};

	vrt::variant<vrt::monostate, NonDefault> v;
	EXPECT_EQ(v.index(), 0);
	EXPECT_TRUE(vrt::holds_alternative<vrt::monostate>(v));
	EXPECT_FALSE(v.valueless_by_exception());
}

TEST_F(ConstructionTest, PerfectForwarding)
{
	struct MoveTracker
	{
		bool was_moved_from = false;
		bool was_moved_to = false;

		MoveTracker() = default;

		MoveTracker(const MoveTracker &) = default;

		MoveTracker(MoveTracker &&other) noexcept : was_moved_to(true)
		{
			other.was_moved_from = true;
		}
	};

	using variant_t = vrt::variant<MoveTracker, int>;
	MoveTracker tracker;
	variant_t v { std::move(tracker) };

	EXPECT_TRUE(tracker.was_moved_from);
	EXPECT_TRUE(vrt::get<MoveTracker>(v).was_moved_to);
	EXPECT_FALSE(v.valueless_by_exception());
}

TEST_F(ConstructionTest, LargeVariant)
{
	using large_variant = vrt::variant<
		int, double, float, char, short, long,
		std::string, std::vector<int>, bool,
		std::unique_ptr<int>
	>;

	large_variant v { 42 };
	EXPECT_EQ(v.index(), 0);
	EXPECT_EQ(vrt::get<int>(v), 42);
	EXPECT_FALSE(v.valueless_by_exception());
}
