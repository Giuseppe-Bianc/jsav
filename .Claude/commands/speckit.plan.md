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

## Outline

1. **Setup**: Run `pwsh -ExecutionPolicy Bypass -File .specify/scripts/powershell/setup-plan.ps1 -Json` from repo root and parse JSON for FEATURE_SPEC, IMPL_PLAN, SPECS_DIR, BRANCH. For single quotes in args like "I'm Groot", use escape syntax: e.g 'I'\''m Groot' (or double-quote if possible: "I'm Groot").

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

The outline above establishes a multi-phase workflow with gate evaluations, research, and artifact generation. The following patterns and anti-patterns guide the reliable execution of this workflow end-to-end.

## Patterns for the Planning Workflow

### Pattern: Constitution-Gated Progression

- **Objective:** Ensure that constitution gate evaluations are performed thoroughly at each checkpoint — both before and after design — so that violations are caught and addressed rather than carried silently into downstream artifacts.
- **Context of application:** Apply during step 3 of the Outline when evaluating gates before Phase 0 and when re-evaluating the Constitution Check post-design after Phase 1.
- **Key characteristics:** Each gate evaluation explicitly lists every constitution rule, states whether the current plan complies or violates it, and provides justification for any deviation. The evaluation is not a pass-through formality — it is a blocking checkpoint. If a violation is identified and no justification is provided, the workflow halts with an ERROR. Post-design re-evaluation checks whether design decisions introduced new violations not present in the initial evaluation.
- **Operational guidance:**
  1. After filling the Technical Context and Constitution Check section, enumerate every rule from `constitution.md` and assess compliance individually.
  2. For each rule, write one of three verdicts: "Compliant," "Violation — justified because [reason]," or "Violation — unjustified." Do not leave any rule unassessed.
  3. If any rule is marked "Violation — unjustified," emit an ERROR and stop. Do not proceed to Phase 0.
  4. After Phase 1 is complete, re-run the same enumeration against the now-complete design artifacts. Check specifically whether entity definitions in data-model.md, interface contracts, or technology choices in research.md introduced new violations.
  5. If post-design evaluation reveals new violations, address them by modifying the offending artifact before reporting completion.

### Pattern: Sequential Phase Completion

- **Objective:** Complete every deliverable and validation within a phase before beginning work on the next phase, ensuring that later phases build on fully resolved foundations rather than tentative or incomplete outputs.
- **Context of application:** Apply at the boundary between Phase 0 and Phase 1, and at the boundary between Phase 1 and the final report.
- **Key characteristics:** Phase 0 is considered complete only when research.md exists and contains resolved decisions for every NEEDS CLARIFICATION item from the Technical Context. Phase 1 is considered complete only when all specified artifacts (data-model.md, contracts/ if applicable, quickstart.md) are generated and the agent context script has been executed. No artifact generation in Phase 1 begins while Phase 0 has unresolved items.
- **Operational guidance:**
  1. Before transitioning from Phase 0 to Phase 1, verify that research.md contains a "Decision," "Rationale," and "Alternatives considered" entry for every unknown extracted from the Technical Context.
  2. Search the Technical Context section for any remaining "NEEDS CLARIFICATION" markers. If any remain, Phase 0 is not complete — return to research and resolve them.
  3. Before transitioning from Phase 1 to the final report, verify that each specified output artifact exists and is non-empty.
  4. Run the agent context update script (`update-agent-context.ps1`) only after all Phase 1 artifacts are written, not during artifact generation.
  5. Proceed to the final report (step 4 of the Outline) only after post-design constitution re-evaluation is complete and passing.

## Anti-Patterns for the Planning Workflow

### Anti-Pattern: Rubber-Stamp Gating

- **Description:** The generator treats constitution gate evaluation as a formality — copying the constitution rules into the check section and marking all as "Compliant" without actually assessing the plan's alignment with each rule, or skipping the post-design re-evaluation entirely because the pre-design check passed.
- **Reasons to avoid:** Constitution rules exist to enforce project-wide constraints (technology choices, architectural boundaries, security requirements). A gate evaluation that does not genuinely assess compliance allows violations to propagate into design artifacts, where they are far more expensive to correct. This typically occurs when the generator treats the constitution check as a template section to fill rather than as an active validation step, or when time pressure encourages a "we already checked this" attitude toward re-evaluation.
- **Negative consequences:** Violations surface during implementation or review, forcing rework of data-model.md, contracts, or research.md after downstream agents have already consumed them. The constitution loses its authority as a governance mechanism — if gates never block, they serve no purpose. Downstream agents (speckit.tasks, speckit.implement) produce plans and code that violate project constraints, requiring costly corrections.
- **Correct alternative:** Apply the **Constitution-Gated Progression** pattern to perform genuine, rule-by-rule evaluation at both checkpoints, blocking on unjustified violations.

### Anti-Pattern: Phase Interleaving

- **Description:** The generator begins Phase 1 work (entity extraction, contract definition) before Phase 0 (research) is fully complete — for example, starting data-model.md while some Technical Context unknowns are still marked NEEDS CLARIFICATION, reasoning that the unresolved items "probably won't affect the data model."
- **Reasons to avoid:** Phase 1 artifacts depend on the decisions made in Phase 0. An entity definition created before a technology decision is resolved may use the wrong data types, assume the wrong storage paradigm, or model relationships that the chosen technology handles differently. This mistake typically occurs when the generator perceives Phase 0 and Phase 1 as independent workstreams rather than as sequential dependencies, or when a seemingly obvious research question tempts the generator to "start on what we can" while that question is being resolved.
- **Negative consequences:** Design artifacts embed assumptions that contradict later research findings. When the research resolves, the artifacts must be regenerated — but the generator may not revisit them, producing an internally inconsistent plan. The agent context update captures incomplete or contradictory information. Downstream agents receive a plan that appears complete but contains decisions based on pre-research assumptions.
- **Correct alternative:** Apply the **Sequential Phase Completion** pattern to treat Phase 0 completion as a hard prerequisite for any Phase 1 work.

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

**Output**: research.md with all NEEDS CLARIFICATION resolved

Phase 0 produces the research foundation that all subsequent design work depends on. The following patterns and anti-patterns guide the scoping and execution of research tasks.

#### Patterns for Research and Unknown Resolution

##### Pattern: Targeted Research Scoping

- **Objective:** Constrain each research task to the specific unknown it was derived from, producing focused findings that directly resolve Technical Context gaps without introducing tangential information or decision fatigue.
- **Context of application:** Apply during step 1 (Extract unknowns) and step 2 (Generate and dispatch research agents) of Phase 0 when defining research task scope.
- **Key characteristics:** Each research task maps one-to-one to a specific NEEDS CLARIFICATION marker, dependency, or integration point from the Technical Context. The task prompt includes the feature context to prevent generic results. Research findings are evaluated solely on whether they resolve the originating unknown — additional findings, however interesting, are excluded from research.md unless they directly address another identified unknown.
- **Operational guidance:**
  1. For each NEEDS CLARIFICATION marker in the Technical Context, create exactly one research task. The task description must reference both the specific unknown and the feature context (e.g., "Research OAuth2 token storage strategies for a mobile-first e-commerce application," not "Research OAuth2").
  2. For each dependency, scope the best-practices task to the intersection of that dependency and the feature's domain. Avoid broad tasks like "Find best practices for PostgreSQL" — instead, specify "Find best practices for PostgreSQL JSON column indexing in a product catalog context."
  3. When consolidating findings, include only the decision, rationale, and alternatives that address the originating unknown. If research surfaces a new unknown not previously identified, add it to the Technical Context as a new NEEDS CLARIFICATION marker and create a separate research task for it.
  4. After consolidation, verify that every original NEEDS CLARIFICATION marker has a corresponding "Decision" entry in research.md. If any marker lacks a decision, the research is incomplete.

##### Pattern: Decision-Ready Consolidation

- **Objective:** Structure research.md so that every entry provides a clear, actionable decision that Phase 1 can consume directly, rather than presenting raw findings that require further interpretation.
- **Context of application:** Apply during step 3 (Consolidate findings) of Phase 0 when writing research.md entries.
- **Key characteristics:** Each entry in research.md follows the three-field format (Decision, Rationale, Alternatives considered) without deviation. The Decision field contains a specific, implementable choice — not a recommendation to "consider" or "evaluate further." The Rationale field explains why this decision is correct for the specific feature context. The Alternatives field lists what was evaluated and why each was rejected, preventing downstream agents from reopening settled questions.
- **Operational guidance:**
  1. Write the Decision field as a declarative statement: "Use JWT with short-lived access tokens and refresh token rotation," not "Consider using JWT or session-based authentication."
  2. In the Rationale field, connect the decision to at least one specific requirement or constraint from the feature spec or constitution.
  3. In the Alternatives field, list at least two alternatives with one-sentence rejection reasons tied to the feature context (e.g., "Session-based auth rejected: requires sticky sessions, incompatible with the stateless API architecture specified in constitution.md").
  4. After writing all entries, re-read each Decision field in isolation. If a Phase 1 artifact author could not act on it without reading the Rationale, the Decision is too vague — add specificity.

#### Anti-Patterns for Research and Unknown Resolution

##### Anti-Pattern: Speculative Research Expansion

- **Description:** The generator broadens research tasks beyond the specific unknowns identified in the Technical Context, investigating tangentially related topics, exploring "nice-to-have" technologies, or researching areas that the feature spec does not require — for example, researching GraphQL subscriptions for a feature that only requires REST endpoints, or investigating caching strategies when no performance requirement exists.
- **Reasons to avoid:** Expanded research produces findings that have no corresponding NEEDS CLARIFICATION marker and no consumer in Phase 1. The additional entries in research.md create the impression that more decisions need to be reflected in design artifacts, leading Phase 1 to produce unnecessarily complex data models or contracts. This typically occurs when the generator conflates "thorough research" with "broad research," or when it anticipates future needs that the current feature spec does not establish.
- **Negative consequences:** research.md becomes bloated with decisions that no downstream artifact references. Phase 1 may incorporate speculative decisions into data-model.md or contracts, adding complexity that the feature spec never required. The planning workflow takes longer without producing proportional value. Downstream agents (speckit.tasks) generate tasks for functionality that originated in speculative research rather than in the feature specification.
- **Correct alternative:** Apply the **Targeted Research Scoping** pattern to limit each research task to a specific, identified unknown and exclude findings that do not resolve an existing gap.

##### Anti-Pattern: Inconclusive Research Entries

- **Description:** Research.md entries present findings without reaching a decision — for example, listing three database options with pros and cons but concluding with "further evaluation needed" or "depends on team preference," effectively passing the decision back to Phase 1 or to the user.
- **Reasons to avoid:** Phase 0's explicit purpose is to resolve all NEEDS CLARIFICATION markers. An inconclusive entry violates the phase completion criterion and forces Phase 1 to make decisions it is not equipped to make — entity extraction and contract definition require settled technology choices, not open questions. This typically occurs when the generator encounters a genuinely difficult tradeoff and avoids committing to a decision, or when it assumes a human reviewer will finalize the choice before Phase 1 begins.
- **Negative consequences:** Phase 1 proceeds with unresolved unknowns, producing artifacts that may be invalidated when the decision is eventually made. The Sequential Phase Completion pattern is violated in spirit even if the research.md file technically exists. Downstream agents inherit ambiguity and make independent, potentially conflicting choices. The planning workflow fails to deliver its core value — converting unknowns into decisions.
- **Correct alternative:** Apply the **Decision-Ready Consolidation** pattern to ensure every research.md entry contains a specific, implementable decision with clear rationale.

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
   - Run `.specify/scripts/powershell/update-agent-context.ps1 -AgentType claude`
   - These scripts detect which AI agent is in use
   - Update the appropriate agent-specific context file
   - Add only new technology from current plan
   - Preserve manual additions between markers

**Output**: data-model.md, /contracts/*, quickstart.md, agent-specific file

Phase 1 translates research decisions and feature requirements into concrete design artifacts. The following patterns and anti-patterns guide the generation of data models, contracts, and supporting files.

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

- Use absolute paths
- ERROR on gate failures or unresolved clarifications
