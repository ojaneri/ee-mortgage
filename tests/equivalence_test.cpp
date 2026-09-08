// equivalence_test.cpp — Equivalence class testing.
//
// Partitions (see requirements/mortgage_requirements.md):
//   Gender : G1 = Male                    G2 = Female
//   Age    : male   -> A1 (..17) invalid | A2 [18,35] | A3 [36,45] | A4 [46,55]
//                                        | A5 (56..)  invalid
//            female -> A1 (..17) invalid | A2 [18,30] | A3 [31,40] | A4 [41,50]
//                                        | A5 [51,55] invalid (spec gap)
//                                        | A6 (56..)  invalid
//   Salary : S1 (..-1) invalid | S2 [0,10000] valid | S3 (10001..) invalid
//
// Four flavours are exercised: weak normal, strong normal, weak robust and
// strong robust equivalence class testing.

#include <gtest/gtest.h>

#include <string>
#include <vector>

#include "mortgage.hpp"

namespace {

struct ValidCase {
  Gender gender;
  int age;
  int salary;
  int expected;
  const char* label;
};

struct InvalidCase {
  Gender gender;
  int age;
  int salary;
  const char* label;
};

constexpr int kSalary = 1000;  // representative of the valid salary class S2

}  // namespace

// ---------------------------------------------------------------------------
// WEAK NORMAL equivalence class testing
// One case per class index; classes are covered "in parallel", so the number
// of cases equals the largest number of classes in any single partition (3
// valid age classes per gender).
// ---------------------------------------------------------------------------
TEST(WeakNormalEquivalence, R4R5R7_OneCasePerValidAgeClassIndex) {
  const std::vector<ValidCase> cases = {
      {Gender::Male, 25, 0, 0, "G1 x A2(male) x S2(min)"},
      {Gender::Female, 35, 5000, 250000, "G2 x A3(female) x S2(mid)"},
      {Gender::Male, 50, 10000, 300000, "G1 x A4(male) x S2(max)"},
  };
  for (const auto& c : cases) {
    EXPECT_EQ(mortgage(c.gender, c.age, c.salary), c.expected) << c.label;
  }
}

// ---------------------------------------------------------------------------
// STRONG NORMAL equivalence class testing
// Cartesian product of the valid classes of every partition:
//   G (2) x valid age classes (3 per gender) x S2 (1) = 6 cases.
// ---------------------------------------------------------------------------
TEST(StrongNormalEquivalence, R4R5R7_CartesianProductOfValidClasses) {
  const std::vector<ValidCase> cases = {
      // Gender::Male x {A2, A3, A4} x S2
      {Gender::Male, 25, kSalary, 75 * kSalary, "G1 x A2[18,35] x S2 -> 75"},
      {Gender::Male, 40, kSalary, 55 * kSalary, "G1 x A3[36,45] x S2 -> 55"},
      {Gender::Male, 50, kSalary, 30 * kSalary, "G1 x A4[46,55] x S2 -> 30"},
      // Gender::Female x {A2, A3, A4} x S2
      {Gender::Female, 25, kSalary, 70 * kSalary, "G2 x A2[18,30] x S2 -> 70"},
      {Gender::Female, 35, kSalary, 50 * kSalary, "G2 x A3[31,40] x S2 -> 50"},
      {Gender::Female, 45, kSalary, 35 * kSalary, "G2 x A4[41,50] x S2 -> 35"},
  };
  ASSERT_EQ(cases.size(), 6u) << "strong normal ECT must cover 2 x 3 x 1 cells";
  for (const auto& c : cases) {
    EXPECT_EQ(mortgage(c.gender, c.age, c.salary), c.expected) << c.label;
  }
}

// ---------------------------------------------------------------------------
// WEAK ROBUST equivalence class testing
// Single-fault assumption: exactly one partition is driven into an invalid
// class per case, everything else stays in a valid class.
// ---------------------------------------------------------------------------
TEST(WeakRobustEquivalence, R2R3R5R6_OneInvalidClassAtATime) {
  const std::vector<InvalidCase> cases = {
      {Gender::Male, 10, kSalary, "A1(male, age<18) x S2"},
      {Gender::Male, 70, kSalary, "A5(male, age>55) x S2"},
      {Gender::Female, 53, kSalary, "A5(female, spec gap [51,55]) x S2"},
      {Gender::Male, 25, -50, "A2 x S1(salary<0)"},
      {Gender::Female, 25, 20000, "A2 x S3(salary>10000)"},
  };
  for (const auto& c : cases) {
    EXPECT_THROW(mortgage(c.gender, c.age, c.salary), std::out_of_range)
        << c.label;
  }
}

// ---------------------------------------------------------------------------
// STRONG ROBUST equivalence class testing
// Multiple-fault assumption: the Cartesian product that combines invalid age
// classes with invalid salary classes as well.
// ---------------------------------------------------------------------------
TEST(StrongRobustEquivalence, R6_CartesianProductIncludingInvalidClasses) {
  const std::vector<InvalidCase> cases = {
      {Gender::Male, 10, -50, "A1(male) x S1"},
      {Gender::Male, 10, 20000, "A1(male) x S3"},
      {Gender::Male, 70, -50, "A5(male) x S1"},
      {Gender::Male, 70, 20000, "A5(male) x S3"},
      {Gender::Female, 10, -50, "A1(female) x S1"},
      {Gender::Female, 10, 20000, "A1(female) x S3"},
      {Gender::Female, 53, -50, "A5(female spec gap) x S1"},
      {Gender::Female, 53, 20000, "A5(female spec gap) x S3"},
      {Gender::Female, 70, -50, "A6(female) x S1"},
      {Gender::Female, 70, 20000, "A6(female) x S3"},
  };
  for (const auto& c : cases) {
    EXPECT_THROW(mortgage(c.gender, c.age, c.salary), std::out_of_range)
        << c.label;
  }
}

// ---------------------------------------------------------------------------
// Partition consistency: over the whole valid age domain every accepted input
// must fall in exactly one band, so the factor sequence must be monotonically
// non-increasing with age and must never come from a hole in the partition.
// This catches gaps (B9) and overlaps (B3) that single points might miss.
// ---------------------------------------------------------------------------
TEST(PartitionConsistency, R4R5_NoGapsOrOverlapsAcrossTheWholeAgeDomain) {
  for (Gender g : {Gender::Male, Gender::Female}) {
    int previous_factor = 1000;  // higher than any specified factor
    for (int age = kMinAge; age <= kMaxAge; ++age) {
      if (g == Gender::Female && age >= 51) {
        EXPECT_THROW(mortgage(g, age, 1), std::out_of_range)
            << "female age " << age << " is a specification gap (R5)";
        continue;
      }
      int factor = 0;
      ASSERT_NO_THROW(factor = mortgage(g, age, 1))
          << (g == Gender::Male ? "male" : "female") << " age " << age
          << " must be accepted (R2)";
      EXPECT_LE(factor, previous_factor)
          << "factor must not increase with age at "
          << (g == Gender::Male ? "male" : "female") << " age " << age;
      previous_factor = factor;
    }
  }
}
