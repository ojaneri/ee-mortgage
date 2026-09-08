// mortgage.cpp — Mortgage Testing Lab
// Osvaldo Janeri Filho <janeri@gmail.com>
//
// The correct implementation. Read include/mortgage.hpp first — it explains
// what the function is for and what the factors 75, 55, 30, 70, 50 and 35
// actually mean.
//
// The structure here is deliberate and worth following, because it is the
// direct answer to how the buggy version goes wrong:
//
//   1. mortgage() validates everything up front and rejects bad input.
//   2. Only then does it pick a lookup table, based on gender.
//   3. The table returns a factor, or throws if the specification has no
//      answer for that age.
//   4. mortgage() multiplies and returns.
//
// The buggy version collapses all four steps into one chain of if/else, which
// is exactly why it has twelve defects and this has none.

#include "mortgage.hpp"

#include <stdexcept>  // std::out_of_range
#include <string>     // std::to_string, for building the error messages

// An unnamed namespace. Everything inside it has internal linkage, meaning it
// is private to this translation unit and cannot be called or linked against
// from anywhere else. These two helpers are implementation detail; only
// mortgage() itself is part of the public contract.
namespace {

// Looks up the factor for a male applicant, R4.
//
// Called only after mortgage() has already checked that age is between 18 and
// 55, so in practice one of the three ranges below always matches.
//
// Every comparison is <= or >=, never < or >. That is the whole ballgame. The
// bands in R4 are closed intervals — 18 to 35 *includes* both 18 and 35 — and
// writing `age < 35` instead of `age <= 35` silently drops the last year of
// the band. The buggy version makes exactly this mistake four separate times
// (bugs B1, B2, B6 and B8).
//
// The three ranges are also contiguous: the first ends at 35 and the second
// starts at 36, with nothing in between and no year belonging to two bands.
// A gap would mean some age falls through to the bottom of the function; an
// overlap would mean the answer depends on the order the ifs happen to be
// written in.
int male_factor(int age) {
  if (age >= 18 && age <= 35) return 75;  // 18-35: lend 75x the monthly salary
  if (age >= 36 && age <= 45) return 55;  // 36-45: 55x
  if (age >= 46 && age <= 55) return 30;  // 46-55: 30x

  // Unreachable as things stand, because mortgage() rejects anything outside
  // 18-55 before calling this. It is here so that the function can never fall
  // off the end and return garbage: if someone later reorders the checks in
  // mortgage(), or widens the age range without extending this table, the
  // result is a loud exception rather than a silent wrong number.
  //
  // (A C++ function that reaches its closing brace without returning a value
  // is undefined behaviour, not a compile error. This throw removes that
  // possibility entirely.)
  throw std::out_of_range("no male factor defined for age " +
                          std::to_string(age));
}

// Looks up the factor for a female applicant, R5.
//
// Same shape as male_factor, with two differences: the band edges are at
// different ages, and the table genuinely runs out before the age range does.
int female_factor(int age) {
  if (age >= 18 && age <= 30) return 70;  // 18-30: lend 70x the monthly salary
  if (age >= 31 && age <= 40) return 50;  // 31-40: 50x
  if (age >= 41 && age <= 50) return 35;  // 41-50: 35x

  // Ages 51 to 55 land here, and unlike the male version this is a path that
  // really does get taken.
  //
  // R2 says the valid age range runs to 55. R5's table stops at 50. So a woman
  // of 53 is a legitimate applicant for whom the specification provides no
  // factor. That is a hole in the requirement, not in this code.
  //
  // Three things could be done about it: guess a number, return some default
  // like zero, or refuse. This refuses, because the other two hide the problem.
  // A guessed factor looks like a real answer and nobody ever finds out the
  // specification was incomplete — which is precisely what bug B11 does, by
  // quietly handing back 35.
  //
  // The message says "specification" rather than "out of range" on purpose:
  // the age is not out of range, the table is. A test asserts on that wording
  // (RejectionReason.R5R6_SpecGapIsNotReportedAsAnOutOfRangeAge) so the
  // distinction cannot be lost in a later edit.
  throw std::out_of_range(
      "specification defines no mortgage factor for a woman aged " +
      std::to_string(age));
}

}  // namespace

// The public entry point. See the contract in include/mortgage.hpp.
int mortgage(Gender gender, int age, int salary) {
  // Step 1 — validate the age, R2 and R6.
  //
  // This runs before anything else touches the inputs. Validating first is not
  // a style preference: the buggy version multiplies the salary before it has
  // checked anything, which is how a robustness test feeding it a huge number
  // ended up triggering signed integer overflow instead of the wrong answer we
  // were trying to assert on. See requirements/test_evidence.md.
  if (age < kMinAge || age > kMaxAge) {
    throw std::out_of_range("age out of range [18, 55]: " +
                            std::to_string(age));
  }

  // Step 2 — validate the salary, R3 and R6.
  //
  // Note that 0 passes. A salary of zero is a valid input that produces a
  // mortgage of zero, which is a correct answer and not an error condition.
  if (salary < kMinSalary || salary > kMaxSalary) {
    throw std::out_of_range("salary out of range [0, 10000]: " +
                            std::to_string(salary));
  }

  // Step 3 — pick the lookup table and get the factor.
  //
  // A switch rather than `gender == Gender::Male ? male_factor : female_factor`.
  // The ternary reads more compactly but has a flaw: it treats *everything*
  // that is not Male as Female. A scoped enum can hold any value its underlying
  // type can hold, and `static_cast<Gender>(7)` at a call site is all it takes
  // to produce one. With the ternary, that nonsense value would quietly get a
  // woman's mortgage.
  //
  // That is the same failure shape as bugs B5 and B11 — a catch-all branch that
  // answers when it should refuse — so it would be a poor look in the file
  // whose job is to demonstrate the fix. The switch names both real cases and
  // throws on anything else.
  int factor = 0;
  switch (gender) {
    case Gender::Male:
      factor = male_factor(age);
      break;
    case Gender::Female:
      factor = female_factor(age);
      break;
    default:
      throw std::out_of_range("unknown gender value: " +
                              std::to_string(static_cast<int>(gender)));
  }

  // Step 4 — apply R7 and return.
  //
  // No overflow check is needed here, and that is a claim worth justifying
  // rather than assuming: salary is at most 10000 (checked in step 2) and the
  // largest factor in either table is 75, so the product cannot exceed 750000.
  // A 32-bit int holds up to 2147483647. The margin is roughly 2800x.
  return salary * factor;
}
