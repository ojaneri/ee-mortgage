# Mortgage Testing Lab

Project #1 — Functional Testing
Sistemas Embarcados, UTFPR — Prof. Max Mauro Dias Santos
Osvaldo Janeri Filho <janeri@gmail.com>

A small C++ lab for practising black-box test design: nominal, boundary value,
robustness, worst-case and equivalence class testing. The system under test is
one function, `mortgage(gender, age, salary)`, which is deliberately trivial —
the interesting part is the test suite, not the code it exercises.

The repository ships two implementations behind the same header. One is
correct, the other has 12 bugs planted in it. The same test suite runs against
either, selected by a CMake flag, so you can watch it go red and then green
without editing a single test.

44 cases in total. All 44 pass against the correct implementation and 27 fail
against the broken one.

## The spec

`mortgage = salary * factor`, where the factor comes from a table indexed by
gender and age band:

| Gender | Age | Factor |
|---|---|---|
| Male | 18–35 | 75 |
| Male | 36–45 | 55 |
| Male | 46–55 | 30 |
| Female | 18–30 | 70 |
| Female | 31–40 | 50 |
| Female | 41–50 | 35 |
| Female | 51–55 | *not specified* |

Valid ages are 18 to 55, valid salaries 0 to 10000. Anything else throws
`std::out_of_range`.

That last row is not a typo. The original specification simply says nothing
about women aged 51 to 55, even though 51–55 is inside the valid age range.
Real specifications have holes like this. The interesting question for the lab
is what a correct implementation should do about it, and the answer taken here
is: refuse the input rather than invent a number. See R5 in
`requirements/mortgage_requirements.md`.

## Layout

```
CMakeLists.txt              fetches GoogleTest v1.15.2, builds four test binaries
include/mortgage.hpp        the one interface both implementations satisfy
src/mortgage.cpp            correct version
src/mortgage_buggy.cpp      12 planted bugs — leave it alone
requirements/
  mortgage_requirements.md  R1–R7
  buggy_defects.md          what each planted bug is and how to catch it
  test_evidence.md          console output from the red and green runs
tests/                      one file per technique
tools/check_test_names.py   fails the build if the docs name a test that is gone
.github/workflows/ci.yml
```

## Running it

You need CMake 3.14 or newer, a C++17 compiler, and a network connection the
first time you configure (GoogleTest is downloaded, not vendored).

```bash
cmake -S . -B build
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

That builds against `src/mortgage.cpp` and everything passes. To see the suite
fail, point it at the broken implementation:

```bash
cmake -S . -B build-buggy -DUSE_BUGGY=ON
cmake --build build-buggy --parallel
ctest --test-dir build-buggy --output-on-failure
```

27 of the 44 cases fail. The tests are byte-for-byte identical in both runs;
only the linked object changes.

Individual binaries take the usual GoogleTest flags:

```bash
./build/boundary_test --gtest_filter='BoundaryFemale.*'
```

## How the four test files are organised

`nominal_test.cpp` picks one age from the middle of each band, nowhere near an
edge, with a round salary. These are the tests you write first and the ones
that catch the least: five of the seven still pass against the broken build.

`boundary_test.cpp` is where most of the work is. It hits the outer edges of
the domain (18, 55, 0, 10000) but more importantly the *internal* seams between
adjacent bands: 30 and 31 for women, 35 and 36 for men, and so on. Eight of the
twelve planted bugs are wrong comparisons on exactly those seams, which is not
a coincidence — it is the whole reason boundary testing exists.

`robustness_test.cpp` steps outside the domain and checks that the function
refuses rather than answers. Ages 17 and 56, salaries −1 and 10001, plus
plus an absurd one for good measure. The broken implementation has no rejection
path at all, so seven of these eight tests fail against it. The one that passes
is the check that men aged 51 to 55 are still valid — the gap belongs to the
female table only. The last test puts two invalid inputs in the same call,
which is the worst-case variant.

`equivalence_test.cpp` covers all four combinations of weak/strong and
normal/robust. Weak normal takes one case per class index; strong normal is the
full cross product of valid classes, six cells; weak robust breaks one
partition at a time; strong robust breaks two at once. There is also a
`PartitionConsistency` test that walks every age from 18 to 55 and asserts the
factor never increases. That one is worth stealing for other projects — it
catches gaps and overlaps that no finite set of sample points is guaranteed to
hit.

## The planted bugs

Each one has a GitHub issue. The full write-up, including the exact line and
the smallest input that exposes it, is in `requirements/buggy_defects.md`.

| # | What's wrong | Breaks |
|---|---|---|
| B1 | Male band starts at 19 because of `18 < age` | R4 |
| B2 | Male band stops at 34 because of `age < 35` | R4 |
| B3 | Second male band starts at 32, overlapping the first | R4 |
| B4 | Second male band stops at 39 instead of 45 | R4 |
| B5 | No age validation anywhere; the `else` swallows everything | R2, R6 |
| B6 | Female band starts at 19 | R5 |
| B7 | Female factor is 75, copied from the male branch, should be 70 | R5, R7 |
| B8 | Female band stops at 29 instead of 30 | R5 |
| B9 | Second female band starts at 32, leaving age 31 unhandled | R5 |
| B10 | Second female band stops at 39 instead of 40 | R5 |
| B11 | Women aged 51–55 quietly get factor 35 instead of being refused | R5, R6 |
| B12 | Salary is never validated and nothing ever throws | R3, R6 |

B3 is the awkward one. The overlap on ages 32–34 is invisible from outside,
because the first branch catches those ages before the second one gets a
chance. It only shows up at age 35, where the returned factor of 55 proves the
second band accepted somebody younger than 36. Worth discussing in class: a
defect can violate the spec and still be unreachable through the public
interface, and no amount of black-box testing will find it directly.

## CI

`.github/workflows/ci.yml` runs on every push and pull request. Three jobs:

The first builds the correct implementation, runs `ctest`, and then runs
`tools/check_test_names.py` so the markdown cannot go on citing tests that have
been renamed.

The second builds the *broken* implementation and fails the pipeline if the
suite passes. A test suite that stops detecting the planted bugs has quietly
become worthless, and without this job nobody would notice.

The third rebuilds under ASan and UBSan. It is there because of something the
first two missed: the robustness tests used to feed `INT_MAX` in as a salary,
and since the broken implementation multiplies before it validates, the red run
was hitting signed overflow rather than the wrong answer being asserted on. The
whole story is in `requirements/test_evidence.md`.

## Working on this

Pick an issue, branch off main as `fix/something`, write the failing test
before touching any code, and reference the issue number in the commit. PR #13
is left open as a worked example of the flow.

Do not fix `src/mortgage_buggy.cpp`. It is the fixture, not the deliverable.
