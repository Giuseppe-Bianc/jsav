# Glossary - Verifiable Multi-Level IR System

This glossary defines canonical terminology for the feature in specs/009-sistema-ir-multilivello.
All feature documents should use these terms consistently.

## Canonical Terms

- HIR: High-level intermediate representation.
- MIR: Mid-level intermediate representation in full SSA form.
- LIR: Low-level intermediate representation with explicit control-flow operations.
- IR Entity: One of Module, Function, BasicBlock, Instruction, Value.
- Module: Global container for functions, user-defined types, and metadata.
- Function: Primary transformation unit with typed signature and CFG.
- BasicBlock: CFG node containing an ordered instruction sequence ending with a control terminator.
- CFG: Directed control-flow graph with one entry and zero or more exits.
- Value: Immutable typed entity with a single definition point.
- Type: Domain and structural rules for values (primitive, composite, user-defined).
- User-defined Type: Type with declared nominal identity and versioning rules.
- Nominal Equivalence: Type equivalence based on declared identity, not structural shape alone.
- Nominal Versioning: New type identity generated when definition changes.
- SSA: Static single assignment form where each value has exactly one definition.
- PHI Node: SSA combiner that merges definitions from distinct CFG predecessors.
- Reaching Definitions (RD): Dataflow analysis identifying definitions that reach each program point.
- Dominance: Relation where a node must be traversed before another in all paths from entry.
- Liveness: Analysis that determines values that may be used in the future.
- Dependence Analysis: Analysis of value and memory dependencies between instructions.
- Alias Analysis: Analysis of whether memory accesses may refer to the same location.
- May-alias: Relation indicating two accesses might refer to the same memory location.
- Strict No-Reorder: Policy forbidding reordering of may-alias accesses without formal proof.
- Formal Proof of Independence: Verifiable evidence allowing a no-reorder exception.
- Proof Witness: Machine-readable evidence bundle for independence checking.
- Pass: Analysis, transformation, optimization, or lowering step with declared contract.
- Pass Pipeline: Explicit ordered sequence of passes.
- PassTransaction: Working-copy execution model with atomic commit and rollback on failure.
- Batch-per-pass Reporting: Error policy that aggregates all detectable pass errors before failing.
- Canonical Key: Stable hierarchical key module/function/block/instruction-index/operand-index.
- Global Immutable ID: Deterministic ID for IR entities derived from canonical structural path.
- Derivation Relation: Explicit traceability link between entities across IR levels.
- Semantic Equivalence: Preservation of observable values and memory effects between representations.
- Eager Normalization: Immediate removal of unreachable predecessor edges and PHI operands.
- Eager Pruning: Immediate elimination of unreachable PHI contributions and local PHI minimization.
- Rewrite-safe Block Elimination: Block removal policy requiring equivalent dominating rewrites before deletion.

## Usage Notes

- Prefer canonical terms above over synonyms.
- If a new technical term is introduced, add it here first and then use it in spec/plan/tasks.
- Keep this file aligned with FR and SC terminology.
