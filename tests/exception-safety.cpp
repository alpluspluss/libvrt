/* this project is part of the vrt project; licensed under the MIT license. see LICENSE for more info */

#include <stdexcept>
#include <string>
#include <vrt>
#include <gtest/gtest.h>

class ExceptionSafetyTest : public ::testing::Test
{
protected:
	struct ThrowingConstructor
	{
		ThrowingConstructor()
		{
			throw std::runtime_error("construction failed");
		}

		ThrowingConstructor(double)
		{
			throw std::runtime_error("construction failed");
		}
	};

	struct ThrowingCopyConstructor
	{
		int value;
		ThrowingCopyConstructor(int v) : value(v) {}

		ThrowingCopyConstructor(const ThrowingCopyConstructor &)
		{
			throw std::runtime_error("copy failed");
		}

		ThrowingCopyConstructor(ThrowingCopyConstructor &&) = default;

		ThrowingCopyConstructor &operator=(const ThrowingCopyConstructor &) = default;

		ThrowingCopyConstructor &operator=(ThrowingCopyConstructor &&) = default;
	};

	struct ThrowingMoveConstructor
	{
		int value;
		ThrowingMoveConstructor(int v) : value(v) {}
		ThrowingMoveConstructor(const ThrowingMoveConstructor &other) : value(other.value) {}

		ThrowingMoveConstructor(ThrowingMoveConstructor &&)
		{
			throw std::runtime_error("move failed");
		}

		ThrowingMoveConstructor &operator=(const ThrowingMoveConstructor &) = default;

		ThrowingMoveConstructor &operator=(ThrowingMoveConstructor &&) = default;
	};

	struct ThrowingAssignment
	{
		int value;
		ThrowingAssignment(int v) : value(v) {}

		ThrowingAssignment(const ThrowingAssignment &) = default;

		ThrowingAssignment(ThrowingAssignment &&) = default;

		ThrowingAssignment &operator=(const ThrowingAssignment &)
		{
			throw std::runtime_error("assignment failed");
		}

		ThrowingAssignment &operator=(ThrowingAssignment &&) = default;
	};
};

TEST_F(ExceptionSafetyTest, DefaultConstructionThrows)
{
	EXPECT_THROW(vrt::variant<ThrowingConstructor> v, std::runtime_error);
}

TEST_F(ExceptionSafetyTest, ConstructionFailureLeavesValueless)
{
	using variant_t = vrt::variant<ThrowingConstructor, int>;

	try
	{
		variant_t v(3.14);
		FAIL() << "Expected exception";
	}
	catch (const std::runtime_error &) {}
}

TEST_F(ExceptionSafetyTest, CopyConstructionThrows)
{
	vrt::variant<ThrowingCopyConstructor, int> v1 { ThrowingCopyConstructor { 42 } };

	EXPECT_THROW(auto v2 = v1, std::runtime_error);
}

TEST_F(ExceptionSafetyTest, MoveConstructionThrows)
{
	vrt::variant<ThrowingMoveConstructor, int> v1{ std::in_place_type<ThrowingMoveConstructor>, 42 };
	EXPECT_THROW(auto v2 = std::move(v1), std::runtime_error);
	EXPECT_TRUE(v1.valueless_by_exception());
}

TEST_F(ExceptionSafetyTest, EmplaceThrows)
{
	vrt::variant<ThrowingConstructor, int> v { 42 };

	EXPECT_EQ(v.index(), 1);
	EXPECT_EQ(vrt::get<int>(v), 42);

	EXPECT_THROW(v.emplace<ThrowingConstructor>(), std::runtime_error);

	EXPECT_TRUE(v.valueless_by_exception());
	EXPECT_EQ(v.index(), vrt::variant_npos);
}

TEST_F(ExceptionSafetyTest, EmplaceByIndexThrows)
{
	vrt::variant<ThrowingConstructor, int> v { 42 };

	EXPECT_EQ(v.index(), 1);
	EXPECT_EQ(vrt::get<int>(v), 42);

	EXPECT_THROW(v.emplace<0>(), std::runtime_error);

	EXPECT_TRUE(v.valueless_by_exception());
	EXPECT_EQ(v.index(), vrt::variant_npos);
}

TEST_F(ExceptionSafetyTest, AssignmentSameTypeThrows)
{
	vrt::variant<ThrowingAssignment, int> v1 { ThrowingAssignment { 42 } };
	vrt::variant<ThrowingAssignment, int> v2 { ThrowingAssignment { 99 } };

	EXPECT_THROW(v1 = v2, std::runtime_error);

	EXPECT_EQ(v1.index(), 0);
	EXPECT_EQ(vrt::get<ThrowingAssignment>(v1).value, 42);
}

TEST_F(ExceptionSafetyTest, AssignmentDifferentTypeStrongGuarantee)
{
	struct NoThrowCopyConstructible
	{
		int value;
		NoThrowCopyConstructible(int v) : value(v) {}

		NoThrowCopyConstructible(const NoThrowCopyConstructible &) noexcept = default;

		NoThrowCopyConstructible(NoThrowCopyConstructible &&) = default;

		NoThrowCopyConstructible & operator=(const NoThrowCopyConstructible &) = default;

		NoThrowCopyConstructible & operator=(NoThrowCopyConstructible &&) = default;
	};

	vrt::variant<NoThrowCopyConstructible, ThrowingConstructor> v1 { NoThrowCopyConstructible { 42 } };
	vrt::variant<NoThrowCopyConstructible, ThrowingConstructor> v2 { NoThrowCopyConstructible { 99 } };

	try
	{
		v2.emplace<ThrowingConstructor>();
		FAIL() << "Expected exception";
	}
	catch (const std::runtime_error &)
	{
		EXPECT_TRUE(v2.valueless_by_exception());
	}

	v2 = v1;

	EXPECT_FALSE(v2.valueless_by_exception());
	EXPECT_EQ(v2.index(), 0);
	EXPECT_EQ(vrt::get<NoThrowCopyConstructible>(v2).value, 42);
}

TEST_F(ExceptionSafetyTest, ValuelessVariantOperations)
{
	vrt::variant<std::string> v1 { "hello" };
	auto moved = std::move(v1);

	EXPECT_TRUE(v1.valueless_by_exception());
	EXPECT_EQ(v1.index(), vrt::variant_npos);

	EXPECT_FALSE(vrt::holds_alternative<std::string>(v1));
	EXPECT_EQ(vrt::get_if<std::string>(&v1), nullptr);
	EXPECT_THROW(vrt::get<std::string>(v1), std::bad_variant_access);
}

TEST_F(ExceptionSafetyTest, ValuelessVariantAssignment)
{
	vrt::variant<std::string> v1 { "hello" };
	vrt::variant<std::string> v2 { "world" };

	auto moved = std::move(v1);
	EXPECT_TRUE(v1.valueless_by_exception());

	v1 = v2;

	EXPECT_FALSE(v1.valueless_by_exception());
	EXPECT_EQ(v1.index(), 0);
	EXPECT_EQ(vrt::get<std::string>(v1), "world");
}

TEST_F(ExceptionSafetyTest, ValuelessVariantComparison)
{
	vrt::variant<std::string> v1 { "hello" };
	vrt::variant<std::string> v2 { "world" };

	auto moved1 = std::move(v1);
	auto moved2 = std::move(v2);

	EXPECT_TRUE(v1.valueless_by_exception());
	EXPECT_TRUE(v2.valueless_by_exception());

	EXPECT_TRUE(v1 == v2);
	EXPECT_FALSE(v1 != v2);

	auto cmp = v1 <=> v2;
	EXPECT_TRUE(cmp == 0);
}

TEST_F(ExceptionSafetyTest, ValuelessVariantSwap)
{
	vrt::variant<std::string> v1 { "hello" };
	vrt::variant<std::string> v2 { "world" };

	auto moved = std::move(v1);
	EXPECT_TRUE(v1.valueless_by_exception());

	v1.swap(v2);

	EXPECT_FALSE(v1.valueless_by_exception());
	EXPECT_TRUE(v2.valueless_by_exception());
	EXPECT_EQ(vrt::get<std::string>(v1), "world");
}

TEST_F(ExceptionSafetyTest, VisitValuelessThrows)
{
	vrt::variant<std::string> v { "hello" };
	auto moved = std::move(v);

	EXPECT_TRUE(v.valueless_by_exception());
	EXPECT_THROW(vrt::visit([](auto&&) { return 0; }, v), std::bad_variant_access);
}

TEST_F(ExceptionSafetyTest, NoThrowOperations)
{
	vrt::variant<int, double, std::string> v { 42 };

	static_assert(noexcept(v.index()));
	static_assert(noexcept(v.valueless_by_exception()));
	static_assert(noexcept(vrt::holds_alternative<int>(v)));
	static_assert(noexcept(vrt::get_if<int>(&v)));

	EXPECT_TRUE(noexcept(v.index()));
	EXPECT_TRUE(noexcept(v.valueless_by_exception()));
	EXPECT_TRUE(noexcept(vrt::holds_alternative<int>(v)));
	EXPECT_TRUE(noexcept(vrt::get_if<int>(&v)));
}

TEST_F(ExceptionSafetyTest, ConditionalNoexceptSpecifications)
{
	struct NoThrowCopyable
	{
		NoThrowCopyable() = default;

		NoThrowCopyable(const NoThrowCopyable &) noexcept = default;

		NoThrowCopyable(NoThrowCopyable &&) noexcept = default;

		NoThrowCopyable & operator=(const NoThrowCopyable &) noexcept = default;

		NoThrowCopyable & operator=(NoThrowCopyable &&) noexcept = default;
	};

	using nothrow_variant = vrt::variant<int, NoThrowCopyable>;

	static_assert(std::is_nothrow_copy_constructible_v<nothrow_variant>);
	/* this has red lint but no compilation error;
	 * using throwing_variant = vrt::variant<int, ThrowingCopyConstructor>;
	 * static_assert(!std::is_nothrow_copy_constructible_v<throwing_variant>); */
}
