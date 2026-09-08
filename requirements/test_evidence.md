# Test Evidence — real build & run output

Environment: Ubuntu 24.04, GCC 13.3.0, CMake 3.28.3, GoogleTest v1.15.2 (FetchContent),
C++17. All output below was captured from actual runs, not reproduced from memory.

Total test cases discovered by CTest: **39**
(7 nominal + 19 boundary + 8 robustness + 5 equivalence/partition, counted per
`TEST(...)` block across the four executables.)

---

## Run 1 — RED: suite linked against `src/mortgage_buggy.cpp`

```console
$ cmake -S . -B build-buggy -DUSE_BUGGY=ON
-- mortgage: linking the BUGGY implementation (expect failures)
-- Build files have been written to: .../mortgage-testing-lab/build-buggy
$ cmake --build build-buggy --parallel
[100%] Built target gmock_main
$ ctest --test-dir build-buggy
41% tests passed, 23 tests failed out of 39

Total Test time (real) =   0.23 sec

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
	 36 - StrongNormalEquivalence.R4R5R7_CartesianProductOfValidClasses (Failed)
	 37 - WeakRobustEquivalence.R2R3R5R6_OneInvalidClassAtATime (Failed)
	 38 - StrongRobustEquivalence.R6_CartesianProductIncludingInvalidClasses (Failed)
	 39 - PartitionConsistency.R4R5_NoGapsOrOverlapsAcrossTheWholeAgeDomain (Failed)
Errors while running CTest
Output from these tests are in: /var/www/html/janeri.com.br/ee/max/mortgage-testing-lab/build-buggy/Testing/Temporary/LastTest.log
Use "--rerun-failed --output-on-failure" to re-run the failed cases verbosely.
```

### Sample failure detail (GoogleTest output, actual vs expected)

```console
$ ./build-buggy/boundary_test --gtest_filter='BoundaryMale.R4_MaleAtLowerDomainEdge18*:BoundaryFemale.R5_FemaleAtUpperEdgeOfYoungBand30*'
Running main() from /var/www/html/janeri.com.br/ee/max/mortgage-testing-lab/build-buggy/_deps/googletest-src/googletest/src/gtest_main.cc
Note: Google Test filter = BoundaryMale.R4_MaleAtLowerDomainEdge18*:BoundaryFemale.R5_FemaleAtUpperEdgeOfYoungBand30*
[==========] Running 2 tests from 2 test suites.
[----------] Global test environment set-up.
[----------] 1 test from BoundaryMale
[ RUN      ] BoundaryMale.R4_MaleAtLowerDomainEdge18_StillGetsFactor75
/var/www/html/janeri.com.br/ee/max/mortgage-testing-lab/tests/boundary_test.cpp:19: Failure
Expected equality of these values:
  mortgage(Gender::Male, 18, kSalary)
    Which is: 30000
  75 * kSalary
    Which is: 75000

[  FAILED  ] BoundaryMale.R4_MaleAtLowerDomainEdge18_StillGetsFactor75 (0 ms)
[----------] 1 test from BoundaryMale (0 ms total)

[----------] 1 test from BoundaryFemale
[ RUN      ] BoundaryFemale.R5_FemaleAtUpperEdgeOfYoungBand30_StillGetsFactor70
/var/www/html/janeri.com.br/ee/max/mortgage-testing-lab/tests/boundary_test.cpp:54: Failure
Expected equality of these values:
  mortgage(Gender::Female, 30, kSalary)
    Which is: 35000
  70 * kSalary
    Which is: 70000

[  FAILED  ] BoundaryFemale.R5_FemaleAtUpperEdgeOfYoungBand30_StillGetsFactor70 (0 ms)
[----------] 1 test from BoundaryFemale (0 ms total)

[----------] Global test environment tear-down
[==========] 2 tests from 2 test suites ran. (0 ms total)
```

`30000` instead of `75000` is defect **B1** (male lower boundary 18 excluded by
`18 < age`); `35000` instead of `70000` is defect **B8** (female upper boundary
30 excluded by `age < 30`).

---

## Run 2 — GREEN: suite linked against `src/mortgage.cpp`

```console
$ cmake -S . -B build -DUSE_BUGGY=OFF
-- mortgage: linking the CORRECT implementation
$ cmake --build build --parallel
[100%] Built target gmock_main
$ ctest --test-dir build --output-on-failure
      Start 38: StrongRobustEquivalence.R6_CartesianProductIncludingInvalidClasses
38/39 Test #38: StrongRobustEquivalence.R6_CartesianProductIncludingInvalidClasses ......   Passed    0.01 sec
      Start 39: PartitionConsistency.R4R5_NoGapsOrOverlapsAcrossTheWholeAgeDomain
39/39 Test #39: PartitionConsistency.R4R5_NoGapsOrOverlapsAcrossTheWholeAgeDomain .......   Passed    0.01 sec

100% tests passed, 0 tests failed out of 39

Total Test time (real) =   0.23 sec
```

---

## Summary

| Implementation under test | Passed | Failed | Total | Verdict |
|---|---|---|---|---|
| `src/mortgage_buggy.cpp` | 16 | **23** | 39 | **RED** — all 12 seeded defects detected |
| `src/mortgage.cpp`       | **39** | 0 | 39 | **GREEN** — fully conformant to R1–R7 |

### Which failing test caught which defect

| Bug | Failing test in the RED run |
|-----|-----------------------------|
| B1  | `BoundaryMale.R4_MaleAtLowerDomainEdge18_StillGetsFactor75` (30000 vs 75000) |
| B2  | `BoundaryMale.R4_MaleAtUpperEdgeOfYoungBand35_StillGetsFactor75` (55000 vs 75000) |
| B3  | `BoundaryMale.R4_MaleAtUpperEdgeOfYoungBand35_StillGetsFactor75` — factor 55 proves the `[36,45]` band accepted age 35; `PartitionConsistency.*` also fails |
| B4  | `NominalMale.R4_MaleAgeInsideMiddleBand_AppliesFactor55`, `BoundaryMale.R4_MaleAtUpperEdgeOfMiddleBand45_StillGetsFactor55` |
| B5  | `RobustnessAge.R2R6_AgeBelowMinimum_ThrowsOutOfRange`, `RobustnessAge.R2R6_AgeAboveMaximum_ThrowsOutOfRange`, `RobustnessAge.R2R6_GrosslyInvalidAges_ThrowOutOfRange`, `BoundaryAge.*` |
| B6  | `BoundaryFemale.R5_FemaleAtLowerDomainEdge18_GetsFactor70` |
| B7  | `NominalFemale.R5_FemaleAgeInsideYoungBand_AppliesFactor70` (75000 vs 70000), `StrongNormalEquivalence.*` |
| B8  | `BoundaryFemale.R5_FemaleAtUpperEdgeOfYoungBand30_StillGetsFactor70` |
| B9  | `BoundaryFemale.R5_FemaleAtLowerEdgeOfMiddleBand31_GetsFactor50` |
| B10 | `BoundaryFemale.R5_FemaleAtUpperEdgeOfMiddleBand40_StillGetsFactor50` |
| B11 | `RobustnessSpecGap.R5R6_FemaleInUndefinedBand_ThrowsOutOfRange`, `BoundaryFemale.R5_FemaleAtFirstAgeOfUndefinedBand51_IsRejected` |
| B12 | `RobustnessSalary.R3R6_NegativeSalary_ThrowsOutOfRange`, `RobustnessSalary.R3R6_SalaryAboveMaximum_ThrowsOutOfRange`, `WeakRobustEquivalence.*`, `StrongRobustEquivalence.*` |

Every one of the 12 seeded defects is caught by at least one test, so the suite
is adequate with respect to the seeded fault set (mutation-style adequacy).

The 16 tests that still pass in the RED run are the ones whose inputs fall in
regions where the defective code happens to agree with the specification
(e.g. male age 25 → 75, female age 45 → 35). That is expected: passing tests
prove nothing about the defects they do not touch.
