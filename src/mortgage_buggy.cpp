// mortgage_buggy.cpp — Mortgage Testing Lab
// Osvaldo Janeri Filho <janeri@gmail.com>
//
// The broken implementation. DO NOT FIX IT. This file is the fixture that
// proves the test suite is worth something: if the suite still passes after
// being pointed at this code, the suite is the thing that is broken.
//
// The routine is the version handed out in class, copied unchanged. Twelve
// defects, catalogued in requirements/buggy_defects.md and tracked as GitHub
// issues B1 to B12. The only thing added is the adapter at the bottom.
//
// HOW TO READ IT
//
// Compare against the tables in include/mortgage.hpp:
//
//   men    18-35 -> 75    36-45 -> 55    46-55 -> 30
//   women  18-30 -> 70    31-40 -> 50    41-50 -> 35    51-55 -> no factor
//
// and against R2 and R3: age must be 18 to 55, salary must be 0 to 10000, and
// anything else must be refused.
//
// Two habits produce all twelve defects. Every comparison is strict (`<`)
// where the specification means inclusive (`<=`), which shaves a year off both
// ends of every band. And each branch ends in a bare `else` that accepts
// whatever is left over, so nothing is ever refused — not a 12-year-old, not a
// negative salary, not the age band the specification forgot to define.

#include "mortgage.hpp"

namespace buggy {

// The original function. `male` is a bool here rather than a Gender, which is
// why the adapter below exists.
//
// Line-by-line, with what each one actually does versus what R4/R5 asked for:
int mortgage(bool male, int age, int salary) {
  if (male) {
    // B1: `18 < age` is strict, so age 18 does NOT enter this band. R4 says
    //     18-35 inclusive. An 18-year-old falls past this line.
    // B2: `age < 35` is strict at the other end, so 35 does not enter either.
    // B3: taken with the next line, this band really covers 19-34, and the
    //     next one claims to start at 32 — so 32, 33 and 34 belong to two
    //     bands at once. The overlap is invisible from outside because this
    //     line wins, but it is still wrong.
    // Correct behaviour: return 75 * salary for every age from 18 to 35.
    if (18 < age && age < 35) return 75 * salary;

    // B3: `31 < age` means this band starts at 32. R4 says it starts at 36.
    //     That is why age 35 comes out as 55 instead of 75 — this line catches
    //     what the line above dropped.
    // B4: `age < 40` means it stops at 39. R4 says it runs to 45, so ages 40
    //     through 45 fall past this line too and end up in the else below,
    //     getting 30 instead of 55.
    // Correct behaviour: return 55 * salary for every age from 36 to 45.
    else if (31 < age && age < 40) return 55 * salary;

    // B5: this is the heart of it. There is no validation anywhere in the
    //     function, so this `else` is reached by three completely different
    //     kinds of input and answers all of them the same way:
    //       - ages 46-55, where 30 is genuinely the right factor
    //       - ages 40-45, which should have been 55 (that is B4)
    //       - ages 3, 17, 90, INT_MIN — which should have been refused
    //     A 12-year-old gets a mortgage offer of 30x salary. Nothing in this
    //     function can throw, so R6 is not partially implemented, it is
    //     entirely absent.
    else return 30 * salary;
  } else {
    // B6: `18 < age`, same strict-comparison mistake as B1. Age 18 drops out.
    // B7: the factor is 75, copied from the male branch above. R5 says 70.
    //     Every woman between 19 and 29 is over-valued by 5x her salary.
    // B8: `age < 30` drops age 30, which R5 puts in this band.
    // Correct behaviour: return 70 * salary for every age from 18 to 30.
    if (18 < age && age < 30) return 75 * salary;

    // B9: `31 < age` means this band starts at 32, but R5 starts it at 31.
    //     Combined with B8, ages 30 and 31 now belong to no band at all even
    //     though they sit in two different bands in the specification.
    // B10: `age < 40` stops the band at 39; R5 runs it to 40.
    // Correct behaviour: return 50 * salary for every age from 31 to 40.
    else if (31 < age && age < 40) return 50 * salary;

    // B11: the female version of the same catch-all, and worse than B5 because
    //      of what it swallows. Ages 51-55 are a hole in the specification —
    //      there is no correct factor to return — and this line answers 35
    //      anyway. The output looks completely reasonable. Nobody reviewing a
    //      report of mortgage offers would spot it, and the gap in the
    //      requirement stays hidden until somebody reads this code.
    // B12: salary reaches this multiplication without ever having been
    //      checked. A negative salary produces a negative mortgage; a salary
    //      above the 10000 cap is accepted without comment. Combined with B5,
    //      the function refuses nothing at all.
    else return 35 * salary;
  }
}

}  // namespace buggy

// Adapter to the interface in include/mortgage.hpp.
//
// The tests are written once, against Gender rather than bool, and linked
// against either this file or src/mortgage.cpp. Without this shim the suite
// would have to be rewritten to run against the broken version, and a suite
// you rewrite to make it fail proves nothing.
//
// It deliberately does no checking of its own. Adding a range check here would
// fix B5 and B12 through the back door: half of robustness_test.cpp would
// start passing, the red run would look much healthier than the code deserves,
// and the point of the exercise would be lost.
int mortgage(Gender gender, int age, int salary) {
  return buggy::mortgage(gender == Gender::Male, age, salary);
}
