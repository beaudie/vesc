#include <gtest/gtest.h>
#include "angle_oclcts_gtest.h"

class test_atomics : public testing::Test
{};

TEST_F(test_atomics, atomic_add)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_atomics, atomic_sub)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_atomics, atomic_xchg)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_atomics, atomic_min)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_atomics, atomic_max)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_atomics, atomic_inc)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_atomics, atomic_dec)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_atomics, atomic_cmpxchg)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_atomics, atomic_and)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_atomics, atomic_or)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_atomics, atomic_xor)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_atomics, atomic_add_index)
{
    runTest(::testing::UnitTest::GetInstance());
}

TEST_F(test_atomics, atomic_add_index_bin)
{
    runTest(::testing::UnitTest::GetInstance());
}
