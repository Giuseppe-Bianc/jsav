---
description: Execute the implementation planning workflow using the plan template to generate design artifacts.
handoffs: 
  - label: Create Tasks
    agent: speckit.tasks
    prompt: Break the plan into tasks
    send: true
  - label: Create Checklist
    agent: speckit.checklist
    prompt: Create a checklist for the following domain...
---

## User Input

```text
$ARGUMENTS
```

You **MUST** consider the user input before proceeding (if not empty).

## Pre-Execution Checks

**Check for extension hooks (before planning)**:
- Check if `.specify/extensions.yml` exists in the project root.
- If it exists, read it and look for entries under the `hooks.before_plan` key
- If the YAML cannot be parsed or is invalid, skip hook checking silently and continue normally
- Filter out hooks where `enabled` is explicitly `false`. Treat hooks without an `enabled` field as enabled by default.
- For each remaining hook, do **not** attempt to interpret or evaluate hook `condition` expressions:
  - If the hook has no `condition` field, or it is null/empty, treat the hook as executable
  - If the hook defines a non-empty `condition`, skip the hook and leave condition evaluation to the HookExecutor implementation
- For each executable hook, output the following based on its `optional` flag:
  - **Optional hook** (`optional: true`):
    ```
    ## Extension Hooks

    **Optional Pre-Hook**: {extension}
    Command: `/{command}`
    Description: {description}

    Prompt: {prompt}
    To execute: `/{command}`
    ```
  - **Mandatory hook** (`optional: false`):
    ```
    ## Extension Hooks

    **Automatic Pre-Hook**: {extension}
    Executing: `/{command}`
    EXECUTE_COMMAND: {command}

    Wait for the result of the hook command before proceeding to the Outline.
    ```
- If no hooks are registered or `.specify/extensions.yml` does not exist, skip silently

## Outline

1. **Setup**: Run `.specify/scripts/powershell/setup-plan.ps1 -Json` from repo root and parse JSON for FEATURE_SPEC, IMPL_PLAN, SPECS_DIR, BRANCH. For single quotes in args like "I'm Groot", use escape syntax: e.g 'I'\''m Groot' (or double-quote if possible: "I'm Groot").

2. **Load context**: Read FEATURE_SPEC and `.specify/memory/constitution.md`. Load IMPL_PLAN template (already copied).

3. **Execute plan workflow**: Follow the structure in IMPL_PLAN template to:
   - Fill Technical Context (mark unknowns as "NEEDS CLARIFICATION")
   - Fill Constitution Check section from constitution
   - Evaluate gates (ERROR if violations unjustified)
   - Phase 0: Generate research.md (resolve all NEEDS CLARIFICATION)
   - Phase 1: Generate data-model.md, contracts/, quickstart.md
   - Phase 1: Update agent context by running the agent script
   - Re-evaluate Constitution Check post-design

4. **Stop and report**: Command ends after Phase 2 planning. Report branch, IMPL_PLAN path, and generated artifacts.

5. **Check for extension hooks**: After reporting, check if `.specify/extensions.yml` exists in the project root.
   - If it exists, read it and look for entries under the `hooks.after_plan` key
   - If the YAML cannot be parsed or is invalid, skip hook checking silently and continue normally
   - Filter out hooks where `enabled` is explicitly `false`. Treat hooks without an `enabled` field as enabled by default.
   - For each remaining hook, do **not** attempt to interpret or evaluate hook `condition` expressions:
     - If the hook has no `condition` field, or it is null/empty, treat the hook as executable
     - If the hook defines a non-empty `condition`, skip the hook and leave condition evaluation to the HookExecutor implementation
   - For each executable hook, output the following based on its `optional` flag:
     - **Optional hook** (`optional: true`):
       ```
       ## Extension Hooks

       **Optional Hook**: {extension}
       Command: `/{command}`
       Description: {description}

       Prompt: {prompt}
       To execute: `/{command}`
       ```
     - **Mandatory hook** (`optional: false`):
       ```
       ## Extension Hooks

       **Automatic Hook**: {extension}
       Executing: `/{command}`
       EXECUTE_COMMAND: {command}
       ```
   - If no hooks are registered or `.specify/extensions.yml` does not exist, skip silently

## Phases

### Phase 0: Outline & Research

1. **Extract unknowns from Technical Context** above:
   - For each NEEDS CLARIFICATION → research task
   - For each dependency → best practices task
   - For each integration → patterns task

2. **Generate and dispatch research agents**:

   ```text
   For each unknown in Technical Context:
     Task: "Research {unknown} for {feature context}"
   For each technology choice:
     Task: "Find best practices for {tech} in {domain}"
   ```

3. **Consolidate findings** in `research.md` using format:
   - Decision: [what was chosen]
   - Rationale: [why chosen]
   - Alternatives considered: [what else evaluated]
   - Notes: [any additional context or references]

**Output**: research.md with all NEEDS CLARIFICATION resolved

### Phase 1: Design & Contracts

**Prerequisites:** `research.md` complete

1. **Extract entities from feature spec** → `data-model.md`:
   - Entity name, fields, relationships
   - Validation rules from requirements
   - State transitions if applicable

2. **Define interface contracts** (if project has external interfaces) → `/contracts/`:
   - Identify what interfaces the project exposes to users or other systems
   - Document the contract format appropriate for the project type
   - Examples: public APIs for libraries, command schemas for CLI tools, endpoints for web services, grammars for parsers, UI contracts for applications
   - Skip if project is purely internal (build scripts, one-off tools, etc.)

3. **Agent context update**:
   - Update the plan reference between the `<!-- SPECKIT START -->` and `<!-- SPECKIT END -->` markers in `.github/copilot-instructions.md` to point to the plan file created in step 1 (the IMPL_PLAN path)

**Output**: data-model.md, /contracts/*, quickstart.md, updated agent context file

#### Patterns for Design Artifact Generation

##### Pattern: Spec-Traced Artifact Derivation

- **Objective:** Ensure that every entity in data-model.md, every contract in contracts/, and every scenario in quickstart.md traces directly to a requirement, user story, or constraint in the feature specification, preventing design artifacts from containing speculative or ungrounded content.
- **Context of application:** Apply during steps 1, 2, and 3 of Phase 1 when generating each artifact from the feature spec and resolved research decisions.
- **Key characteristics:** Each entity, field, relationship, and contract element can be justified by pointing to a specific passage in the feature spec. If an element cannot be traced, it is either derived from a research.md decision (which itself traces to a Technical Context unknown) or it does not belong in the artifact. The generator treats the feature spec as the authoritative source and research.md as the supplementary source — no third source of requirements is introduced.
- **Operational guidance:**
  1. Before adding an entity to data-model.md, identify the user story or functional requirement in the feature spec that necessitates it. Record this traceability as a comment or annotation in the data model.
  2. For each field on an entity, verify that it corresponds to a data element mentioned or implied by the spec. If the spec describes "users can set their display name and email," the User entity gets `display_name` and `email` fields — not `phone_number` unless the spec mentions it.
  3. For each interface contract, identify the user-facing interaction or system integration in the spec that requires it. If no spec requirement calls for an external interface, do not create a contract.
  4. For quickstart.md scenarios, derive each scenario from the acceptance criteria or user scenarios in the spec. Each scenario should exercise a specific requirement, not a hypothetical use case.
  5. After generating all artifacts, perform a reverse check: scan the feature spec for requirements that have no corresponding artifact element. If a requirement is unrepresented, add the missing element or document why it is deferred.

##### Pattern: Project-Appropriate Contract Selection

- **Objective:** Match the format, scope, and existence of interface contracts to the actual project type, generating contracts only when the project exposes external interfaces and using the format that matches how those interfaces are consumed.
- **Context of application:** Apply during step 2 of Phase 1 when deciding whether to create contracts and what format they should take.
- **Key characteristics:** The generator evaluates the project type (library, CLI tool, web service, desktop application, internal script) before generating any contract. The decision to create or skip contracts is deliberate, not defaulted. When contracts are created, their format reflects the consumption model — OpenAPI for REST services, type signatures for libraries, command schemas for CLI tools, component contracts for UI applications. The contract format is chosen based on how consumers will interact with the interface, not on a universal template.
- **Operational guidance:**
  1. Determine the project type from plan.md's Technical Context and the feature spec. Classify it as one of: library, CLI tool, web service, desktop/mobile application, internal tool, or hybrid.
  2. If the project is purely internal (build scripts, one-off data migrations, internal automation), skip contract generation entirely and document the skip reason in the plan.
  3. For libraries: generate contracts as type signatures, public API surface documentation, or interface definitions in the language's idiomatic format.
  4. For CLI tools: generate contracts as command schemas documenting arguments, flags, input formats, and output formats.
  5. For web services: generate contracts as endpoint definitions with request/response schemas, status codes, and error formats.
  6. For UI applications: generate contracts as component interface definitions documenting props, events, and state requirements.
  7. Do not mix contract formats within a single project unless the project genuinely exposes multiple interface types (e.g., a web service with both a REST API and a CLI admin tool).

#### Anti-Patterns for Design Artifact Generation

##### Anti-Pattern: Assumption-Driven Modeling

- **Description:** The generator adds entities, fields, relationships, or contract endpoints to design artifacts based on what "most applications like this would need" rather than on what the feature spec actually requires — for example, adding a `Roles` entity and role-based access control fields to a data model when the spec describes a single-user application, or adding CRUD endpoints for every entity when the spec only requires read access.
- **Reasons to avoid:** Speculative modeling inflates the design surface area, creating implementation work for functionality that was never specified. It contradicts the feature spec's authority as the single source of requirements. This typically occurs when the generator draws on general domain knowledge ("most web apps need roles and permissions") rather than reading the specific spec, or when it confuses completeness with comprehensiveness — a complete data model covers every spec requirement, not every possible requirement.
- **Negative consequences:** Downstream agents (speckit.tasks) generate implementation tasks for entities and endpoints that no user story requires, inflating the task count and extending timelines. The implementation includes functionality that was never validated by stakeholders, creating maintenance burden. The data model and contracts diverge from the spec, making consistency analysis (speckit.analyze) produce false positives or miss genuine gaps. Reviewers must distinguish between spec-required and assumption-added elements, increasing review effort.
- **Correct alternative:** Apply the **Spec-Traced Artifact Derivation** pattern to ensure every artifact element traces to a specific requirement in the feature spec or a resolved decision in research.md.

##### Anti-Pattern: Universal Contract Generation

- **Description:** The generator creates interface contracts for every project regardless of type — producing REST API endpoint definitions for a command-line utility, OpenAPI specifications for an internal build script, or formal contract documents for a project whose only "interface" is a function call within a larger codebase.
- **Reasons to avoid:** Interface contracts exist to document boundaries between a system and its external consumers. When a project has no external consumers — or when its interface model does not match the contract format being generated — the contracts are at best unused and at worst misleading. This mistake occurs when the generator treats contract generation as a mandatory step rather than a conditional one, or when it defaults to a web-service contract format without assessing the project type. The Phase 1 instructions explicitly state to "skip if project is purely internal."
- **Negative consequences:** The contracts/ directory contains documents that no consumer will reference, creating a maintenance burden with no corresponding value. Downstream agents generate tasks to implement interfaces described in the contracts, producing code (e.g., REST endpoints) that the project does not need. The mismatch between contract format and actual interface type confuses implementers who expect the contract to match their consumption model. The project accumulates artifacts that signal a different architecture than what is actually being built.
- **Correct alternative:** Apply the **Project-Appropriate Contract Selection** pattern to evaluate the project type before generating contracts, skip generation for internal projects, and use the format that matches the actual interface consumption model.

## Key rules

- Use absolute paths for filesystem operations; use project-relative paths for references in documentation and agent context files
- ERROR on gate failures or unresolved clarifications
