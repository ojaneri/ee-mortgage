// nominal_test.cpp — Mortgage Testing Lab
// Osvaldo Janeri Filho <janeri@gmail.com>
//
// WHAT NOMINAL TESTING IS
//
// Pick one value from the middle of each valid range and check the answer.
// No edges, no invalid input, nothing clever. It answers one question: does
// the function work at all for ordinary input?
//
// It is the weakest of the five techniques in this lab and it is here to make
// that point. Five of the seven tests below still pass against the broken
// implementation, because a bug that only shows up at a band edge is invisible
// to a test that stays away from the edges. That is the argument for
// boundary_test.cpp.
//
// HOW TO READ A TEST BELOW
//
// EXPECT_EQ(a, b) checks that a equals b and, if not, records a failure and
// carries on with the rest of the test. (ASSERT_EQ would stop the test dead
// instead; EXPECT is right here because each line is independent.)
//
// The names are Suite.R<requirement>_<situation>_<expected result>, so a
// failure line in the output tells you which requirement broke without having
// to open this file.

#include <gtest/gtest.h>

#include "mortgage.hpp"

namespace {
// A round, unremarkable salary. 1000 is convenient because the expected value
// is just the factor with three zeros after it, so a wrong answer in the test
// output is readable at a glance: 75000 is right, 55000 came from the wrong
// band, 30000 fell through to the catch-all.
constexpr int kSalary = 1000;
}  // namespace

// R4, men aged 18-35, factor 75.
// Age 25 sits in the middle of that band with 7 years of slack on either side,
// so no off-by-one at either edge can reach it. This test passes against the
// broken implementation, which is the point.
TEST(NominalMale, R4_MaleAgeInsideYoungBand_AppliesFactor75) {
  EXPECT_EQ(mortgage(Gender::Male, 25, kSalary), 75 * kSalary);  // 75000
}

// R4, men aged 36-45, factor 55.
// Age 40 is four years inside the band on the left and five on the right, so
// this should be a comfortable nominal case. It is not: the broken version
// stops the band at 39 (bug B4), so 40 falls through to the catch-all and
// comes back as 30000. One of the two nominal tests that catches anything.
TEST(NominalMale, R4_MaleAgeInsideMiddleBand_AppliesFactor55) {
  EXPECT_EQ(mortgage(Gender::Male, 40, kSalary), 55 * kSalary);  // 55000
}

// R4, men aged 46-55, factor 30.
// Passes against the broken version too, but for a bad reason worth noticing:
// there, 30 is what the catch-all returns for everything it swallows. The
// right answer arriving by the wrong route still counts as a pass, which is
// exactly why a green test tells you so little on its own.
TEST(NominalMale, R4_MaleAgeInsideSeniorBand_AppliesFactor30) {
  EXPECT_EQ(mortgage(Gender::Male, 50, kSalary), 30 * kSalary);  // 30000
}

// R5, women aged 18-30, factor 70.
// The other nominal test that catches something. The broken version returns 75
// here (bug B7) because the male branch was copied and the constant was never
// changed. Note that this is not an edge case at all — age 25 is nowhere near
// a boundary — so it is the one bug in the twelve that plain nominal testing
// was always going to find.
TEST(NominalFemale, R5_FemaleAgeInsideYoungBand_AppliesFactor70) {
  EXPECT_EQ(mortgage(Gender::Female, 25, kSalary), 70 * kSalary);  // 70000
}

// R5, women aged 31-40, factor 50.
TEST(NominalFemale, R5_FemaleAgeInsideMiddleBand_AppliesFactor50) {
  EXPECT_EQ(mortgage(Gender::Female, 35, kSalary), 50 * kSalary);  // 50000
}

// R5, women aged 41-50, factor 35.
// Compare with the same age for a man: 45 is in the men's 36-45 band, so he
// would get 55 and she gets 35. The two tables genuinely differ.
TEST(NominalFemale, R5_FemaleAgeInsideSeniorBand_AppliesFactor35) {
  EXPECT_EQ(mortgage(Gender::Female, 45, kSalary), 35 * kSalary);  // 35000
}

// R7, the multiplication itself.
//
// Every test above uses the same salary of 1000, which means an implementation
// that ignored salary entirely and returned a hardcoded 75000, 55000 and so on
// would pass all six. This one varies the salary so that cannot happen:
//   man aged 25, salary 2000  -> factor 75, so 150000
//   woman aged 45, salary 200 -> factor 35, so 7000
TEST(NominalArithmetic, R7_MortgageIsSalaryTimesApplicableFactor) {
  EXPECT_EQ(mortgage(Gender::Male, 25, 2000), 150000);
  EXPECT_EQ(mortgage(Gender::Female, 45, 200), 7000);
}
