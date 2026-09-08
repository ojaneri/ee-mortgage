# Requirements

Osvaldo Janeri Filho <janeri@gmail.com>
Project #1 — Functional Testing · Sistemas Embarcados, UTFPR

The system under test is a single function that works out how much mortgage an
applicant qualifies for, given their gender, age and monthly salary.

## R1 — R7

**R1.** The function shall take exactly three inputs: gender, age and salary.

**R2.** Age shall be an integer between 18 and 55 inclusive. Anything else is
outside the domain.

**R3.** Salary shall be an integer between 0 and 10000 inclusive.

**R4.** For a male applicant the function shall apply the factor given by his
age band: 75 for ages 18 to 35, 55 for 36 to 45, 30 for 46 to 55.

**R5.** For a female applicant the function shall apply the factor given by her
age band: 70 for ages 18 to 30, 50 for 31 to 40, 35 for 41 to 50. Ages 51 to 55
are inside the valid age range of R2 but the specification assigns them no
factor. This is a gap in the specification, not an oversight in the
requirement, and the function shall reject those inputs rather than guess.

**R6.** Any input outside the accepted domain — age outside R2, salary outside
R3, or a woman in the undefined band of R5 — shall be rejected by throwing
`std::out_of_range`.

**R7.** For accepted inputs the function shall return `salary × factor`.

## The tables spelled out

Men:

| Age | Factor |
|---|---|
| below 18 | rejected, R2 |
| 18 to 35 | 75 |
| 36 to 45 | 55 |
| 46 to 55 | 30 |
| above 55 | rejected, R2 |

Women:

| Age | Factor |
|---|---|
| below 18 | rejected, R2 |
| 18 to 30 | 70 |
| 31 to 40 | 50 |
| 41 to 50 | 35 |
| 51 to 55 | rejected, R5 — no factor specified |
| above 55 | rejected, R2 |

Salary is simpler: below 0 or above 10000 is rejected, everything in between is
accepted, including 0 itself. A salary of zero produces a mortgage of zero,
which is a perfectly good answer and not an error.

## Equivalence classes

Derived from the tables above, for use in `tests/equivalence_test.cpp`.

Gender splits into two classes, male and female, both valid.

Age splits differently per gender, which is worth noticing — it is why the age
partition cannot be defined independently of gender:

- men: `<18` invalid, `18–35`, `36–45`, `46–55`, `>55` invalid — five classes
- women: `<18` invalid, `18–30`, `31–40`, `41–50`, `51–55` invalid, `>55`
  invalid — six classes

Salary splits into three: `<0` invalid, `0–10000` valid, `>10000` invalid.

Weak equivalence class testing needs as many cases as the largest partition has
classes, covering the class indices in parallel. Strong equivalence class
testing needs the full cross product: for the valid classes that is 2 × 3 × 1 =
6 cases. The robust variants of both add the invalid classes, either one at a
time (weak robust, single-fault assumption) or in combination (strong robust).

## Boundary values

The values worth testing, collected in one place:

- age, at the domain edges: 17, 18, 55, 56
- age, at the seams between male bands: 35, 36, 45, 46
- age, at the seams between female bands: 30, 31, 40, 41, 50, 51
- salary: −1, 0, 1, 9999, 10000, 10001

The seams matter more than the domain edges. A domain edge is usually guarded
by an explicit range check that someone remembered to write; a seam between two
adjacent bands is guarded by whichever comparison operator got typed first, and
that is where off-by-one errors live.
