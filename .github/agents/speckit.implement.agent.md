---
description: Execute the implementation plan by processing and executing all tasks defined in tasks.md
---

## User Input

```text
$ARGUMENTS
```

You **MUST** consider the user input before proceeding (if not empty).

## Pre-Execution Checks

**Check for extension hooks (before implementation)**:

- Check if `.specify/extensions.yml` exists in the project root.
- If it exists, read it and look for entries under the `hooks.before_implement` key
- If the YAML cannot be parsed or is invalid, skip hook checking silently and continue normally
- Filter out hooks where `enabled` is explicitly `false`. Treat hooks without an `enabled` field as enabled by default.
- For each remaining hook, do **not** attempt to interpret or evaluate hook `condition` expressions:
  - If the hook has no `condition` field, or it is null/empty, treat the hook as executable
  - If the hook defines a non-empty `condition`, skip the hook and leave condition evaluation to the HookExecutor implementation
- For each executable hook, output the following based on its `optional` flag:
  - **Optional hook** (`optional: true`):

    ```text
    ## Extension Hooks

    **Optional Pre-Hook**: {extension}
    Command: `/{command}`
    Description: {description}

    Prompt: {prompt}
    To execute: `/{command}`
    ```

  - **Mandatory hook** (`optional: false`):

    ```text
    ## Extension Hooks

    **Automatic Pre-Hook**: {extension}
    Executing: `/{command}`
    EXECUTE_COMMAND: {command}
    
    Wait for the result of the hook command before proceeding to the Outline.
    ```

- If no hooks are registered or `.specify/extensions.yml` does not exist, skip silently

## Outline

1. Run `pwsh -ExecutionPolicy Bypass -File .specify/scripts/powershell/check-prerequisites.ps1 -Json -RequireTasks -IncludeTasks` from repo root and parse FEATURE_DIR and AVAILABLE_DOCS list. All paths must be absolute. For single quotes in args like "I'm Groot", use escape syntax: e.g 'I'\''m Groot' (or double-quote if possible: "I'm Groot").

2. **Check checklists status** (if FEATURE_DIR/checklists/ exists):
   - Scan all checklist files in the checklists/ directory
   - For each checklist, count:
     - Total items: All lines matching `- [ ]` or `- [X]` or `- [x]`
     - Completed items: Lines matching `- [X]` or `- [x]`
     - Incomplete items: Lines matching `- [ ]`
   - Create a status table:

     ```text
     | Checklist | Total | Completed | Incomplete | Status |
     |-----------|-------|-----------|------------|--------|
     | ux.md     | 12    | 12        | 0          | ✓ PASS |
     | test.md   | 8     | 5         | 3          | ✗ FAIL |
     | security.md | 6   | 6         | 0          | ✓ PASS |
     ```

   - Calculate overall status:
     - **PASS**: All checklists have 0 incomplete items
     - **FAIL**: One or more checklists have incomplete items

   - **If any checklist is incomplete**:
     - Display the table with incomplete item counts
     - **STOP** and ask: "Some checklists are incomplete. Do you want to proceed with implementation anyway? (yes/no)"
     - Wait for user response before continuing
     - If user says "no" or "wait" or "stop", halt execution
     - If user says "yes" or "proceed" or "continue", proceed to step 3

   - **If all checklists are complete**:
     - Display the table showing all checklists passed
     - Automatically proceed to step 3

3. Load and analyze the implementation context:
   - **REQUIRED**: Read tasks.md for the complete task list and execution plan
   - **REQUIRED**: Read plan.md for tech stack, architecture, and file structure
   - **IF EXISTS**: Read data-model.md for entities and relationships
   - **IF EXISTS**: Read contracts/ for API specifications and test requirements
   - **IF EXISTS**: Read research.md for technical decisions and constraints
   - **IF EXISTS**: Read quickstart.md for integration scenarios

4. **Project Setup Verification**:
   - **REQUIRED**: Create/verify ignore files based on actual project setup:

   **Detection & Creation Logic**:
   - Check if the following command succeeds to determine if the repository is a git repo (create/verify .gitignore if so):

     ```sh
     git rev-parse --git-dir 2>/dev/null
     ```

   - Check if Dockerfile* exists or Docker in plan.md → create/verify .dockerignore
   - Check if .eslintrc* exists → create/verify .eslintignore
   - Check if eslint.config.* exists → ensure the config's `ignores` entries cover required patterns
   - Check if .prettierrc* exists → create/verify .prettierignore
   - Check if .npmrc or package.json exists → create/verify .npmignore (if publishing)
   - Check if terraform files (*.tf) exist → create/verify .terraformignore
   - Check if .helmignore needed (helm charts present) → create/verify .helmignore

   **If ignore file already exists**: Verify it contains essential patterns, append missing critical patterns only
   **If ignore file missing**: Create with full pattern set for detected technology

   **Common Patterns by Technology** (from plan.md tech stack):
   - **Node.js/JavaScript/TypeScript**: `node_modules/`, `dist/`, `build/`, `*.log`, `.env*`
   - **Python**: `__pycache__/`, `*.pyc`, `.venv/`, `venv/`, `dist/`, `*.egg-info/`
   - **Java**: `target/`, `*.class`, `*.jar`, `.gradle/`, `build/`
   - **C#/.NET**: `bin/`, `obj/`, `*.user`, `*.suo`, `packages/`
   - **Go**: `*.exe`, `*.test`, `vendor/`, `*.out`
   - **Ruby**: `.bundle/`, `log/`, `tmp/`, `*.gem`, `vendor/bundle/`
   - **PHP**: `vendor/`, `*.log`, `*.cache`, `*.env`
   - **Rust**: `target/`, `debug/`, `release/`, `*.rs.bk`, `*.rlib`, `*.prof*`, `.idea/`, `*.log`, `.env*`
   - **Kotlin**: `build/`, `out/`, `.gradle/`, `.idea/`, `*.class`, `*.jar`, `*.iml`, `*.log`, `.env*`
   - **C++**: `build/`, `bin/`, `obj/`, `out/`, `*.o`, `*.so`, `*.a`, `*.exe`, `*.dll`, `.idea/`, `*.log`, `.env*`
   - **C**: `build/`, `bin/`, `obj/`, `out/`, `*.o`, `*.a`, `*.so`, `*.exe`, `*.dll`, `autom4te.cache/`, `config.status`, `config.log`, `.idea/`, `*.log`, `.env*`
   - **Swift**: `.build/`, `DerivedData/`, `*.swiftpm/`, `Packages/`
   - **R**: `.Rproj.user/`, `.Rhistory`, `.RData`, `.Ruserdata`, `*.Rproj`, `packrat/`, `renv/`
   - **Universal**: `.DS_Store`, `Thumbs.db`, `*.tmp`, `*.swp`, `.vscode/`, `.idea/`

   **Tool-Specific Patterns**:
   - **Docker**: `node_modules/`, `.git/`, `Dockerfile*`, `.dockerignore`, `*.log*`, `.env*`, `coverage/`
   - **ESLint**: `node_modules/`, `dist/`, `build/`, `coverage/`, `*.min.js`
   - **Prettier**: `node_modules/`, `dist/`, `build/`, `coverage/`, `package-lock.json`, `yarn.lock`, `pnpm-lock.yaml`
   - **Terraform**: `.terraform/`, `*.tfstate*`, `*.tfvars`, `.terraform.lock.hcl`
   - **Kubernetes/k8s**: `*.secret.yaml`, `secrets/`, `.kube/`, `kubeconfig*`, `*.key`, `*.crt`

Steps 1 through 4 establish the pre-implementation foundation — validating prerequisites, enforcing checklist gates, loading design context, and preparing the project environment. The following patterns and anti-patterns guide the reliable execution of these preparatory steps.

### Patterns for Pre-Implementation Setup

#### Pattern: Explicit Checklist Gate Enforcement

- **Objective:** Ensure that incomplete checklists are surfaced to the user as a blocking concern and that implementation proceeds only with conscious, documented user consent — never silently.
- **Context of application:** Apply during step 2 of the Outline when evaluating checklist status and determining whether to proceed.
- **Key characteristics:** The executor treats the checklist gate as an interactive checkpoint, not a passive report. When any checklist has incomplete items, the executor displays the full status table, halts execution, and waits for an explicit user response. The executor does not infer intent from silence, from prior conversation context, or from the user's initial $ARGUMENTS. The gate distinguishes between "all pass" (automatic proceed) and "any fail" (require explicit consent) with no intermediate behavior.
- **Operational guidance:**
  1. After scanning all checklist files, build the status table with exact counts. Do not round, estimate, or summarize — every checklist file must appear as its own row.
  2. If any checklist has incomplete items, display the table and the stop prompt exactly as specified. Do not append suggestions, do not recommend proceeding, and do not proceed without a response.
  3. Accept only unambiguous affirmative responses ("yes," "proceed," "continue") as consent to continue. Treat ambiguous responses ("maybe," "I think so," "let me check") as non-affirmative — ask again.
  4. If the user consents to proceed despite incomplete checklists, note this consent in the final completion report (step 9) so that reviewers know the gate was bypassed.
  5. If all checklists pass, display the passing table and proceed without prompting. Do not ask "Would you like to continue?" when no gate condition is triggered.

#### Pattern: Additive Ignore File Management

- **Objective:** Preserve all existing patterns in ignore files while ensuring that technology-appropriate critical patterns are present, preventing both destructive overwrites and incomplete coverage.
- **Context of application:** Apply during step 4 (Project Setup Verification) when creating or updating ignore files (.gitignore, .dockerignore, .eslintignore, etc.).
- **Key characteristics:** The executor distinguishes between two scenarios — file exists and file missing — and handles them differently. For existing files, it reads the current content, identifies which critical patterns from the technology-specific list are missing, and appends only those missing patterns in a clearly marked block. For missing files, it creates the file with the full pattern set. It never replaces, reorders, or removes existing patterns.
- **Operational guidance:**
  1. For each ignore file to be managed, check whether the file exists before taking any action.
  2. If the file exists, read its full content. For each critical pattern in the technology-specific list, check whether the pattern (or a functionally equivalent glob) is already present. Only append patterns that are genuinely missing.
  3. When appending, add a comment marker (e.g., `# Added by speckit.implement`) followed by the missing patterns. This makes the additions identifiable and reversible.
  4. If the file does not exist, create it with the full technology-appropriate pattern set, including a header comment noting it was generated.
  5. Do not modify ignore files for technologies not detected in the project. If no Dockerfile exists and Docker is not in plan.md, do not create or modify .dockerignore.

### Anti-Patterns for Pre-Implementation Setup

#### Anti-Pattern: Silent Checklist Bypass

- **Description:** The executor detects incomplete checklists but proceeds with implementation without displaying the status table, without halting, or without waiting for user confirmation — typically because it interprets the user's initial $ARGUMENTS (e.g., "implement everything") as implicit consent to skip the gate.
- **Reasons to avoid:** Checklists exist as quality gates created by earlier workflow stages (speckit.checklist). Bypassing them silently defeats their purpose and means that the implementation may violate constraints that the checklists were designed to enforce (UX standards, security requirements, test coverage thresholds). This mistake typically occurs when the executor treats $ARGUMENTS as overriding all interactive checkpoints, or when it optimizes for speed by skipping what it perceives as a "formality."
- **Negative consequences:** Implementation proceeds against incomplete quality criteria. Reviewers and stakeholders discover post-implementation that gates were bypassed, requiring rework or audit. The user loses the opportunity to address checklist items before they become implementation defects. Trust in the checklist workflow erodes because the implementation agent ignores its outputs.
- **Correct alternative:** Apply the **Explicit Checklist Gate Enforcement** pattern to always display the status table and halt on incomplete checklists, proceeding only with explicit user consent.

#### Anti-Pattern: Destructive Ignore File Overwrite

- **Description:** The executor replaces existing ignore files with newly generated content, discarding patterns that were manually added by developers, inherited from project templates, or produced by other tools.
- **Reasons to avoid:** Ignore files accumulate project-specific patterns over time — patterns for local tooling, IDE configurations, generated artifacts specific to the project's build system, or temporary exclusions for ongoing migrations. These patterns cannot be inferred from the tech stack alone. Overwriting them forces developers to manually recover lost patterns from version control, and patterns added after the overwrite may be lost again on subsequent runs. This mistake occurs when the executor treats "create/verify" as "create/replace" or when it does not read the existing file before writing.
- **Negative consequences:** Previously ignored files (secrets, local configuration, build artifacts) are no longer ignored and may be committed to version control. Developers must spend time restoring lost patterns and may not notice the loss until sensitive files are exposed. Repeated overwrites train developers to distrust the implementation agent and manually revert its changes to ignore files, undermining automation.
- **Correct alternative:** Apply the **Additive Ignore File Management** pattern to read existing content, identify only missing critical patterns, and append them without disturbing existing entries.

5. Parse tasks.md structure and extract:
   - **Task phases**: Setup, Tests, Core, Integration, Polish
   - **Task dependencies**: Sequential vs parallel execution rules
   - **Task details**: ID, description, file paths, parallel markers [P]
   - **Execution flow**: Order and dependency requirements

6. Execute implementation following the task plan:
   - **Phase-by-phase execution**: Complete each phase before moving to the next
   - **Respect dependencies**: Run sequential tasks in order, parallel tasks [P] can run together  
   - **Follow TDD approach**: Execute test tasks before their corresponding implementation tasks
   - **File-based coordination**: Tasks affecting the same files must run sequentially
   - **Validation checkpoints**: Verify each phase completion before proceeding

7. Implementation execution rules:
   - **Setup first**: Initialize project structure, dependencies, configuration
   - **Tests before code**: If you need to write tests for contracts, entities, and integration scenarios
   - **Core development**: Implement models, services, CLI commands, endpoints
   - **Integration work**: Database connections, middleware, logging, external services
   - **Polish and validation**: Unit tests, performance optimization, documentation

Steps 5 through 7 define how tasks are parsed and executed. The core challenge is maintaining correct execution order while maximizing throughput through parallelism. The following patterns and anti-patterns address task execution discipline.

### Patterns for Task Execution

#### Pattern: Artifact-Informed Implementation

- **Objective:** Ensure that every implementation task is executed with full awareness of the design artifacts loaded in step 3, producing code that conforms to the data model, respects interface contracts, and reflects research decisions — not just the task description in tasks.md.
- **Context of application:** Apply during steps 6 and 7 when executing each individual task. The tasks.md entry provides the action and target file; the design artifacts provide the specification for what that file should contain.
- **Key characteristics:** The executor treats tasks.md as the execution plan (what to do and in what order) and the design artifacts as the specification (what the output should look like). When implementing a model, it consults data-model.md for entity definitions, field types, and relationships. When implementing an endpoint, it consults contracts/ for request/response schemas. When making technology choices within a task, it consults research.md for resolved decisions. The task description alone is never sufficient — it provides direction, not specification.
- **Operational guidance:**
  1. Before executing each task, identify which design artifacts are relevant. A task targeting `src/models/user.py` should reference data-model.md for the User entity definition. A task targeting `src/routes/auth.py` should reference the relevant contract in contracts/.
  2. If data-model.md defines an entity with specific fields, validation rules, or relationships, the implementation must include all of them — not just the fields mentioned in the task description.
  3. If contracts/ defines a request/response schema for an endpoint, the implementation must conform to that schema exactly — matching field names, types, status codes, and error formats.
  4. If research.md documents a technology decision (e.g., "Use bcrypt for password hashing"), the implementation must follow that decision even if the task description does not mention it.
  5. If a design artifact is absent (data-model.md does not exist), implement based on the task description and plan.md. Do not halt — but note in the progress report that the implementation was derived from task descriptions rather than formal design artifacts.

#### Pattern: Dependency-Aware Parallel Execution

- **Objective:** Execute tasks marked with [P] concurrently only when genuine independence has been verified, while maintaining strict sequential order for unmarked tasks and tasks sharing file targets.
- **Context of application:** Apply during step 6 when determining execution order for tasks within each phase.
- **Key characteristics:** The executor respects two ordering constraints simultaneously: the explicit dependency order encoded in task IDs and the [P] marker, and the implicit file-based dependency that tasks writing to the same file cannot run concurrently. A task without [P] is always sequential — it waits for the previous task to complete. A task with [P] may run alongside other [P] tasks in the same phase, but only if they target different files. The executor never introduces parallelism beyond what tasks.md specifies.
- **Operational guidance:**
  1. Within each phase, identify all tasks marked [P] and group them into a candidate parallel set.
  2. Within the parallel set, check for file path overlap. If two [P] tasks target the same file (e.g., both modify `src/app.py`), execute them sequentially in task ID order despite both having [P] markers.
  3. Execute all sequential (non-[P]) tasks before any [P] tasks in the same phase, unless the [P] tasks appear earlier in the task ID sequence — in which case, follow the ID order.
  4. When a phase contains a mix of sequential and parallel tasks, process them in task ID order: execute sequential tasks one at a time, and when a contiguous block of [P] tasks is reached, execute that block concurrently (subject to file-overlap checks).
  5. After all tasks in a phase complete (or fail), verify phase completion before proceeding to the next phase. Do not start the next phase while any task in the current phase is still in progress.

#### Pattern: Incremental Checkpoint Persistence

- **Objective:** Mark each task as complete in tasks.md (`[X]`) immediately after it is successfully executed, maintaining an accurate, real-time record of implementation progress that survives interruptions.
- **Context of application:** Apply during step 8 (Progress tracking) after each individual task completes successfully.
- **Key characteristics:** The executor updates tasks.md after every single task completion, not after each phase or at the end of the implementation run. This means that if execution is interrupted at any point, tasks.md accurately reflects which tasks are done and which remain. The update is atomic — the task's `- [ ]` is changed to `- [X]` and the file is saved before the next task begins.
- **Operational guidance:**
  1. After a task executes successfully, immediately open tasks.md, locate the task by its ID (e.g., T005), and change `- [ ]` to `- [X]`.
  2. Save the file before beginning the next task. Do not batch updates.
  3. For parallel tasks that complete simultaneously, update tasks.md with all completed task checkboxes before starting any new tasks.
  4. If a task fails, leave its checkbox as `- [ ]`. Do not mark failed tasks.
  5. If implementation is resumed after an interruption, read tasks.md to determine which tasks are already marked `[X]` and skip them, starting from the first unmarked task.

### Anti-Patterns for Task Execution

#### Anti-Pattern: Context-Blind Implementation

- **Description:** The executor implements each task using only the description in tasks.md without consulting the design artifacts loaded in step 3 — for example, creating a User model with fields guessed from the task description ("Create User model in src/models/user.py") rather than reading the User entity definition from data-model.md.
- **Reasons to avoid:** Task descriptions in tasks.md are intentionally concise — they specify what to do and where, not the full specification of what the output should contain. The design artifacts (data-model.md, contracts/, research.md) exist precisely to provide that specification. Ignoring them produces implementations that may be internally consistent but diverge from the agreed design. This typically occurs when the executor treats tasks.md as a self-contained specification rather than as an execution plan that references other documents.
- **Negative consequences:** Models are created with incomplete or incorrect fields. Endpoints do not conform to defined contracts. Technology decisions documented in research.md are ignored, producing implementations that violate resolved architectural choices. Integration between components fails because each task was implemented in isolation without reference to shared design constraints. The consistency analysis agent (speckit.analyze) later flags numerous mismatches between design and implementation.
- **Correct alternative:** Apply the **Artifact-Informed Implementation** pattern to consult the relevant design artifact for every task, using the artifact as the specification and the task description as the directive.

#### Anti-Pattern: False Parallelism

- **Description:** The executor treats all tasks within a phase as parallelizable, ignoring the distinction between [P]-marked and unmarked tasks, or executes [P] tasks concurrently even when they target the same file.
- **Reasons to avoid:** The [P] marker in tasks.md represents a deliberate analysis by the task generation agent that a specific task is safe to execute concurrently. Unmarked tasks were left sequential because they depend on the output of preceding tasks or modify shared files. Ignoring this distinction introduces race conditions — two tasks writing to the same file produce unpredictable results, and a task that imports from a model created by a preceding task will fail if both run simultaneously. This typically occurs when the executor optimizes for speed without understanding the dependency semantics encoded in the task list.
- **Negative consequences:** File write conflicts produce corrupted or incomplete source files. Tasks that depend on the output of earlier tasks fail with import errors or missing references. The executor must backtrack, undo parallel results, and re-execute sequentially — taking longer than a correct sequential execution would have. Failed parallel tasks may be reported as infrastructure errors rather than as dependency violations, misdirecting debugging effort.
- **Correct alternative:** Apply the **Dependency-Aware Parallel Execution** pattern to execute only [P]-marked tasks concurrently, verify file-target independence within the parallel set, and maintain strict sequential order for all unmarked tasks.

#### Anti-Pattern: Deferred Task Marking

- **Description:** The executor completes multiple tasks or an entire phase before updating tasks.md, batching all checkbox updates into a single write at the end of a phase or at the end of the entire implementation run.
- **Reasons to avoid:** Deferred marking means that tasks.md does not reflect actual progress during execution. If the implementation is interrupted — by a timeout, an error, a user cancellation, or a session limit — tasks.md still shows all tasks as incomplete, and the next invocation re-executes work that was already successfully completed. This wastes time and risks overwriting correct implementations with potentially different outputs. The mistake typically occurs when the executor treats tasks.md updates as bookkeeping to be done "when convenient" rather than as a critical persistence mechanism.
- **Negative consequences:** Interrupted implementations must restart from scratch because there is no record of completed work. Users monitoring progress see no movement in tasks.md even as implementation proceeds. In the worst case, a nearly complete implementation is interrupted, and the subsequent re-run produces subtly different code for already-completed tasks (because LLM outputs are not deterministic), introducing inconsistencies. The tasks.md file loses its value as a real-time progress tracker.
- **Correct alternative:** Apply the **Incremental Checkpoint Persistence** pattern to update tasks.md immediately after each task completes, ensuring that the file always reflects the true state of implementation progress.

8. Progress tracking and error handling:
   - Report progress after each completed task
   - Halt execution if any non-parallel task fails
   - For parallel tasks [P], continue with successful tasks, report failed ones
   - Provide clear error messages with context for debugging
   - Suggest next steps if implementation cannot proceed
   - **IMPORTANT** For completed tasks, make sure to mark the task off as [X] in the tasks file.

9. Completion validation:
   - Verify all required tasks are completed
   - Check that implemented features match the original specification
   - Validate that tests pass and coverage meets requirements
   - Confirm the implementation follows the technical plan
   - Report final status with summary of completed work

Steps 8 and 9 define how the executor reports progress, handles failures, and validates the finished implementation. The following patterns and anti-patterns guide reliable error handling and thorough completion verification.

### Patterns for Progress Tracking and Validation

#### Pattern: Graduated Failure Handling

- **Objective:** Apply different failure responses to sequential and parallel tasks, halting on sequential failures that block downstream work while isolating parallel failures to allow independent tasks to complete.
- **Context of application:** Apply during step 8 when a task fails during execution.
- **Key characteristics:** The executor distinguishes between two failure modes. A sequential task failure is a hard stop — execution halts because subsequent tasks depend on the failed task's output. A parallel task failure is an isolated incident — other [P] tasks in the same batch continue because they are independent by definition. In both cases, the failure is reported with full context (task ID, file path, error message, suggested fix). Failed tasks are never silently skipped.
- **Operational guidance:**
  1. When a sequential (non-[P]) task fails, immediately halt execution. Do not attempt the next task. Report the failure with: task ID, the file path involved, the error message, and a suggested next step (e.g., "Fix the import error in src/models/user.py and re-run implementation").
  2. When a parallel [P] task fails, record the failure but continue executing other [P] tasks in the same batch. After the batch completes, report all failures together with individual context for each.
  3. After reporting parallel failures, assess whether any subsequent phase depends on the failed tasks. If so, halt before that phase and report the dependency. If not, continue with the next phase.
  4. Never retry a failed task automatically. Report the failure and let the user or a subsequent invocation handle it.
  5. In the final completion report, list all failed tasks separately from completed tasks, with their error context preserved.

#### Pattern: Spec-Aligned Completion Verification

- **Objective:** Verify that the completed implementation satisfies the original feature specification and technical plan, not just that all tasks in tasks.md were executed.
- **Context of application:** Apply during step 9 (Completion validation) after all tasks have been executed or after execution has halted due to failures.
- **Key characteristics:** The executor performs verification at two levels. The first level checks that every task in tasks.md is marked [X] (execution completeness). The second level checks that the implemented code aligns with the design artifacts — models match data-model.md, endpoints conform to contracts/, and technology choices follow research.md decisions (specification alignment). A fully checked-off tasks.md with implementations that diverge from the spec is not a successful completion.
- **Operational guidance:**
  1. Count tasks marked [X] versus total tasks. Report the completion ratio (e.g., "42/45 tasks completed").
  2. For each completed phase, review whether the artifacts produced match the corresponding design document. Spot-check at minimum: one model against data-model.md, one endpoint against contracts/, and one technology usage against research.md.
  3. If tests were generated and executed, report test results (pass/fail counts, coverage if available).
  4. If any tasks remain incomplete, list them with their task IDs and the reason they were not completed (failure, dependency on a failed task, or user-initiated halt).
  5. Produce a final summary that includes: total tasks, completed tasks, failed tasks, checklist gate status (passed or bypassed with consent), and any specification alignment issues detected.

### Anti-Patterns for Progress Tracking and Validation

#### Anti-Pattern: Undifferentiated Failure Response

- **Description:** The executor applies the same failure response to all tasks regardless of their sequential or parallel designation — either halting on every failure (including independent [P] tasks) or continuing past every failure (including sequential tasks that block downstream work).
- **Reasons to avoid:** Halting on parallel task failures wastes the opportunity to complete independent work that could proceed unaffected. Continuing past sequential task failures produces cascading errors — every subsequent task that depends on the failed task's output will also fail, generating a chain of error reports that obscure the original problem. This mistake occurs when the executor lacks a branching error-handling strategy and defaults to a single behavior for all failures.
- **Negative consequences:** If the executor halts on all failures: independent tasks that could have completed successfully are left undone, and the next invocation must re-execute them. If the executor continues past all failures: sequential failures cascade into dozens of follow-on errors, the error report becomes unreadable, and tasks.md shows some tasks as [X] even though they depend on a failed predecessor and their output is invalid.
- **Correct alternative:** Apply the **Graduated Failure Handling** pattern to halt on sequential failures and isolate parallel failures, producing accurate progress reports and minimizing wasted work.

#### Anti-Pattern: Checkbox-Only Validation

- **Description:** The executor considers the implementation complete when all tasks in tasks.md are marked [X], without verifying that the implemented code actually conforms to the design artifacts (data-model.md, contracts/, research.md) or the original feature specification.
- **Reasons to avoid:** A task being marked complete means it was executed — not that its output is correct. An LLM implementing a model task may produce a syntactically valid file that omits fields defined in data-model.md, uses incorrect types, or ignores validation rules. An endpoint implementation may return different status codes or field names than what contracts/ specifies. If the executor's validation stops at "all checkboxes are checked," these discrepancies propagate to production. This mistake occurs when the executor equates task completion with specification compliance.
- **Negative consequences:** The completion report declares success while the implementation contains specification violations. The consistency analysis agent (speckit.analyze) later discovers mismatches that could have been caught during implementation. Reviewers trust the completion report and may not perform their own artifact-level verification, allowing defects through. The implementation must be corrected post-completion, which is more expensive than catching issues during execution.
- **Correct alternative:** Apply the **Spec-Aligned Completion Verification** pattern to perform both execution completeness checks (all tasks marked [X]) and specification alignment checks (implementations match design artifacts).

Note: This command assumes a complete task breakdown exists in tasks.md. If tasks are incomplete or missing, suggest running `/speckit.tasks` first to regenerate the task list.

10. **Check for extension hooks**: After completion validation, check if `.specify/extensions.yml` exists in the project root.
    - If it exists, read it and look for entries under the `hooks.after_implement` key
    - If the YAML cannot be parsed or is invalid, skip hook checking silently and continue normally
    - Filter out hooks where `enabled` is explicitly `false`. Treat hooks without an `enabled` field as enabled by default.
    - For each remaining hook, do **not** attempt to interpret or evaluate hook `condition` expressions:
      - If the hook has no `condition` field, or it is null/empty, treat the hook as executable
      - If the hook defines a non-empty `condition`, skip the hook and leave condition evaluation to the HookExecutor implementation
    - For each executable hook, output the following based on its `optional` flag:
      - **Optional hook** (`optional: true`):

        ```text
        ## Extension Hooks

        **Optional Hook**: {extension}
        Command: `/{command}`
        Description: {description}

        Prompt: {prompt}
        To execute: `/{command}`
        ```

      - **Mandatory hook** (`optional: false`):

        ```text
        ## Extension Hooks

        **Automatic Hook**: {extension}
        Executing: `/{command}`
        EXECUTE_COMMAND: {command}
        ```

    - If no hooks are registered or `.specify/extensions.yml` does not exist, skip silently
