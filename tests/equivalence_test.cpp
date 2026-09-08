// equivalence_test.cpp — Mortgage Testing Lab
// Osvaldo Janeri Filho <janeri@gmail.com>
//
// WHAT EQUIVALENCE CLASS TESTING IS
//
// There are billions of possible inputs and you cannot try them all. But most
// of them are interchangeable: if the function is right for a man of 25, it is
// almost certainly right for a man of 26, because both take the same path
// through the code. Ages 18 to 35 form one equivalence class, and testing one
// member of a class stands in for testing all of them.
//
// The technique is: partition each input into classes, then choose which
// combinations to test. Two independent choices give four flavours, and all
// four are in this file.
//
//   WEAK vs STRONG
//     Weak takes one case per class INDEX. If age has three valid classes and
//     salary has one, you need three cases, and you cover class 1 of age with
//     class 1 of salary, class 2 with class 2, and so on — classes advance
//     together, in parallel. Cheap. It assumes the inputs do not interact.
//     Strong takes the CROSS PRODUCT: every class of every input against every
//     class of every other. More cases, no assumption.
//
//   NORMAL vs ROBUST
//     Normal stays inside the valid classes. Robust adds the invalid ones —
//     one at a time (single-fault assumption) or in combination.
//
// THE PARTITIONS FOR THIS FUNCTION
//
//   gender  G1 male, G2 female — both valid
//
//   age     men    A1 <18 invalid, A2 18-35, A3 36-45, A4 46-55,
//                  A5 >55 invalid                            (5 classes)
//           women  A1 <18 invalid, A2 18-30, A3 31-40, A4 41-50,
//                  A5 51-55 invalid (the spec gap), A6 >55 invalid
//                                                            (6 classes)
//
//   salary  S1 <0 invalid, S2 0-10000 valid, S3 >10000 invalid  (3 classes)
//
// Notice that the age partition depends on gender — the classes are at
// different ages and women have an extra invalid one in the middle. Age cannot
// be partitioned independently, which is exactly the kind of interaction that
// weak testing assumes away and strong testing does not.

#include <gtest/gtest.h>

#include <vector>

#include "mortgage.hpp"

namespace {

// One row of a table-driven test. Bundling the inputs with the expected output
// and a label keeps the cases readable as data: the test body below is one
// loop, and adding a case means adding a line, not another EXPECT.
struct ValidCase {
  Gender gender;
  int age;
  int salary;
  int expected;      // what mortgage() should return
  const char* label; // printed only if this row fails
};

// Same idea for rows that must be rejected. No expected value — the expected
// outcome is an exception.
struct InvalidCase {
  Gender gender;
  int age;
  int salary;
  const char* label;
};

constexpr int kSalary = 1000;

}  // namespace

// ===========================================================================
// WEAK NORMAL — one case per class index, valid classes only
// ===========================================================================
//
// Three cases, because the largest partition among the valid classes has three
// (A2, A3, A4). Gender and salary are varied alongside rather than held fixed:
// the whole point of "weak" is that class indices advance together instead of
// being combined exhaustively.
//
// Working through the expected values:
//   man 25, salary 0      -> A2, factor 75, and 75 * 0 = 0
//   woman 35, salary 5000 -> A3 female, factor 50, so 250000
//   man 50, salary 10000  -> A4 male, factor 30, so 300000
TEST(WeakNormalEquivalence, R4R5R7_OneCasePerValidAgeClassIndex) {
  const std::vector<ValidCase> cases = {
      {Gender::Male, 25, 0, 0, "G1 x A2(men) x S2 at its minimum"},
      {Gender::Female, 35, 5000, 250000, "G2 x A3(women) x S2 mid-range"},
      {Gender::Male, 50, 10000, 300000, "G1 x A4(men) x S2 at its maximum"},
  };
  for (const auto& c : cases) {
    EXPECT_EQ(mortgage(c.gender, c.age, c.salary), c.expected) << c.label;
  }
}

// ===========================================================================
// STRONG NORMAL — the full cross product of the valid classes
// ===========================================================================
//
// 2 genders x 3 valid age classes x 1 valid salary class = 6 cases. Salary has
// only one valid class, so it contributes a factor of one and the product is
// really just gender x age.
//
// Six cases is the entire valid input space at class granularity. Every
// gender/age combination the specification defines a factor for appears
// exactly once, so this is the table from include/mortgage.hpp written out as
// executable assertions.
//
// The ASSERT_EQ on the size is a guard against the list being edited down by
// accident. ASSERT rather than EXPECT because if the list is the wrong length
// there is no point running the loop. Note the `6u` — comparing a size_t to a
// signed 6 would draw a warning.
TEST(StrongNormalEquivalence, R4R5R7_CartesianProductOfValidClasses) {
  const std::vector<ValidCase> cases = {
      {Gender::Male, 25, kSalary, 75 * kSalary, "G1 x A2 18-35 -> 75"},
      {Gender::Male, 40, kSalary, 55 * kSalary, "G1 x A3 36-45 -> 55"},
      {Gender::Male, 50, kSalary, 30 * kSalary, "G1 x A4 46-55 -> 30"},
      {Gender::Female, 25, kSalary, 70 * kSalary, "G2 x A2 18-30 -> 70"},
      {Gender::Female, 35, kSalary, 50 * kSalary, "G2 x A3 31-40 -> 50"},
      {Gender::Female, 45, kSalary, 35 * kSalary, "G2 x A4 41-50 -> 35"},
  };
  ASSERT_EQ(cases.size(), 6u) << "the product is 2 x 3 x 1, so six cells";
  for (const auto& c : cases) {
    EXPECT_EQ(mortgage(c.gender, c.age, c.salary), c.expected) << c.label;
  }
}

// ===========================================================================
// WEAK ROBUST — one invalid class at a time
// ===========================================================================
//
// The single-fault assumption: real failures usually come from one thing being
// wrong, not two at once. Each case below pushes exactly one partition into an
// invalid class and leaves the rest valid, so when a case fails you know which
// guard is missing without having to work it out.
//
// Five cases, one per invalid class worth covering: age below range, age above
// range, the female spec gap, salary below range, salary above range.
TEST(WeakRobustEquivalence, R2R3R5R6_OneInvalidClassAtATime) {
  const std::vector<InvalidCase> cases = {
      {Gender::Male, 10, kSalary, "A1 men, under age"},
      {Gender::Male, 70, kSalary, "A5 men, over age"},
      {Gender::Female, 53, kSalary, "A5 women, the 51-55 spec gap"},
      {Gender::Male, 25, -50, "S1, negative salary"},
      {Gender::Female, 25, 20000, "S3, salary over the cap"},
  };
  for (const auto& c : cases) {
    EXPECT_THROW(mortgage(c.gender, c.age, c.salary), std::out_of_range)
        << c.label;
  }
}

// ===========================================================================
// STRONG ROBUST — invalid classes crossed with each other
// ===========================================================================
//
// Drop the single-fault assumption: every invalid age class against every
// invalid salary class. Five invalid age classes across the two genders (men
// have two, women have three counting the spec gap) times two invalid salary
// classes gives ten.
//
// In a function with entangled validation this is where you find that fixing
// one guard broke another. Here it mostly confirms the guards are independent,
// which is itself worth knowing.
TEST(StrongRobustEquivalence, R6_CartesianProductIncludingInvalidClasses) {
  const std::vector<InvalidCase> cases = {
      {Gender::Male, 10, -50, "A1 men x S1"},
      {Gender::Male, 10, 20000, "A1 men x S3"},
      {Gender::Male, 70, -50, "A5 men x S1"},
      {Gender::Male, 70, 20000, "A5 men x S3"},
      {Gender::Female, 10, -50, "A1 women x S1"},
      {Gender::Female, 10, 20000, "A1 women x S3"},
      {Gender::Female, 53, -50, "A5 women, spec gap x S1"},
      {Gender::Female, 53, 20000, "A5 women, spec gap x S3"},
      {Gender::Female, 70, -50, "A6 women x S1"},
      {Gender::Female, 70, 20000, "A6 women x S3"},
  };
  for (const auto& c : cases) {
    EXPECT_THROW(mortgage(c.gender, c.age, c.salary), std::out_of_range)
        << c.label;
  }
}

// ===========================================================================
// Checking the partition itself rather than samples from it
// ===========================================================================
//
// Everything above picks representatives and trusts that the classes are what
// the specification says. But a gap or an overlap in the implementation's
// bands can hide between the representatives — that is precisely how bug B3
// (bands 32-35 belonging to two classes) and bug B9 (age 31 belonging to none)
// stay invisible to sampling.
//
// So this walks every single age from 18 to 55 for both genders and checks two
// properties that must hold if the partition is sound:
//
//   1. Every age is answered exactly once — it either returns a factor or, for
//      women 51 to 55, is refused. Anything that throws where it should answer
//      means a gap.
//
//   2. The factor never goes UP as age increases. Both tables are monotonically
//      non-increasing, so a value that jumps back up means an age fell into the
//      wrong band. This is what catches a hole: an age that drops through to a
//      catch-all returning a factor from a later band shows up as a dip
//      followed by a rise.
//
// Salary is fixed at 1 so the return value IS the factor, which makes the
// comparison in property 2 direct.
//
// ASSERT_NO_THROW rather than EXPECT here because if an age unexpectedly
// throws, `factor` keeps its previous value and every subsequent comparison
// becomes meaningless — better to stop this iteration than to report a cascade
// of confusing failures. It also assigns inside the macro, which is why
// `factor` is declared just above it.
TEST(PartitionConsistency, R4R5_NoGapsOrOverlapsAcrossTheWholeAgeDomain) {
  for (Gender g : {Gender::Male, Gender::Female}) {
    const char* who = (g == Gender::Male) ? "man" : "woman";
    int previous = 1000;  // higher than any factor in either table

    for (int age = kMinAge; age <= kMaxAge; ++age) {
      // The one region where refusing is the correct answer.
      if (g == Gender::Female && age >= 51) {
        EXPECT_THROW(mortgage(g, age, 1), std::out_of_range)
            << who << " aged " << age << " falls in the spec gap";
        continue;
      }

      int factor = 0;
      ASSERT_NO_THROW(factor = mortgage(g, age, 1))
          << who << " aged " << age << " should be a valid input";

      EXPECT_LE(factor, previous)
          << "factor went back up at " << who << " aged " << age;
      previous = factor;
    }
  }
}
