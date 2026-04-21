# Phase 1 Data Model - Sistema IR Multi-Livello Verificabile

## 1. Module
- Purpose: Contesto globale IR e boundary di validazione.
- Core fields:
  - module_id: GlobalEntityId
  - name: string
  - type_table: map<TypeName, TypeDefVersion>
  - function_table: map<FunctionName, FunctionSignature>
  - functions: vector<Function>
  - metadata: ModuleMetadata
- Validation rules:
  - Nomi globali univoci nel modulo.
  - type_table e function_table coerenti con definizioni concrete.

## 2. Function
- Purpose: Unita primaria di pass e dominio SSA.
- Core fields:
  - function_id: GlobalEntityId
  - signature: FunctionSignature
  - entry_block_id: BlockId
  - blocks: vector<BasicBlock>
  - cfg: ControlFlowGraph
  - ssa_index: SsaIndex
- Validation rules:
  - Un solo entry block.
  - Ogni block termina con control terminator.
  - Raggiungibilita/archi CFG validi.

## 3. BasicBlock
- Purpose: Nodo CFG con sequenza istruzioni.
- Core fields:
  - block_id: GlobalEntityId
  - instructions: vector<Instruction>
  - predecessors: vector<BlockId>
  - successors: vector<BlockId>
  - terminator: Instruction
- Validation rules:
  - terminator obbligatorio.
  - predecessori/successori consistenti bidirezionalmente.

## 4. Instruction
- Purpose: Operazione atomica calcolo/controllo/memoria.
- Core fields:
  - instruction_id: GlobalEntityId
  - opcode: OpCode
  - operands: vector<ValueRef>
  - result: optional<ValueDef>
  - memory_effect: MemoryEffectKind (none/read/write/readwrite)
  - type_constraints: ConstraintSet
- Validation rules:
  - Operandi compatibili con opcode.
  - Result type coerente con type_constraints.
  - Accessi memoria marcati e ordinabili per dipendenza.

## 5. Value
- Purpose: Entita SSA immutabile con definizione unica.
- Core fields:
  - value_id: GlobalEntityId
  - defining_instruction: InstructionId
  - value_type: TypeRef
  - version: SsaVersion
  - use_sites: vector<UseSite>
- Validation rules:
  - Una sola definizione per value_id.
  - Ogni uso dominato dalla definizione in MIR.

## 6. Type System
- Purpose: Modello tipi primitivi/composti/utente.
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
  - Equivalenza utente nominale (identita + versione), non strutturale.
  - Nessuna ritipizzazione implicita retroattiva.

## 7. PhiNode
- Purpose: Convergenza definizioni in SSA.
- Core fields:
  - phi_id: GlobalEntityId
  - target_value: ValueId
  - incoming: vector<PhiIncoming>  # (predecessor_block_id, value_id)
- Validation rules:
  - Un operando per predecessore corrente del blocco.
  - Ogni incoming value raggiunge il blocco dal predecessore associato.
  - Pruning eager su predecessori non raggiungibili.

## 8. PassTransaction
- Purpose: Isolare mutazioni e abilitare rollback totale.
- Core fields:
  - transaction_id: DeterministicPassTxnId
  - scope: FunctionId | ModuleId
  - working_copy: IrSnapshot
  - status: started | validated | committed | rolled_back
  - errors: vector<CompileError>
- Validation rules:
  - Commit ammesso solo con status=validated e errors vuoto.
  - On failure: status=rolled_back e nessun effetto osservabile persistente.

## 9. AnalysisReport
- Purpose: Output deterministico per dominanza/RD/liveness/dependence.
- Core fields:
  - report_kind: dominance | reaching_defs | liveness | dependence
  - items: vector<AnalysisItem>
  - sort_key: CanonicalStableKey
- Validation rules:
  - Ordinamento totale stabile: modulo/funzione/blocco/idx-istruzione/idx-operando.
  - Ripetibilita bit-identica con stesso input/pipeline/config.

## Relationships
- Module 1..N Function
- Function 1..N BasicBlock
- BasicBlock 1..N Instruction
- Instruction 0..1 ValueDef
- ValueDef 1..N UseSite
- BasicBlock 0..N PhiNode
- PassTransaction 1..1 WorkingCopy

## State Transitions
- PassTransaction: started -> validated -> committed
- PassTransaction failure path: started|validated -> rolled_back
- Type version: created(vN) -> superseded(vN+1) without mutating existing value bindings

## Edge/Corner Case Mapping
- Unreachable predecessor removal: update CFG + remove PHI incoming immediately.
- Block elimination rewrite-safe: rewrite all uses before structural delete.
- May-alias reorder proposal: reject unless formal proof artifact is valid and reproducible.
- Non-reducible CFG: SSA validation still enforced by RD and dominance checks.
