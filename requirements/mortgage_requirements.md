# Mortgage — Requirements Specification

Project #1 — Functional Testing
Disciplina: Sistemas Embarcados — UTFPR
Prof. Max Mauro Dias Santos

The system under test is a single function, `mortgage`, that computes a mortgage
amount from an applicant's gender, age and monthly salary.

---

## Requirements

| ID | Requirement |
|----|-------------|
| **R1** | The function **shall** accept exactly three inputs: `gender`, `age` and `salary`. |
| **R2** | The function **shall** accept `age` only in the closed interval `[18, 55]` (years, integer). |
| **R3** | The function **shall** accept `salary` only in the closed interval `[0, 10000]` (currency units, integer). |
| **R4** | For `gender = Male`, the function **shall** apply the multiplication factor defined by the age band: `[18, 35] → 75`, `[36, 45] → 55`, `[46, 55] → 30`. |
| **R5** | For `gender = Female`, the function **shall** apply the multiplication factor defined by the age band: `[18, 30] → 70`, `[31, 40] → 50`, `[41, 50] → 35`. The band `[51, 55]` is **not defined by the specification** (specification gap) and **shall** be rejected. |
| **R6** | Any input outside the accepted domain (R2, R3, R5-gap) **shall** be rejected by throwing `std::out_of_range`. |
| **R7** | For accepted inputs, the returned value **shall** be `salary × factor`, where `factor` is the applicable factor from R4/R5. |

---

## Derived decision tables

### R4 — Male

| Age band | Factor | Valid |
|---|---|---|
| `age < 18`   | —  | rejected (R2/R6) |
| `18 … 35`    | 75 | yes |
| `36 … 45`    | 55 | yes |
| `46 … 55`    | 30 | yes |
| `age > 55`   | —  | rejected (R2/R6) |

### R5 — Female

| Age band | Factor | Valid |
|---|---|---|
| `age < 18`   | —  | rejected (R2/R6) |
| `18 … 30`    | 70 | yes |
| `31 … 40`    | 50 | yes |
| `41 … 50`    | 35 | yes |
| `51 … 55`    | —  | rejected (R5 specification gap / R6) |
| `age > 55`   | —  | rejected (R2/R6) |

### R3 — Salary

| Salary | Valid |
|---|---|
| `salary < 0`      | rejected (R3/R6) |
| `0 … 10000`       | accepted |
| `salary > 10000`  | rejected (R3/R6) |

---

## Equivalence classes

**Age (male):** `A1 = (-inf, 17]` invalid · `A2 = [18, 35]` · `A3 = [36, 45]` · `A4 = [46, 55]` · `A5 = [56, +inf)` invalid
**Age (female):** `A1 = (-inf, 17]` invalid · `A2 = [18, 30]` · `A3 = [31, 40]` · `A4 = [41, 50]` · `A5 = [51, 55]` invalid (gap) · `A6 = [56, +inf)` invalid
**Salary:** `S1 = (-inf, -1]` invalid · `S2 = [0, 10000]` valid · `S3 = [10001, +inf)` invalid
**Gender:** `G1 = Male` · `G2 = Female`

**Weak equivalence class testing:** one test case per class index, classes covered in
parallel (max(|A|,|S|,|G|) cases).
**Strong equivalence class testing:** the Cartesian product `G × A × S` of the valid
classes (2 × ~3 × 1 = 6 nominal combinations), plus the invalid classes exercised
one-at-a-time (robustness / weak robustness testing).

---

## Boundary values

Per R2/R3/R4/R5 the boundary values that must be exercised are:

- Age domain: `17, 18, 55, 56`
- Male band edges: `18, 35, 36, 45, 46, 55`
- Female band edges: `18, 30, 31, 40, 41, 50, 51, 55`
- Salary domain: `-1, 0, 1, 9999, 10000, 10001`
