# Evidence

Osvaldo Janeri Filho <janeri@gmail.com>

Everything below is console output, pasted from real runs. Ubuntu 24.04,
GCC 13.3.0, CMake 3.28.3, GoogleTest v1.15.2 pulled in by FetchContent, C++17.

43 test cases, split 7 nominal / 19 boundary / 12 robustness / 5 equivalence.
Every test traces to a requirement in R1-R7; nothing here tests behaviour the
specification does not ask for.

## Red — linked against src/mortgage_buggy.cpp

```console
$ cmake -S . -B build-buggy -DUSE_BUGGY=ON
-- mortgage: linking the BUGGY implementation (expect failures)
$ cmake --build build-buggy --parallel
$ ctest --test-dir build-buggy
40% tests passed, 26 tests failed out of 43

Total Test time (real) =   0.27 sec

The following tests FAILED:
	  2 - NominalMale.R4_MaleAgeInsideMiddleBand_AppliesFactor55 (Failed)
	  4 - NominalFemale.R5_FemaleAgeInsideYoungBand_AppliesFactor70 (Failed)
	  8 - BoundaryMale.R4_MaleAtLowerDomainEdge18_StillGetsFactor75 (Failed)
	  9 - BoundaryMale.R4_MaleAtUpperEdgeOfYoungBand35_StillGetsFactor75 (Failed)
	 11 - BoundaryMale.R4_MaleAtUpperEdgeOfMiddleBand45_StillGetsFactor55 (Failed)
	 14 - BoundaryFemale.R5_FemaleAtLowerDomainEdge18_GetsFactor70 (Failed)
	 15 - BoundaryFemale.R5_FemaleAtUpperEdgeOfYoungBand30_StillGetsFactor70 (Failed)
	 16 - BoundaryFemale.R5_FemaleAtLowerEdgeOfMiddleBand31_GetsFactor50 (Failed)
	 17 - BoundaryFemale.R5_FemaleAtUpperEdgeOfMiddleBand40_StillGetsFactor50 (Failed)
	 20 - BoundaryFemale.R5_FemaleAtFirstAgeOfUndefinedBand51_IsRejected (Failed)
	 25 - BoundaryAge.R2_AgeJustBelowLowerDomainEdge17_IsRejected (Failed)
	 26 - BoundaryAge.R2_AgeJustAboveUpperDomainEdge56_IsRejected (Failed)
	 27 - RobustnessAge.R2R6_AgeBelowMinimum_ThrowsOutOfRange (Failed)
	 28 - RobustnessAge.R2R6_AgeAboveMaximum_ThrowsOutOfRange (Failed)
	 29 - RobustnessAge.R2R6_GrosslyInvalidAges_ThrowOutOfRange (Failed)
	 30 - RobustnessSalary.R3R6_NegativeSalary_ThrowsOutOfRange (Failed)
	 31 - RobustnessSalary.R3R6_SalaryAboveMaximum_ThrowsOutOfRange (Failed)
	 32 - RobustnessSpecGap.R5R6_FemaleInUndefinedBand_ThrowsOutOfRange (Failed)
	 34 - RobustnessWorstCase.R6_AgeAndSalaryBothInvalid_ThrowsOutOfRange (Failed)
	 35 - RejectionReason.R2R6_AgeOutOfRangeSaysSo (Failed)
	 36 - RejectionReason.R3R6_SalaryOutOfRangeSaysSo (Failed)
	 37 - RejectionReason.R5R6_SpecGapIsNotReportedAsAnOutOfRangeAge (Failed)
	 40 - StrongNormalEquivalence.R4R5R7_CartesianProductOfValidClasses (Failed)
	 41 - WeakRobustEquivalence.R2R3R5R6_OneInvalidClassAtATime (Failed)
	 42 - StrongRobustEquivalence.R6_CartesianProductIncludingInvalidClasses (Failed)
	 43 - PartitionConsistency.R4R5_NoGapsOrOverlapsAcrossTheWholeAgeDomain (Failed)
Errors while running CTest
```

Two of those in full, so the actual numbers are on the record:

```console
$ ./build-buggy/boundary_test --gtest_filter='BoundaryMale.R4_MaleAtLowerDomainEdge18*:BoundaryFemale.R5_FemaleAtUpperEdgeOfYoungBand30*'
[ RUN      ] BoundaryMale.R4_MaleAtLowerDomainEdge18_StillGetsFactor75
/var/www/html/janeri.com.br/ee/max/mortgage-testing-lab/tests/boundary_test.cpp:49: Failure
Expected equality of these values:
  mortgage(Gender::Male, 18, kSalary)
    Which is: 30000
  75 * kSalary
    Which is: 75000

[  FAILED  ] BoundaryMale.R4_MaleAtLowerDomainEdge18_StillGetsFactor75 (0 ms)
[----------] 1 test from BoundaryMale (0 ms total)

[----------] 1 test from BoundaryFemale
[ RUN      ] BoundaryFemale.R5_FemaleAtUpperEdgeOfYoungBand30_StillGetsFactor70
/var/www/html/janeri.com.br/ee/max/mortgage-testing-lab/tests/boundary_test.cpp:107: Failure
Expected equality of these values:
  mortgage(Gender::Female, 30, kSalary)
    Which is: 35000
  70 * kSalary
    Which is: 70000

[  FAILED  ] BoundaryFemale.R5_FemaleAtUpperEdgeOfYoungBand30_StillGetsFactor70 (0 ms)
[----------] 1 test from BoundaryFemale (0 ms total)
```

30000 where 75000 was expected is B1: `18 < age` throws away the lower edge of
the men's first band. 35000 where 70000 was expected is B8, the same mistake at
the top of the women's first band.

And the specification gap, which fails differently — no wrong number, just an
answer where there should have been a refusal:

```console
$ ./build-buggy/robustness_test --gtest_filter='RobustnessSpecGap.R5R6_FemaleInUndefinedBand*'
[ RUN      ] RobustnessSpecGap.R5R6_FemaleInUndefinedBand_ThrowsOutOfRange
/var/www/html/janeri.com.br/ee/max/mortgage-testing-lab/tests/robustness_test.cpp:150: Failure
Expected: mortgage(Gender::Female, age, kSalary) throws an exception of type std::out_of_range.
  Actual: it throws nothing.
woman aged 51 should be refused, not priced

/var/www/html/janeri.com.br/ee/max/mortgage-testing-lab/tests/robustness_test.cpp:150: Failure
Expected: mortgage(Gender::Female, age, kSalary) throws an exception of type std::out_of_range.
  Actual: it throws nothing.
woman aged 52 should be refused, not priced

/var/www/html/janeri.com.br/ee/max/mortgage-testing-lab/tests/robustness_test.cpp:150: Failure
Expected: mortgage(Gender::Female, age, kSalary) throws an exception of type std::out_of_range.
  Actual: it throws nothing.
```

## Green — linked against src/mortgage.cpp

```console
$ cmake -S . -B build
-- mortgage: linking the CORRECT implementation
$ cmake --build build --parallel
$ ctest --test-dir build --output-on-failure

100% tests passed, 0 tests failed out of 43

Total Test time (real) =   0.25 sec
```

## Summary

| Linked against | Passed | Failed | Total |
|---|---|---|---|
| `src/mortgage_buggy.cpp` | 17 | 26 | 43 |
| `src/mortgage.cpp` | 43 | 0 | 43 |

Per file against the broken build: nominal 2 of 7 fail, boundary 10 of 19,
robustness 10 of 12, equivalence 4 of 5.

All twelve planted bugs are caught, and every one of the 26 failures is caused
by one of them:

| Bug | Caught by |
|-----|-----------|
| B1  | `BoundaryMale.R4_MaleAtLowerDomainEdge18_StillGetsFactor75` |
| B2  | `BoundaryMale.R4_MaleAtUpperEdgeOfYoungBand35_StillGetsFactor75` |
| B3  | same test as B2 — factor 55 at age 35 proves the second band took someone under 36; also `PartitionConsistency.R4R5_NoGapsOrOverlapsAcrossTheWholeAgeDomain` |
| B4  | `NominalMale.R4_MaleAgeInsideMiddleBand_AppliesFactor55`, `BoundaryMale.R4_MaleAtUpperEdgeOfMiddleBand45_StillGetsFactor55` |
| B5  | `RobustnessAge.*`, `BoundaryAge.*`, `RejectionReason.R2R6_AgeOutOfRangeSaysSo` |
| B6  | `BoundaryFemale.R5_FemaleAtLowerDomainEdge18_GetsFactor70` |
| B7  | `NominalFemale.R5_FemaleAgeInsideYoungBand_AppliesFactor70`, `StrongNormalEquivalence.R4R5R7_CartesianProductOfValidClasses` |
| B8  | `BoundaryFemale.R5_FemaleAtUpperEdgeOfYoungBand30_StillGetsFactor70` |
| B9  | `BoundaryFemale.R5_FemaleAtLowerEdgeOfMiddleBand31_GetsFactor50` |
| B10 | `BoundaryFemale.R5_FemaleAtUpperEdgeOfMiddleBand40_StillGetsFactor50` |
| B11 | `RobustnessSpecGap.R5R6_FemaleInUndefinedBand_ThrowsOutOfRange`, `BoundaryFemale.R5_FemaleAtFirstAgeOfUndefinedBand51_IsRejected`, `RejectionReason.R5R6_SpecGapIsNotReportedAsAnOutOfRangeAge` |
| B12 | `RobustnessSalary.*`, `WeakRobustEquivalence.*`, `StrongRobustEquivalence.*`, `RejectionReason.R3R6_SalaryOutOfRangeSaysSo` |

The seventeen that still pass against the broken build are the ones whose inputs
land where the broken code and the specification happen to agree — a man of 25,
a woman of 45, a man of 51. Nothing follows from a passing test about the parts
of the input space it never touches.

## Mutation check

Boundary and equivalence testing can look thorough and still leave a constant
unpinned, so the correct implementation was mutated eighteen ways — every band
edge moved by one, every factor changed by one, every guard loosened — and the
suite rerun against each.

Eighteen mutants, eighteen killed. Two survived the first attempt: widening the
age guard from 55 to 56, and from 18 to 17, changed nothing any test could
observe, because `male_factor` and `female_factor` have defensive throws that
catch the overshoot and raise `std::out_of_range` of their own. The behaviour
was right, but for a reason the tests had not asked for. The `RejectionReason`
tests were added to pin the message down, and both mutants died.

## Things found while reviewing, not while writing

**Undefined behaviour in the red run.** The robustness tests used to pass
`INT_MAX` as a salary. The broken implementation multiplies before it validates
anything, so `INT_MAX * 35` was signed overflow rather than the wrong answer
being asserted on — confirmed under `-fsanitize=undefined`:

```
src/mortgage_buggy.cpp:24:22: runtime error: signed integer overflow:
2147483647 * 35 cannot be represented in type 'int'
```

The salary was replaced with 20000000, still far outside the domain but small
enough that the product stays inside an `int`. A sanitizer job was added to CI
so the next one gets caught by a machine rather than by hand.

**Warnings were not reaching the tests.** `-Wall -Wextra` was set `PRIVATE` on
the library target, so the four test binaries compiled with none of it. Moved to
an interface target that both the library and the tests link. The build is clean
under `-Wall -Wextra -Wpedantic`.

**Gender is not validated, and deliberately stays that way.** `mortgage` selects
the factor table with `gender == Gender::Male ? male : female`, so a value cast
in from outside the enum goes down the female branch without a word. A revision
during review replaced this with a `switch` that threw on anything unexpected.
It was backed out.

The reasoning is worth recording, because the argument cuts both ways. Against
the switch: R1 admits exactly two genders and R6 lists exactly three things that
must be rejected, none of them a gender. Adding a fourth rejection is deciding
in code something the requirement did not decide — which is the same category of
error as B11 inventing a factor for the 51-55 gap. For the switch: an
unvalidated input silently taking a branch is a genuine robustness hole in a lab
about robustness.

Strict adherence to R1-R7 won. If an invalid gender is a real concern, the fix
belongs in the specification before it belongs in the code, and the comment in
`src/mortgage.cpp` says so.
