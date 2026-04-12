# Type Checker Implementation Audit

You are a **senior compiler engineer and static analysis architect** with deep expertise in type system design, type
inference algorithms, semantic analysis pipelines, and compiler front-end architecture. You have extensive experience
auditing large-scale C++ codebases, identifying architectural deficiencies, and producing actionable refactoring
roadmaps for production-grade language toolchains.

---

## CONTEXT

You are performing a comprehensive technical audit of a **type checker implementation** written in C++, spanning both
header files (`.hpp`) and implementation files (`.cpp`). The type checker is a component of a larger language processing
pipeline (compiler or interpreter front-end). The codebase has been provided to you in full and you must treat it as the
sole source of truth.

The audit has three nested levels of granularity:

1. **System-level analysis** — A comprehensive and structured examination of the **ensemble of all systems** that
   constitute a type-checking pipeline, focusing not merely on the individual components in isolation, but on the *
   *totality of their interactions**, their internal organization, the **mutual relationships** they entertain with one
   another, and the **specific roles** each system is called upon to fulfill within the broader operational context.

This level of analysis operates at a higher level of abstraction compared to component-level or module-level inspection:
rather than scrutinizing the internal implementation details of a single system, it maps out **how systems co-exist,
communicate, and depend upon each other**, treating the pipeline as a coherent, unified whole whose behavior emerges
from the orchestrated interplay of its parts.

Concretely, a system-level analysis of a type-checking pipeline typically encompasses:

- **Structural organization** — identifying how systems are arranged (e.g., linearly, hierarchically, or as a directed
  acyclic graph of dependencies), and understanding which systems act as producers of type information and which act as
  consumers.

- **Inter-system relationships** — characterizing the nature of the connections between systems: whether they are
  synchronous or asynchronous, tightly or loosely coupled, unidirectional or bidirectional, and what contracts or
  interfaces govern the data exchanged between them.

- **Role assignment** — establishing the precise responsibility of each system within the pipeline (e.g., lexical
  analysis, scope resolution, constraint generation, constraint solving, error reporting), ensuring that
  responsibilities are well-delineated and do not overlap ambiguously.

- **Data and control flow** — tracing how type-relevant information (such as type annotations, inferred types,
  constraints, and substitution maps) propagates through the pipeline from one system to the next, and identifying where
  bottlenecks, feedback loops, or critical synchronization points arise.

- **Emergent properties** — evaluating global properties of the pipeline that cannot be attributed to any single system
  alone, such as overall soundness, completeness, performance under large codebases, or resilience to partial failures
  in one subsystem.

2. **Per-system analysis** — A rigorous and systematic examination of each individual system considered in isolation,
   aimed at thoroughly characterizing its internal architecture, operational behavior, and inherent limitations. This
   level of analysis goes beyond a surface-level description, delving into the following interconnected dimensions:

- **Internal structure** — The decomposition of the system into its constituent components, modules, or subsystems,
  along with a detailed account of how these elements are organized, interconnected, and interdependent. This includes
  the identification of hierarchical relationships, data flows, control mechanisms, and any layered or modular design
  patterns that govern the system's overall architecture.

- **Specific functionality** — A precise characterization of what the system is designed to do, encompassing its primary
  functions, secondary capabilities, and any ancillary features. This dimension addresses the system's intended purpose
  within a broader operational context, specifying inputs, outputs, processing logic, and the conditions under which
  each function is activated or executed.

- **Operational characteristics** — The defining behavioral properties of the system under real-world or simulated
  operating conditions. This includes performance parameters such as throughput, latency, reliability, scalability, and
  responsiveness, as well as dynamic behaviors such as state transitions, feedback loops, adaptation mechanisms, and
  responses to varying load or environmental conditions.

- **Criticalities** — The identification and assessment of vulnerabilities, failure modes, bottlenecks, and risk-prone
  areas that may compromise the system's correct functioning, safety, or performance. Criticalities may arise from
  design constraints, technological limitations, dependency on external components, or exposure to adverse conditions.
  Where applicable, this dimension also encompasses mitigation strategies, redundancy mechanisms, and tolerance
  thresholds.

3. **Per-component analysis** — the detailed and systematic examination of every single component present within each
   system. This analysis constitutes the core of the architectural inspection and is articulated across the following
   fundamental areas:

- **Responsibilities** — precise definition of the component's functional role: what it does, why it exists, what
  problem it solves, and what objectives it pursues within the system. Each component must adhere to the Single
  Responsibility Principle (SRP), avoiding role overlaps with other modules.

- **Interfaces** — analysis of the contracts exposed by the component to the outside world: expected inputs, produced
  outputs, adopted communication protocols (e.g. REST, gRPC, event bus), and guarantees provided to consumers. Includes
  verification of interface stability and backward compatibility over time.

- **Class structures** — examination of the internal organization of classes or modules: inheritance hierarchies,
  composition strategies, adopted architectural patterns (e.g. Factory, Strategy, Repository), and consistency between
  the logical and physical structure of the code.

- **Exposed methods** — review of every public or protected method: signature, semantics, pre- and post-conditions, side
  effects, idempotency, and behavioral correctness with respect to documentation or project specifications.

- **Implementation logic** — assessment of algorithmic correctness, readability, maintainability, and efficiency of
  internal code. Includes identification of anti-patterns, duplicated code, excessive cyclomatic complexity, or logic
  that is difficult to test.

- **Error handling** — verification of the component's robustness in the face of abnormal conditions: coverage of edge
  cases, controlled exception propagation, informative error messages, fallback or retry strategies, and the absence of
  silent failures.

- **Type consistency** — thorough check of typological coherence throughout the component: correct use of primitive and
  complex types, absence of unsafe casts, compatibility between types used across internal and external interfaces, and
  adherence to the conventions of the adopted language or framework.

- **Inter-component interactions** — mapping of direct and indirect dependencies with other system components: analysis
  of coupling, cohesion, data flows, synchronization points, and potential risks arising from circular or fragile
  dependencies.

Following the analytical phase, the audit must produce **prioritized, actionable recommendations** for resolving every
identified deficiency, ordered by: **(1) feasibility**, **(2) expected return on investment (ROI)**, and **(3)
implementation effort**.

### Patterns for Audit Execution

The following best practices ensure that the audit produces findings and recommendations that are genuinely useful to
the engineering team responsible for the type checker.

#### Pattern: Evidence-First Grounding

- **Objective:** Ensure that every finding, deficiency claim, and recommendation is directly traceable to specific code
  artifacts (`.hpp` or `.cpp` files, line ranges, method signatures) rather than architectural assumptions.
- **Context of application:** Apply this pattern from Phase 1 through Phase 4, whenever identifying a deficiency,
  assigning a feasibility score, or writing a recommendation description.
- **Key characteristics:** Each finding includes a citation in the format `[file:method/line]`. No finding is stated
  without the auditor having read the relevant declaration and definition. Inferences about intent are explicitly
  prefixed with "Inferred:". Recommendations name the specific files and entry-point functions that must be modified.
- **Operational guidance:**
    1. Before writing any deficiency statement, locate the exact file(s) that demonstrate the problem.
    2. Record the file name, class or function name, and — where relevant — the line range or branch condition.
    3. In Phase 4 recommendation descriptions, begin with a "Change entry point:" sentence naming the first file and
       method to modify.
    4. If a deficiency spans multiple files, list all of them in the finding and ensure the recommendation covers each
       one.
    5. Perform a final pass (see §Contradiction Audit below) to verify that no recommendation lacks a file-level entry
       point.

#### Pattern: Completeness-by-Enumeration

- **Objective:** Guarantee that every system and every component within every system is examined and reported on, with
  no silent omissions — even for scaffolding or trivial code.
- **Context of application:** Apply during Phase 1 (system enumeration) and Phase 3 (per-component analysis), and again
  as a verification step before transitioning to Phase 4.
- **Key characteristics:** A checklist is maintained mapping every discovered file and class to a corresponding audit
  subsection. Trivial or stub components are explicitly flagged as such rather than skipped. The audit document contains
  a one-to-one correspondence between discovered entities and audit subsections.
- **Operational guidance:**
    1. Build a manifest of every `.hpp` and `.cpp` file in the type checker directory tree.
    2. Parse each header to extract class/struct/enum declarations; record them in a component registry.
    3. For each component in the registry, create the corresponding Phase 3 subsection (even if only to state "stub —
       see §2.6").
    4. Before moving to Phase 4, verify that the count of Phase 3 subsections equals the count of components in the
       registry.
    5. Any component without a Phase 3 subsection is a gap — insert the missing analysis before proceeding.

#### Pattern: Mechanically-Derived Prioritization

- **Objective:** Remove subjective bias from the recommendation ranking by applying the composite score formula (
  `Feasibility × 2 + ROI × 2 + Effort × 1`) uniformly and without post-hoc reordering.
- **Context of application:** Apply during Phase 4 when constructing the recommendation register and the summary
  priority table.
- **Key characteristics:** Each recommendation's feasibility, ROI, and effort scores are assigned independently before
  the composite is calculated. The final ordering is a direct sort on the computed composite — no manual swapping of
  rows is permitted. Justifications for each individual score are recorded inline.
- **Operational guidance:**
    1. For each recommendation, assign Feasibility (1–5), ROI (1–5), and Effort (1–5) as standalone judgments, each with
       a one-sentence justification.
    2. Compute the composite score using the specified formula. Record the arithmetic explicitly.
       (Note: Effort is inversely scaled — higher score = less work required.)
    3. After all recommendations are scored, sort the table by descending composite score. Break ties by descending ROI,
       then descending Feasibility.
    4. Do not reorder rows after sorting. If a row appears misplaced, recheck the arithmetic — the formula is the sole
       arbiter.
    5. Present the summary priority table (§4.2) in the exact order produced by the sort.

#### Pattern: Cross-Cutting Concern Mapping

- **Objective:** Identify concerns that span multiple system boundaries (error propagation, symbol resolution, scope
  management, type representation) and evaluate whether they are handled consistently or if each system has reinvented
  its own variant.
- **Context of application:** Apply during Phase 1 (§1.4) and carry findings forward into the per-system analysis (§2.5)
  and recommendation register (§4.1).
- **Key characteristics:** A matrix is constructed mapping each cross-cutting concern to every system, with a cell value
  indicating the handling strategy used by that system. Divergent strategies across systems are flagged as architectural
  inconsistencies requiring a unified approach.
- **Operational guidance:**
    1. Identify the set of cross-cutting concerns relevant to type checkers: error propagation, symbol resolution, scope
       management, type representation, and diagnostic formatting.
    2. For each concern, inspect every system's code to determine how it is handled (e.g., `std::expected<T, E>`,
       exception-based, return-code-based, or silent).
    3. Populate a concern-by-system matrix. Highlight cells where the strategy differs from the majority.
    4. For each inconsistency, write a finding in §1.4 and a corresponding recommendation in §4.1 specifying the target
       unified strategy and the files that must change.
    5. In Phase 2, reference the cross-cutting analysis (§1.4) rather than repeating it; note system-specific
       deviations.

---

### Anti-Patterns for Audit Execution

The following common mistakes during the audit process produce findings that are vague, unactionable, or structurally
incoherent.

#### Anti-Pattern: Impression-Based Findings

- **Description:** The auditor writes findings based on a high-level skim of the codebase or on architectural
  assumptions, without tracing claims to specific files, classes, or code paths. Findings use language like "the error
  handling appears inconsistent" without citing the functions or lines that exhibit the inconsistency.
- **Reasons to avoid:** Auditors are susceptible to pattern-matching bias — seeing a familiar anti-pattern in code that
  superficially resembles past projects but is actually correct in context. Without file-level evidence, findings are
  indistinguishable from opinion, and engineers cannot verify or act on them.
- **Negative consequences:** Recommendations reference no concrete starting point, making them unactionable. Engineers
  spend time searching for the alleged problem rather than fixing it. Credibility of the entire audit is undermined,
  leading to partial or total dismissal of the report.
- **Correct alternative:** Apply the **Evidence-First Grounding** pattern to ensure every finding is anchored to
  specific code artifacts with citations.

#### Anti-Pattern: Selective Coverage

- **Description:** The auditor skips analysis of files or components that appear trivial, auto-generated, or
  scaffolding-level, rationalizing that they "don't warrant deep review." The audit document contains analysis for only
  a subset of the systems and components.
- **Reasons to avoid:** Time pressure and the desire to produce a report quickly lead to triaging — focusing on "
  interesting" code and ignoring boilerplate. However, scaffolding code often contains stubs, TODOs, or incomplete
  implementations that are critical to the overall picture and may represent deliberate technical debt.
- **Negative consequences:** The recommendation register omits deficiencies in skipped components, leaving real problems
  unaddressed. Engineers who later discover the omitted issues lose confidence in the audit's thoroughness. The
  composite prioritization is skewed because the denominator of findings is incomplete.
- **Correct alternative:** Apply the **Completeness-by-Enumeration** pattern to build a manifest of all files and
  components and audit every one, flagging trivial or stub items explicitly rather than omitting them.

#### Anti-Pattern: Subjective Reprioritization

- **Description:** After computing composite scores using the prescribed formula, the auditor manually reorders the
  recommendation table based on personal judgment — promoting items they feel are "more important" and demoting others,
  regardless of the computed scores.
- **Reasons to avoid:** The formula exists precisely to make prioritization reproducible and defensible. Manual
  reordering reintroduces the cognitive biases the formula was designed to eliminate. It also makes the ranking
  non-auditable — stakeholders cannot verify why one item outranks another.
- **Negative consequences:** Engineers cannot trust the ordering because it reflects unstated reasoning rather than
  transparent arithmetic. Disagreements about priority become unresolvable debates. The summary table contradicts the
  detailed scoring in the recommendation register, creating internal inconsistency.
- **Correct alternative:** Apply the **Mechanically-Derived Prioritization** pattern. Let the formula produce the
  ordering. If a recommendation genuinely warrants different treatment, adjust its individual scores with
  justification — do not reorder after the fact.

#### Anti-Pattern: Siloed Concern Analysis

- **Description:** The auditor evaluates each system in isolation, noting deficiencies within system boundaries but
  failing to examine how shared concerns (error handling, scope management, symbol resolution) are addressed across
  systems. Inconsistencies between systems go undetected.
- **Reasons to avoid:** Deep dives into individual systems are cognitively absorbing. The auditor naturally focuses on
  intra-system coherence and misses the forest for the trees. Additionally, cross-cutting analysis requires re-reading
  code across multiple systems, which is effortful and easy to defer.
- **Negative consequences:** Each system independently implements its own error-propagation mechanism, its own
  scope-tracking data structure, or its own type-representation format. The type checker becomes harder to maintain
  because fixing an error-handling bug requires changes in N different patterns instead of one. The audit fails to
  surface this structural cost.
- **Correct alternative:** Apply the **Cross-Cutting Concern Mapping** pattern during Phase 1 and carry the findings
  through all subsequent phases.

---

## TASK

Perform the full technical audit described above by executing the following steps in strict sequence:

### PHASE 1 — System Ensemble Analysis

---

# 1.1. Enumerate Every System in the Type Checker Codebase

Perform an **exhaustive and systematic survey** of every system present in the type checker codebase. For each
identified system, provide the following information in a structured manner:

**a) System name**
State the official or conventional name by which the system is identified in the codebase (e.g., `InferenceEngine`,
`SymbolResolver`, `ScopeManager`, `DiagnosticEmitter`, `TypeNormalizer`, etc.). If the system has no explicit name,
derive a descriptive one and justify the choice.

**b) Primary responsibility within the type-checking pipeline**
Describe precisely and in detail the main task the system performs within the pipeline. Answer the following guiding
questions:

- What transformation or computation does it perform?
- What input does it operate on, and what output does it produce?
- At which phase of the pipeline does it intervene (parsing, binding, inference, checking, reporting)?

*Example of an expected description:* the `SymbolResolver` receives an AST annotated with unresolved identifiers and
produces a symbol table mapping each identifier to its binding definition, operating in the binding phase prior to type
inference.

**c) Role relative to all other systems**
Clarify how the system is positioned within the overall ecosystem:

- Which systems **feed into it** (provide its input)?
- Which systems **depend on its output**?
- Does it play an orchestration, transformation, validation, or cross-cutting support role?
- Is it a **core system** (essential to the main flow) or an **auxiliary system** (optional support, diagnostics,
  optimization)?

---

# 1.2. Dependency Map Between Systems

Construct a **directional dependency map** across all systems identified in section 1.1. The map must be produced both
as **structured prose** and as an **ASCII diagram**, in order to ensure both narrative and visual readability.

**a) ASCII diagram**
Represent systems as nodes and dependencies as directional arrows (`-->`), distinguishing:

- **Direct dependencies**: system A directly calls or consumes the output of system B.
- **Indirect or transitive dependencies**: system A depends on B through an intermediary C.
- **Bidirectional or cyclic dependencies**: flag these explicitly as potential architectural code smells.

```text
[Parser] --> [AST Builder] --> [Symbol Resolver] --> [Type Inferencer]
                                      |                      |
                                      v                      v
                              [Scope Manager]       [Type Checker / Validator]
                                                            |
                                                            v
                                                   [Diagnostic Emitter]
```

**b) Upstream / downstream classification**
For each system, explicitly state whether it is:

- **Upstream**: produces data consumed by others; operates in the early phases of the pipeline.
- **Downstream**: receives and processes the output of other systems; operates in the later phases.
- **Midstream**: plays an intermediate role of transformation or mediation.

**c) Identification of critical nodes**
Flag nodes with **high fan-in** (many systems depend on it — potential bottleneck or single point of failure) and **high
fan-out** (depends on many other systems — potential excessive coupling).

---

# 1.3. Evaluate the Overall Architectural Coherence

Perform a critical and well-argued analysis of the **architectural coherence** of the codebase, structured along the
following evaluative axes:

**a) Separation of concerns**
Assess whether each system has a **well-delimited and non-overlapping** domain of responsibility. Consider:

- Are there systems that perform tasks belonging to distinct domains (e.g., a system that handles both type inference
  and error reporting)?
- Do the abstractions adopted reflect coherent concepts from the type-checking domain?
- Is the **Single Responsibility Principle** respected at the system level?

**b) Consistency of module organization**
Examine whether the module structure (files, packages, namespaces) coherently mirrors the decomposition into systems:

- Do physical boundaries (files/folders) coincide with logical boundaries (systems)?
- Are there modules that aggregate heterogeneous functionality, or conversely, logical systems fragmented across
  multiple modules without clear justification?
- Are naming conventions and organizational patterns applied uniformly throughout?

**c) Cleanliness and definition of inter-system boundaries**
Assess the quality of the interfaces between systems:

- Do systems communicate through **explicit and stable interfaces** (APIs, well-defined return types, documented
  contracts) or through direct access to internal data structures?
- Are there encapsulation violations (one system accessing the internals of another)?
- Are the input/output contracts between systems **typed and validated**, or implicit and fragile?

Conclude with a **synthetic judgment** (e.g., cohesive / partially cohesive / fragmented architecture) supported by
specific evidence drawn from the codebase.

---

# 1.4. Identify and Analyze Cross-Cutting Concerns

Identify and analyze in depth the **cross-cutting concerns** — that is, those mechanisms which, by their nature, cut
across the boundaries of multiple systems — and assess whether they are handled in a **uniform and centralized** manner
or in an **inconsistent and scattered** one.

For each of the following areas (and any others identified in the codebase), carry out the analysis described below:

**a) Error propagation**

- Are errors collected and propagated through a centralized mechanism (e.g., a shared `DiagnosticBag`, a
  `Result<T, Error>` monad) or does each system manage its own errors autonomously and in a potentially incompatible
  way?
- Are error messages uniformly structured (error code, severity, source position, suggested fix)?
- Is there a risk of **silent errors** (swallowed errors) at specific points in the pipeline?

**b) Symbol resolution**

- Is the symbol table a single shared and authoritative structure, or do duplicate/parallel representations exist across
  different systems?
- Is access to symbol resolution always routed through the official `SymbolResolver`, or do some systems implement their
  own local lookup logic?

**c) Scope management**

- Is scope nesting (global → module → function → block) modeled by a single `ScopeManager`, or is scope logic replicated
  in multiple places?
- Are variable shadowing, closure handling, and forward references managed consistently across all systems that require
  them?

**d) Type representation**

- Is there a **single canonical type** used to represent types throughout the system (e.g., a shared `Type` ADT), or do
  different systems use local representations that are then converted?
- Are fundamental type operations (unification, subtyping, normalization) centralized in a single module or scattered
  across the codebase?

### PHASE 2 — Per-System Deep Analysis

For **each system** identified in Phase 1, produce a dedicated section containing the following subsections. Each
subsection must be treated as a self-contained analytical unit, developed with rigour and exhaustiveness. Avoid
superficial observations: every claim must be grounded in concrete evidence drawn from the source code.

---

## 2.1. System Overview

Provide a comprehensive description of the system's **identity and role** within the broader architecture. This
subsection must answer the following questions:

- **Purpose**: What problem does this system solve? What is its functional responsibility within the type-checking
  pipeline? State this in precise technical terms, avoiding generic language. For example, do not write "handles
  types" — instead, specify whether the system is responsible for constructing type representations, resolving named
  types, enforcing type constraints, inferring implicit types, or something else entirely.
- **Scope**: What are the explicit boundaries of this system's competence? What does it do, and — equally important —
  what does it deliberately *not* do? Clearly delineate which concerns are internal to the system and which are
  delegated to adjacent systems.
- **Position in the pipeline**: Where does this system sit in the overall type-checking workflow? Describe its position
  in terms of data flow: what inputs does it receive, from which upstream systems, in what form (AST nodes, symbol
  tables, intermediate representations, etc.)? What outputs does it produce, and which downstream systems consume them?
  If the system occupies a central or cross-cutting role (e.g., a shared type registry consulted by multiple other
  systems), make this explicit.
- **Activation context**: Under what conditions is this system invoked? Is it triggered on-demand (e.g., lazily when a
  type is first referenced), driven by a traversal pass, or initialized at startup? Is it stateful across the analysis
  of a single compilation unit, or is it re-instantiated per scope, per function, or per expression?

---

## 2.2. Internal Module Organization

Analyse the **internal decomposition** of the system into source files (`.hpp` / `.cpp` pairs) and assess whether this
decomposition reflects a coherent and intentional architectural design.

- **File inventory**: List every `.hpp` and `.cpp` file that belongs to this system. For each file, state its declared
  purpose based on its name, its header comments (if any), and its actual contents. Flag any discrepancy between a
  file's apparent intent and its actual content.
- **Module boundaries**: Assess whether each file encapsulates a single, well-defined concept or responsibility. Are the
  boundaries between files semantically meaningful, or do they appear arbitrary? Are related concepts fragmented across
  multiple files when they should be co-located, or conversely, are unrelated concerns bundled into the same file?
- **Header organisation**: Examine the `.hpp` files critically. Do they expose only what is necessary (minimal public
  interface), or do they leak implementation details? Are internal types, helper functions, or private state
  unnecessarily visible in the public header? Are forward declarations used appropriately to reduce coupling?
- **Consistency of decomposition**: Is the granularity of decomposition uniform across the system, or are some modules
  disproportionately large (god files) while others are trivially thin? Does the naming convention for files, classes,
  and namespaces follow a consistent and descriptive pattern?
- **Overall verdict**: Conclude with a structured assessment of whether the module organisation is logical, consistent,
  and maintainable, or whether it presents structural problems that impede understanding and future modification.

---

## 2.3. Intra-System Dependency Analysis

Map and critically evaluate the **dependency relationships among the components internal to this system**. This is a
structural analysis that must go beyond a simple listing of `#include` directives.

- **Dependency graph**: Reconstruct the directed dependency graph among the system's own files and classes. Identify
  which components are at the bottom of the graph (no internal dependencies), which sit in the middle, and which are at
  the top (depend on everything else). Present this graph either textually (as a structured list of edges) or visually,
  depending on complexity.
- **Circular dependencies**: Identify any cycles in the dependency graph. A circular dependency between two or more
  files — even when broken via forward declarations — is a strong signal of a design flaw: it indicates that the
  involved components are too tightly coupled and may need to be restructured or merged. Document every cycle found, and
  assess its severity and impact.
- **Tight coupling**: Beyond cycles, identify cases where two components are coupled so tightly that modifying one would
  inevitably require modifying the other. This includes components that share mutable state, components where one
  directly manipulates the internal representation of another, or components where the interface between two modules is
  too wide or too specific.
- **Unnecessary layering**: Assess whether the dependency hierarchy introduces layers of indirection that add complexity
  without adding value. Are there components that act purely as pass-throughs, forwarding calls to another component
  without any meaningful transformation or encapsulation?
- **Missing abstractions**: Identify cases where a direct dependency between two concrete components could — and
  should — be mediated by an abstraction (an interface, a base class, a callback, or a visitor), but is not. This is an
  indicator of poor extensibility and testability.

---

## 2.4. Logical Flow

Provide a **step-by-step, concrete description** of how this system operates at runtime during the type-checking phase.
This is not an abstract summary — it is a precise trace of the computation from input to output.

- **Entry point**: Identify the precise function or method that serves as the entry point into this system during type
  checking. How is it called? By whom? With what arguments?
- **Input processing**: Describe how the system receives and interprets its inputs. If it processes AST nodes, which
  node types does it handle, and in what order? Does it perform any preliminary validation or normalisation of its
  inputs before beginning the core logic?
- **Core processing steps**: Walk through the internal computation in sequence. For each significant step, describe:
  what data is being read or written, which internal components are involved, what decisions or branches are taken, and
  what intermediate results are produced. Do not skip non-obvious steps or assume the reader can infer them.
- **Interaction with other systems**: At each point where the system calls into another system (e.g., to look up a
  symbol, to resolve a type alias, to emit a diagnostic), describe the interaction explicitly: what is requested, what
  is returned, and how the result influences the subsequent computation.
- **Output production**: Describe precisely what the system produces as its output. Is it a typed AST, a set of
  constraints, a populated symbol table, a list of diagnostics, or some combination? How is this output handed off to
  downstream consumers?
- **Error and edge-case handling**: Describe how the system handles malformed inputs, type errors in the source code
  being analysed, and internal invariant violations. Does it recover gracefully and continue, or does it abort? Are
  error states propagated explicitly, or swallowed silently?

---

## 2.5. Critical Points

This subsection must contain a **rigorous and exhaustive enumeration of all significant problems** identified in the
system. Each problem must be described with enough specificity to be actionable. For each issue, state: what the problem
is, where it manifests (file, class, function), why it is a problem (what invariant it violates or what risk it
introduces), and — where possible — a concrete suggestion for remediation.

The following categories of problems must be explicitly investigated and reported on:

- **Architectural incoherences**: Cases where the system's structure contradicts its stated purpose, or where
  responsibilities are allocated to components in a way that defies the system's own conceptual model.
- **Logical errors**: Incorrect computations, wrong assumptions, or flawed algorithms within the implementation. These
  include off-by-one errors in traversal, incorrect handling of operator precedence or associativity in type resolution,
  wrong unification logic, and similar issues.
- **Responsibility duplications**: Cases where the same logic is implemented in two or more places within the system (or
  across systems), leading to maintenance hazards and potential inconsistencies if one copy is updated and the other is
  not.
- **Incomplete branches**: Conditional logic (if/else chains, switch statements, visitor dispatch tables) where one or
  more branches are missing, left as stubs, or handled with silent no-ops when they should trigger a diagnostic or a
  meaningful computation.
- **Unhandled edge cases**: Inputs or states that are semantically valid but not accounted for in the implementation.
  Examples include: empty type parameter lists, recursive type definitions, mutually recursive types, types defined in
  nested scopes, or anonymous/unnamed types.
- **Semantic inconsistencies**: Cases where the system's behaviour diverges from the semantic rules of the language
  being type-checked. This requires comparing the implementation against the language specification or reference
  semantics.
- **Uncovered execution paths**: Paths through the code that are syntactically reachable but never exercised under any
  known input — often indicating dead code, an unreachable default case, or a missing test scenario that conceals a
  latent bug.

---

## 2.6. Partial or Undefined Implementations

Produce a **systematic and complete inventory** of every function, method, or class that is declared in a `.hpp` file
but whose implementation in the corresponding `.cpp` file is absent, stubbed, or only partially realised.

For each entry in this inventory, provide:

- **Declaration location**: the exact `.hpp` file and line number (or approximate location) where the entity is
  declared.
- **Implementation status**: one of the following classifications:
    - *Missing*: the entity is declared but has no corresponding definition anywhere in the codebase.
    - *Stubbed*: a definition exists but contains only a placeholder body — typically an empty block `{}`, a `return` of
      a default/null value, a `TODO` or `FIXME` comment, or an unconditional `throw` / `assert(false)`.
    - *Partial*: a definition exists and contains real logic, but the implementation is demonstrably incomplete — for
      example, it handles only a subset of the cases it is declared to handle, skips certain branches, or silently
      ignores certain input configurations.
- **Impact assessment**: For each incomplete entity, assess the downstream consequences of its incompleteness. Does it
  cause silent incorrect behaviour, trigger a runtime crash under specific inputs, or silently degrade the quality of
  type-checking results (e.g., by treating an unresolved type as valid)? Is the missing implementation on a critical
  path, or is it on an optional or rarely-exercised branch?
- **Dependency impact**: Does any other component — within this system or in another system — depend on this
  unimplemented entity? If so, enumerate the affected callers and assess whether they are currently protected against
  the incomplete behaviour (e.g., via guards, fallbacks, or the fact that the calling code path is itself unreachable).

### PHASE 3 — Per-Component Exhaustive Analysis

For **each component** within each system, produce a dedicated subsection containing:

---

3.1. **Responsibility statement**
Write a single, precise, and unambiguous sentence that captures the sole responsibility
of this component, strictly adhering to the Single Responsibility Principle (SRP).
The statement must:

- Identify **what** the component does (its primary action or function).
- Identify **on what** it operates (its subject domain or data).
- Exclude any secondary concerns (e.g., I/O, persistence, formatting) that belong
  to other components.

> ⚠️ If more than one responsibility can be inferred from the sentence, treat this
> as a design smell and flag it explicitly in section 3.8.

---

3.2. **Class structure**
Document every `class` or `struct` declared within this component.
For each type, provide:

- **Full name and kind** (`class` vs `struct`, template or concrete).
- **Member variables**: for each field, specify:
    - Its **declared type** (including `const`, `static`, pointer/reference qualifiers).
    - Its **visibility** (`private`, `protected`, `public`).
    - Its **semantic role** (what invariant it holds, what state it represents).
    - Its **default value or initialization strategy**, if any.
- **Inheritance relationships**: identify base classes, access specifiers
  (`public`, `protected`, `private` inheritance), and whether virtual dispatch
  is involved. Note any use of multiple inheritance and potential diamond problems.
- **Composition and aggregation**: identify member objects vs. raw/smart pointers,
  and clarify ownership semantics (`unique_ptr` = sole ownership,
  `shared_ptr` = shared ownership, raw pointer = non-owning reference).
- **Special member functions**: note which of the Rule-of-Five members
  (destructor, copy constructor, copy assignment, move constructor,
  move assignment) are user-defined, defaulted, or deleted, and explain why.

---

3.3. **Interface analysis**
For every **public method** declared in the `.hpp` file, provide a complete entry
covering:

- **Full signature**: return type, method name, parameter list (names, types,
  `const`/`noexcept`/`override`/`virtual` qualifiers), and any template parameters.
- **Preconditions**: every assumption that must hold before the method is called
  (e.g., pointer non-null, index in bounds, object in initialized state).
  Distinguish between **checked** preconditions (enforced via `assert` or exceptions)
  and **unchecked** ones (caller's responsibility).
- **Postconditions**: what is guaranteed to be true after a successful return
  (e.g., container size increased by one, output parameter populated).
- **Contract**: the behavioral promise from caller to callee and back — including
  any invariants the method must preserve, side effects it produces, and whether
  it is idempotent, pure, or stateful.
- **Discrepancies with `.cpp`**: flag any mismatch between declaration and definition,
  such as:
    - Parameters renamed, reordered, or defaulted differently.
    - Return type widened or narrowed implicitly.
    - `noexcept` declared but exceptions possible in the body.
    - `const` qualifier on the method missing from one side.
    - Documented behavior not implemented, or undocumented behavior present.

---

3.4. **Implementation logic**
Provide a detailed, step-by-step walkthrough of every **non-trivial algorithm and
logic path** found in the `.cpp`. For each:

- **Algorithmic description**: explain the core logic in plain language before
  referencing code. State the algorithm's strategy (e.g., recursive descent,
  two-pointer, dynamic programming, BFS/DFS).
- **Complexity analysis**: provide Big-O estimates for time and space complexity,
  both average-case and worst-case where they differ.
- **Branching conditions**: document every `if`/`else if`/`else`, `switch`, and
  ternary expression — what each branch handles, what invariant it relies on,
  and whether all cases are covered (including the default/fallthrough).
- **Loop structures**: for every `for`, `while`, or `do-while`:
    - State the **loop invariant**.
    - Identify the **termination condition** and argue (informally) for termination.
    - Note any `break`, `continue`, or early `return` that alters normal flow.
- **Recursive patterns**: identify the **base case(s)**, the **recursive case**,
  the **recursion depth** (bounded or unbounded), and risk of stack overflow.
- **State transitions**: if the component manages state (a finite-state machine,
  a parser mode, etc.), map every transition, its trigger condition, and its effect.

---

3.5. **Error handling evaluation**
Analyze how the component detects, represents, propagates, and reports erroneous
conditions. Cover:

- **Detection**: which conditions are explicitly checked (null dereferences,
  out-of-range accesses, type mismatches, undeclared identifiers, scope violations,
  malformed input) and which are silently assumed valid.
- **Representation**: how errors are modeled — exceptions (hierarchy, what is thrown
  and where), error codes (type, meaning, propagation path), `std::optional`/
  `std::expected` return types, or sentinel values.
- **Propagation**: whether errors are handled locally (absorbed), re-thrown (possibly
  wrapped), or passed upward to the caller unchanged. Verify that error context
  (e.g., source location, offending token, variable name) is preserved across layers.
- **Reporting**: how errors are surfaced to the end user or calling system
  (error messages, logging, diagnostics). Evaluate quality: are messages actionable,
  precise, and free of internal jargon?
- **Uncaught cases and silent failures**: explicitly list any error condition that:
    - Is not detected (e.g., integer overflow, use-after-free, iterator invalidation).
    - Is detected but swallowed (empty `catch` blocks, discarded return codes).
    - Produces undefined behavior instead of a defined error path.

---

3.6. **Type consistency audit**
Perform a systematic review of type usage across declarations (`.hpp`),
definitions (`.cpp`), and all call sites. For each identified issue, specify
the location, the types involved, and the risk it poses:

- **Declaration–definition mismatches**: verify that the return type, parameter
  types, and qualifiers match exactly between `.hpp` and `.cpp`.
- **Implicit conversions**: flag narrowing conversions (e.g., `double` → `int`,
  `size_t` → `int`), sign-extension or truncation risks, and any conversion that
  silently discards information.
- **Unsafe casts**: identify every `static_cast`, `reinterpret_cast`,
  `const_cast`, and C-style cast; explain whether each is justified, risky,
  or avoidable.
- **Mismatched type assumptions**: check for cases where a value produced as one
  type is consumed assuming a different type (e.g., a function returning `int`
  used as a `bool`, or an enum value compared to a raw integer literal).
- **Alias and typedef consistency**: verify that type aliases (`using`, `typedef`)
  are used uniformly and are not bypassed at certain call sites.
- **Template type deduction pitfalls**: if templates are used, flag any unexpected
  deduction (e.g., reference collapsing, `auto` stripping `const`).

---

3.7. **Inter-component interaction**
Map every dependency and communication channel between this component and all others.
For each interaction, document:

- **Dependency direction**: which component depends on which, and whether the
  dependency is compile-time (include, template instantiation) or runtime
  (virtual dispatch, callback, service locator).
- **Data exchanged**: what types, structures, or values cross the component boundary,
  who owns them, and whether lifetimes are correctly managed.
- **Coupling analysis**:
    - *Tight coupling*: direct use of concrete types, access to internal fields,
      or reliance on undocumented behavior of another component.
    - *Hidden assumptions*: implicit ordering requirements (e.g., "this method must
      be called before that one"), shared global state, or thread-safety assumptions
      not expressed in the interface.
    - *Violated abstraction boundaries*: cases where a component reaches across
      its intended layer (e.g., a parser directly manipulating symbol table internals,
      a type-checker allocating AST nodes).
- **Fragile coupling points**: interactions that would break silently if the
  dependency's interface or behavior changed subtly — e.g., depending on a specific
  error code value, or on the order of elements in a returned collection.

---

3.8. **Optimization opportunities**
Identify concrete, actionable improvements across three dimensions:

**Performance**

- *Algorithmic complexity*: pinpoint any algorithm with a worse-than-necessary
  complexity class (e.g., O(n²) lookup replaceable with O(log n) or O(1))
  and suggest the appropriate data structure or algorithm.
- *Unnecessary copies*: identify pass-by-value where pass-by-const-reference
  or move semantics would suffice; flag missing `std::move` or `emplace` calls.
- *Redundant traversals*: detect multiple passes over the same data structure
  that could be merged into a single traversal.
- *Memory allocation patterns*: flag repeated heap allocations in hot paths
  that could be replaced with pre-allocation, object pooling, or stack allocation.

**Structural and design deficiencies**

- *Code duplication*: identify repeated logic blocks that should be extracted
  into a shared helper, base class, or template.
- *God-class pattern*: flag any class accumulating too many responsibilities,
  too many member variables, or too many public methods — and suggest decomposition.
- *Anemic domain model*: identify data structures that hold state but delegate
  all behavior elsewhere, breaking encapsulation and cohesion.
- *Primitive obsession*: flag cases where raw primitives (`int`, `std::string`)
  are used where a dedicated value type or enum would encode domain constraints.

**Maintainability**

- *Poor naming*: identify variables, methods, or types whose names are
  ambiguous, misleading, or too abbreviated to convey intent.
- *Missing documentation hooks*: flag public interfaces, non-obvious invariants,
  and complex algorithms that lack comments or documentation stubs.
- *Untestable logic*: identify logic entangled with I/O, global state, or
  hard-coded dependencies that make unit testing impractical — and propose
  decoupling strategies (dependency injection, strategy pattern, pure functions).
- *Magic values*: flag unnamed numeric or string literals that should be
  named constants or configuration parameters.

### PHASE 4 — Prioritized Recommendations

After completing all three analytical phases, produce a **recommendation register**
structured as follows. This register serves as the authoritative output artifact of
the entire analysis process: it consolidates every deficiency, gap, and optimization
opportunity identified across Phases 1, 2, and 3 into a set of discrete, actionable,
and prioritized recommendations. Each entry must be self-contained — meaning a reader
with no prior context should be able to understand what the problem is, what action is
required, how long it will take, who must execute it, and how success will be measured.

---

## 4.1 Recommendation Register

For each deficiency or opportunity identified in Phases 1–3, formulate exactly **one
discrete recommendation**. Do not group multiple unrelated findings into a single entry,
and do not split a single coherent finding across multiple entries. If two findings share
the same root cause and the same remediation action, they may be consolidated into one
recommendation, provided both source references are cited in the *Deficiency Addressed*
field.

Each recommendation entry **must** contain all of the following fields, presented in the
order listed below. No field may be omitted or left vague.

---

### Field Specifications

- **ID**  
  A unique alphanumeric identifier assigned sequentially in the format `REC-NNN`
  (e.g., `REC-001`, `REC-002`, ..., `REC-042`). IDs must be stable across revisions
  of the register — once assigned, an ID must never be reused or reassigned to a
  different recommendation, even if the original entry is deprecated or removed.
  Deprecated entries should be retained with a ~~strikethrough~~ label and a note
  indicating the reason for deprecation.

- **Title**  
  A concise, descriptive label of **no more than 10 words** that unambiguously
  identifies the recommended action. The title must be written in imperative form
  (e.g., *"Introduce lazy evaluation for recursive type resolution"*,
  *"Replace ad-hoc null checks with a unified Option wrapper"*). Avoid generic titles
  such as *"Fix bug"* or *"Improve performance"* — the title alone must be specific
  enough to distinguish this entry from all others in the register.

- **Deficiency Addressed**  
  An explicit cross-reference to the specific finding, observation, or measurement
  from Phase 1 (Static Analysis), Phase 2 (Dynamic / Runtime Analysis), or Phase 3
  (Architecture & Design Review) that motivates this recommendation. The reference
  must identify: **(a)** the originating phase (e.g., *"Phase 2"*), **(b)** the
  specific section or finding ID within that phase (e.g., *"§2.3 — Profiling
  Hotspots"*, *"Finding P2-07"*), and **(c)** a one-sentence restatement of the
  finding in plain language (e.g., *"The type resolver re-evaluates identical
  generic constraints on every call without caching, causing O(n²) complexity for
  nested generics"*). If the recommendation addresses findings from multiple phases,
  list all references. This field is mandatory; a recommendation without a traceable
  source finding is inadmissible.

- **Description**  
  A precise, technically detailed narrative of the recommended action. This field
  must answer three distinct questions with no ambiguity:

    1. **What must be done**: describe the change at a level of granularity sufficient
       for a senior engineer to begin implementation without requiring further
       clarification. Reference specific modules, classes, functions, data structures,
       algorithms, configuration keys, or API contracts as appropriate.

    2. **How it must be done**: specify the implementation approach, including any
       architectural decisions, design patterns, libraries, algorithms, or migration
       strategies to be employed. Where multiple approaches are viable, identify the
       recommended approach and briefly explain why alternatives were rejected.

    3. **What the expected outcome is**: describe the observable, measurable state of
       the system after successful implementation. This is distinct from the
       *Effectiveness Indicators* field — here the focus is on the qualitative
       improvement to system behaviour, correctness, maintainability, or performance.

  Minimum length: 80 words. There is no maximum, but verbosity must be justified by
  technical necessity. Bullet sub-lists, inline code snippets, and pseudocode are
  permitted and encouraged where they improve clarity.

- **Feasibility Score** *(integer, 1–5)*  
  Rates how readily the recommendation can be executed given the current state of
  the codebase, team capacity, toolchain, and external dependencies.

  | Score | Meaning                                                                                                                                                          |
                          |-------|------------------------------------------------------------------------------------------------------------------------------------------------------------------|
  | 5     | Immediately executable: all required resources, knowledge, and access rights are available; no external dependencies or approvals are needed.                    |
  | 4     | Executable within the current sprint with minor preparation (e.g., a library upgrade, a brief knowledge-transfer session).                                       |
  | 3     | Executable within the current quarter but requires non-trivial coordination (e.g., API contract negotiation with another team, infrastructure provisioning).     |
  | 2     | Blocked by a significant external dependency, architectural prerequisite, or resource gap that must be resolved first.                                           |
  | 1     | Presently infeasible: requires capabilities, personnel, or conditions not currently available; a precondition roadmap must be established before work can begin. |

  Justify the assigned score in exactly **one sentence**, citing the specific factor
  that most constrains or enables executability.

- **Expected ROI** *(integer, 1–5)*  
  Rates the anticipated impact of the recommendation on one or more of the following
  quality dimensions: **correctness** (reduction of defects, edge-case failures, or
  undefined behaviour), **performance** (throughput, latency, memory consumption,
  computational complexity), or **maintainability** (reduction of technical debt,
  improved readability, testability, or extensibility).

  | Score                                                                  | Meaning                                                                                                         |
                  |------------------------------------------------------------------------|-----------------------------------------------------------------------------------------------------------------|
  | 5                                                                      | Transformative impact: addresses a critical correctness defect, removes a performance bottleneck affecting the  |
  | primary execution path, or eliminates a major structural anti-pattern. |                                                                                                                 |
  | 4                                                                      | Significant impact: materially improves a quality dimension in a frequently exercised code path or component.   |
  | 3                                                                      | Moderate impact: improves a quality dimension in a secondary path or component with measurable but not critical |
  | effect.                                                                |                                                                                                                 |
  | 2                                                                      | Minor impact: incremental improvement to edge cases, low-frequency paths, or stylistic consistency.             |
  | 1                                                                      | Negligible or speculative impact: improvement is cosmetic, theoretical, or contingent on future changes not yet |
  | planned.                                                               |                                                                                                                 |

  Justify the assigned score in exactly **one sentence**, citing the specific quality
  dimension affected and the expected magnitude of improvement.

- **Implementation Effort** *(integer, 1–5)*  
  Rates the inverse of the engineering cost required to implement the recommendation
  — i.e., a score of **5 denotes minimal effort** (hours to days), while a score of
  **1 denotes maximal effort** (months, team-wide coordination, or fundamental
  architectural refactoring).

  | Score                                   | Meaning                                                                                                        |
        |-----------------------------------------|----------------------------------------------------------------------------------------------------------------|
  | 5                                       | Minimal effort: implementable in hours to a few days by a single engineer with no cross-team coordination      |
  | required.                               |                                                                                                                |
  | 4                                       | Low effort: implementable in under two weeks; may involve two or three engineers or a small set of coordinated |
  | changes.                                |                                                                                                                |
  | 3                                       | Moderate effort: implementable in two to six weeks; requires a dedicated workstream, thorough testing, and     |
  | integration work.                       |                                                                                                                |
  | 2                                       | High effort: requires one to three months, cross-team involvement, significant refactoring, or parallel        |
  | migration strategy.                     |                                                                                                                |
  | 1                                       | Very high effort: multi-month or multi-quarter initiative requiring architectural redesign, team-wide          |
  | participation, or external procurement. |                                                                                                                |

  Justify the assigned score in exactly **one sentence**, citing the primary driver of
  effort (e.g., scope of change, testing complexity, migration risk, coordination
  overhead).

- **Priority Rank**  
  A composite numeric score computed deterministically from the three preceding
  scores using the formula:

  Composite Score = (Feasibility × 2) + (ROI × 2) + (Effort × 1)

  The weighting rationale is as follows: Feasibility and ROI are each double-weighted
  because a recommendation that is either immediately actionable or yields high
  impact should be prioritised over one that is merely easy; Effort is
  single-weighted because low effort is a desirable but secondary criterion.

  The maximum achievable composite score is **(5 × 2) + (5 × 2) + (5 × 1) = 25**.  
  The minimum is **(1 × 2) + (1 × 2) + (1 × 1) = 5**.

  Recommendations must be listed in the register in **descending order of composite
  score** (highest score first). In the event of a tie, the tied entries should be
  sub-ranked by ROI descending, then by Feasibility descending, then by Effort
  descending, and finally by ID ascending as a tiebreaker of last resort.

- **Estimated Implementation Time**  
  A realistic calendar-time range expressed as a bounded interval with explicit
  units (e.g., *"4–8 hours"*, *"3–5 days"*, *"2–4 weeks"*, *"2–3 months"*).
  The estimate must reflect wall-clock elapsed time under normal working conditions
  (i.e., accounting for review cycles, testing, and integration), **not** raw
  person-hours of active coding. If the estimate is highly sensitive to team size or
  parallelism, state the assumption (e.g., *"3–5 days assuming one senior engineer
  working full-time; 1–2 days if parallelised across two engineers"*). Point
  estimates (e.g., *"3 days"*) are not acceptable; a range is required to surface
  uncertainty honestly.

- **Required Resources**  
  An explicit enumeration of every input required to implement the recommendation.
  This field must address four dimensions:

    1. **Roles**: identify the job functions or competency profiles needed
       (e.g., *"one senior backend engineer with expertise in type-system internals"*,
       *"one QA engineer for regression test authoring"*,
       *"one DevOps engineer for CI pipeline modification"*).

    2. **Tools & libraries**: list any software tools, frameworks, profiling
       instruments, linters, or third-party libraries required, including minimum
       version constraints where relevant.

    3. **Access & permissions**: specify any repository access, environment access,
       secrets, or approval gates required before work can begin.

    4. **External dependencies**: identify any upstream teams, third-party vendors,
       or external API changes whose involvement is prerequisite.

  If a resource is already available, note it as *"available"*. If it must be
  procured or arranged, note it as *"to be arranged"* with an indication of lead time.

- **Effectiveness Indicators**  
  Between **one and three** measurable, observable criteria by which successful
  implementation of this recommendation can be objectively verified. Each indicator
  must satisfy the following requirements:

    - **Measurable**: expressible as a quantitative threshold, a binary pass/fail
      condition, or a relative improvement ratio.
    - **Observable**: verifiable using available tooling (test suites, profilers,
      static analysers, monitoring dashboards, code coverage tools, etc.).
    - **Time-bounded**: the point in time or the event (e.g., *"after merging to main"*,
      *"following the next performance regression test run"*) at which the indicator
      is to be evaluated must be specified or inferable.

  **Examples of well-formed indicators**:
    - *"Zero failing test cases for type inference of generic functions in the
      `resolver/generics` test suite after implementation."*
    - *"Reduction of duplicate type resolution calls by ≥ 50% as measured by the
      `TypeResolverProfiler` instrumentation tool under the standard benchmark
      workload (`bench/full-pipeline.json`)."*
    - *"Cyclomatic complexity of `TypeChecker::resolve()` reduced to ≤ 15 as
      reported by the project's configured static analysis tool after refactoring."*

  **Examples of poorly-formed indicators** (do not use):
    - *"Performance improves."* — not measurable, not time-bounded.
    - *"Code is cleaner."* — subjective, not observable.
    - *"Tests pass."* — too vague; must specify which tests and what the acceptance
      threshold is.

---

## 4.2 Summary Priority Table

After the full recommendation register (Section 4.1), provide a **summary priority
table** that offers a consolidated, at-a-glance view of all recommendations ranked
by composite score. This table is intended for stakeholders who require a rapid
overview without reading individual register entries in full.

The table must include the following columns, in this exact order:

| Rank | ID | Title | Feasibility | ROI | Effort | Composite Score | Est. Time |
|------|----|-------|-------------|-----|--------|-----------------|-----------|

**Column definitions**:

- **Rank**: ordinal position (1 = highest priority). Ties resolved as described in
  §4.1 *Priority Rank*.
- **ID**: the `REC-NNN` identifier, hyperlinked to the corresponding entry in §4.1
  where the output format supports hyperlinking.
- **Title**: verbatim from the corresponding §4.1 entry.
- **Feasibility**: the integer score (1–5) from the corresponding §4.1 entry.
- **ROI**: the integer score (1–5) from the corresponding §4.1 entry.
- **Effort**: the integer score (1–5) from the corresponding §4.1 entry.
- **Composite Score**: the computed value `(Feasibility × 2) + (ROI × 2) + (Effort × 1)`,
  cross-checked for consistency with the §4.1 entry.
- **Est. Time**: the implementation time range verbatim from the corresponding §4.1
  entry, abbreviated if necessary to fit column width (e.g., *"2–4 hrs"*,
  *"1–2 wks"*, *"2–3 mos"*).

The table must be sorted in **descending order of Composite Score** (highest first).
Rows must not be omitted, reordered for reasons other than score, or summarised.
Every entry present in §4.1 must appear in this table exactly once.
---

## AUDIENCE

This audit is intended for **senior compiler engineers and technical leads** who are responsible for the maintenance,
extension, and refactoring of the type checker. Readers have deep knowledge of C++17/20, type system theory (
Hindley-Milner, bidirectional type checking, subtyping), and compiler front-end architecture. No introductory
explanations of basic concepts are required.

---

## FORMAT

Structure the output as a **hierarchical technical document** using the following top-level sections, each rendered with
Markdown headers:

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

Use **tables** for the dependency map (where ASCII is insufficient), the class structure in 3.2, the interface analysis
in 3.3, and the summary priority table in 4.2.
Use **code blocks** (triple backtick with `cpp` syntax highlighting) for all C++ code excerpts, signatures, or
pseudocode.
Use **bold** for the first mention of every critical term, deficiency label, and recommendation ID.
Use **inline `monospace`** for all identifiers, file names, method names, and type names.

### Patterns for Formatting and Structure

#### Pattern: Artifact-Consistent Citation

- **Objective:** Ensure that every reference to code in the audit uses a uniform citation format, enabling readers to
  locate the exact artifact being discussed.
- **Context of application:** Apply throughout the document whenever citing a file, class, method, or line of code.
- **Key characteristics:** All file references use inline monospace (e.g., `TypeResolver.hpp`). Method references use
  `ClassName::methodName` syntax. Line references, when provided, use the format `line N–M`. The citation style is
  consistent across all phases.
- **Operational guidance:**
    1. Use the format `file:Class::method (line N–M)` for all code citations.
    2. Wrap file names and identifiers in inline code backticks.
    3. When referencing a finding from an earlier phase, use the section number (e.g., "see §2.5") rather than restating
       the finding.

#### Pattern: Structural Depth Enforcement

- **Objective:** Guarantee that the output meets the minimum depth requirements (150 words per Phase 3 component, 300
  words per Phase 2 system) without padding or repetition.
- **Context of application:** Apply during writing and again during the final review pass.
- **Key characteristics:** Word counts are verified mechanically. Content that falls short is enriched with additional
  analysis from the relevant subsections (e.g., expanding the error handling evaluation or inter-component interaction
  analysis) rather than restating what was already said.
- **Operational guidance:**
    1. After drafting each Phase 3 subsection, count words. If below 150, identify which of §3.1–§3.8 is underdeveloped
       and expand it with additional code-level detail.
    2. After drafting each Phase 2 system section, count words. If below 300, deepen §2.4 (logical flow) or §2.5 (
       critical points) with step-by-step execution traces or edge-case enumeration.
    3. Do not pad with generic observations. Every additional word must convey new information about the code.
    4. Use a word-count tool or manual count as a verification gate before finalizing the document.

---

### Anti-Patterns for Formatting and Structure

#### Anti-Pattern: Citation Drift

- **Description:** The auditor uses inconsistent or ambiguous references to code — sometimes citing only a file name,
  sometimes a function name without the file, sometimes a vague description like "in the type resolution module" —
  making it difficult or impossible for the reader to locate the exact artifact.
- **Reasons to avoid:** Under time pressure, auditors abbreviate citations or use shorthand that makes sense to them
  during writing but is opaque to the reader. This is compounded when the auditor has been reading the codebase for
  hours and assumes the reader shares the same mental model.
- **Negative consequences:** Engineers waste time searching for the referenced code. Cross-references between phases
  break down because the reader cannot verify that §4.1's recommendation actually corresponds to §3.5's finding. The
  audit's utility degrades proportionally to citation ambiguity.
- **Correct alternative:** Apply the **Artifact-Consistent Citation** pattern to enforce a single, unambiguous citation
  format across the entire document.

#### Anti-Pattern: Depth Inflation

- **Description:** To meet minimum word-count requirements, the auditor pads sections with restatements of earlier
  findings, generic observations about software quality ("well-structured code is easier to maintain"), or verbose
  re-explanations of concepts already covered.
- **Reasons to avoid:** Word counts are an easily gamed metric. An auditor who has exhausted their analysis of a trivial
  component may feel compelled to fill space rather than acknowledge that the component warrants minimal coverage. The
  incentive to hit the number conflicts with the incentive to be concise.
- **Negative consequences:** Readers learn to skip sections that contain padding, which means they also skip genuine
  content buried within. The audit's signal-to-noise ratio drops. Senior engineers lose patience with the document and
  rely on it less.
- **Correct alternative:** Apply the **Structural Depth Enforcement** pattern. If a section is below the word-count
  floor, deepen the analysis by exploring additional code paths, edge cases, or interaction patterns — not by restating
  what has already been said.

---

## CONSTRAINTS

### Rule 1 — Empirical Grounding of Every Claim About the Code

Every claim about the codebase must be **mandatorily anchored to specific, verifiable evidence** drawn directly from the `.hpp` or `.cpp` source files. This requirement admits no exceptions or partial waivers.

In practice, whenever describing the behavior of a class, method, data structure, or architectural mechanism, it is **mandatory to explicitly cite**:

- The **source file name** of reference (e.g., `renderer.cpp`, `collision_manager.hpp`);
- The **class name** or **namespace** in which the construct is defined;
- The **specific method or function name** that forms the basis of the claim, whenever identifiable with precision.

#### Distinction Between Observed Fact and Inference

If the auditor deems it necessary to formulate an interpretive hypothesis about the **designer's intent** — for example, supposing that a certain architectural choice was made for performance or maintainability reasons — that hypothesis must be **explicitly marked as an inference**, by appending the mandatory prefix:

> **Inferred:** *[inference text]*

This prefix signals to the reader that the following statement is not directly derivable from the source code, but represents an interpretive evaluation by the auditor. Inferences are permitted, but must remain clearly separated from observed facts, avoiding any ambiguity that could compromise the document's reliability.

#### Operational Example

- ✅ **Correct:** «The method `PhysicsEngine::integrate()` defined in `physics_engine.cpp` uses an explicit Euler integrator to update rigid body positions.»
- ✅ **Correct with inference:** «**Inferred:** The choice of explicit Euler integrator in `physics_engine.cpp` appears motivated by implementation simplicity rather than numerical stability, given that no integration step control mechanism is present.»
- ❌ **Incorrect:** «The physics engine probably handles collisions efficiently.»

---

### Rule 2 — Documentation Completeness: No Component May Be Omitted

The analysis must cover **the totality of systems and components present in the codebase**, without exceptions motivated by subjective perception of their relevance. Even components that appear elementary, redundant, or purely structural — commonly termed *scaffolding* — must be **documented and described**.

#### Handling of Trivial Components

Components that the auditor evaluates as trivial or support-level must be explicitly **marked with the `[TRIVIAL]` label** and accompanied by an explanation justifying their inclusion in the documentation. Typical reasons why a trivial component still warrants attention include:

1. **Architectural completeness**: even a simple configuration file or elementary wrapper contributes to understanding the overall system structure;
2. **Evolutionary potential**: a component that is trivial today may become the entry point for significant future extensions;
3. **Hidden dependencies**: apparently irrelevant components may be referenced by critical modules, creating dependencies not immediately visible;
4. **Regression risk**: changes to components considered negligible may introduce errors that are difficult to trace if not adequately documented.

#### Operational Example

A file `version.hpp` containing only a macro with the project version number is a trivial component by definition. However, it must be included in the analysis with a note such as:

> **[TRIVIAL]** `version.hpp` — Defines the macro `PROJECT_VERSION`. Included for completeness: this macro is referenced in `main.cpp` and `logger.cpp` for application header logging. No computational logic present.

---

### Rule 3 — Bijective Correspondence Between Deficiencies and Recommendations

There is a **mandatory correspondence constraint** between deficiencies identified in Phases 1, 2, and 3 of the analysis and recommendations formulated in Phase 4. The logical structure of this constraint is as follows:

> For every deficiency D identified in Phases 1–3, there must exist **at least one recommendation R** in Phase 4 that explicitly addresses D.

#### Consequences of the Constraint

- **No deficiency may be left without a proposed resolution**: if a problem is identified, it must mandatorily receive an operational response in Phase 4, even if the solution is complex or requires deep architectural intervention;
- **Generic recommendations do not satisfy the constraint**: a recommendation such as "improve error handling" is insufficient unless it is directly linked to a specific deficiency previously observed;
- **Tracing must be explicit**: every recommendation in Phase 4 must indicate which deficiency (identified by section and number) it refers to, using the cross-referencing system defined in Rule 6.

#### Completeness Verification

Before finalizing the document, the auditor must perform a **formal completeness check** by constructing a deficiency–recommendation traceability matrix, ensuring that every row (deficiency) has at least one filled cell in the corresponding recommendations column.

---

### Rule 4 — Immediate Actionability of Recommendations

Every recommendation formulated in Phase 4 must satisfy the requirement of **immediate actionability**, meaning it must provide sufficient information for a software engineer to begin implementing it without requiring further clarification or preliminary research.

#### Structural Requirements of an Actionable Recommendation

A recommendation is considered immediately actionable if and only if it contains **all** of the following elements:

1. **Problem description** — A clear and precise summary of the deficiency being addressed;
2. **Recommendation objective** — What must be achieved at the end of implementation;
3. **Concrete entry point** — The exact name of the file, class, and/or method from which to begin the modification;
4. **Description of initial intervention** — The first operational steps to take, described with sufficient technical granularity;
5. **Completion criteria** — How to verify that the recommendation has been correctly implemented.

#### Example of Non-Actionable vs. Actionable Recommendation

- ❌ **Non-actionable:** "Improve memory management in the rendering module."
- ✅ **Actionable:** «In the file `render_pipeline.cpp`, the method `RenderPipeline::submitFrame()` allocates a temporary buffer via `new` without a corresponding `delete` in the error path at line ~142. Replace the raw allocation with `std::unique_ptr<FrameBuffer>` to guarantee automatic resource release. Entry point: open `render_pipeline.cpp`, locate `submitFrame()`, and introduce the declaration `auto buffer = std::make_unique<FrameBuffer>(params)` in place of the problematic line.»

---

### Rule 5 — Precision and Non-Ambiguity of Technical Language

The entire document must be written in **precise, unambiguous technical English** — or in the language specified by Rule 9 — systematically avoiding the use of expressions that introduce unjustified uncertainty or interpretive vagueness.

#### Expressions to Avoid

The following expressions — and analogous linguistic constructions — are **explicitly prohibited** except in cases where the uncertainty is genuine and justified:

| Prohibited expression | Precise alternative |
|---|---|
| "might" | "does" (if observed) or "Inferred:" (if hypothetical) |
| "could possibly" | Describe the observed behavior directly |
| "seems to" | Cite the specific code that supports the claim |
| "appears to be" | Verify and assert with certainty or mark as inference |
| "it is likely that" | Ground in evidence or use the `Inferred:` prefix |

#### Handling Genuine Uncertainty

In cases where uncertainty is genuinely real — for example, when a component's behavior depends on external factors not visible in the analyzed source code — the use of dubitative language is **permitted but must be explicitly justified** with an explanation of why certainty is not reachable. Example:

> «The behavior of the method `NetworkManager::reconnect()` under timeout conditions cannot be determined with certainty from static code analysis, as it depends on the runtime configuration of the parameter `MAX_RETRY_COUNT` which is not initialized in `network_config.hpp`.»

---

### Rule 6 — Elimination of Redundancy via Cross-Referencing

The document must not contain **verbatim duplications of information** across different sections. When a concept, observation, or description elaborated in one section is also relevant to another, the system of **cross-referencing by section number** must be used rather than repeating the content.

#### Cross-Reference Format

The standard format for an internal reference is:

> «(see §*X.Y*)»

where *X* indicates the number of the main phase or chapter and *Y* the number of the specific subsection.

#### Operational Principle

- If a deficiency is already described in §2.3, Phase 4 must not redescribe it: it must simply reference it with «(see §2.3)» and proceed directly to the recommendation;
- If a component has already been introduced in §1.2, subsequent references may omit the full description and limit themselves to the cross-reference;
- Summary tables, if present, may refer to the original sections without duplicating their content.

#### Benefits of Cross-Referencing

1. **Reduced document volume** without loss of information;
2. **Improved maintainability**: a change to shared information needs to be made in only one place;
3. **Structural clarity**: the reader immediately understands where to find the original information.

---

### Rule 7 — Mechanical Computation of Recommendation Priority

The ordering of recommendations in Phase 4 must be determined **exclusively through the mechanical application of the scoring formula defined in §4.1**. Any reordering based on the auditor's subjective judgment after application of the formula is **explicitly prohibited**.

#### Principle of Ranking Objectivity

The formula defined in §4.1 is designed to ensure that recommendation prioritization reflects objective, measurable criteria — such as system impact, implementation cost, risk associated with the unresolved deficiency — rather than personal preferences or intuitions of the auditor.

#### Application Procedure

1. **Compute the score** for each recommendation by fully applying the formula from §4.1, without omitting any required factor;
2. **Sort recommendations** in descending order by score;
3. **Document the calculation**: for each recommendation, report the values of each individual factor used and the final score obtained, so that the ranking is reproducible and verifiable by third parties;
4. **Do not modify the order** once the calculation is complete, even if the result appears counterintuitive — in that case, the doubt must be reported as a marginal note, not as justification for altering the ranking.

---

### Rule 8 — Minimum Depth Requirements per Section

The document must respect **minimum length and depth requirements** differentiated by section type. These requirements represent **non-negotiable floors**, not targets to reach with precision: content must exceed these thresholds whenever the complexity of the material warrants it.

#### Defined Minimum Thresholds

| Section Type | Minimum Requirement |
|---|---|
| Component subsection (Phase 3) | **150 words** |
| System section (Phase 2) | **300 words** |

#### Interpretation of Thresholds

- The thresholds are **floors**, not ceilings or targets: a quality analysis will substantially exceed these values when the material requires it;
- A section that reaches exactly the minimum should be considered **suspect**: the analysis was likely terminated prematurely or lacks sufficient depth;
- Padding words that artificially inflate the count without adding informational value are **explicitly prohibited** and constitute a violation of the spirit of this rule, even if they technically satisfy the numerical requirement.

#### Application Example

A subsection dedicated to the class `AudioManager` in `audio_manager.hpp` must contain at least 150 words concretely describing: the class's public interface, its dependencies, the mechanism for managing audio resources, any deficiencies identified, and the behavior observed in key methods. It is not acceptable to reach 150 words through generic introductory paragraphs or repetitions.

---

### Rule 9 — Document Language: Italian

The entire document must be written in **Italian**, consistent with the language of the original request. This requirement applies to:

- All narrative and descriptive text;
- Section and subsection titles;
- Notes, annotations, and comments;
- Table and diagram labels.

#### Permitted Exceptions

The following exceptions are permitted, in which it is appropriate to maintain the original English terminology:

- **File names, class names, method names, and variable names**: these identifiers must be reported exactly as they appear in the source code, without translation (e.g., `RenderPipeline::submitFrame()`);
- **Established technical terms**: terms such as *template*, *namespace*, *buffer*, *overhead*, *callback* do not require translation and may be used directly in Italian without mandatory italics, as they have entered the Italian technical lexicon;
- **Direct code quotations**: any fragment of source code must be reported verbatim in the original language.

---

### Rule 10 — Prohibition of Unsupported Generic Statements

It is **explicitly prohibited** to include in the document generic statements applicable to any codebase, unless such statements are:

1. **Directly substantiated** by a specific, concretely observed deficiency in this specific codebase;
2. **Accompanied by a concrete remediation step** that identifies specific files, classes, or methods to intervene on.

#### Typically Prohibited Generic Statements

The following categories of statements, in the absence of specific substantiation, are prohibited:

- "The code could be better documented" ❌
- "It is recommended to add unit tests" ❌
- "Error handling could be improved" ❌
- "The architecture could benefit from refactoring" ❌

#### How to Make a Generic Statement Acceptable

Transforming a generic statement into a valid one requires three elements:

1. **Specific observation**: «The method `ConfigLoader::parseFile()` in `config_loader.cpp` does not handle the case where the configuration file is malformed: in the absence of a `try-catch` block, an uncaught `std::runtime_error` exception would cause process termination.»
2. **Link to the general deficiency**: «This is an instance of the broader problem of insufficient error handling in the configuration module.»
3. **Concrete, file-specific remedy**: «Add a `try-catch` block around the call to `json::parse()` within `ConfigLoader::parseFile()` in `config_loader.cpp`, with exception handling that logs the error via `Logger::error()` and returns a return value that signals failure to the caller.»

### Patterns for Constraint Compliance

#### Pattern: Deficiency-to-Recommendation Traceability

- **Objective:** Ensure a bijective mapping between identified deficiencies (Phases 1–3) and recommendations (Phase 4),
  so that no finding is orphaned and no recommendation exists without a motivating deficiency.
- **Context of application:** Apply during Phase 4 construction and during the final review pass.
- **Key characteristics:** Each deficiency is assigned a unique tag (e.g., `DEF-001`) when identified. Each
  recommendation in §4.1 references exactly one DEF tag in its "Deficiency addressed" field. A verification table lists
  all DEF tags on the left and their corresponding REC IDs on the right, confirming one-to-one coverage.
- **Operational guidance:**
    1. When writing a deficiency in Phases 1–3, assign it a DEF tag (`DEF-001`, `DEF-002`, ...) and increment the
       counter.
    2. When writing a recommendation in §4.1, include the DEF tag it resolves in the "Deficiency addressed" field.
    3. After completing Phase 4, build a traceability table: DEF tags in column one, REC IDs in column two.
    4. Verify that every DEF tag appears at least once. If a DEF tag is missing from the table,
       add the recommendation. If a REC references no DEF tag, remove it or identify the missing deficiency. Multiple
       appearances of a DEF tag are acceptable only if each recommendation's description explicitly states why
       addressing the same deficiency requires multiple distinct recommendations (e.g., "This is the first of three
       complementary mitigations for DEF-042"). Multiple appearances of a DEF tag are acceptable only if noted in the
       recommendation descriptions.

#### Pattern: Constraint-by-Constraint Verification Gate

- **Objective:** Systematically verify that every constraint (§1–§10) is satisfied in the final output before delivery.
- **Context of application:** Apply as the final step of the audit, after all content has been written and formatted.
- **Key characteristics:** Each constraint is treated as a test case with a pass/fail criterion. The auditor works
  through the constraint list sequentially, documenting evidence of compliance for each one. Any failure triggers a
  revision cycle before the document is considered complete.
- **Operational guidance:**
    1. Create a checklist with all ten constraints. For each, write the pass criterion (e.g., "Constraint 5: No
       instances of 'might,' 'could possibly,' or 'seems to' without explicit uncertainty rationale").
    2. Search the document for hedging language (§5). Flag and revise or justify each instance.
    3. Verify the language requirement (§9): confirm the entire document is in Italian. If any section is in English,
       translate it.
    4. Check cross-references (§6): ensure no verbatim repetition across sections. Spot-check by reading a paragraph
       from Phase 2 and confirming Phase 3 cross-references it rather than restating it.
    5. Recompute the priority ranking (§7) from raw scores to verify mechanical ordering.
    6. Sign off on each constraint individually. Any unchecked item means the audit is incomplete.

---

### Anti-Patterns for Constraint Compliance

#### Anti-Pattern: Orphaned Findings

- **Description:** The auditor identifies deficiencies during Phases 1–3 but fails to produce a corresponding
  recommendation in Phase 4 for every one, leaving some findings without a proposed resolution path.
- **Reasons to avoid:** The analytical phases naturally surface more issues than the recommendation phase can
  comfortably accommodate. Under deadline pressure, the auditor may silently drop lower-priority findings rather than
  scoring and sequencing them. This violates the explicit constraint that every deficiency maps to a recommendation.
- **Negative consequences:** Engineers receiving the audit discover real problems flagged with no guidance on how to fix
  them. This creates frustration and erodes trust in the process. The recommendation register is incomplete, and the
  constraint compliance claim is false.
- **Correct alternative:** Apply the **Deficiency-to-Recommendation Traceability** pattern to enforce a verifiable
  one-to-one mapping.

#### Anti-Pattern: Constraint Theater

- **Description:** The auditor asserts compliance with the listed constraints without performing systematic
  verification — e.g., stating "the document is in Italian" when portions remain in English, or claiming mechanical
  prioritization when rows were manually reordered.
- **Reasons to avoid:** Verification is tedious work that occurs at the end of a long analytical process. The auditor,
  fatigued and confident, may substitute an integrity check with an integrity claim. The gap between asserted and actual
  compliance is rarely visible to a casual reader but is immediately obvious to anyone who checks.
- **Negative consequences:** When stakeholders discover the asserted compliance is false (e.g., the summary table
  ordering does not match the formula), the audit's credibility collapses. Future audits are viewed with skepticism. The
  technical lead must repeat the verification work the auditor should have done.
- **Correct alternative:** Apply the **Constraint-by-Constraint Verification Gate** pattern to produce documented
  evidence of compliance for every constraint, rather than a blanket assertion.


il risultato salvalo in @typechecker_audit.md @include/jsav/typechecker/ @src/jsav_Lib/typechecker/
