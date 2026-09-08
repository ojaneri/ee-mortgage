# Mortgage Testing Lab

**Project #1 — Functional Testing**
Disciplina: Sistemas Embarcados — UTFPR
Prof. Max Mauro Dias Santos

A complete, runnable lab that demonstrates the five classical functional
(black-box) testing techniques — **nominal**, **boundary value**, **robustness**,
**worst-case** and **equivalence class** testing — on a deliberately small
system under test: a `mortgage(gender, age, salary)` function.

The point of the lab is *not* the function. It is the demonstration that a test
suite derived methodically from requirements finds defects that a suite written
by intuition does not. The repository therefore ships **two** implementations of
the same interface and one test suite that runs against either:

| Implementation | Purpose | Expected result |
|---|---|---|
| `src/mortgage_buggy.cpp` | 12 seeded defects (B1…B12) | suite goes **RED** (23/39 fail) |
| `src/mortgage.cpp` | conformant to R1–R7 | suite goes **GREEN** (39/39 pass) |

---

## The specification in one table

`mortgage = salary × factor`, with the factor selected by gender and age band.

| Gender | Age band | Factor |
|---|---|---|
| Male | 18 – 35 | 75 |
| Male | 36 – 45 | 55 |
| Male | 46 – 55 | 30 |
| Female | 18 – 30 | 70 |
| Female | 31 – 40 | 50 |
| Female | 41 – 50 | 35 |
| Female | 51 – 55 | **undefined — specification gap, rejected** |

Domain: `age ∈ [18, 55]`, `salary ∈ [0, 10000]`. Anything outside the domain —
including the female `[51, 55]` gap — is rejected with `std::out_of_range`.

The formal requirements R1–R7 live in
[`requirements/mortgage_requirements.md`](requirements/mortgage_requirements.md).

---

## Repository layout

```
mortgage-testing-lab/
├── README.md
├── CMakeLists.txt                        # FetchContent -> GoogleTest v1.15.2
├── requirements/
│   ├── mortgage_requirements.md          # R1–R7 as "shall" statements
│   ├── buggy_defects.md                  # B1–B12: bug, requirement, minimal test
│   └── test_evidence.md                  # real RED and GREEN console output
├── include/
│   └── mortgage.hpp                      # single interface for both versions
├── src/
│   ├── mortgage_buggy.cpp                # 12 seeded defects — DO NOT FIX
│   └── mortgage.cpp                      # corrected implementation
├── tests/
│   ├── nominal_test.cpp                  #  7 cases
│   ├── boundary_test.cpp                 # 19 cases
│   ├── robustness_test.cpp               #  8 cases
│   └── equivalence_test.cpp              #  5 cases (weak/strong, normal/robust)
└── .github/workflows/ci.yml              # cmake + build + ctest on ubuntu-latest
```

---

## Building and running

Requirements: CMake ≥ 3.14, a C++17 compiler, and network access on the first
configure (GoogleTest is fetched from GitHub).

### GREEN — the corrected implementation

```bash
cmake -S . -B build -DUSE_BUGGY=OFF
cmake --build build --parallel
ctest --test-dir build --output-on-failure
# 100% tests passed, 0 tests failed out of 39
```

### RED — the defective implementation

```bash
cmake -S . -B build-buggy -DUSE_BUGGY=ON
cmake --build build-buggy --parallel
ctest --test-dir build-buggy --output-on-failure
# 41% tests passed, 23 tests failed out of 39
```

A single CMake option, `USE_BUGGY`, selects which `.cpp` is linked into the
`mortgage` library. The tests never change — that is what makes the RED/GREEN
comparison meaningful.

Run one technique in isolation:

```bash
./build/boundary_test
./build/boundary_test --gtest_filter='BoundaryFemale.*'
```

---

## How the techniques are applied

**Nominal testing** (`nominal_test.cpp`) — one interior value per valid age
band, far from every boundary, with a mid-domain salary. Establishes that the
happy path works at all.

**Boundary value testing** (`boundary_test.cpp`) — min, min⁺, max⁻, max of each
partition *plus* the internal edges between adjacent age bands (18, 30, 31, 35,
36, 40, 41, 45, 46, 50, 51, 55) and the salary edges (0, 1, 9999, 10000). This
is where the off-by-one defects live: 8 of the 12 seeded bugs are caught here.

**Robustness testing** (`robustness_test.cpp`) — pushes past the domain edges
(17, 56, −1, 10001, `INT_MAX`, `INT_MIN`) and asserts that each is *rejected*
rather than silently priced. The defective version has no rejection mechanism
at all, so this file fails almost entirely against it.

**Worst-case testing** — combines extreme values in the same call
(`age = 17` and `salary = -1` simultaneously), covered at the end of
`robustness_test.cpp` and in the strong-robust equivalence cases.

**Equivalence class testing** (`equivalence_test.cpp`) — all four flavours:
*weak normal* (one case per class index), *strong normal* (Cartesian product of
the valid classes, 2 × 3 × 1 = 6 cells), *weak robust* (single-fault: one
invalid class at a time) and *strong robust* (multiple-fault: invalid age ×
invalid salary). A final `PartitionConsistency` test sweeps the whole valid age
domain to prove the bands contain neither a gap nor an overlap.

---

## The 12 seeded defects

Full analysis, with the requirement violated and the minimal exposing test for
each, in [`requirements/buggy_defects.md`](requirements/buggy_defects.md).

| Bug | Summary | Requirement |
|-----|---------|-------------|
| B1  | Male lower boundary 18 excluded (`18 < age`) | R4 |
| B2  | Male upper boundary 35 excluded (`age < 35`) | R4 |
| B3  | Male second band starts at 32 instead of 36 (overlap) | R4 |
| B4  | Male second band ends at 39 instead of 45 | R4 |
| B5  | No age-domain validation; unbounded catch-all | R2, R6 |
| B6  | Female lower boundary 18 excluded | R5 |
| B7  | Female first band uses factor 75 instead of 70 | R5, R7 |
| B8  | Female first band upper boundary 30 excluded | R5 |
| B9  | Female second band starts at 32 instead of 31 (gap at 31) | R5 |
| B10 | Female second band ends at 39 instead of 40 | R5 |
| B11 | Female `[51,55]` specification gap silently priced at 35 | R5, R6 |
| B12 | No salary-domain validation; no exception ever thrown | R3, R6 |

Each bug is tracked as a GitHub issue in this repository.

---

## Continuous integration

[`.github/workflows/ci.yml`](.github/workflows/ci.yml) runs on every push and
pull request with two jobs:

1. **build-and-test** — configures with `USE_BUGGY=OFF`, builds and runs
   `ctest`; the job fails if any test fails.
2. **buggy-must-fail** — configures with `USE_BUGGY=ON` and runs the same
   suite, failing the job if the suite *passes*. This guards against the suite
   silently degrading: a test suite that no longer detects the seeded defects
   has stopped being worth anything.

---

## Contribution flow for the team

1. Pick an issue (`B1` … `B12`).
2. Branch from `main`: `git checkout -b fix/<short-description>`.
3. Write the failing test **first**, confirm it goes RED against
   `src/mortgage_buggy.cpp`.
4. Commit, push, open a PR referencing the issue (`Refs #N` / `Closes #N`).
5. CI must be green before merging.

See the example PR opened from the `fix/age-boundaries` branch.

---

## License

Academic coursework — UTFPR, Sistemas Embarcados.
