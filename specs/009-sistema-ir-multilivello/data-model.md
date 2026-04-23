# Phase 1 Data Model - Verifiable Multi-Level IR System

## 1. Module

- Purpose: Global IR context and validation boundary.
- Core fields:
  - module_id: GlobalEntityId
  - name: string
  - type_table: map<TypeName, TypeDefVersion>
  - function_table: map<FunctionName, FunctionSignature>
  - functions: vector<Function>
  - metadata: ModuleMetadata
- Validation rules:
  - type_table entries must have exactly one corresponding UserDefinedTypeVersion definition.
  - function_table entries must have exactly one corresponding Function definition with matching signature.
  - No forward declarations are permitted; all referenced entities must be defined within the module.
  - Bidirectional function consistency: the functions vector and function_table must contain exactly the same set of function names.
  - Circular type dependencies must be detected and either prohibited or require explicit indirection (e.g., pointer/reference types).
  - module_id must be globally unique and follow the GlobalEntityId format specification.
  - name must be non-empty and contain only valid identifier characters.
  - All TypeRef and FunctionRef used within the module must resolve to entries in type_table or function_table respectively.


## 2. Function

- Purpose: Primary pass unit and SSA domain.
- Core fields:
  - function_id: GlobalEntityId
  - signature: FunctionSignature
  - entry_block_id: BlockId
  - blocks: vector<BasicBlock>
  - cfg: ControlFlowGraph
  - ssa_index: SsaIndex
- Validation rules:
  - A single entry block.
  - Each block ends with a control terminator.
  - All blocks must be reachable from the entry_block_id via CFG edges.
  - CFG edges are bidirectionally consistent: block B is in A's successors iff A is in B's predecessors.
  - Each terminator instruction's control-flow semantics must match the block's successor set.
  - SSA dominance property: every Value use must be dominated by its defining instruction.

## 3. BasicBlock

- Purpose: CFG node with an instruction sequence.
- Core fields:
  - block_id: GlobalEntityId
  - instructions: vector<Instruction>
  - predecessors: vector<BlockId>
  - successors: vector<BlockId>
  - terminator: Instruction
- Validation rules:
  - Terminator is mandatory and must be the last element of the instructions vector.
  - The terminator instruction must have a control-flow opcode.

  - The instructions vector must contain at least one instruction (the terminator).
  - All phi nodes (if any) must appear consecutively at the beginning of the instructions vector, before any non-phi instructions.
  - Within a single basic block, instruction operands must only reference values defined in dominating blocks or earlier in the current block (local SSA ordering).
  - block_id must be unique within the containing function.
  - All BlockIds in predecessors and successors vectors must reference blocks that exist in the function's blocks vector.
  - Predecessors/successors are bidirectionally consistent.

## 4. Instruction

- Purpose: Atomic computation/control/memory operation.
- Core fields:
  - instruction_id: GlobalEntityId
  - opcode: OpCode
  - operands: vector<ValueRef>
  - result: optional<ValueDef>
  - memory_effect: MemoryEffectKind (none/read/write/readwrite)
  - type_constraints: ConstraintSet
- Validation rules:
  - Operands are compatible with opcode.
  - Result type is consistent with type_constraints.
  - Memory accesses are marked and orderable by dependency.

## 5. Value

- Purpose: Immutable SSA entity with a single definition.
- Core fields:
  - value_id: GlobalEntityId
  - defining_instruction: InstructionId
  - value_type: TypeRef
  - version: SsaVersion
  - use_sites: vector<UseSite>
- Validation rules:
  - A single definition for each value_id.
  - Every use is dominated by the definition in MIR.

## 6. Type System

- Purpose: Primitive/composite/user type model.
- Entities:
  - PrimitiveType
  - CompositeType
  - UserDefinedTypeVersion
- User-defined nominal version fields:
  - type_identity: TypeIdentity
  - version_id: DeterministicVersionId
  - layout_spec: TypeLayout
  - equivalence_rule: nominal
- Validation rules:
  - User type equivalence is nominal (identity + version), not structural.
  - No implicit retroactive retyping:
    - PROHIBITED: Changing the type of an existing Value after definition.
    - PROHIBITED: Modifying fields of an existing TypeLayout.
    - PROHIBITED: Silently reinterpreting values between type versions.

## 7. PhiNode

- Purpose: Definition convergence in SSA.
- Core fields:
  - phi_id: GlobalEntityId
  - target_value: ValueId
  - incoming: vector<PhiIncoming>  # (predecessor_block_id, value_id)
- Validation rules:
  - One operand for each current predecessor of the block.
  - Each incoming value reaches the block from its associated predecessor.
  - Eager pruning on unreachable predecessors.

## 8. PassTransaction

- Purpose: Isolate mutations and enable full rollback.
- Core fields:
  - transaction_id: DeterministicPassTxnId
  - scope: FunctionId | ModuleId
  - working_copy: IrSnapshot
  - status: started | validated | committed | rolled_back
  - errors: vector<CompileError>
- Validation rules:
  - Commit is allowed only with status=validated and empty errors.
  - On failure: status=rolled_back with no persistent observable effects.
- Lifecycle:
  - started: Transaction opened by pass; mutations applied to working_copy.
  - validated: Pass invokes validate(); all IR invariants checked; errors recorded.
  - committed: If errors is empty, working_copy replaces live IR; transaction becomes immutable.
  - rolled_back: If validation fails or pass aborts, working_copy discarded; no side effects.
- Concurrency:
  - Multiple function-scoped transactions may execute concurrently if they operate on disjoint functions.
  - Module-scoped transactions are exclusive; no concurrent transactions permitted.
  - Nested transactions are not supported; attempting to begin a transaction within an active transaction raises an error.


## 9. AnalysisReport

- Purpose: Deterministic output for dominance/RD/liveness/dependence.
- Core fields:
  - report_kind: dominance | reaching_defs | liveness | dependence
  - items: vector<AnalysisItem>
  - sort_key: CanonicalStableKey
- Validation rules:
  - Stable total ordering: module/function/block/instruction-index/operand-index.
  - Bit-identical repeatability with the same input/pipeline/config.
- Invalidation rules:
  - Analyses are scoped to specific IR snapshots and do not automatically update.
  - After a PassTransaction commits with status=committed, all analyses scoped to the modified Function or Module are invalidated.
  - Incremental analysis updates are not required but may be provided as an optimization.
- Transaction interaction:
  - Analyses may be computed on a PassTransaction's working_copy before commit.
  - Such analyses are discarded if the transaction is rolled back.

## Relationships

- Module 0..N Function (owns)
- Module 0..N UserDefinedTypeVersion (owns, via type_table)
- Function 1..N BasicBlock
- BasicBlock 1..N Instruction
- Instruction 0..1 ValueDef
- ValueDef 1..N UseSite
- BasicBlock 0..N PhiNode
- PhiNode 1..1 BasicBlock (owned by)
- PassTransaction 1..1 WorkingCopy
- PassTransaction 1..1 (Function | Module) (scoped to)
- Instruction 0..N TypeRef (operand types + result type)
- Value 1..1 TypeRef (value_type)
- Function 1..1 SsaIndex (owns all Values in function)

### Ownership semantics

- Composition (owns): Parent lifetime controls child lifetime
- Reference: Parent references child but doesn't control lifetime  
- Relationships marked "(owns)" are compositional

## State Transitions

- PassTransaction: started -> validated -> committed
- PassTransaction failure path: started|validated -> rolled_back
- Type version: created(vN) -> superseded(vN+1) without mutating existing value bindings

## Edge/Corner Case Mapping

- Unreachable predecessor removal: update CFG + remove PHI incoming immediately.
- Block elimination rewrite-safe: rewrite all uses before structural delete.
- May-alias reorder proposal: reject unless formal proof artifact is valid and reproducible.
- Non-reducible CFG: SSA validation still enforced by RD and dominance checks.

## 10. Formal Proof & Independence Certification

- Purpose: Document and verify formal proofs that allow exceptions to strict no-reorder on may-alias (FR-030).
- Core fields (ProofWitness):
  - witness_id: GlobalEntityId
  - target_accesses: pair<InstructionId, InstructionId>
  - evidence: Variant<AliasWitness, LivenessWitness, DependenceWitness>
  - inference_rules: vector<InferenceRuleKind> (e.g. NoOverlap, ImmutableRead, DisjointPointers)
  - conclusion: IndependenceProofStatus (verified | rejected | inconclusive)
  - internal_log: vector<InferenceStep> (machine-readable format for reproducibility)
- Validation rules (FormalProofChecker):
  - Soundness check: each InferenceStep must be valid with respect to the applied rules.
  - Witness completeness: evidence must cover all declared may-alias scenarios.
  - Determinism: the proof must be regenerable as bit-identical with the same input and analysis.
  - Standard format: serializable structure compliant with an independent verification schema.
