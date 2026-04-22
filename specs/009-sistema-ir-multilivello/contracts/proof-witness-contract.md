# Contract: Proof Witness Format for Memory Independence

**Feature**: `009-sistema-ir-multilivello`
**Artifact Status**: Finalized
**Reference Requirement**: `FR-030`

## 1. Scope

This document defines the canonical JSON schema and data structures for formal proofs of memory independence (ProofWitness) required to bypass the `strict no-reorder` policy on `may-alias` accesses.

## 2. JSON Schema definition

The ProofWitness MUST follow this JSON schema for both external certificates and internal logs:

```json
{
  "$schema": "http://json-schema.org/draft-07/schema#",
  "title": "ProofWitness",
  "type": "object",
  "required": ["version", "type", "metadata", "assertions"],
  "properties": {
    "version": { "type": "string", "const": "1.0.0" },
    "type": { "type": "string", "enum": ["external_certificate", "internal_log"] },
    "metadata": {
      "type": "object",
      "required": ["module_id", "function_id", "timestamp"],
      "properties": {
        "module_id": { "type": "string" },
        "function_id": { "type": "string" },
        "timestamp": { "type": "string", "format": "date-time" }
      }
    },
    "assertions": {
      "type": "array",
      "items": {
        "type": "object",
        "required": ["instr_a", "instr_b", "inference"],
        "properties": {
          "instr_a": { "type": "string", "description": "GlobalID of the first memory instruction" },
          "instr_b": { "type": "string", "description": "GlobalID of the second memory instruction" },
          "inference": {
            "type": "object",
            "required": ["rule", "witness"],
            "properties": {
              "rule": { 
                "type": "string", 
                "enum": ["AliasDisjoint", "LivenessDisjoint", "DominationSeparated", "StaticBoundIndependent"] 
              },
              "witness": {
                "type": "object",
                "description": "Specific evidence data based on the rule (e.g., alias sets, live intervals)"
              }
            }
          }
        }
      }
    }
  }
}
```

## 3. Inference Rules and Witness Data

| Rule | Description | Witness Data Requirements |
|------|-------------|---------------------------|
| `AliasDisjoint` | Base pointers belong to disjoint alias sets. | `set_a_id`, `set_b_id`, `disjoint_proof_hash` |
| `LivenessDisjoint` | Instruction A's result is dead before B, or vice-versa. | `live_range_a`, `live_range_b`, `point_of_death` |
| `DominationSeparated` | Control flow guarantees A and B never execute in the same trace. | `dom_tree_path`, `post_dom_tree_path`, `exit_node` |
| `StaticBoundIndependent` | Accesses use different static offsets from the same base. | `base_id`, `offset_a`, `offset_b`, `size_a`, `size_b` |

## 4. Determinism Requirements

- The GlobalIDs MUST match the canonical structural paths defined in `FR-019`.
- The order of assertions in the array MUST be sorted by the hierarchical canonical key (Modulo/Funzione/Blocco/Indice-Istruzione) of `instr_a`, then `instr_b`.
- The JSON output MUST be pretty-printed with a fixed 2-space indentation to ensure bit-identical results across platforms.
