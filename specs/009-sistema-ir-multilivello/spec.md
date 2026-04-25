# Feature Specification: Verifiable Multi-Level IR System

**Feature Branch**: `009-sistema-ir-multilivello`  
**Created**: 19 April 2026  
**Status**: Draft  
**Glossary**: `specs/009-sistema-ir-multilivello/GLOSSARY.md`  
**Input**: User description: "Design a multi-level intermediate representation system that enables modeling, validation, analysis, and transformation of programs along a compilation pipeline with progressive abstraction reduction, ensuring semantic preservation, data-flow precision, and formal verifiability of transformations.

Goal and success:
Users can represent programs as formal structures based on control-flow graphs and data flow, and transform them through HIR, MIR, and LIR without loss of observable meaning. The system guarantees that each transformation is semantically equivalent to the previous one with respect to effects on values and memory. Success is determined by: absence of structural errors after each pass, valid and minimal SSA form, correct handling of PHI nodes, consistency of the type system including user-defined types, and deterministic analysis results. Each phase must produce a validated representation or be interrupted with explicit errors.

Main entities:

- Module:
  Contains all global definitions. Includes Functions, user-defined Types, and metadata. Defines visibility context and global validation rules. Maintains a Type table and Function signature table.
- Function:
  Primary transformation unit. Defined by a signature with typed parameters and results. Contains a control-flow graph with a single entry point. It is the domain on which SSA form is applied.
- Basic block:
  Node in the control-flow graph. Contains an ordered sequence of Instructions with no internal interruption. Has explicit predecessors and successors. Ends with a control Instruction.
- Control-flow graph:
  Directed structure connecting Blocks through execution edges. Defines possible execution order. Includes one entry node and zero or more exit nodes.
- Instruction:
  Atomic operation. It can represent computation, control, or memory access. Consumes Values and produces new Values.
- Value:
  Immutable entity associated with a single definition. Each Value has a Type and a unique definition point.
- Type:
  Defines domain, structure, and usage rules for Values. Includes primitive, composite, and user-defined types.

Relationships:

- A Module contains Functions and Types.
- A Function contains Blocks connected in the control-flow graph.
- Each Block contains Instructions and ends with a control Instruction.
- Instructions define Values that can be used by other Instructions.
- The control-flow graph determines which definitions reach each use.
- PHI nodes combine Values coming from distinct predecessors.

Abstraction levels:

HIR:

- Users can express the program in high-level semantic form.
- Operations preserve direct logical meaning and can represent complex constructs.
- Variables can be conceptually reassigned before full SSA conversion.
- Analyses include semantic validation, type consistency, and control structure checks.
- There are no resource- or architecture-related constraints.
- Validation rules: See FR-004 (CFG), FR-011 (type compatibility); SSA form
 (FR-006) is not required; def-before-use is enforced locally.


MIR:

- The program is fully converted to SSA form.
- All definitions are unique and PHI nodes are explicit.
- Operations are granular and represent elementary computations.
- Analyses include dominance, reaching definitions, and instruction dependencies.
- Optimizations modify data flow while preserving semantic equivalence.
- Validation rules: Must satisfy FR-006 (complete SSA), FR-007 (RD-based PHI),
 FR-008 (PHI operand completeness), and the fundamental SSA constraint (Line 380).

LIR:

- Instructions are constrained by the execution model.
- Control flow is expressed through explicit jumps.
- Temporal and memory dependencies are fully explicit.
- Operations are reduced to directly executable forms.
- No high-level abstractions are present.
- Validation rules: SSA form may be deconstructed; all control flow must be
 explicit jumps/branches; operations must be execution-model compatible (see FR-002).

SSA form and construction:

- Each Value is defined only once.
- Each use is dominated by its definition.
- The system maintains an explicit mapping between definitions and uses.
- PHI nodes are inserted at control convergence points.

PHI construction:

- The system computes reaching definitions for each variable.
- For each Block with multiple predecessors, it checks whether distinct definitions arrive.
- It inserts a PHI node only when needed.
- Each PHI node contains a complete association between predecessors and Values.
- It avoids redundant insertions to keep SSA form minimal.
- It automatically updates PHIs when the control-flow graph changes.

SSA maintenance:

- Variable renaming to preserve uniqueness.
- Elimination of unnecessary PHIs.
- Value propagation when possible.

Type system:

- Every Value has an explicit and immutable Type.
- Instructions define constraints on operand and result Types.
- The system supports:
    - Primitive types
    - Composite types
    - User-defined types
- User-defined Types must specify:
    - structure
    - equivalence rules
    - operation compatibility
- Operations must be valid with respect to Types.
- Transformations must preserve Types or declare valid conversions.
- The system rejects operations with incompatible Types.

Data flow and analysis:

- The system builds a dependency graph among Instructions.
- It supports forward and backward analyses.
- Reaching definitions:
    - determine which definitions reach each point in the program
    - drive PHI insertion and optimizations
- Dominance:
    - defines definition validity with respect to uses
- Liveness analysis:
    - identifies live Values at each point
- Analyses are consistent with SSA form.

Memory management and aliasing:

- Memory operations are explicit and typed.
- Each access declares whether it is a read or write.
- Dependencies among accesses are tracked.
- The system models possible aliases among references.
- Analyses can determine when accesses are independent.
- Transformations cannot modify observable access order when dependencies exist.
- The system guarantees consistency of the memory model.

Processing pipeline:

- Users can define pass sequences.
- Each pass operates on a valid representation.
- Pass types:
    - Analysis: does not modify structure
    - Transformation: modifies representation
    - Optimization: reduces redundancy
    - Lowering: reduces abstraction level
- Each pass must declare:
    - preconditions
    - preserved invariants
    - structural effects
- Pass ordering is explicit and controlled.
- The system supports incremental pass execution.

Verification and validation:

- The system executes checks after each pass.
- Control-flow graph check:
    - single entry presence
    - valid edges
- SSA check:
    - single definition per Value
    - dominance invariant: each use MUST be dominated by its unique definition point (fundamental SSA structural property)
- PHI check:
    - one operand per predecessor
- Type check:
    - compatibility between operands and results
- Dependency check:
    - no use without definition
- Memory check:
    - dependency and aliasing compliance
- Errors are reported with precise localization.

Expected behavior:

- Users can build IR at any level.
- Users can transform IR while preserving validity.
- Users can introduce new Types.
- Users can run analyses and obtain program information.
- The system maintains traceability across representations.
- The system prevents invalid intermediate states.

Included scope:

- Multi-level IR HIR MIR LIR
- Complete SSA form with PHI nodes
- Precise PHI construction based on reaching definitions
- Rigorous and extensible type system
- Data-flow and control analyses
- Transformation and optimization pipeline
- Continuous structural and semantic verification
- Memory and aliasing management

Excluded scope:

- Implementation choices
- Hardware-specific architectures
- Direct machine code generation
- User interfaces"

## Clarifications

### Session 2026-04-19

- Q: Which SSA/PHI strategy must be canonical in MIR? -> A: PHI based on reaching definitions; dominance frontier only as non-binding analytical support.
- Q: How must semantic equivalence between IR levels be defined? -> A: Same final values and same observable effects on memory, with preserved relative order of dependencies.
- Q: What is the commit/failure policy for passes? -> A: Each pass is atomic; on error, full rollback to the previous valid IR state occurs.
- Q: What equivalence rule must the system use for user-defined types? -> A: Nominal equivalence (same declared type identity).
- Q: Which ordering policy must apply to analysis output, errors, and reports? -> A: Deterministic total ordering with a stable canonical key.
- Q: What structure must the stable canonical ordering key have? -> A: Stable hierarchical key: module/function/block/instruction-index/operand-index.
- Q: Which traceability strategy between levels must be canonical? -> A: Global immutable ID for each IR entity with explicit derivation relations across HIR, MIR, and LIR.
- Q: What scale target must be assumed for this feature? -> A: Medium scale: up to 100k instructions per function and 2M per module.
- Q: When a predecessor becomes unreachable, what PHI policy must be canonical? -> A: Immediate removal of the corresponding edge and PHI operand during CFG update (eager normalization).
- Q: For global immutable IDs, what generation policy must be canonical? -> A: Deterministic IDs derived from canonical structural paths, stable for identical input and pipeline.
- Q: When the definition of a user Type changes while Values are already typed, what policy must apply? -> A: Nominal versioning: a new definition creates a new type identity; existing Values remain on the previous version.

### Session 2026-04-20

- Q: Which error reporting policy must be applied when validation of a failed pass occurs? -> A: Batch per pass: collect all errors in the current representation, then fail the pass as a whole.
- Q: When a transformation removes a block that defines values used in multiple places (including PHIs), what canonical policy must apply? -> A: Rewrite-safe: rewrite all uses to dominated equivalent definitions, update PHI/CFG, then remove the block only if validation passes.
- Q: When two memory accesses are in a may-alias relation and a transformation proposes reordering, what canonical policy must apply? -> A: Strict no-reorder: reordering may-alias accesses is forbidden unless there is formal proof of independence.
- Q: Which canonical policy must apply to PHI minimality when convergence includes unreachable paths? -> A: Eager pruning: immediate removal of unreachable contributions and local recomputation of PHI minimality at each CFG update.
- Q: Which concurrency model must be canonical for pass execution and analyses? -> A: Deterministic single-thread execution per pass (no intra-pass parallelism).
- Q: What evidence must be canonical to accept formal proof of independence that allows exception to strict no-reorder? -> A: Dual criterion: verifiable certificate or reproducible internal proof with alias/liveness/dependence analysis and full logging.
- Q: Which IR mutation model must be canonical during a pass? -> A: Transactional working copy per function or pass, with atomic commit only after validation completes.

## User Scenarios & Testing *(mandatory)*

### User Story 1 - IR Construction and Validation (Priority: P1)

As a compiler developer, I want to build programs in HIR, MIR, or LIR and immediately obtain structural and semantic validation, so that invalid intermediate states are prevented.

**Why this priority**: It is the minimum indispensable value: without robust validation, no subsequent transformation is reliable.

**Independent Test**: It can be tested by building a module with valid/invalid functions and blocks at each level; the system must accept only valid representations and stop invalid cases with explicit errors.

**Acceptance Scenarios**:

1. **Given** a Module with well-typed Functions and a valid control-flow graph, **When** validation of the current level is executed, **Then** the representation is marked as valid and ready for subsequent passes.
2. **Given** a Function with blocks missing a terminator or with inconsistent edges, **When** validation is executed, **Then** the pipeline stops with localized, descriptive errors.
3. **Given** a Value use without a reachable definition, **When** validation is executed, **Then** the system reports a dependency error with precise localization.

---

### User Story 2 - Semantic Transformations Across HIR/MIR/LIR (Priority: P2)

As a compiler developer, I want to transform the program from HIR to MIR and then to LIR while preserving observable meaning on values and memory, so that I get a reliable lowering pipeline.

**Why this priority**: The core of the feature is progressive abstraction reduction without semantic regressions.

**Independent Test**: It can be tested by applying pass sequences to programs with complex control and memory behavior; observable results must remain equivalent between consecutive levels.

**Acceptance Scenarios**:

1. **Given** a valid HIR program, **When** the MIR lowering pass is applied, **Then** the resulting program is valid and semantically equivalent with respect to values and memory.
2. **Given** a valid MIR program, **When** the LIR lowering pass is applied, **Then** control flow is expressed with explicit jumps and observable semantics is preserved.
3. **Given** a transformation that would violate memory dependencies, **When** the pass is executed, **Then** the system rejects the transformation with an explicit error.

---

### User Story 3 - Deterministic Analyses and Traceability (Priority: P3)

As a compiler developer, I want to run analyses (dominance, reaching definitions, liveness, dependencies) with deterministic and traceable results across levels, so that I can support repeatable and verifiable optimizations.

**Why this priority**: It improves reliability and auditability of the whole pipeline, but depends on validation and transformations already being operational.

**Independent Test**: It can be tested by rerunning the same analyses on the same input and comparing results; they must be identical and consistent with SSA form.

**Acceptance Scenarios**:

1. **Given** a valid MIR representation in SSA, **When** forward and backward analyses are executed, **Then** results are deterministic and consistent with dominance and reaching definitions.
2. **Given** control-flow graph modifications due to optimizations, **When** the system updates SSA and PHI, **Then** subsequent analyses remain consistent and free of inconsistencies.

---

### Edge Cases

- When a predecessor becomes unreachable after a transformation, the system immediately removes the corresponding edge and PHI operand during CFG update (eager normalization), preserving SSA/PHI validity with no temporary mismatches.
- If a transformation eliminates a block with definitions used in multiple points (including PHIs), the system applies rewrite-safe policy: rewrites all uses toward dominated equivalent definitions, updates PHI/CFG, and allows elimination only after successful validation.
- When the definition of a user Type changes, the system applies nominal versioning: it creates a new type identity and keeps already typed Values bound to the previous version.
- When control convergence includes unreachable paths, the system applies eager pruning: immediately removes unreachable contributions and locally recomputes PHI minimality at each CFG update.
- If two memory accesses are in a may-alias relation, the system applies strict no-reorder: it forbids reordering unless formal proof of independence demonstrates absence of observable dependency (see FR-028, FR-030 for evidence criteria).

## Requirements *(mandatory)*

### Functional Requirements

- **FR-001**: The system MUST represent programs as Modules containing Functions, Types, and metadata with global visibility and validation rules.
- **FR-002**: The system MUST support three distinct IR levels (HIR, MIR, LIR) with level-specific validity rules.
- **FR-003**: The system MUST allow IR construction at any level, provided each representation satisfies the validation rules of the selected level.
- **FR-004**: The system MUST maintain a control-flow graph for each Function with a single entry, valid edges, and control terminators in each block.
- **FR-005**: The system MUST model immutable Values with a single definition point, explicit Type, and full def-use traceability.
- **FR-006**: The system MUST maintain complete SSA form in MIR, including insertion, update, and minimal removal of PHI nodes, using reaching definitions as canonical placement criterion.
- **FR-007**: The system MUST insert PHI nodes only at convergence points where reaching definitions indicate distinct definitions coming from different predecessors; dominance frontier may be used only as non-binding analytical support.
- **FR-008**: The system MUST guarantee that each PHI node includes exactly one operand for each predecessor of the target block.
- **FR-009**: The system MUST automatically update SSA structure and PHIs after any control-flow graph modification, preserving the canonical criterion based on reaching definitions; if a predecessor becomes unreachable, it MUST immediately remove the corresponding edge and PHI operand (eager normalization).
- **FR-010**: The system MUST support a type system with primitive types (integers, floats, bool), composite types (fixed-size arrays, structs with typed fields, typed pointers), and user-defined types. User-defined types MUST explicitly specify: unique name, total size in bytes, alignment, and ordered field list (each with Type and relative offset), thereby defining structure rules, nominal equivalence, and operational compatibility.
- **FR-011**: The system MUST reject operations with incompatible Types and stop the current pass with explicit, localized errors.
- **FR-012**: The system MUST preserve Types during transformations or declare valid and verifiable conversions through the IrValidator executed at the end of each pass.
- **FR-013**: The system MUST support dominance, reaching definitions, liveness, alias, and instruction-dependency analyses in forward and backward modes.
- **FR-014**: The system MUST guarantee deterministic analysis results for equal input, pass order, and configuration.
- **FR-015**: The system MUST model typed memory accesses, distinguishing read/write and tracking dependencies and possible aliases.
- **FR-016**: The system MUST prevent transformations that alter observable order of memory accesses when dependencies exist.
- **FR-017**: The system MUST allow explicit pass pipelines (analysis, transformation, optimization, lowering) with declared preconditions, preserved invariants, and effects.
- **FR-018**: The system MUST execute automatic checks after each pass and produce validated output or interrupt the pipeline with errors.
- **FR-019**: The system MUST maintain traceability between consecutive representations (HIR->MIR->LIR) through global immutable IDs for IR entities (Module/Function/Block/Instruction/Value) and explicit derivation relations, to support transformation audits; ID generation MUST be deterministic and derived from a canonical structural path of the entity (module/function/block/index and entity type), stable for equal input, pass order, and configuration.
- **FR-020**: The system MUST prevent persistence of invalid intermediate states.
- **FR-021**: The system MUST consider two representations semantically equivalent only if they preserve both final observable values and observable memory effects, including preservation of relative order among dependent accesses (as defined in FR-016). For floating point values, "same final values" MUST be interpreted as bitwise equality (IEEE 754 bit-identical representation) to ensure deterministic results across all transformations.
- **FR-022**: The system MUST execute each pass through a **PassTransaction** mechanism on a working copy of the target representation (function or pass): commit to observable IR state occurs only after validation completes; in case of validation or transformation error, the pre-pass observable state must be fully preserved through rollback, with no partial commits.
- **FR-023**: The system MUST apply nominal equivalence to user-defined types: two user types are equivalent only if they share the same declared identity in the module or defined visibility scope; each change in definition/shape/rules MUST generate a new nominal identity (version), without implicitly retyping existing Values. Versions MUST be uniquely and deterministically identified through the hash of the type's structural definition using a collision-resistant hash algorithm (minimum SHA-256 or equivalent with 2^128+ collision resistance); the structural definition MUST be serialized in a canonical binary format (fields sorted lexicographically by name, names encoded as UTF-8, and numeric values—size, alignment, offsets—encoded as big-endian 64-bit integers) before hashing; the system MUST detect and reject hash collisions by comparing full structural definitions on identifier match; each Value MUST keep an explicit reference to the Type version it belongs to.
- **FR-024**: The system MUST emit analysis output, errors, and reports in deterministic total order based on a stable hierarchical canonical key (module/function/block/instruction-index/operand-index), for equal input, pass order, and configuration; this key MUST be aligned with the canonical deterministic ID generation policy defined in FR-019.
- **FR-025**: The system MUST support the target medium-scale use case: up to 100k instructions per Function and up to 2M total instructions per Module, while maintaining complete validation, analyses, and transformations.
- **FR-026**: If pass validation fails, the system MUST apply batch-per-pass reporting: during post-transformation validation, collect all detectable errors through applicable structural, type, SSA, CFG, and memory checks on the post-transformation representation, then fail the pass as a whole with no partial commits (consistent with FR-022). If an error prevents dependent checks, the system MUST explicitly annotate omitted checks.
- **FR-027**: If a transformation removes a block with definitions used in multiple places (including PHI nodes), the system MUST apply rewrite-safe policy: (1) preliminarily verify that for each value V defined in the candidate block there exists an SSA-identical or semantically equivalent alternative definition that dominates all uses of V; (2) if verification fails, reject the transformation without modifications; (3) otherwise, rewrite all uses toward the alternative definitions, coherently update PHI/CFG, then (4) allow block elimination only after successful post-transformation validation.
- **FR-028**: If two memory accesses are in may-alias relation, the system MUST apply strict no-reorder: forbid access reordering unless formal proof of independence (as defined in FR-030) is available and shows preservation of observable memory effects.
- **FR-029**: Canonical execution of passes and analyses MUST be deterministic single-thread per pass (no intra-pass parallelism), preserving canonical ordering and bit-identical repeatability of results for equal input, pipeline, and configuration.
- **FR-030**: To override strict no-reorder in the presence of may-alias (FR-028), the system MUST accept only formal proof of independence with a canonical dual criterion: (a) verifiable certificate checked by a dedicated checker conforming to the standard JSON format defined in `contracts/proof-witness-contract.md` (data structure containing independence witness, applied inference rules, and conclusive fact), whose checker MUST verify derivation soundness; or (b) reproducible internal proof based on alias/liveness/dependence analysis with full deterministic logging conforming to the format defined in `contracts/proof-witness-contract.md`, containing: alias facts for involved accesses, liveness intervals of values, relevant dependence edges, applied inference steps, and independence conclusion, in machine-readable format for independent verification. In both cases, the proof MUST be generated deterministically and reproducibly for equal input, pipeline, and configuration (consistent with FR-014, FR-029).

### Key Entities *(include if feature involves data)*

- **Module**: Global container for Functions, user-defined Types, and structural metadata (IR version, source origin, target triple), type table, and function signatures.
- **Function**: Primary transformation unit with typed signature, single entry, and SSA domain.
- **Basic block**: Control-flow graph node with ordered Instruction sequence and mandatory control terminator.
- **Control-flow graph**: Directed execution structure among blocks with reachability and convergence constraints.
- **Instruction**: Atomic computation, control, or memory operation that consumes and/or produces Values.
- **Value**: Immutable entity with Type and unique definition.
- **Type**: Formal definition of value domain and usage rules, including extension with user-defined types.
- **PHI node**: SSA combiner of definitions coming from multiple predecessors.
- **Pass**: Transformation or analysis with explicit contract (preconditions, invariants, effects).

### Scope Boundaries

**In scope**:

- Multi-level IR HIR/MIR/LIR.
- Complete SSA form with minimal PHI management.
- PHI construction based on reaching definitions.
- Rigorous and extensible type system.
- Deterministic data/control-flow analyses.
- Pass pipeline with continuous verification.
- Explicit memory and aliasing modeling.

**Out of scope**:

- Specific implementation choices.
- Hardware-specific architectures.
- Direct machine code generation.
- User interfaces.

## Success Criteria *(mandatory)*

### Measurable Outcomes

- **SC-001**: 100% of passes executed on valid input produce either a validated representation or an explicit error, with no persistent invalid intermediate states.
- **SC-002**: In 100% of valid MIR cases, each Value use is dominated by its definition and each Value has exactly one definition.
- **SC-003**: In 100% of blocks with PHI, each predecessor contributes exactly one operand and no observable redundant PHIs remain after minimization.
- **SC-004**: In 100% of HIR->MIR and MIR->LIR transformations on the approved regression suite, observable semantic equivalence over values and memory is preserved.
- **SC-005**: In 100% of repeated executions with the same input and same pipeline, analysis results (dominance, reaching definitions, liveness, dependencies) are identical.
- **SC-006**: In 100% of type incompatibility or structural violation cases, the pipeline stops at the correct phase with localized error and verifiable rationale.
- **SC-007**: In 100% of cases validated as equivalent across levels, both final observable values and observable memory effects remain unchanged, with preserved relative order of dependencies.
- **SC-008**: In 100% of failed passes, post-failure observable IR state matches the last valid pre-pass state (full rollback, no partial commit).
- **SC-009**: In 100% of type-to-type checks for user-defined types, equivalence outcome depends only on declared nominal identity and not solely on structure; in case of type redefinition, 100% of pre-existing Values remain associated with the original nominal version.
- **SC-010**: In 100% of repeated executions with the same input and same pipeline, analysis/error/report ordering matches exactly according to the stable hierarchical canonical key (module/function/block/instruction-index/operand-index).
- **SC-011**: In 100% of valid HIR->MIR->LIR transformations on the approved regression suite, each tracked IR entity keeps a deterministic global immutable ID (derived from canonical structural path) and has at least one explicit, verifiable derivation relation toward its immediate source entity in the previous level; for full audit support, the system SHOULD also keep transitive relations to the original first-level HIR entity when the entity is derived from HIR.
- **SC-012**: On the approved benchmark suite at target scale (up to 100k instructions per Function and 2M per Module), in 100% of cases the system completes validation, core analyses, and planned passes without violating functional constraints defined in FR-001..FR-030.
- **SC-013**: In 100% of passes that fail validation, the report includes the complete set of detectable errors for that pass on the current representation, and execution ends with a single pass-failure outcome (no partial commit).
- **SC-014**: In 100% of transformations that remove blocks with multiple uses (including PHIs), all uses are rewritten toward dominated equivalent definitions and post-pass validation confirms absence of dangling use-def and SSA/CFG consistency.
- **SC-015**: In 100% of cases with may-alias memory accesses, no pass performs reordering without formal proof of independence; when proof is present, post-pass validation confirms preservation of observable memory effects.
- **SC-016**: In 100% of executions with same input, pipeline, and configuration, passes and analyses run in canonical single-thread mode produce bit-identical output and reports, with no dependency on intra-pass concurrent interleavings.
- **SC-017**: In 100% of cases where a pass applies an exception to strict no-reorder on may-alias accesses, verifiable evidence compliant with the canonical dual criterion is present (checker certificate or reproducible internal proof with complete log), and post-pass validation confirms preservation of observable memory effects.

## Assumptions

- Users are compiler developers and operate on programs already lexically and syntactically valid.
- Required validation concerns structural, type, flow, and memory coherence of IR representations.
- The notion of semantic equivalence requires equality of final observable values and preservation of observable memory effects, including preservation of dependency-imposed relative order.
- Pass pipelines are explicitly declared and evaluated in deterministic order.
- The target feature scale is medium: up to 100k instructions per Function and 2M instructions per Module.
- User-defined types provide sufficient metadata for equivalence and operational compatibility.
- For user-defined types, the adopted canonical equivalence is nominal.
- Evolution of user-defined types follows nominal versioning and does not retroactively change typing of already emitted Values.
- GUI needs and machine code output are outside this feature.

# TECHNICAL EXTENSION: SSA MODEL AND PHI CONSTRUCTION

This section formally integrates literature results on SSA, dominance frontier, and reaching definitions at MIR level.

---

## 1. Fundamental SSA Constraint in MIR

MIR is constrained to SSA form:

\forall v,\ \exists!\ \text{def}(v) \land \text{def}(v) \text{ dominates all uses}(v)

This implies complete structural control over flow graph and definitions.

---

## 2. Classical Dominance-Based Construction

The standard strategy uses dominance frontier.

DF(n)={m \mid n \text{ dominates a predecessor of } m \land n \not\succ m}

Iteration:

DF^{+}(S)=\mu X.; S \cup DF(X)

Use:

- PHI insertion at join points
- dependency on global dominance

Limit:

- over-approximation
- possible redundant PHIs

---

## 3. Structural Limit of DF Model

The DF model introduces approximation:

- join points estimated by structure, not by actual reachability
- sensitivity to irreducible CFGs
- unnecessary PHIs in local cases

---

## 4. Reaching-Definitions-Based Construction

Dataflow-based alternative.

RD(n)={d \mid d \text{ reaches } n}

PHI condition:

\text{PHI}(n,x) \iff |RD_{pred}(n,x)| > 1

Properties:

- PHIs only when semantically necessary
- reduced redundancy
- dependency on real data flow

---

## 5. Incremental SSA Construction

Alternative model:

- PHIs inserted during CFG construction
- simultaneous renaming
- no global DF phase needed

Implication:

- MIR can be built directly in valid SSA
- local updates propagate SSA changes

---

## 6. Architectural Impact on IR System

MIR adopts a canonical SSA mode:

1. Reaching-definitions SSA (normative criterion for PHI insertion)

Dominance frontier is allowed as support for analysis and optimization, but alone does not define final PHI validity.

Global constraint:

\forall \phi,\ \forall i,\ operand_i \in RD(pred_i)

---

## 7. Requirements Integration

FR-006 complete SSA requires canonical RD criterion for PHI placement.

FR-007 PHI placement depends on reaching definitions; DF remains auxiliary.

FR-009 SSA update requires local recomputation coherent with RD.

---

## 8. Structural Summary

The IR system adopts a canonical SSA theory in MIR:

- RD-based: normative criterion of PHI correctness and minimality
- DF-based: optional support to accelerate analysis or insertion candidates
- Incremental: allowed only if it produces output equivalent to RD criterion

MIR does not delegate canonical choice to individual pass: final validity is always checked against reaching definitions.
