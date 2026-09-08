// nominal_test.cpp — Nominal (happy-path) functional tests.
//
// One representative value taken from the interior of each valid age band,
// well away from the boundaries, with a salary in the middle of its domain.
// Test names read as: R<req>_<scenario>_<expected outcome>.

#include <gtest/gtest.h>

#include "mortgage.hpp"

namespace {
constexpr int kSalary = 1000;  // interior point of the salary domain [0,10000]
}

// --- R4: male bands -------------------------------------------------------

TEST(NominalMale, R4_MaleAgeInsideYoungBand_AppliesFactor75) {
  EXPECT_EQ(mortgage(Gender::Male, 25, kSalary), 75 * kSalary);
}

TEST(NominalMale, R4_MaleAgeInsideMiddleBand_AppliesFactor55) {
  // Also covers B4: the defective code truncates this band at 39, so age 40
  // wrongly falls through to factor 30.
  EXPECT_EQ(mortgage(Gender::Male, 40, kSalary), 55 * kSalary);
}

TEST(NominalMale, R4_MaleAgeInsideSeniorBand_AppliesFactor30) {
  EXPECT_EQ(mortgage(Gender::Male, 50, kSalary), 30 * kSalary);
}

// --- R5: female bands -----------------------------------------------------

TEST(NominalFemale, R5_FemaleAgeInsideYoungBand_AppliesFactor70) {
  // Covers B7: the defective code copies the male factor 75 here.
  EXPECT_EQ(mortgage(Gender::Female, 25, kSalary), 70 * kSalary);
}

TEST(NominalFemale, R5_FemaleAgeInsideMiddleBand_AppliesFactor50) {
  EXPECT_EQ(mortgage(Gender::Female, 35, kSalary), 50 * kSalary);
}

TEST(NominalFemale, R5_FemaleAgeInsideSeniorBand_AppliesFactor35) {
  EXPECT_EQ(mortgage(Gender::Female, 45, kSalary), 35 * kSalary);
}

// --- R7: the arithmetic itself -------------------------------------------

TEST(NominalArithmetic, R7_MortgageIsSalaryTimesApplicableFactor) {
  EXPECT_EQ(mortgage(Gender::Male, 25, 2000), 150000);
  EXPECT_EQ(mortgage(Gender::Female, 45, 200), 7000);
}
