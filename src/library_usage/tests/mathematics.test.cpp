module;

#include <gtest/gtest.h>

module library_usage.mathematics:test;

import library_usage.mathematics;

namespace math
{
namespace
{

TEST(VectorAlgebra, DotProductIsCommutative)
{
    Vec<2> constexpr first_vector{2., 3.};
    Vec<2> const second_vector{4., 5.};

    EXPECT_EQ(first_vector.dot(second_vector), second_vector.dot(first_vector));
}

TEST(VectorAlgebra, ScalarDotProductIsAssociative)
{
    Vec<2> constexpr first_vector{2., 3.};
    Vec<2> const second_vector{4., 5.};
    auto constexpr scalar_multiplier = 3.0;

    EXPECT_EQ(scalar_multiplier * first_vector.dot(second_vector),
              (scalar_multiplier * first_vector).dot(second_vector));
}

} // namespace
} // namespace math
