// mortgage.cpp — CORRECT implementation, conformant to R1..R7.
//
// Every band uses closed intervals with <= / >= so that there is neither a gap
// nor an overlap between consecutive bands, and every input outside the
// accepted domain is rejected with std::out_of_range (R6).

#include "mortgage.hpp"

#include <stdexcept>
#include <string>

namespace {

// R4 — male factor table. Bands are contiguous and non-overlapping.
int male_factor(int age) {
  if (age >= 18 && age <= 35) return 75;
  if (age >= 36 && age <= 45) return 55;
  if (age >= 46 && age <= 55) return 30;
  // Unreachable once R2 has been validated; kept as a defensive guard.
  throw std::out_of_range("male age band undefined for age " +
                          std::to_string(age));
}

// R5 — female factor table. The band [51, 55] is a specification gap and is
// therefore rejected rather than silently given a factor.
int female_factor(int age) {
  if (age >= 18 && age <= 30) return 70;
  if (age >= 31 && age <= 40) return 50;
  if (age >= 41 && age <= 50) return 35;
  throw std::out_of_range(
      "R5 specification gap: no mortgage factor is defined for female age " +
      std::to_string(age));
}

}  // namespace

int mortgage(Gender gender, int age, int salary) {
  // R2 + R6 — age domain.
  if (age < kMinAge || age > kMaxAge) {
    throw std::out_of_range("age out of range [18, 55]: " +
                            std::to_string(age));
  }
  // R3 + R6 — salary domain.
  if (salary < kMinSalary || salary > kMaxSalary) {
    throw std::out_of_range("salary out of range [0, 10000]: " +
                            std::to_string(salary));
  }

  const int factor =
      (gender == Gender::Male) ? male_factor(age) : female_factor(age);

  return salary * factor;  // R7
}
