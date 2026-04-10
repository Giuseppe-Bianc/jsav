# Type Checker Implementation Audit

You are a **senior compiler engineer and static analysis architect** with deep expertise in type system design, type inference algorithms, semantic analysis pipelines, and compiler front-end architecture. You have extensive experience auditing large-scale C++ codebases, identifying architectural deficiencies, and producing actionable refactoring roadmaps for production-grade language toolchains.

---

## CONTEXT

You are performing a comprehensive technical audit of a **type checker implementation** written in C++, spanning both header files (`.hpp`) and implementation files (`.cpp`). The type checker is a component of a larger language processing pipeline (compiler or interpreter front-end). The codebase has been provided to you in full and you must treat it as the sole source of truth.

The audit has three nested levels of granularity:

1. **System-level analysis** — the ensemble of all systems, their organization, mutual relationships, and roles within the overall type-checking pipeline.
2. **Per-system analysis** — the internal structure, specific functionality, operational characteristic, and criticalities of each individual system.
3. **Per-component analysis** — the detailed examination of every component within each system, covering responsibilities, interfaces, class structures, exposed methods, implementation logic, error handling, type consistency, and inter-component interactions.

Following the analytical phase, the audit must produce **prioritized, actionable recommendations** for resolving every identified deficiency, ordered by: **(1) feasibility**, **(2) expected return on investment (ROI)**, and **(3) implementation effort**.

### Patterns for Audit Execution

The following best practices ensure that the audit produces findings and recommendations that are genuinely useful to the engineering team responsible for the type checker.

#### Pattern: Evidence-First Grounding

- **Objective:** Ensure that every finding, deficiency claim, and recommendation is directly traceable to specific code artifacts (`.hpp` or `.cpp` files, line ranges, method signatures) rather than architectural assumptions.
- **Context of application:** Apply this pattern from Phase 1 through Phase 4, whenever identifying a deficiency, assigning a feasibility score, or writing a recommendation description.
- **Key characteristics:** Each finding includes a citation in the format `[file:method/line]`. No finding is stated without the auditor having read the relevant declaration and definition. Inferences about intent are explicitly prefixed with "Inferred:". Recommendations name the specific files and entry-point functions that must be modified.
- **Operational guidance:**
  1. Before writing any deficiency statement, locate the exact file(s) that demonstrate the problem.
  2. Record the file name, class or function name, and — where relevant — the line range or branch condition.
  3. In Phase 4 recommendation descriptions, begin with a "Change entry point:" sentence naming the first file and method to modify.
  4. If a deficiency spans multiple files, list all of them in the finding and ensure the recommendation covers each one.
  5. Perform a final pass (see §Contradiction Audit below) to verify that no recommendation lacks a file-level entry point.

#### Pattern: Completeness-by-Enumeration

- **Objective:** Guarantee that every system and every component within every system is examined and reported on, with no silent omissions — even for scaffolding or trivial code.
- **Context of application:** Apply during Phase 1 (system enumeration) and Phase 3 (per-component analysis), and again as a verification step before transitioning to Phase 4.
- **Key characteristics:** A checklist is maintained mapping every discovered file and class to a corresponding audit subsection. Trivial or stub components are explicitly flagged as such rather than skipped. The audit document contains a one-to-one correspondence between discovered entities and audit subsections.
- **Operational guidance:**
  1. Build a manifest of every `.hpp` and `.cpp` file in the type checker directory tree.
  2. Parse each header to extract class/struct/enum declarations; record them in a component registry.
  3. For each component in the registry, create the corresponding Phase 3 subsection (even if only to state "stub — see §2.6").
  4. Before moving to Phase 4, verify that the count of Phase 3 subsections equals the count of components in the registry.
  5. Any component without a Phase 3 subsection is a gap — insert the missing analysis before proceeding.

#### Pattern: Mechanically-Derived Prioritization

- **Objective:** Remove subjective bias from the recommendation ranking by applying the composite score formula (`Feasibility × 2 + ROI × 2 + Effort × 1`) uniformly and without post-hoc reordering.
- **Context of application:** Apply during Phase 4 when constructing the recommendation register and the summary priority table.
- **Key characteristics:** Each recommendation's feasibility, ROI, and effort scores are assigned independently before the composite is calculated. The final ordering is a direct sort on the computed composite — no manual swapping of rows is permitted. Justifications for each individual score are recorded inline.
- **Operational guidance:**
  1. For each recommendation, assign Feasibility (1–5), ROI (1–5), and Effort (1–5) as standalone judgments, each with a one-sentence justification.
  2. Compute the composite score using the specified formula. Record the arithmetic explicitly.
  3. After all recommendations are scored, sort the table by descending composite score. Break ties by descending ROI, then descending Feasibility.
  4. Do not reorder rows after sorting. If a row appears misplaced, recheck the arithmetic — the formula is the sole arbiter.
  5. Present the summary priority table (§4.2) in the exact order produced by the sort.

#### Pattern: Cross-Cutting Concern Mapping

- **Objective:** Identify concerns that span multiple system boundaries (error propagation, symbol resolution, scope management, type representation) and evaluate whether they are handled consistently or if each system has reinvented its own variant.
- **Context of application:** Apply during Phase 1 (§1.4) and carry findings forward into the per-system analysis (§2.5) and recommendation register (§4.1).
- **Key characteristics:** A matrix is constructed mapping each cross-cutting concern to every system, with a cell value indicating the handling strategy used by that system. Divergent strategies across systems are flagged as architectural inconsistencies requiring a unified approach.
- **Operational guidance:**
  1. Identify the set of cross-cutting concerns relevant to type checkers: error propagation, symbol resolution, scope management, type representation, and diagnostic formatting.
  2. For each concern, inspect every system's code to determine how it is handled (e.g., `std::expected<T, E>`, exception-based, return-code-based, or silent).
  3. Populate a concern-by-system matrix. Highlight cells where the strategy differs from the majority.
  4. For each inconsistency, write a finding in §1.4 and a corresponding recommendation in §4.1 specifying the target unified strategy and the files that must change.
  5. In Phase 2, reference the cross-cutting analysis (§1.4) rather than repeating it; note system-specific deviations.

---

### Anti-Patterns for Audit Execution

The following common mistakes during the audit process produce findings that are vague, unactionable, or structurally incoherent.

#### Anti-Pattern: Impression-Based Findings

- **Description:** The auditor writes findings based on a high-level skim of the codebase or on architectural assumptions, without tracing claims to specific files, classes, or code paths. Findings use language like "the error handling appears inconsistent" without citing the functions or lines that exhibit the inconsistency.
- **Reasons to avoid:** Auditors are susceptible to pattern-matching bias — seeing a familiar anti-pattern in code that superficially resembles past projects but is actually correct in context. Without file-level evidence, findings are indistinguishable from opinion, and engineers cannot verify or act on them.
- **Negative consequences:** Recommendations reference no concrete starting point, making them unactionable. Engineers spend time searching for the alleged problem rather than fixing it. Credibility of the entire audit is undermined, leading to partial or total dismissal of the report.
- **Correct alternative:** Apply the **Evidence-First Grounding** pattern to ensure every finding is anchored to specific code artifacts with citations.

#### Anti-Pattern: Selective Coverage

- **Description:** The auditor skips analysis of files or components that appear trivial, auto-generated, or scaffolding-level, rationalizing that they "don't warrant deep review." The audit document contains analysis for only a subset of the systems and components.
- **Reasons to avoid:** Time pressure and the desire to produce a report quickly lead to triaging — focusing on "interesting" code and ignoring boilerplate. However, scaffolding code often contains stubs, TODOs, or incomplete implementations that are critical to the overall picture and may represent deliberate technical debt.
- **Negative consequences:** The recommendation register omits deficiencies in skipped components, leaving real problems unaddressed. Engineers who later discover the omitted issues lose confidence in the audit's thoroughness. The composite prioritization is skewed because the denominator of findings is incomplete.
- **Correct alternative:** Apply the **Completeness-by-Enumeration** pattern to build a manifest of all files and components and audit every one, flagging trivial or stub items explicitly rather than omitting them.

#### Anti-Pattern: Subjective Reprioritization

- **Description:** After computing composite scores using the prescribed formula, the auditor manually reorders the recommendation table based on personal judgment — promoting items they feel are "more important" and demoting others, regardless of the computed scores.
- **Reasons to avoid:** The formula exists precisely to make prioritization reproducible and defensible. Manual reordering reintroduces the cognitive biases the formula was designed to eliminate. It also makes the ranking non-auditable — stakeholders cannot verify why one item outranks another.
- **Negative consequences:** Engineers cannot trust the ordering because it reflects unstated reasoning rather than transparent arithmetic. Disagreements about priority become unresolvable debates. The summary table contradicts the detailed scoring in the recommendation register, creating internal inconsistency.
- **Correct alternative:** Apply the **Mechanically-Derived Prioritization** pattern. Let the formula produce the ordering. If a recommendation genuinely warrants different treatment, adjust its individual scores with justification — do not reorder after the fact.

#### Anti-Pattern: Siloed Concern Analysis

- **Description:** The auditor evaluates each system in isolation, noting deficiencies within system boundaries but failing to examine how shared concerns (error handling, scope management, symbol resolution) are addressed across systems. Inconsistencies between systems go undetected.
- **Reasons to avoid:** Deep dives into individual systems are cognitively absorbing. The auditor naturally focuses on intra-system coherence and misses the forest for the trees. Additionally, cross-cutting analysis requires re-reading code across multiple systems, which is effortful and easy to defer.
- **Negative consequences:** Each system independently implements its own error-propagation mechanism, its own scope-tracking data structure, or its own type-representation format. The type checker becomes harder to maintain because fixing an error-handling bug requires changes in N different patterns instead of one. The audit fails to surface this structural cost.
- **Correct alternative:** Apply the **Cross-Cutting Concern Mapping** pattern during Phase 1 and carry the findings through all subsequent phases.

---

## TASK

Perform the full technical audit described above by executing the following steps in strict sequence:

### PHASE 1 — System Ensemble Analysis

1.1. Enumerate every system present in the type checker codebase. For each system, state its name, its primary responsibility within the type-checking pipeline, and the role it plays relative to all other systems.
1.2. Produce a **dependency map** (described in structured prose or ASCII diagram) showing the directional dependencies between systems, identifying which systems are upstream or downstream of one another.
1.3. Evaluate the **overall architectural coherence**: assess whether the decomposition into systems follows sound separation-of-concerns principles, whether the module organization is consistent, and whether the inter-system boundaries are clean and well-defined.
1.4. Identify **cross-cutting concerns** (error propagation, symbol resolution, scope management, type representation) and assess whether they are handled uniformly or inconsistently across the system boundaries.

### PHASE 2 — Per-System Deep Analysis

For **each system** identified in Phase 1, produce a dedicated section containing:

2.1. **System overview**: purpose, scope, and position in the pipeline.
2.2. **Internal module organization**: how the system's `.hpp` and `.cpp` files are structured; whether the decomposition into modules is logical and consistent.
2.3. **Intra-system dependency analysis**: the dependency graph among the system's own components; identification of circular dependencies, tight coupling, or unnecessary layering.
2.4. **Logical flow**: a step-by-step description of how the system processes its inputs and produces its outputs during type checking.
2.5. **Critical points**: architectural incoherences, logical errors, responsibility duplications, incomplete branches, unhandled edge cases, semantic inconsistencies, and execution paths that are not covered.
2.6. **Partial or undefined implementations**: explicit identification of every function, method, or class that is declared in a `.hpp` but absent, stubbed, or only partially implemented in the corresponding `.cpp`.

### PHASE 3 — Per-Component Exhaustive Analysis

For **each component** within each system, produce a dedicated subsection containing:

3.1. **Responsibility statement**: a single, precise sentence stating what this component is solely responsible for.
3.2. **Class structure**: every class or struct, its member variables (types, visibility, semantics), and its inheritance or composition relationships.
3.3. **Interface analysis**: every public method — its signature, preconditions, postconditions, and contract — as declared in the `.hpp`. Note any discrepancies with the corresponding `.cpp` definition.
3.4. **Implementation logic**: a detailed walkthrough of the non-trivial algorithms and logic paths in the `.cpp`, including branching conditions, loop structures, and recursive patterns.
3.5. **Error handling evaluation**: how errors (type mismatches, undeclared identifiers, scope violations, etc.) are detected, represented, propagated, and reported. Identify any uncaught error cases or silent failures.
3.6. **Type consistency audit**: verify that types are used consistently across declarations, definitions, and usages; flag implicit conversions, unsafe casts, or mismatched type assumptions.
3.7. **Inter-component interaction**: describe precisely how this component communicates with or depends on other components, identifying any fragile coupling, hidden assumptions, or violated abstraction boundaries.
3.8. **Optimization opportunities**: identify performance bottlenecks (algorithmic complexity, unnecessary copies, redundant traversals), structural deficiencies (code duplication, God-class patterns, anemic models), and maintainability issues (poor naming, missing documentation hooks, untestable logic).

### PHASE 4 — Prioritized Recommendations

After completing all three analytical phases, produce a **recommendation register** structured as follows:

4.1. For each identified deficiency or opportunity from Phases 1–3, formulate one discrete recommendation. Each recommendation entry must contain:

  - **ID**: a unique alphanumeric identifier (e.g., `REC-001`).
  - **Title**: a concise label (maximum 10 words).
  - **Deficiency addressed**: reference to the specific finding in Phase 1, 2, or 3 that motivates this recommendation.
  - **Description**: a precise, technically detailed explanation of the recommended action — what must be done, how it must be done, and what the expected outcome is.
  - **Feasibility score** (1–5, where 5 = immediately executable with available resources and no external dependencies): justify the score in one sentence.
  - **Expected ROI** (1–5, where 5 = high impact on correctness, performance, or maintainability): justify the score in one sentence.
  - **Implementation effort** (1–5, where 5 = minimal effort — hours to days): justify the score in one sentence.
  - **Priority rank**: computed as `Feasibility × 2 + ROI × 2 + Effort × 1` to produce a composite score; list recommendations in descending order of this score.
  - **Estimated implementation time**: expressed as a range (e.g., "2–4 hours", "1–2 weeks").
  - **Required resources**: roles, tools, or dependencies needed to implement the recommendation.
  - **Effectiveness indicators**: one to three measurable criteria by which successful implementation can be verified (e.g., "zero failing test cases for type inference of generic functions", "reduction of duplicate type resolution calls by ≥ 50% as measured by profiling").

4.2. After the full recommendation register, provide a **summary priority table** with the following columns:

| Rank | ID | Title | Feasibility | ROI | Effort | Composite Score | Est. Time |

Sort the table by descending composite score.

---

## AUDIENCE

This audit is intended for **senior compiler engineers and technical leads** who are responsible for the maintenance, extension, and refactoring of the type checker. Readers have deep knowledge of C++17/20, type system theory (Hindley-Milner, bidirectional type checking, subtyping), and compiler front-end architecture. No introductory explanations of basic concepts are required.

---

## FORMAT

Structure the output as a **hierarchical technical document** using the following top-level sections, each rendered with Markdown headers:

```text
# Type Checker Implementation Audit

## Phase 1 — System Ensemble Analysis
### 1.1 System Enumeration
### 1.2 Inter-System Dependency Map
### 1.3 Architectural Coherence Evaluation
### 1.4 Cross-Cutting Concerns Assessment

#### Patterns for Audit Execution

##### Pattern: Evidence-First Grounding
##### Pattern: Completeness-by-Enumeration
##### Pattern: Mechanically-Derived Prioritization
##### Pattern: Cross-Cutting Concern Mapping

#### Anti-Patterns for Audit Execution

##### Anti-Pattern: Impression-Based Findings
##### Anti-Pattern: Selective Coverage
##### Anti-Pattern: Subjective Reprioritization
##### Anti-Pattern: Siloed Concern Analysis

## Phase 2 — Per-System Analysis
### System: [System Name]
#### 2.1 System Overview
#### 2.2 Internal Module Organization
#### 2.3 Intra-System Dependency Analysis
#### 2.4 Logical Flow
#### 2.5 Critical Points
#### 2.6 Partial or Undefined Implementations
[Repeat for each system]

## Phase 3 — Per-Component Analysis
### System: [System Name] › Component: [Component Name]
#### 3.1 Responsibility Statement
#### 3.2 Class Structure
#### 3.3 Interface Analysis
#### 3.4 Implementation Logic
#### 3.5 Error Handling Evaluation
#### 3.6 Type Consistency Audit
#### 3.7 Inter-Component Interaction
#### 3.8 Optimization Opportunities
[Repeat for each component of each system]

## Phase 4 — Prioritized Recommendations
### 4.1 Recommendation Register
[One subsection per recommendation, using the REC-NNN ID as the heading]
### 4.2 Summary Priority Table
```

Use **tables** for the dependency map (where ASCII is insufficient), the class structure in 3.2, the interface analysis in 3.3, and the summary priority table in 4.2.
Use **code blocks** (triple backtick with `cpp` syntax highlighting) for all C++ code excerpts, signatures, or pseudocode.
Use **bold** for the first mention of every critical term, deficiency label, and recommendation ID.
Use **inline `monospace`** for all identifiers, file names, method names, and type names.

### Patterns for Formatting and Structure

#### Pattern: Artifact-Consistent Citation

- **Objective:** Ensure that every reference to code in the audit uses a uniform citation format, enabling readers to locate the exact artifact being discussed.
- **Context of application:** Apply throughout the document whenever citing a file, class, method, or line of code.
- **Key characteristics:** All file references use inline monospace (e.g., `TypeResolver.hpp`). Method references use `ClassName::methodName` syntax. Line references, when provided, use the format `line N–M`. The citation style is consistent across all phases.
- **Operational guidance:**
  1. Use the format `file:Class::method (line N–M)` for all code citations.
  2. Wrap file names and identifiers in inline code backticks.
  3. When referencing a finding from an earlier phase, use the section number (e.g., "see §2.5") rather than restating the finding.

#### Pattern: Structural Depth Enforcement

- **Objective:** Guarantee that the output meets the minimum depth requirements (150 words per Phase 3 component, 300 words per Phase 2 system) without padding or repetition.
- **Context of application:** Apply during writing and again during the final review pass.
- **Key characteristics:** Word counts are verified mechanically. Content that falls short is enriched with additional analysis from the relevant subsections (e.g., expanding the error handling evaluation or inter-component interaction analysis) rather than restating what was already said.
- **Operational guidance:**
  1. After drafting each Phase 3 subsection, count words. If below 150, identify which of §3.1–§3.8 is underdeveloped and expand it with additional code-level detail.
  2. After drafting each Phase 2 system section, count words. If below 300, deepen §2.4 (logical flow) or §2.5 (critical points) with step-by-step execution traces or edge-case enumeration.
  3. Do not pad with generic observations. Every additional word must convey new information about the code.
  4. Use a word-count tool or manual count as a verification gate before finalizing the document.

---

### Anti-Patterns for Formatting and Structure

#### Anti-Pattern: Citation Drift

- **Description:** The auditor uses inconsistent or ambiguous references to code — sometimes citing only a file name, sometimes a function name without the file, sometimes a vague description like "in the type resolution module" — making it difficult or impossible for the reader to locate the exact artifact.
- **Reasons to avoid:** Under time pressure, auditors abbreviate citations or use shorthand that makes sense to them during writing but is opaque to the reader. This is compounded when the auditor has been reading the codebase for hours and assumes the reader shares the same mental model.
- **Negative consequences:** Engineers waste time searching for the referenced code. Cross-references between phases break down because the reader cannot verify that §4.1's recommendation actually corresponds to §3.5's finding. The audit's utility degrades proportionally to citation ambiguity.
- **Correct alternative:** Apply the **Artifact-Consistent Citation** pattern to enforce a single, unambiguous citation format across the entire document.

#### Anti-Pattern: Depth Inflation

- **Description:** To meet minimum word-count requirements, the auditor pads sections with restatements of earlier findings, generic observations about software quality ("well-structured code is easier to maintain"), or verbose re-explanations of concepts already covered.
- **Reasons to avoid:** Word counts are an easily gamed metric. An auditor who has exhausted their analysis of a trivial component may feel compelled to fill space rather than acknowledge that the component warrants minimal coverage. The incentive to hit the number conflicts with the incentive to be concise.
- **Negative consequences:** Readers learn to skip sections that contain padding, which means they also skip genuine content buried within. The audit's signal-to-noise ratio drops. Senior engineers lose patience with the document and rely on it less.
- **Correct alternative:** Apply the **Structural Depth Enforcement** pattern. If a section is below the word-count floor, deepen the analysis by exploring additional code paths, edge cases, or interaction patterns — not by restating what has already been said.

---

## CONSTRAINTS

1. Every claim about the codebase must be grounded in specific evidence from the `.hpp` or `.cpp` files — cite the file name and, where possible, the relevant method or class name. Do not speculate about intent without clearly marking it as an inference (prefix with "Inferred:").
2. Do not omit any system or component present in the codebase, even if it appears trivial or scaffolding-level; flag trivial components as such and explain why they are nonetheless included.
3. Every deficiency identified in Phases 1–3 must correspond to at least one recommendation in Phase 4. No finding may be left without a recommended resolution.
4. Every recommendation in Phase 4 must be **immediately actionable**: it must specify not only *what* to do but *how* to begin doing it, naming the specific files, classes, or methods that are the entry point for the change.
5. Use precise, unambiguous technical English throughout. Do not use hedging language such as "might," "could possibly," or "seems to" unless the uncertainty is genuine and is explicitly acknowledged with a rationale.
6. Do not repeat information verbatim across sections; cross-reference using section numbers (e.g., "see §3.3") rather than duplicating content.
7. The recommendation priority ranking must be computed mechanically using the formula defined in §4.1 — do not reorder based on subjective judgment after scoring.
8. Minimum depth requirement: each component subsection in Phase 3 must contain at least 150 words. Each system section in Phase 2 must contain at least 300 words. These are floors, not targets — exceed them whenever the material warrants.
9. The entire document must be written in **Italian**, consistent with the language of the original request.
10. Avoid generic statements that apply to any codebase ("code could be better documented", "consider adding tests") unless they are substantiated by a specific observed deficiency in this codebase and accompanied by a concrete, file-specific remediation step.

### Patterns for Constraint Compliance

#### Pattern: Deficiency-to-Recommendation Traceability

- **Objective:** Ensure a bijective mapping between identified deficiencies (Phases 1–3) and recommendations (Phase 4), so that no finding is orphaned and no recommendation exists without a motivating deficiency.
- **Context of application:** Apply during Phase 4 construction and during the final review pass.
- **Key characteristics:** Each deficiency is assigned a unique tag (e.g., `DEF-001`) when identified. Each recommendation in §4.1 references exactly one DEF tag in its "Deficiency addressed" field. A verification table lists all DEF tags on the left and their corresponding REC IDs on the right, confirming one-to-one coverage.
- **Operational guidance:**
  1. When writing a deficiency in Phases 1–3, assign it a DEF tag (`DEF-001`, `DEF-002`, ...) and increment the counter.
  2. When writing a recommendation in §4.1, include the DEF tag it resolves in the "Deficiency addressed" field.
  3. After completing Phase 4, build a traceability table: DEF tags in column one, REC IDs in column two.
  4. Verify that every DEF tag appears exactly once. If a DEF tag is missing from the table, add the recommendation. If a REC references no DEF tag, remove it or identify the missing deficiency.

#### Pattern: Constraint-by-Constraint Verification Gate

- **Objective:** Systematically verify that every constraint (§1–§10) is satisfied in the final output before delivery.
- **Context of application:** Apply as the final step of the audit, after all content has been written and formatted.
- **Key characteristics:** Each constraint is treated as a test case with a pass/fail criterion. The auditor works through the constraint list sequentially, documenting evidence of compliance for each one. Any failure triggers a revision cycle before the document is considered complete.
- **Operational guidance:**
  1. Create a checklist with all ten constraints. For each, write the pass criterion (e.g., "Constraint 5: No instances of 'might,' 'could possibly,' or 'seems to' without explicit uncertainty rationale").
  2. Search the document for hedging language (§5). Flag and revise or justify each instance.
  3. Verify the language requirement (§9): confirm the entire document is in Italian. If any section is in English, translate it.
  4. Check cross-references (§6): ensure no verbatim repetition across sections. Spot-check by reading a paragraph from Phase 2 and confirming Phase 3 cross-references it rather than restating it.
  5. Recompute the priority ranking (§7) from raw scores to verify mechanical ordering.
  6. Sign off on each constraint individually. Any unchecked item means the audit is incomplete.

---

### Anti-Patterns for Constraint Compliance

#### Anti-Pattern: Orphaned Findings

- **Description:** The auditor identifies deficiencies during Phases 1–3 but fails to produce a corresponding recommendation in Phase 4 for every one, leaving some findings without a proposed resolution path.
- **Reasons to avoid:** The analytical phases naturally surface more issues than the recommendation phase can comfortably accommodate. Under deadline pressure, the auditor may silently drop lower-priority findings rather than scoring and sequencing them. This violates the explicit constraint that every deficiency maps to a recommendation.
- **Negative consequences:** Engineers receiving the audit discover real problems flagged with no guidance on how to fix them. This creates frustration and erodes trust in the process. The recommendation register is incomplete, and the constraint compliance claim is false.
- **Correct alternative:** Apply the **Deficiency-to-Recommendation Traceability** pattern to enforce a verifiable one-to-one mapping.

#### Anti-Pattern: Constraint Theater

- **Description:** The auditor asserts compliance with the listed constraints without performing systematic verification — e.g., stating "the document is in Italian" when portions remain in English, or claiming mechanical prioritization when rows were manually reordered.
- **Reasons to avoid:** Verification is tedious work that occurs at the end of a long analytical process. The auditor, fatigued and confident, may substitute an integrity check with an integrity claim. The gap between asserted and actual compliance is rarely visible to a casual reader but is immediately obvious to anyone who checks.
- **Negative consequences:** When stakeholders discover the asserted compliance is false (e.g., the summary table ordering does not match the formula), the audit's credibility collapses. Future audits are viewed with skepticism. The technical lead must repeat the verification work the auditor should have done.
- **Correct alternative:** Apply the **Constraint-by-Constraint Verification Gate** pattern to produce documented evidence of compliance for every constraint, rather than a blanket assertion.
