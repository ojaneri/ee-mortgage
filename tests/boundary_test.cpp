// boundary_test.cpp — Boundary value testing.
//
// Exercises min, min+, max-, max of every valid partition and, crucially, the
// *internal* edges between adjacent age bands, where the seeded off-by-one
// defects (B1, B2, B3, B4, B6, B8, B9, B10) live.

#include <gtest/gtest.h>

#include "mortgage.hpp"

namespace {
constexpr int kSalary = 1000;
}

// --- R4 male: [18,35] -> 75 ----------------------------------------------

TEST(BoundaryMale, R4_MaleAtLowerDomainEdge18_StillGetsFactor75) {
  // B1: defective code uses `18 < age`, dropping 18 to the catch-all (30).
  EXPECT_EQ(mortgage(Gender::Male, 18, kSalary), 75 * kSalary);
}

TEST(BoundaryMale, R4_MaleAtUpperEdgeOfYoungBand35_StillGetsFactor75) {
  // B2 + B3: defective code uses `age < 35` and lets the next band (which
  // wrongly starts at 32) capture age 35, yielding factor 55.
  EXPECT_EQ(mortgage(Gender::Male, 35, kSalary), 75 * kSalary);
}

TEST(BoundaryMale, R4_MaleJustInsideMiddleBandAt36_GetsFactor55) {
  EXPECT_EQ(mortgage(Gender::Male, 36, kSalary), 55 * kSalary);
}

TEST(BoundaryMale, R4_MaleAtUpperEdgeOfMiddleBand45_StillGetsFactor55) {
  // B4: defective code truncates the band at 39, so 45 gets factor 30.
  EXPECT_EQ(mortgage(Gender::Male, 45, kSalary), 55 * kSalary);
}

TEST(BoundaryMale, R4_MaleJustInsideSeniorBandAt46_GetsFactor30) {
  EXPECT_EQ(mortgage(Gender::Male, 46, kSalary), 30 * kSalary);
}

TEST(BoundaryMale, R4_MaleAtUpperDomainEdge55_GetsFactor30) {
  EXPECT_EQ(mortgage(Gender::Male, 55, kSalary), 30 * kSalary);
}

// --- R5 female: [18,30] -> 70, [31,40] -> 50, [41,50] -> 35 ---------------

TEST(BoundaryFemale, R5_FemaleAtLowerDomainEdge18_GetsFactor70) {
  // B6 (+ B7): defective code drops 18 to the catch-all (35).
  EXPECT_EQ(mortgage(Gender::Female, 18, kSalary), 70 * kSalary);
}

TEST(BoundaryFemale, R5_FemaleAtUpperEdgeOfYoungBand30_StillGetsFactor70) {
  // B8: defective code uses `age < 30`, so 30 falls to the catch-all (35).
  EXPECT_EQ(mortgage(Gender::Female, 30, kSalary), 70 * kSalary);
}

TEST(BoundaryFemale, R5_FemaleAtLowerEdgeOfMiddleBand31_GetsFactor50) {
  // B9: defective code uses `31 < age`, leaving a gap exactly at 31.
  EXPECT_EQ(mortgage(Gender::Female, 31, kSalary), 50 * kSalary);
}

TEST(BoundaryFemale, R5_FemaleAtUpperEdgeOfMiddleBand40_StillGetsFactor50) {
  // B10: defective code uses `age < 40`, so 40 falls to the catch-all (35).
  EXPECT_EQ(mortgage(Gender::Female, 40, kSalary), 50 * kSalary);
}

TEST(BoundaryFemale, R5_FemaleAtLowerEdgeOfSeniorBand41_GetsFactor35) {
  EXPECT_EQ(mortgage(Gender::Female, 41, kSalary), 35 * kSalary);
}

TEST(BoundaryFemale, R5_FemaleAtUpperEdgeOfSeniorBand50_GetsFactor35) {
  EXPECT_EQ(mortgage(Gender::Female, 50, kSalary), 35 * kSalary);
}

TEST(BoundaryFemale, R5_FemaleAtFirstAgeOfUndefinedBand51_IsRejected) {
  // B11: the [51,55] specification gap must not be silently priced.
  EXPECT_THROW(mortgage(Gender::Female, 51, kSalary), std::out_of_range);
}

// --- R3 salary domain boundaries -----------------------------------------

TEST(BoundarySalary, R3_SalaryAtLowerDomainEdgeZero_IsAcceptedAndYieldsZero) {
  EXPECT_EQ(mortgage(Gender::Male, 30, 0), 0);
}

TEST(BoundarySalary, R3_SalaryJustAboveZero_IsAccepted) {
  EXPECT_EQ(mortgage(Gender::Male, 30, 1), 75);
}

TEST(BoundarySalary, R3_SalaryAtUpperDomainEdge10000_IsAccepted) {
  EXPECT_EQ(mortgage(Gender::Male, 30, 10000), 750000);
}

TEST(BoundarySalary, R3_SalaryJustBelowUpperEdge9999_IsAccepted) {
  EXPECT_EQ(mortgage(Gender::Female, 45, 9999), 349965);
}

// --- R2 age domain boundaries --------------------------------------------

TEST(BoundaryAge, R2_AgeJustBelowLowerDomainEdge17_IsRejected) {
  EXPECT_THROW(mortgage(Gender::Male, 17, kSalary), std::out_of_range);
}

TEST(BoundaryAge, R2_AgeJustAboveUpperDomainEdge56_IsRejected) {
  EXPECT_THROW(mortgage(Gender::Male, 56, kSalary), std::out_of_range);
}
