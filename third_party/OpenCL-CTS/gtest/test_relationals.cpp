#include <gtest/gtest.h>
#include "angle_oclcts_gtest.h"

class test_relationals : public testing::Test
{};

TEST_F(test_relationals, relational_any)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_relationals, relational_all)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_relationals, relational_bitselect)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_relationals, relational_select_signed)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_relationals, relational_select_unsigned)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_relationals, relational_isequal)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_relationals, relational_isnotequal)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_relationals, relational_isgreater)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_relationals, relational_isgreaterequal)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_relationals, relational_isless)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_relationals, relational_islessequal)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_relationals, relational_islessgreater)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_relationals, shuffle_copy)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_relationals, shuffle_function_call)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_relationals, shuffle_array_cast)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_relationals, shuffle_built_in)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_relationals, shuffle_built_in_dual_input)
{
    runTest(::testing::UnitTest::GetInstance());
}
