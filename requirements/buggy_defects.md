# Seeded Defects in `src/mortgage_buggy.cpp` (B1 … B12)

Line-by-line analysis of the defective implementation against requirements
R1–R7 (`requirements/mortgage_requirements.md`).

Code under analysis:

```cpp
 1  int mortgage(bool male, int age, int salary) {
 2    if (male) {
 3      if (18 < age && age < 35) return 75 * salary;
 4      else if (31 < age && age < 40) return 55 * salary;
 5      else return 30 * salary;
 6    } else {
 7      if (18 < age && age < 30) return 75 * salary;
 8      else if (31 < age && age < 40) return 50 * salary;
 9      else return 35 * salary;
10    }
11  }
```

Legend of the "minimal test" column: the smallest input triple that produces an
observable difference between the defective and the conformant behaviour.

---

## B1 — Male lower boundary 18 is excluded

* **Line:** 3 — `18 < age` (strict) instead of `18 <= age`.
* **Bug:** age 18 does not enter the `[18,35]` band; it falls through both
  `else if` (31 < 18 is false) into the catch-all and receives factor **30**.
* **Requirement violated:** **R4** (`[18,35] → 75`).
* **Minimal test:** `mortgage(Male, 18, 1000)` → expected `75000`, actual `30000`.

## B2 — Male upper boundary 35 is excluded

* **Line:** 3 — `age < 35` (strict) instead of `age <= 35`.
* **Bug:** age 35 leaves the first band and is captured by line 4
  (`31 < 35 && 35 < 40`), receiving factor **55**.
* **Requirement violated:** **R4** (`[18,35] → 75`).
* **Minimal test:** `mortgage(Male, 35, 1000)` → expected `75000`, actual `55000`.

## B3 — Male second band starts at 32 instead of 36 (band overlap)

* **Line:** 4 — `31 < age` instead of `age >= 36`; the guard claims ages
  32…39, overlapping the specified `[18,35]` band on 32…35.
* **Bug:** the `[36,45]` band is specified to begin at 36, but the predicate
  admits ages 32…35. The overlap on 32…34 is *masked* by the branch ordering
  (line 3 catches them first); it becomes observable exactly at age 35, where
  the returned factor 55 can only originate from the second band — direct
  evidence that the band accepts an age below 36.
* **Requirement violated:** **R4** (`[36,45] → 55`, bands must not overlap).
* **Minimal test:** `mortgage(Male, 35, 1000)` returns `55000` (factor 55) —
  the second band accepted an age < 36. Structural inspection of line 4
  confirms the overlap on 32…35.

## B4 — Male second band ends at 39 instead of 45

* **Line:** 4 — `age < 40` instead of `age <= 45`.
* **Bug:** ages 40…45 fall to the catch-all on line 5 and receive factor
  **30** instead of 55.
* **Requirement violated:** **R4** (`[36,45] → 55`).
* **Minimal test:** `mortgage(Male, 40, 1000)` → expected `55000`, actual `30000`.

## B5 — No age-domain validation; unbounded catch-all

* **Lines:** 5 and 9 — the `else` branches accept *any* age.
* **Bug:** ages below 18 and above 55 are silently accepted and return
  `30 * salary` (male) or `35 * salary` (female) instead of being rejected.
* **Requirements violated:** **R2**, **R6**.
* **Minimal test:** `mortgage(Male, 17, 1000)` must throw `std::out_of_range`;
  the defective version returns `30000`. Same for `age = 56`.

## B6 — Female lower boundary 18 is excluded

* **Line:** 7 — `18 < age` (strict) instead of `18 <= age`.
* **Bug:** age 18 misses the first band, misses line 8 (`31 < 18` false) and
  is caught by line 9 with factor **35**.
* **Requirement violated:** **R5** (`[18,30] → 70`).
* **Minimal test:** `mortgage(Female, 18, 1000)` → expected `70000`, actual `35000`.

## B7 — Female first band uses factor 75 instead of 70

* **Line:** 7 — `return 75 * salary` (copy/paste from the male branch).
* **Bug:** every accepted female applicant in `[18,30]` is over-valued by a
  factor of 5 per salary unit.
* **Requirement violated:** **R5** (`[18,30] → 70`), **R7**.
* **Minimal test:** `mortgage(Female, 25, 1000)` → expected `70000`, actual `75000`.

## B8 — Female first band upper boundary 30 is excluded

* **Line:** 7 — `age < 30` instead of `age <= 30`.
* **Bug:** age 30 misses band 1, misses band 2 (`31 < 30` false) and is caught
  by line 9 with factor **35**.
* **Requirement violated:** **R5** (`[18,30] → 70`).
* **Minimal test:** `mortgage(Female, 30, 1000)` → expected `70000`, actual `35000`.

## B9 — Female second band starts at 32 instead of 31 (gap at 31)

* **Line:** 8 — `31 < age` instead of `age >= 31`.
* **Bug:** age 31 belongs to no band and falls to the catch-all with factor
  **35**. Combined with B8 this creates a hole in the age partition.
* **Requirement violated:** **R5** (`[31,40] → 50`, bands must not have gaps).
* **Minimal test:** `mortgage(Female, 31, 1000)` → expected `50000`, actual `35000`.

## B10 — Female second band ends at 39 instead of 40

* **Line:** 8 — `age < 40` instead of `age <= 40`.
* **Bug:** age 40 falls to the catch-all and receives factor **35**.
* **Requirement violated:** **R5** (`[31,40] → 50`).
* **Minimal test:** `mortgage(Female, 40, 1000)` → expected `50000`, actual `35000`.

## B11 — Female specification gap `[51,55]` is silently given factor 35

* **Line:** 9 — the catch-all also swallows the undefined female band.
* **Bug:** R5 defines no factor for female ages 51…55; the defective code
  returns `35 * salary` instead of rejecting the input, hiding a known
  specification gap behind a plausible-looking number.
* **Requirements violated:** **R5**, **R6**.
* **Minimal test:** `mortgage(Female, 51, 1000)` must throw `std::out_of_range`;
  the defective version returns `35000`. Same for `age = 55`.

## B12 — No salary-domain validation

* **Lines:** 1–11 — `salary` is never checked and no exception is ever thrown.
* **Bug:** negative salaries produce negative mortgages and salaries above
  10000 are accepted, so the function has no rejection mechanism at all.
* **Requirements violated:** **R3**, **R6**.
* **Minimal test:** `mortgage(Male, 30, -1)` must throw `std::out_of_range`;
  the defective version returns `-75`. Same for `salary = 10001`.

---

## Traceability summary

| Bug | Requirement(s) | Kind | Exposed by (file) |
|-----|----------------|------|-------------------|
| B1  | R4 | boundary, `<` instead of `<=` | `boundary_test.cpp` |
| B2  | R4 | boundary, `<` instead of `<=` | `boundary_test.cpp` |
| B3  | R4 | band overlap 32…35 | `boundary_test.cpp` |
| B4  | R4 | band truncated at 39 | `boundary_test.cpp`, `nominal_test.cpp` |
| B5  | R2, R6 | missing domain validation | `robustness_test.cpp` |
| B6  | R5 | boundary, `<` instead of `<=` | `boundary_test.cpp` |
| B7  | R5, R7 | wrong factor (75 vs 70) | `nominal_test.cpp`, `equivalence_test.cpp` |
| B8  | R5 | boundary, `<` instead of `<=` | `boundary_test.cpp` |
| B9  | R5 | band gap at 31 | `boundary_test.cpp` |
| B10 | R5 | band truncated at 39 | `boundary_test.cpp` |
| B11 | R5, R6 | specification gap not rejected | `robustness_test.cpp` |
| B12 | R3, R6 | missing domain validation | `robustness_test.cpp` |
