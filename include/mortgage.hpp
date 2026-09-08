// mortgage.hpp — Mortgage Testing Lab (Project #1, Functional Testing)
// UTFPR — Sistemas Embarcados — Prof. Max Mauro Dias Santos
//
// Single public interface shared by BOTH implementations:
//   - src/mortgage.cpp        (correct, requirements-conformant)
//   - src/mortgage_buggy.cpp  (intentionally defective, 12 seeded bugs)
// The test suite is written once against this header and linked against
// whichever implementation the build selects (see CMake option USE_BUGGY).

#ifndef MORTGAGE_HPP
#define MORTGAGE_HPP

#include <stdexcept>

/// R1 — the gender input.
enum class Gender { Male, Female };

/// Domain limits (R2, R3).
constexpr int kMinAge    = 18;
constexpr int kMaxAge    = 55;
constexpr int kMinSalary = 0;
constexpr int kMaxSalary = 10000;

/// Computes the mortgage value (R7: salary * factor).
///
/// @param gender  R1 — Male or Female.
/// @param age     R2 — must be in [18, 55].
/// @param salary  R3 — must be in [0, 10000].
/// @return        R7 — salary * factor, with the factor taken from R4 (male)
///                or R5 (female).
/// @throws std::out_of_range  R6 — age outside [18,55], salary outside
///                [0,10000], or female age in the undefined band [51,55] (R5).
int mortgage(Gender gender, int age, int salary);

#endif  // MORTGAGE_HPP
