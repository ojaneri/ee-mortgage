// mortgage.hpp — Mortgage Testing Lab
// Osvaldo Janeri Filho <janeri@gmail.com>
//
// This header is the contract. It declares one function and the constants that
// bound its inputs, and nothing else. Both implementations in src/ satisfy this
// same contract, so the tests include this file and never mention which .cpp
// they are actually running against — that choice is made at link time by the
// USE_BUGGY option in CMakeLists.txt.
//
// WHAT THE FUNCTION IS FOR
//
// A bank wants to know how large a mortgage an applicant can take on. It does
// not do a full affordability calculation; it uses a lookup table. The
// applicant's gender and age select a multiplier, and the mortgage is that
// multiplier times the monthly salary:
//
//     mortgage = salary * factor
//
// So a factor of 75 means "we will lend this person seventy-five times what
// they earn in a month". A 25-year-old man earning 1000 gets 75 * 1000 =
// 75000.
//
// WHAT THE NUMBERS 75, 55, 35, 30 AND 70, 50 MEAN
//
// They are those multipliers, and they only ever go down as the applicant gets
// older. The specification does not say why, but the shape is not mysterious:
// somebody younger has more working years left to pay the loan back, so the
// bank is willing to lend them a larger multiple of their salary. A man of 25
// gets 75x, a man of 50 gets 30x — the same salary, less than half the
// mortgage, purely because of the years remaining.
//
// The men's and women's tables are different and do not line up. The bands are
// at different ages (men change at 36 and 46, women at 31 and 41) and the
// multipliers differ too. Again the specification does not justify this; the
// lab takes the table as given, because arguing with the requirement is not the
// exercise. Reproducing it exactly is.

#ifndef MORTGAGE_HPP
#define MORTGAGE_HPP

// <stdexcept> gives us std::out_of_range. It is included here rather than in
// the .cpp files because it is part of the contract: callers need the type in
// scope to write `catch (const std::out_of_range&)`.
#include <stdexcept>

// The gender input, R1.
//
// `enum class` rather than a plain `enum` or a `bool`. A bool would work, but
// `mortgage(true, 30, 1000)` at a call site tells the reader nothing — true
// meaning what? — whereas `mortgage(Gender::Male, 30, 1000)` is self-evident.
// The `class` part keeps the names scoped (you must write `Gender::Male`, not
// bare `Male`) and stops the values silently converting to int.
//
// Note the buggy implementation predates this and takes a bare `bool male`.
// See the adapter at the bottom of src/mortgage_buggy.cpp.
enum class Gender { Male, Female };

// The domain limits, R2 and R3. They live in the header rather than being
// written as literals inside the function for two reasons: the tests assert
// against the same constants the implementation uses, and a reader looking for
// "where does 55 come from" finds it in one place.
//
// `constexpr` means these are compile-time constants — no storage, no runtime
// lookup, and usable in contexts that require a constant expression.
constexpr int kMinAge = 18;      // youngest applicant the bank will consider
constexpr int kMaxAge = 55;      // oldest applicant the bank will consider
constexpr int kMinSalary = 0;    // zero is allowed; it just yields a zero mortgage
constexpr int kMaxSalary = 10000;  // the cap above which the table stops applying

// Works out the mortgage for one applicant.
//
// Parameters:
//   gender  which of the two lookup tables to use (R1)
//   age     the applicant's age in whole years; must be 18 to 55 (R2)
//   salary  monthly salary in whole currency units; must be 0 to 10000 (R3)
//
// Returns:
//   salary * factor, where factor comes from the R4 table for men or the R5
//   table for women (R7). The largest value it can return is 10000 * 75 =
//   750000, comfortably inside a 32-bit int.
//
// Throws std::out_of_range when (R6):
//   - age is below 18 or above 55
//   - salary is below 0 or above 10000
//   - gender is Female and age is 51 to 55
//
// That last case is the interesting one. Ages 51 to 55 are inside the valid age
// range, and for a man they are a perfectly ordinary lookup returning 30. The
// women's table simply stops at 50. There is no factor to return, so the
// function refuses rather than inventing one. See R5 in
// requirements/mortgage_requirements.md, and bug B11 for what happens when an
// implementation guesses instead.
int mortgage(Gender gender, int age, int salary);

#endif  // MORTGAGE_HPP
