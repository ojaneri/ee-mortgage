// boundary_test.cpp — Mortgage Testing Lab
// Osvaldo Janeri Filho <janeri@gmail.com>
//
// WHAT BOUNDARY VALUE TESTING IS
//
// Programmers get the middle of a range right and the ends wrong. Boundary
// value testing takes that seriously: instead of sampling the middle, it
// tests the first and last value of every range, and the values just outside.
//
// There are two kinds of boundary here and the second matters more.
//
// The OUTER boundaries are the edges of the domain: 18 and 55 for age, 0 and
// 10000 for salary. These are usually guarded by an explicit range check that
// somebody remembered to write.
//
// The INNER boundaries are the seams where one age band ends and the next
// begins: 35/36 and 45/46 for men, 30/31 and 40/41 for women. Nobody writes a
// range check for those. They are guarded by whichever comparison operator got
// typed, and `<` versus `<=` is a coin flip made under no scrutiny at all.
// Eight of the twelve planted bugs live on those seams.
//
// Every seam is tested from BOTH sides — 35 and 36, then 45 and 46 — because
// getting the last age of one band right says nothing about the first age of
// the next. The broken version manages to get both sides of the same seam
// wrong more than once.
//
// Expected values below assume a salary of 1000, so the expected result is the
// factor followed by three zeros.

#include <gtest/gtest.h>

#include "mortgage.hpp"

namespace {
constexpr int kSalary = 1000;
}  // namespace

// ===========================================================================
// R4 — the men's table: 18-35 -> 75, 36-45 -> 55, 46-55 -> 30
// ===========================================================================

// First value of the first band, and also the lowest age the bank accepts, so
// this one point is both an outer and an inner boundary.
//
// Broken version returns 30000. Its guard is `18 < age`, which is strict, so
// 18 misses the first band; it then misses the second as well (`31 < 18` is
// false) and lands in the catch-all. Bug B1.
TEST(BoundaryMale, R4_MaleAtLowerDomainEdge18_StillGetsFactor75) {
  EXPECT_EQ(mortgage(Gender::Male, 18, kSalary), 75 * kSalary);  // 75000
}

// Last value of the first band — the low side of the 35/36 seam.
//
// Broken version returns 55000, and the specific wrong number is informative.
// It is not the catch-all's 30000, so 35 did not simply fall through: the
// second band caught it. That proves the second band accepts ages below 36,
// which is bug B3, on top of bug B2 dropping 35 out of the first band.
TEST(BoundaryMale, R4_MaleAtUpperEdgeOfYoungBand35_StillGetsFactor75) {
  EXPECT_EQ(mortgage(Gender::Male, 35, kSalary), 75 * kSalary);  // 75000
}

// High side of the same seam: first value of the second band. Passes even
// against the broken version, which is why testing only one side of a seam is
// not enough.
TEST(BoundaryMale, R4_MaleJustInsideMiddleBandAt36_GetsFactor55) {
  EXPECT_EQ(mortgage(Gender::Male, 36, kSalary), 55 * kSalary);  // 55000
}

// Last value of the second band — low side of the 45/46 seam.
// Broken version returns 30000: its second band stops at 39, so 45 falls all
// the way through to the catch-all. Bug B4.
TEST(BoundaryMale, R4_MaleAtUpperEdgeOfMiddleBand45_StillGetsFactor55) {
  EXPECT_EQ(mortgage(Gender::Male, 45, kSalary), 55 * kSalary);  // 55000
}

// High side of the 45/46 seam. Passes against the broken version by accident:
// 46 lands in the catch-all, and the catch-all happens to return 30, which is
// the right answer for this age. Right result, wrong reason.
TEST(BoundaryMale, R4_MaleJustInsideSeniorBandAt46_GetsFactor30) {
  EXPECT_EQ(mortgage(Gender::Male, 46, kSalary), 30 * kSalary);  // 30000
}

// Last value of the last band, and the oldest age the bank accepts.
TEST(BoundaryMale, R4_MaleAtUpperDomainEdge55_GetsFactor30) {
  EXPECT_EQ(mortgage(Gender::Male, 55, kSalary), 30 * kSalary);  // 30000
}

// ===========================================================================
// R5 — the women's table: 18-30 -> 70, 31-40 -> 50, 41-50 -> 35,
//                         51-55 -> no factor exists
// ===========================================================================

// First value of the first band. Broken version returns 35000, the same B1
// mistake repeated in the female branch (bug B6), landing on the female
// catch-all instead of the male one.
TEST(BoundaryFemale, R5_FemaleAtLowerDomainEdge18_GetsFactor70) {
  EXPECT_EQ(mortgage(Gender::Female, 18, kSalary), 70 * kSalary);  // 70000
}

// Low side of the 30/31 seam. Broken version returns 35000 (bug B8).
//
// This seam is broken on both sides at once. Age 30 falls out of the first
// band because of `age < 30`, and age 31 never enters the second because of
// `31 < age`. So two consecutive ages, which belong to two different bands in
// the specification, both come back with the catch-all's 35.
TEST(BoundaryFemale, R5_FemaleAtUpperEdgeOfYoungBand30_StillGetsFactor70) {
  EXPECT_EQ(mortgage(Gender::Female, 30, kSalary), 70 * kSalary);  // 70000
}

// High side of the same seam. Broken version returns 35000 (bug B9).
TEST(BoundaryFemale, R5_FemaleAtLowerEdgeOfMiddleBand31_GetsFactor50) {
  EXPECT_EQ(mortgage(Gender::Female, 31, kSalary), 50 * kSalary);  // 50000
}

// Low side of the 40/41 seam. Broken version returns 35000 (bug B10).
TEST(BoundaryFemale, R5_FemaleAtUpperEdgeOfMiddleBand40_StillGetsFactor50) {
  EXPECT_EQ(mortgage(Gender::Female, 40, kSalary), 50 * kSalary);  // 50000
}

// High side of the 40/41 seam. Passes against the broken version by accident,
// the same way age 46 does for men.
TEST(BoundaryFemale, R5_FemaleAtLowerEdgeOfSeniorBand41_GetsFactor35) {
  EXPECT_EQ(mortgage(Gender::Female, 41, kSalary), 35 * kSalary);  // 35000
}

// Last age for which the women's table defines anything at all.
TEST(BoundaryFemale, R5_FemaleAtUpperEdgeOfSeniorBand50_GetsFactor35) {
  EXPECT_EQ(mortgage(Gender::Female, 50, kSalary), 35 * kSalary);  // 35000
}

// The other side of that last seam, and the strangest boundary in the lab.
//
// 50 and 51 are one year apart. Both are inside the age range R2 allows. For a
// man they are both ordinary lookups. For a woman, 50 returns 35 and 51 has no
// defined answer at all, because R5's table stops and R2's range does not.
//
// EXPECT_THROW(expr, type) runs expr and checks it threw an exception of that
// type. Here it asserts the function refuses rather than answers.
//
// Broken version returns 35000 — it extends the previous band silently. Bug
// B11, and the one that would survive longest unnoticed in production.
TEST(BoundaryFemale, R5_FemaleAtFirstAgeOfUndefinedBand51_IsRejected) {
  EXPECT_THROW(mortgage(Gender::Female, 51, kSalary), std::out_of_range);
}

// ===========================================================================
// R3 — the salary domain, 0 to 10000
// ===========================================================================

// The bottom edge. Zero is valid input, not an error: a mortgage of zero is
// the correct answer for someone earning nothing. An implementation that
// treated 0 as "missing" and rejected it would fail here, and should.
TEST(BoundarySalary, R3_SalaryAtLowerDomainEdgeZero_IsAcceptedAndYieldsZero) {
  EXPECT_EQ(mortgage(Gender::Male, 30, 0), 0);
}

// One above the bottom edge. Separates "accepts zero" from "accepts small
// positive numbers", which are different guards even though they look alike.
TEST(BoundarySalary, R3_SalaryJustAboveZero_IsAccepted) {
  EXPECT_EQ(mortgage(Gender::Male, 30, 1), 75);  // 1 * 75
}

// The top edge, and the largest result the function can ever produce:
// 10000 * 75 = 750000. Well inside a 32-bit int, so no overflow concern —
// which is worth having a test pin down rather than leaving as an assumption.
TEST(BoundarySalary, R3_SalaryAtUpperDomainEdge10000_IsAccepted) {
  EXPECT_EQ(mortgage(Gender::Male, 30, 10000), 750000);
}

// One below the top edge, with an awkward number so the arithmetic is actually
// exercised: 9999 * 35 = 349965.
TEST(BoundarySalary, R3_SalaryJustBelowUpperEdge9999_IsAccepted) {
  EXPECT_EQ(mortgage(Gender::Female, 45, 9999), 349965);
}

// ===========================================================================
// R2 — just outside the age domain
// ===========================================================================
//
// Strictly this is robustness territory rather than boundary testing, but 17
// and 56 are the immediate neighbours of 18 and 55 and belong next to them.
// robustness_test.cpp pushes much further out.

// Broken version returns 30000 for a 17-year-old. Bug B5.
TEST(BoundaryAge, R2_AgeJustBelowLowerDomainEdge17_IsRejected) {
  EXPECT_THROW(mortgage(Gender::Male, 17, kSalary), std::out_of_range);
}

// Broken version returns 30000 for a 56-year-old. Also B5.
TEST(BoundaryAge, R2_AgeJustAboveUpperDomainEdge56_IsRejected) {
  EXPECT_THROW(mortgage(Gender::Male, 56, kSalary), std::out_of_range);
}

// ===========================================================================
// Worst case — age boundaries crossed with salary boundaries
// ===========================================================================
//
// Everything above moves one input at a time and holds the other at a nominal
// value. Worst-case testing takes the extreme points of both inputs and
// combines them, which is where two guards that are each correct on their own
// can still interact badly.
//
// Added on branch fix/age-boundaries as the worked example of the team flow.
// Refs #1 (B1), #2 (B2).

// Age 18 is the low edge of the men's first band (bug B1 territory), crossed
// with both ends of the salary domain. 18 with salary 0 gives 0; 18 with
// salary 10000 gives the largest number this function can produce, 750000.
TEST(WorstCaseBoundary, R4R3_MaleAtBandEdge18WithSalaryDomainEdges_UsesFactor75) {
  EXPECT_EQ(mortgage(Gender::Male, 18, 0), 0);
  EXPECT_EQ(mortgage(Gender::Male, 18, 10000), 750000);
}

// The other end of the same band, age 35 (bug B2 and B3 territory), against
// the same two salaries.
TEST(WorstCaseBoundary, R4R3_MaleAtBandEdge35WithSalaryDomainEdges_UsesFactor75) {
  EXPECT_EQ(mortgage(Gender::Male, 35, 0), 0);
  EXPECT_EQ(mortgage(Gender::Male, 35, 10000), 750000);
}

// The women's 30/31 seam (bugs B8 and B9) against the salary edges.
TEST(WorstCaseBoundary, R5R3_FemaleAtBandEdge30WithSalaryDomainEdges_UsesFactor70) {
  EXPECT_EQ(mortgage(Gender::Female, 30, 0), 0);
  EXPECT_EQ(mortgage(Gender::Female, 30, 10000), 700000);
}

// An age outside the domain combined with a valid salary edge. The age guard
// must fire regardless of what the salary happens to be, including the two
// values most likely to be special-cased somewhere.
TEST(WorstCaseBoundary, R2R3R6_AgeEdgeOutsideDomainWithSalaryEdge_IsRejected) {
  EXPECT_THROW(mortgage(Gender::Male, 17, 10000), std::out_of_range);
  EXPECT_THROW(mortgage(Gender::Male, 56, 0), std::out_of_range);
}
