// robustness_test.cpp — Robustness testing (worst-case / invalid inputs).
//
// Robustness testing extends boundary value testing beyond the edges of the
// domain: min-, max+, and grossly invalid values. Per R6 every one of these
// must be rejected with std::out_of_range. The defective implementation has
// no rejection mechanism at all, so this whole file goes RED against it
// (B5, B11, B12).

#include <gtest/gtest.h>

#include <climits>

#include "mortgage.hpp"

namespace {
constexpr int kSalary = 1000;
constexpr int kAge = 30;
}

// --- R2 / R6: age below the domain ---------------------------------------

TEST(RobustnessAge, R2R6_AgeBelowMinimum_ThrowsOutOfRange) {
  // B5: defective code returns 30 * salary for a 17-year-old male.
  EXPECT_THROW(mortgage(Gender::Male, 17, kSalary), std::out_of_range);
  EXPECT_THROW(mortgage(Gender::Female, 17, kSalary), std::out_of_range);
}

TEST(RobustnessAge, R2R6_AgeAboveMaximum_ThrowsOutOfRange) {
  // B5: defective code returns 30 * salary for a 56-year-old male.
  EXPECT_THROW(mortgage(Gender::Male, 56, kSalary), std::out_of_range);
  EXPECT_THROW(mortgage(Gender::Female, 56, kSalary), std::out_of_range);
}

TEST(RobustnessAge, R2R6_GrosslyInvalidAges_ThrowOutOfRange) {
  // Worst-case values: negative, zero and INT_MAX.
  EXPECT_THROW(mortgage(Gender::Male, -1, kSalary), std::out_of_range);
  EXPECT_THROW(mortgage(Gender::Male, 0, kSalary), std::out_of_range);
  EXPECT_THROW(mortgage(Gender::Female, INT_MAX, kSalary), std::out_of_range);
  EXPECT_THROW(mortgage(Gender::Female, INT_MIN, kSalary), std::out_of_range);
}

// --- R3 / R6: salary outside the domain ----------------------------------

TEST(RobustnessSalary, R3R6_NegativeSalary_ThrowsOutOfRange) {
  // B12: defective code happily returns a negative mortgage.
  EXPECT_THROW(mortgage(Gender::Male, kAge, -1), std::out_of_range);
  EXPECT_THROW(mortgage(Gender::Female, kAge, -100000), std::out_of_range);
}

TEST(RobustnessSalary, R3R6_SalaryAboveMaximum_ThrowsOutOfRange) {
  // B12: defective code accepts any salary whatsoever.
  EXPECT_THROW(mortgage(Gender::Male, kAge, 10001), std::out_of_range);
  EXPECT_THROW(mortgage(Gender::Female, kAge, INT_MAX), std::out_of_range);
}

// --- R5 / R6: the female specification gap [51,55] ------------------------

TEST(RobustnessSpecGap, R5R6_FemaleInUndefinedBand_ThrowsOutOfRange) {
  // B11: R5 defines no factor for female ages 51..55; the defective code
  // invents factor 35 instead of surfacing the specification gap.
  for (int age = 51; age <= 55; ++age) {
    EXPECT_THROW(mortgage(Gender::Female, age, kSalary), std::out_of_range)
        << "female age " << age << " must be rejected (R5 specification gap)";
  }
}

TEST(RobustnessSpecGap, R5R6_MaleInSameAgeBandIsStillValid_NoThrow) {
  // The gap is gender-specific: males 51..55 remain valid with factor 30.
  for (int age = 51; age <= 55; ++age) {
    EXPECT_NO_THROW(mortgage(Gender::Male, age, kSalary));
    EXPECT_EQ(mortgage(Gender::Male, age, kSalary), 30 * kSalary);
  }
}

// --- Worst case: both inputs simultaneously invalid ------------------------

TEST(RobustnessWorstCase, R6_AgeAndSalaryBothInvalid_ThrowsOutOfRange) {
  EXPECT_THROW(mortgage(Gender::Male, 17, -1), std::out_of_range);
  EXPECT_THROW(mortgage(Gender::Female, 99, 999999), std::out_of_range);
}
