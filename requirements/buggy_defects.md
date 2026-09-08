# The twelve planted bugs

Osvaldo Janeri Filho <janeri@gmail.com>

What follows is a line-by-line reading of `src/mortgage_buggy.cpp` against
R1–R7. Twelve defects, numbered B1 to B12, each with the requirement it breaks
and the smallest call that shows it.

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

Eleven lines, twelve bugs. That ratio is not an accident: every comparison on
lines 3, 4, 7 and 8 is strict where it should be inclusive, and both `else`
branches on lines 5 and 9 are doing duty as a catch-all for inputs that should
never have got that far.

---

### B1 — age 18 is not in the men's first band

Line 3 says `18 < age`, so 18 itself misses the band. It then misses line 4 as
well, because `31 < 18` is false, and ends up in the catch-all with factor 30.

Breaks R4. `mortgage(Male, 18, 1000)` returns 30000 where 75000 is correct.
Caught by `BoundaryMale.R4_MaleAtLowerDomainEdge18_StillGetsFactor75`.

### B2 — age 35 is not in the men's first band either

Same line, other end: `age < 35`. This one is worse than B1 because 35 does not
fall through to the catch-all — line 4 accepts it, since `31 < 35 && 35 < 40`,
and hands back 55.

Breaks R4. `mortgage(Male, 35, 1000)` returns 55000 instead of 75000.
Caught by `BoundaryMale.R4_MaleAtUpperEdgeOfYoungBand35_StillGetsFactor75`.

### B3 — the men's second band starts at 32, not 36

Line 4 opens with `31 < age`, so the band claims ages 32 to 39. R4 says it
starts at 36. Ages 32 to 35 therefore belong to two bands at once.

This is the only one of the twelve that cannot be observed directly. The
overlap on 32, 33 and 34 is hidden because line 3 catches those ages first, so
from outside the function they look correct. The only visible trace is at age
35, where the answer of 55 could only have come from the second band — which
proves the band accepted somebody under 36.

Breaks R4. Evidenced by the same test as B2, plus
`PartitionConsistency.R4R5_NoGapsOrOverlapsAcrossTheWholeAgeDomain`; the
overlap itself is confirmed by reading line 4.

### B4 — the men's second band stops at 39, not 45

`age < 40` on line 4. Ages 40 through 45 fall past it into the catch-all and
get 30 instead of 55.

Breaks R4. `mortgage(Male, 40, 1000)` returns 30000 instead of 55000.
Caught by `NominalMale.R4_MaleAgeInsideMiddleBand_AppliesFactor55` and
`BoundaryMale.R4_MaleAtUpperEdgeOfMiddleBand45_StillGetsFactor55`.

### B5 — nothing validates the age

Lines 5 and 9. Both `else` branches accept any integer at all, so a 12-year-old
and a 90-year-old both get a mortgage, at factor 30 or 35 depending on gender.
There is no code path in this function that refuses anything.

Breaks R2 and R6. `mortgage(Male, 17, 1000)` should throw; it returns 30000.
Caught across `RobustnessAge.*` and `BoundaryAge.*`.

### B6 — age 18 is not in the women's first band

Line 7, `18 < age`, the same mistake as B1. Age 18 misses band one, misses band
two, lands on 35.

Breaks R5. `mortgage(Female, 18, 1000)` returns 35000 instead of 70000.
Caught by `BoundaryFemale.R5_FemaleAtLowerDomainEdge18_GetsFactor70`.

### B7 — the women's first band uses the men's factor

Line 7 returns `75 * salary`. R5 says 70. This looks like the male branch was
copied and the comparisons were adjusted but the constant was not.

Breaks R5 and R7. `mortgage(Female, 25, 1000)` returns 75000 instead of 70000.
Caught by `NominalFemale.R5_FemaleAgeInsideYoungBand_AppliesFactor70` and
`StrongNormalEquivalence.R4R5R7_CartesianProductOfValidClasses`.

### B8 — the women's first band stops at 29

`age < 30` on line 7, so 30 drops out. It is not picked up by line 8 either,
because that starts at 32, so it lands on the catch-all with 35.

Breaks R5. `mortgage(Female, 30, 1000)` returns 35000 instead of 70000.
Caught by `BoundaryFemale.R5_FemaleAtUpperEdgeOfYoungBand30_StillGetsFactor70`.

### B9 — the women's second band starts at 32, leaving 31 homeless

Line 8, `31 < age`. R5 puts 31 in the second band; the code puts it nowhere.
Together with B8 this makes two consecutive ages, 30 and 31, that belong to two
different bands and both come back as 35.

Breaks R5. `mortgage(Female, 31, 1000)` returns 35000 instead of 50000.
Caught by `BoundaryFemale.R5_FemaleAtLowerEdgeOfMiddleBand31_GetsFactor50` and
by the partition sweep.

### B10 — the women's second band stops at 39

`age < 40` on line 8, so 40 falls through to 35 instead of 50.

Breaks R5. `mortgage(Female, 40, 1000)` returns 35000 instead of 50000.
Caught by `BoundaryFemale.R5_FemaleAtUpperEdgeOfMiddleBand40_StillGetsFactor50`.

### B11 — women aged 51 to 55 get a made-up factor

Line 9 again. The catch-all also swallows the band the specification never
defined, and answers 35 for it.

Of the twelve this is the one that would survive longest in production. A wrong
factor can be spotted by holding the output next to the table. An invented
factor for a band the table does not mention looks entirely reasonable, and the
gap in the specification stays hidden until somebody audits the code.

Breaks R5 and R6. `mortgage(Female, 51, 1000)` should throw; it returns 35000,
and the same for 52 through 55.
Caught by `RobustnessSpecGap.R5R6_FemaleInUndefinedBand_ThrowsOutOfRange` and
`BoundaryFemale.R5_FemaleAtFirstAgeOfUndefinedBand51_IsRejected`.

### B12 — nothing validates the salary

The parameter is read once, on whichever return line fires, and never checked.
Negative salaries produce negative mortgages; salaries above the cap are
accepted without comment. Combined with B5, the function has no rejection
mechanism of any kind, which is why the whole of `robustness_test.cpp` fails
against it.

Breaks R3 and R6. `mortgage(Male, 30, -1)` should throw; it returns −75.
Caught by `RobustnessSalary.*`, `WeakRobustEquivalence.*` and
`StrongRobustEquivalence.*`.

---

## Where each bug is caught

| Bug | Requirement | Kind | Test file |
|-----|---|---|---|
| B1  | R4 | strict comparison at a band edge | boundary |
| B2  | R4 | strict comparison at a band edge | boundary |
| B3  | R4 | bands overlap on 32–35 | boundary, equivalence |
| B4  | R4 | band truncated at 39 | boundary, nominal |
| B5  | R2, R6 | no age validation | robustness |
| B6  | R5 | strict comparison at a band edge | boundary |
| B7  | R5, R7 | wrong constant, 75 for 70 | nominal, equivalence |
| B8  | R5 | strict comparison at a band edge | boundary |
| B9  | R5 | gap at age 31 | boundary, equivalence |
| B10 | R5 | band truncated at 39 | boundary |
| B11 | R5, R6 | spec gap answered instead of refused | robustness, boundary |
| B12 | R3, R6 | no salary validation | robustness, equivalence |
