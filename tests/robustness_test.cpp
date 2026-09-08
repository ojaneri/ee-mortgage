// robustness_test.cpp — Mortgage Testing Lab
// Osvaldo Janeri Filho <janeri@gmail.com>
//
// WHAT ROBUSTNESS TESTING IS
//
// Boundary testing asks whether the edges of the valid domain are handled.
// Robustness testing asks what happens past them, and it is only satisfied by
// one answer: the function refuses. Returning something plausible for invalid
// input is worse than crashing, because a plausible wrong number gets used.
//
// Worst-case testing is the same idea with two inputs invalid at once instead
// of one. The last test in the file does that.
//
// This is where the broken implementation collapses. It has no rejection path
// of any kind — no throw, no error return, nothing — so eleven of these
// thirteen tests fail against it.
//
// A NOTE ON EXPECT_THROW
//
// EXPECT_THROW(expr, type) runs expr and passes only if it threw an exception
// of that type. If nothing is thrown, or something else is, it fails. It is
// the only assertion in the lab that checks for absence of an answer.

#include <gtest/gtest.h>

#include <climits>  // INT_MAX, INT_MIN
#include <string>

#include "mortgage.hpp"

namespace {
constexpr int kSalary = 1000;  // a valid salary, for tests about age
constexpr int kAge = 30;       // a valid age, for tests about salary

// A salary far outside the 0-10000 domain, but deliberately NOT INT_MAX.
//
// This used to be INT_MAX and it was a real mistake, caught by a sanitizer
// during review. The broken implementation multiplies before it validates
// anything, so INT_MAX * 35 is signed integer overflow — undefined behaviour,
// not the wrong answer this test is trying to assert on. A test whose failure
// mode is UB is not testing what it claims to.
//
// 20 million is still three orders of magnitude outside the domain, and
// 20000000 * 75 is about 1.5 billion, which still fits in a 32-bit int.
constexpr int kAbsurdSalary = 20'000'000;

// Runs mortgage() and returns the message of the out_of_range it threw, or an
// empty string if it did not throw at all.
//
// Why this exists: R6 makes every rejection the same exception type, so
// catching std::out_of_range tells you the input was refused but not why. An
// age of 60 and a woman of 53 both throw, for completely different reasons —
// one is outside the range the bank accepts, the other is inside it but falls
// in the hole R5 leaves in the women's table. A caller that wants to tell an
// applicant what went wrong needs to distinguish them, and so does anyone
// maintaining this code.
//
// A mutation run made the case concrete: widening the age guard from 55 to 56
// changed nothing any test could observe, because the defensive throw in
// male_factor() caught the overshoot and produced an out_of_range of its own.
// Right behaviour, wrong reason, and nothing noticed. The RejectionReason
// tests at the bottom of this file close that.
std::string rejection_reason(Gender gender, int age, int salary) {
  try {
    mortgage(gender, age, salary);
  } catch (const std::out_of_range& e) {
    return e.what();
  }
  return "";  // no exception at all
}

// Small readability helper: does haystack contain needle?
// std::string::find returns npos when it does not.
bool mentions(const std::string& haystack, const char* needle) {
  return haystack.find(needle) != std::string::npos;
}
}  // namespace

// ===========================================================================
// R2 and R6 — ages outside 18 to 55
// ===========================================================================

// One year below the minimum, both genders. The broken version answers 30000
// for the man and 35000 for the woman. Bug B5.
TEST(RobustnessAge, R2R6_AgeBelowMinimum_ThrowsOutOfRange) {
  EXPECT_THROW(mortgage(Gender::Male, 17, kSalary), std::out_of_range);
  EXPECT_THROW(mortgage(Gender::Female, 17, kSalary), std::out_of_range);
}

// One year above the maximum, both genders. Also bug B5.
TEST(RobustnessAge, R2R6_AgeAboveMaximum_ThrowsOutOfRange) {
  EXPECT_THROW(mortgage(Gender::Male, 56, kSalary), std::out_of_range);
  EXPECT_THROW(mortgage(Gender::Female, 56, kSalary), std::out_of_range);
}

// Values nobody would type on purpose: negative, zero, and the extremes of the
// int type.
//
// These are not realistic inputs and that is the point. A guard written as an
// explicit range check — `age < 18 || age > 55` — handles all four for free. A
// guard written as a chain of if/else that assumes the input is roughly
// sensible does not. The two look equally correct until you try this.
//
// Age is never multiplied by anything, so INT_MAX and INT_MIN are safe to pass
// here in a way they are not for salary. See kAbsurdSalary above.
TEST(RobustnessAge, R2R6_GrosslyInvalidAges_ThrowOutOfRange) {
  EXPECT_THROW(mortgage(Gender::Male, -1, kSalary), std::out_of_range);
  EXPECT_THROW(mortgage(Gender::Male, 0, kSalary), std::out_of_range);
  EXPECT_THROW(mortgage(Gender::Female, INT_MAX, kSalary), std::out_of_range);
  EXPECT_THROW(mortgage(Gender::Female, INT_MIN, kSalary), std::out_of_range);
}

// ===========================================================================
// R3 and R6 — salaries outside 0 to 10000
// ===========================================================================

// A negative salary is nonsense input, and the broken version's response to it
// is the clearest illustration of why R6 matters: it returns -75. A negative
// mortgage. That number is arithmetically consistent with everything else the
// function does, carries no error flag, and would travel a long way through a
// report before anyone stopped to ask what it meant. Bug B12.
TEST(RobustnessSalary, R3R6_NegativeSalary_ThrowsOutOfRange) {
  EXPECT_THROW(mortgage(Gender::Male, kAge, -1), std::out_of_range);
  EXPECT_THROW(mortgage(Gender::Female, kAge, -100000), std::out_of_range);
}

// One above the cap, and then absurdly above it. Also bug B12.
TEST(RobustnessSalary, R3R6_SalaryAboveMaximum_ThrowsOutOfRange) {
  EXPECT_THROW(mortgage(Gender::Male, kAge, 10001), std::out_of_range);
  EXPECT_THROW(mortgage(Gender::Female, kAge, kAbsurdSalary), std::out_of_range);
}

// ===========================================================================
// R5 and R6 — the hole in the women's table, ages 51 to 55
// ===========================================================================

// Every age in the gap, not just the first one. Sweeping the whole band costs
// nothing and rules out an implementation that handles 51 and forgets 55.
//
// The `<<` after the assertion adds a message that is only printed when that
// iteration fails, so the output names the specific age rather than just
// pointing at the loop.
//
// This is bug B11, and it is the most dangerous of the twelve precisely
// because it produces no visible symptom. A wrong factor can be caught by
// holding the output next to the table. An invented factor for a band the
// table never mentions looks entirely reasonable.
TEST(RobustnessSpecGap, R5R6_FemaleInUndefinedBand_ThrowsOutOfRange) {
  for (int age = 51; age <= 55; ++age) {
    EXPECT_THROW(mortgage(Gender::Female, age, kSalary), std::out_of_range)
        << "woman aged " << age << " should be refused, not priced";
  }
}

// The mirror image, and a guard against over-correcting. The gap belongs to
// the women's table only: ages 51 to 55 are perfectly valid for a man and
// return 30. Somebody fixing B11 by rejecting the whole 51-55 band for
// everybody would break this.
//
// Asserting the value rather than merely that it does not throw, so that a
// "fix" returning some default like zero does not slip past either.
//
// This is one of only two tests in the file that passes against the broken
// implementation — its catch-all returns 30 for men, which happens to be
// correct here.
TEST(RobustnessSpecGap, R5R6_MaleInSameAgeBandIsStillPriced) {
  for (int age = 51; age <= 55; ++age) {
    EXPECT_EQ(mortgage(Gender::Male, age, kSalary), 30 * kSalary)
        << "man aged " << age;
  }
}

// ===========================================================================
// Worst case — two invalid inputs in the same call
// ===========================================================================

// Everything above breaks one input at a time. This breaks two, which is the
// worst-case variant of robustness testing.
//
// The test does not care which of the two guards fires first. That ordering is
// not specified, and a test that asserted on it would be testing an
// implementation detail rather than a requirement.
TEST(RobustnessWorstCase, R6_AgeAndSalaryBothInvalid_ThrowsOutOfRange) {
  EXPECT_THROW(mortgage(Gender::Male, 17, -1), std::out_of_range);
  EXPECT_THROW(mortgage(Gender::Female, 99, 999999), std::out_of_range);
}

// ===========================================================================
// Why the rejection happened, not just that it did
// ===========================================================================
//
// See rejection_reason() at the top of the file for the reasoning. In short:
// one exception type for three different problems means the type cannot tell
// them apart, so the message has to.

// An age outside 18-55 must be reported as an age problem. Both directions,
// both genders, so a guard that only covers one end is caught.
TEST(RejectionReason, R2R6_AgeOutOfRangeSaysSo) {
  for (const auto& reason : {rejection_reason(Gender::Male, 56, kSalary),
                             rejection_reason(Gender::Female, 17, kSalary)}) {
    EXPECT_TRUE(mentions(reason, "age out of range")) << "got: " << reason;
  }
}

// A salary outside 0-10000 must be reported as a salary problem, not blamed on
// the age, which in both cases below is perfectly valid.
TEST(RejectionReason, R3R6_SalaryOutOfRangeSaysSo) {
  for (const auto& reason : {rejection_reason(Gender::Male, kAge, 10001),
                             rejection_reason(Gender::Male, kAge, -1)}) {
    EXPECT_TRUE(mentions(reason, "salary out of range")) << "got: " << reason;
  }
}

// The specification gap must NOT be reported as an out-of-range age.
//
// A woman of 53 is 53, which is inside the 18-55 range R2 allows. Telling her
// the age is out of range would be a false statement, and it would hide the
// real problem: the requirement is incomplete. The message has to say so, so
// that whoever reads the log goes and looks at R5 rather than at the applicant.
TEST(RejectionReason, R5R6_SpecGapIsNotReportedAsAnOutOfRangeAge) {
  const std::string reason = rejection_reason(Gender::Female, 53, kSalary);
  EXPECT_FALSE(mentions(reason, "age out of range")) << "got: " << reason;
  EXPECT_TRUE(mentions(reason, "specification")) << "got: " << reason;
}

// Control case for the three tests above.
//
// rejection_reason() returns "" when nothing was thrown. If it were broken and
// always returned some message, the three tests above would still pass while
// checking nothing at all. This pins the other end: valid input, no rejection.
TEST(RejectionReason, R6_ValidInputProducesNoRejection) {
  EXPECT_EQ(rejection_reason(Gender::Male, 30, kSalary), "");
  EXPECT_EQ(rejection_reason(Gender::Female, 30, kSalary), "");
}
