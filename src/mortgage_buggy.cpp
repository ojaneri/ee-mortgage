// mortgage_buggy.cpp — INTENTIONALLY DEFECTIVE implementation.
//
// This is the "as-written" version handed to the class: it carries 12 seeded
// defects (B1..B12) documented in requirements/buggy_defects.md.
// DO NOT FIX THIS FILE. It exists so the test suite can be shown to go RED.
//
// The defective routine below is reproduced verbatim; only a thin adapter is
// added underneath so that the very same test suite can be linked against
// either implementation without being rewritten.

#include "mortgage.hpp"

namespace buggy {

// ---------------------------------------------------------------------------
// VERBATIM defective code under test (12 seeded defects):
// ---------------------------------------------------------------------------
int mortgage(bool male, int age, int salary) {
  if (male) {
    if (18 < age && age < 35) return 75 * salary;
    else if (31 < age && age < 40) return 55 * salary;
    else return 30 * salary;
  } else {
    if (18 < age && age < 30) return 75 * salary;
    else if (31 < age && age < 40) return 50 * salary;
    else return 35 * salary;
  }
}
// ---------------------------------------------------------------------------

}  // namespace buggy

// Adapter to the public interface declared in include/mortgage.hpp.
// It performs NO validation of its own — faithfully preserving the defective
// behaviour, in particular the total absence of domain checks (B5, B12).
int mortgage(Gender gender, int age, int salary) {
  return buggy::mortgage(gender == Gender::Male, age, salary);
}
