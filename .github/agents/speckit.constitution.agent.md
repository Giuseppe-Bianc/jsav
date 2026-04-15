---
description: Create or update the project constitution from interactive or provided principle inputs, ensuring all dependent templates stay in sync.
handoffs: 
  - label: Build Specification
    agent: speckit.specify
    prompt: Implement the feature specification based on the updated constitution. I want to build...
---

## User Input

```text
$ARGUMENTS
```

You **MUST** consider the user input before proceeding (if not empty).

## Pre-Execution Checks

**Check for extension hooks (before constitution update)**:
- Check if `.specify/extensions.yml` exists in the project root.
- If it exists, read it and look for entries under the `hooks.before_constitution` key
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

You are updating the project constitution at `.specify/memory/constitution.md`. This file is a TEMPLATE containing placeholder tokens in square brackets (e.g. `[PROJECT_NAME]`, `[PRINCIPLE_1_NAME]`). Your job is to (a) collect/derive concrete values, (b) fill the template precisely, and (c) propagate any amendments across dependent artifacts.

**Note**: If `.specify/memory/constitution.md` does not exist yet, it should have been initialized from `.specify/templates/constitution-template.md` during project setup. If it's missing, copy the template first.

Follow this execution flow:

1. Load the existing constitution at `.specify/memory/constitution.md`.
   - Identify every placeholder token of the form `[ALL_CAPS_IDENTIFIER]`.
   **IMPORTANT**: The user might require less or more principles than the ones used in the template. If a number is specified, respect that - follow the general template. You will update the doc accordingly.

2. Collect/derive values for placeholders:
   - If user input (conversation) supplies a value, use it.
   - Otherwise infer from existing repo context (README, docs, prior constitution versions if embedded).
   - For governance dates: `RATIFICATION_DATE` is the original adoption date (if unknown ask or mark TODO), `LAST_AMENDED_DATE` is today if changes are made, otherwise keep previous.
   - `CONSTITUTION_VERSION` must increment according to semantic versioning rules:
     - MAJOR: Backward incompatible governance/principle removals or redefinitions.
     - MINOR: New principle/section added or materially expanded guidance.
     - PATCH: Clarifications, wording, typo fixes, non-semantic refinements.
   - If version bump type ambiguous, propose reasoning before finalizing.

3. Draft the updated constitution content:
   - Replace every placeholder with concrete text (no bracketed tokens left except intentionally retained template slots that the project has chosen not to define yet—explicitly justify any left).
   - Preserve heading hierarchy and comments can be removed once replaced unless they still add clarifying guidance.
   - Ensure each Principle section: succinct name line, paragraph (or bullet list) capturing non‑negotiable rules, explicit rationale if not obvious.
   - Ensure Governance section lists amendment procedure, versioning policy, and compliance review expectations.

Steps 1 through 3 define how the constitution is loaded, populated, and authored. The core challenges are resolving placeholders accurately and producing principles that downstream agents can evaluate unambiguously. The following patterns and anti-patterns guide these authoring decisions.

### Patterns for Constitution Authoring

#### Pattern: Hierarchical Placeholder Resolution

- **Objective:** Resolve every placeholder token through a strict priority cascade so that user-provided values are never overridden by inference, repo-derived values take precedence over guesses, and TODOs are introduced only as a last resort with explicit justification.
- **Context of application:** Apply during steps 1 and 2 when processing each `[ALL_CAPS_IDENTIFIER]` placeholder in the constitution template.
- **Key characteristics:** Each placeholder is processed through a four-tier cascade: (1) explicit user input from the conversation or $ARGUMENTS, (2) values derived from existing repository artifacts (README, docs, prior constitution versions), (3) domain-informed inference based on project type and context, (4) a `TODO(<FIELD_NAME>): explanation` marker when all three preceding sources are exhausted. The executor never skips a tier — if user input provides a value, tiers 2–4 are not consulted for that placeholder. If user input is absent, the executor moves to tier 2 before attempting inference.
- **Operational guidance:**
  1. For each placeholder, first search the user's input ($ARGUMENTS and conversation context) for an explicit value. If found, use it and move to the next placeholder.
  2. If user input provides no value, search the repository: README.md for project name and description, existing constitution versions for ratification date and prior principle names, package manifests for technology context.
  3. If repository context provides no value, infer from the project type and domain. Document the inference in the Sync Impact Report under a "Derived Values" subsection so reviewers can verify it.
  4. If no reasonable inference is possible, insert `TODO(<FIELD_NAME>): <explanation of what is needed and why it could not be inferred>`. Include this in the Sync Impact Report under deferred items.
  5. After processing all placeholders, count remaining TODOs. If more than two exist, reconsider whether additional repo context or domain inference could resolve them before finalizing.

#### Pattern: Declarative Principle Formulation

- **Objective:** Write every principle as a testable, enforceable rule that downstream agents — particularly the constitution gate evaluation in speckit.plan — can assess with a binary compliant/violation verdict.
- **Context of application:** Apply during step 3 when drafting or revising each Principle section in the constitution.
- **Key characteristics:** Principles use directive language (MUST, MUST NOT, SHALL) rather than advisory language (should, ideally, where possible). Each principle describes an observable property or constraint that can be checked against a concrete artifact — a code file, a configuration, a design document. Principles that cannot be evaluated without subjective judgment are rewritten until they can.
- **Operational guidance:**
  1. For each principle, write a one-sentence rule using MUST or MUST NOT. If the sentence requires "when appropriate" or "where possible," identify the specific conditions under which the rule applies and state them explicitly (e.g., "All public API endpoints MUST validate input parameters against the contract schema" rather than "Input should be validated where appropriate").
  2. Test each principle by asking: "Could an agent reviewing a plan.md or a source file determine compliance without asking a human?" If the answer is no, the principle is too vague.
  3. Include rationale for non-obvious principles. Rationale explains *why* the rule exists, helping downstream agents assess whether a justified exception is legitimate.
  4. Avoid compound principles that bundle multiple rules. "All services MUST use structured logging AND MUST include correlation IDs" should be two separate principles so that compliance can be tracked per-rule.
  5. After drafting all principles, review them as a set. Check for contradictions (one principle requires X, another prohibits it) and redundancies (two principles that express the same constraint differently).

#### Pattern: Semantic Version Justification

- **Objective:** Ensure every version bump is explicitly mapped to the specific changes that justify the chosen tier (MAJOR, MINOR, PATCH), preventing both under-versioning that hides breaking changes and over-versioning that inflates the version number without cause.
- **Context of application:** Apply during step 2 when determining `CONSTITUTION_VERSION` and during step 5 when documenting the version change in the Sync Impact Report.
- **Key characteristics:** The executor lists every change made to the constitution and classifies each as MAJOR, MINOR, or PATCH-level. The final version bump corresponds to the highest-tier change in the set. The classification and reasoning are documented in the Sync Impact Report so that reviewers can verify the bump.
- **Operational guidance:**
  1. Before deciding the version bump, create a list of all changes: principles added, principles removed, principles renamed, principles whose rules were materially altered, wording clarifications, governance procedure changes.
  2. Classify each change: removing or fundamentally redefining a principle is MAJOR; adding a new principle or materially expanding an existing one is MINOR; rewording without changing meaning is PATCH.
  3. The final bump is the highest tier present. If any change is MAJOR, the bump is MAJOR regardless of how many PATCH-level changes also exist.
  4. Document the classification in the Sync Impact Report: "Version bump: 1.2.0 → 2.0.0 (MAJOR). Reason: Principle 'Observability' removed; Principle 'Testing Discipline' fundamentally redefined from unit-test focus to integration-test mandate."
  5. If the classification is ambiguous (e.g., a principle's wording changed but it is unclear whether the meaning changed), state the ambiguity explicitly and propose both interpretations before finalizing.

### Anti-Patterns for Constitution Authoring

#### Anti-Pattern: Vague Principle Language

- **Description:** Principles are written with hedge words and untestable qualifiers — "Code should be clean and well-organized," "Security best practices should be followed where appropriate," "The team should strive for comprehensive test coverage."
- **Reasons to avoid:** The constitution's primary consumer is the gate evaluation in speckit.plan, which must produce a binary compliant/violation verdict for each principle. A principle stating "should be followed where appropriate" cannot be evaluated because it defers the compliance decision to the evaluator's judgment. This mistake occurs when the author writes principles as aspirational values (what the team believes in) rather than as enforceable rules (what the project requires). Conversational phrasing from user input ("we want clean code") is transcribed directly into the constitution without translation into directive language.
- **Negative consequences:** Gate evaluations in speckit.plan become rubber stamps — every principle passes because no principle can definitively fail. Different agents interpret the same principle differently, producing inconsistent compliance assessments. The constitution loses its authority as a governance document because it expresses preferences rather than requirements. Over time, teams stop consulting the constitution because it provides no actionable constraints.
- **Correct alternative:** Apply the **Declarative Principle Formulation** pattern to rewrite each principle as a testable rule with MUST/MUST NOT language, explicit conditions, and observable criteria.

#### Anti-Pattern: Context-Free Placeholder Inference

- **Description:** The executor infers placeholder values from general domain knowledge or template defaults rather than from the specific project's repository artifacts — for example, setting `[PROJECT_NAME]` to a generic name based on the feature description instead of reading it from README.md or package.json, or inferring principles from common industry standards without checking whether the project's existing documentation establishes different conventions.
- **Reasons to avoid:** The constitution must reflect the specific project's governance, not a generic template's assumptions. Inferred values that do not match actual project context create a constitution that the team does not recognize as their own, undermining adoption. This mistake occurs when the executor skips tier 2 (repo context) of the resolution cascade — either because it does not check for repository artifacts or because it treats inference as faster than reading existing files.
- **Negative consequences:** The constitution contains a project name, description, or principles that conflict with what the repository already documents. Downstream templates that reference constitution values propagate the incorrect information. When team members review the constitution, they must correct values that the executor could have derived from existing artifacts, eroding trust in the workflow. The Sync Impact Report does not flag the values as inferred, so reviewers have no signal to verify them.
- **Correct alternative:** Apply the **Hierarchical Placeholder Resolution** pattern to exhaust user input and repository context before resorting to inference, and to document all inferred values in the Sync Impact Report.

#### Anti-Pattern: Reflexive Patch Bumping

- **Description:** The executor defaults to a PATCH version bump for every constitution update, regardless of whether principles were added, removed, or fundamentally redefined, typically because PATCH feels "safe" and avoids the perceived overhead of justifying a larger bump.
- **Reasons to avoid:** Semantic versioning communicates the nature of change to consumers. A PATCH bump signals "no meaningful change to governance" — if a principle was removed or redefined, the PATCH signal is false. Downstream agents and human reviewers that depend on version semantics (e.g., deciding whether to re-evaluate existing plans against the updated constitution) will not trigger the necessary re-evaluation for a PATCH bump. This mistake occurs when the executor treats versioning as a formality rather than as a communication mechanism.
- **Negative consequences:** Breaking governance changes (principle removals, redefinitions) are shipped under a PATCH label. Existing plans and implementations that were compliant under the old constitution may violate the new constitution, but the PATCH version signal does not prompt re-evaluation. The version history becomes unreliable — a project at version 1.0.47 has undergone 47 "non-semantic" changes, but some of those changes may have been materially significant. Teams cannot use version numbers to understand the evolution of their governance.
- **Correct alternative:** Apply the **Semantic Version Justification** pattern to classify each change individually and set the bump to the highest tier present.

4. Consistency propagation checklist (convert prior checklist into active validations):
   - Read `.specify/templates/plan-template.md` and ensure any "Constitution Check" or rules align with updated principles.
   - Read `.specify/templates/spec-template.md` for scope/requirements alignment—update if constitution adds/removes mandatory sections or constraints.
   - Read `.specify/templates/tasks-template.md` and ensure task categorization reflects new or removed principle-driven task types (e.g., observability, versioning, testing discipline).
   - Read each command file in `.specify/templates/commands/*.md` (including this one) to verify no outdated references (agent-specific names like CLAUDE only) remain when generic guidance is required.
   - Read any runtime guidance docs (e.g., `README.md`, `docs/quickstart.md`, or agent-specific guidance files if present). Update references to principles changed.

Step 4 propagates constitution changes across all dependent files. The challenge is ensuring that every affected file is updated while avoiding unnecessary modifications to files that are not impacted by the specific changes. The following patterns and anti-patterns guide reliable propagation.

### Patterns for Consistency Propagation

#### Pattern: Change-Scoped Propagation

- **Objective:** Limit propagation updates to files that are actually affected by the specific principles that changed, rather than rewriting all dependent files on every constitution update.
- **Context of application:** Apply during step 4 when determining which files need modification and what modifications are necessary.
- **Key characteristics:** The executor compares the old and new constitution content, identifies which principles were added, modified, or removed, and searches each dependent file only for references to those specific principles. Files that reference only unchanged principles are read (to confirm no stale references) but not modified. This precision prevents introducing unintended changes into stable templates while ensuring that every genuinely affected file is updated.
- **Operational guidance:**
  1. Before beginning propagation, produce a change diff: list each principle that was added, removed, renamed, or whose rules were materially altered.
  2. For each file in the propagation checklist, search for references to the changed principles — by name, by concept, or by rule text.
  3. If a file references a changed principle, update the reference to reflect the new name, rule, or absence. If a principle was removed, remove or replace any gate checks, task categories, or section requirements that depended on it.
  4. If a file references only unchanged principles, confirm the references are still accurate and move on without modification.
  5. Record every file that was modified and every file that was checked but not modified in the Sync Impact Report, so reviewers can verify the propagation scope.

#### Pattern: Bidirectional Reference Verification

- **Objective:** Verify not only that dependent files reference the correct constitution principles (forward direction) but also that every constitution principle that is intended to affect downstream behavior has at least one dependent file that references it (reverse direction).
- **Context of application:** Apply as a validation sub-step at the end of step 4, after all dependent files have been checked and updated.
- **Key characteristics:** The executor performs two passes. The forward pass (already covered in step 4's checklist) checks that dependent files reference current, valid principles. The reverse pass checks that every principle in the constitution is referenced by at least one template, command, or guidance document. A principle with no downstream references is either newly added (and needs propagation) or is a governance statement that has no operational effect (and should be flagged for review).
- **Operational guidance:**
  1. After completing the forward propagation pass, extract the list of all principle names from the updated constitution.
  2. For each principle, search all files in the propagation checklist for at least one reference.
  3. If a principle has no downstream references and it was just added, determine which template or command file should enforce it and add the reference.
  4. If a principle has no downstream references and it has existed since a prior version, flag it in the Sync Impact Report as "unreferenced principle — may have no operational enforcement."
  5. Include the reverse-pass results in the Sync Impact Report alongside the forward-pass results.

### Anti-Patterns for Consistency Propagation

#### Anti-Pattern: Read-Only Propagation

- **Description:** The executor reads all files listed in the propagation checklist and confirms that changes are needed, but does not actually modify the files — either because it treats propagation as a verification step rather than a synchronization step, or because it defers all modifications to "manual follow-up" without making any updates itself.
- **Reasons to avoid:** Step 4 is titled "consistency propagation," not "consistency verification." Its purpose is to ensure dependent files are in sync with the updated constitution, which requires writing changes to those files when misalignment is found. Treating it as read-only produces a Sync Impact Report that lists problems without fixing them, pushing the synchronization burden onto the user. This mistake occurs when the executor is uncertain about whether it has permission to modify templates or when it defaults to conservative behavior ("flag, don't fix").
- **Negative consequences:** Templates and command files retain stale principle references. The next agent to consume plan-template.md or tasks-template.md evaluates compliance against principles that no longer exist or enforces categorizations that have been superseded. The Sync Impact Report lists items as "⚠ pending" indefinitely because no one manually follows up. The constitution update is technically complete, but the project's operational governance is out of sync.
- **Correct alternative:** Apply the **Change-Scoped Propagation** pattern to identify affected files and update them directly, marking each as "✅ updated" in the Sync Impact Report.

#### Anti-Pattern: Blanket Template Rewriting

- **Description:** Every constitution update — regardless of scope — triggers a full rewrite of all dependent templates and documentation, even when only a single principle's wording was clarified with no change in meaning.
- **Reasons to avoid:** Blanket rewriting introduces unnecessary diffs into version-controlled files, making it difficult to distinguish meaningful updates from noise. It risks introducing errors into stable templates that were correctly aligned before the update. This mistake typically occurs when the executor does not compare old and new constitution content to identify which principles actually changed, and instead treats "constitution was modified" as a signal to rewrite everything.
- **Negative consequences:** Version control diffs for dependent files are cluttered with trivial changes, obscuring genuine updates. Reviewers must inspect every modified file to determine whether the change was meaningful or cosmetic. Stable templates may be subtly altered by the rewriting process (different phrasing, reordered sections) without any governance justification. Teams learn to ignore template diffs because most are noise, increasing the risk that a genuine misalignment goes unnoticed.
- **Correct alternative:** Apply the **Change-Scoped Propagation** pattern to modify only the files affected by the specific principles that changed, leaving all other files untouched.

5. Produce a Sync Impact Report (prepend as an HTML comment at top of the constitution file after update):
   - Version change: old → new
   - List of modified principles (old title → new title if renamed)
   - Added sections
   - Removed sections
   - Templates requiring updates (✅ updated / ⚠ pending) with file paths
   - Follow-up TODOs if any placeholders intentionally deferred.

6. Validation before final output:
   - No remaining unexplained bracket tokens.
   - Version line matches report.
   - Dates ISO format YYYY-MM-DD.
   - Principles are declarative, testable, and free of vague language ("should" → replace with MUST/SHOULD rationale where appropriate).

Steps 5 and 6 produce the traceability report and validate the constitution before it is written. The following patterns and anti-patterns guide thorough validation and accurate reporting.

### Patterns for Validation and Reporting

#### Pattern: Cross-Field Consistency Validation

- **Objective:** Verify that all fields within the constitution are mutually consistent — version numbers match the Sync Impact Report, dates are correctly assigned, principle counts match the actual sections, and no field contradicts another.
- **Context of application:** Apply during step 6 as a structured validation pass before the constitution is written to disk.
- **Key characteristics:** The validator checks relationships between fields rather than each field in isolation. It is not sufficient that `CONSTITUTION_VERSION` is a valid semver string — it must match the version stated in the Sync Impact Report. It is not sufficient that `LAST_AMENDED_DATE` is a valid ISO date — it must be today's date if changes were made, or the previous date if no changes were made. Each validation check is relational.
- **Operational guidance:**
  1. Verify that `CONSTITUTION_VERSION` in the document body matches the "new version" stated in the Sync Impact Report.
  2. Verify that `LAST_AMENDED_DATE` is today's date (ISO 8601, YYYY-MM-DD) if any content was changed. If no content was changed, verify it matches the previous value.
  3. Count the principle sections in the document and compare against any principle count stated in the governance section or implied by the template structure. If the user requested a specific number of principles (noted in step 1), verify the count matches.
  4. Scan the entire document for bracket tokens `[ALL_CAPS_IDENTIFIER]`. For each one found, verify it is either a `TODO(<FIELD_NAME>)` marker with an explanation (justified deferral) or an intentionally retained template slot with explicit justification. Flag any unexplained bracket token as a validation failure.
  5. Read each principle section and verify it contains both a rule statement (MUST/MUST NOT) and, for non-obvious rules, a rationale. Flag principles that are purely declarative with no rationale when the rule's necessity is not self-evident.

#### Pattern: Actionable Report Generation

- **Objective:** Produce a Sync Impact Report that enables reviewers to verify every change, understand every deferred item, and take action on every pending follow-up — without needing to compare file versions manually.
- **Context of application:** Apply during step 5 when constructing the Sync Impact Report.
- **Key characteristics:** The report is structured as a decision record, not a summary. Each entry includes what changed, why it changed, what downstream impact it has, and whether the impact has been addressed. Deferred items include the specific action needed and the reason for deferral. The report is machine-parseable (consistent formatting) and human-readable (clear language).
- **Operational guidance:**
  1. For each modified principle, state the old name/rule and the new name/rule side by side. Do not simply list "Principle 3 updated" — state what changed.
  2. For each added or removed section, state what it contains and why it was added or removed, referencing the user's input or the governance rationale.
  3. For each template in the propagation checklist, state one of: "✅ updated — [description of change made]" or "⚠ pending — [description of update needed and why it was not performed]" or "✓ no changes needed — no references to modified principles."
  4. For each deferred TODO, state the field name, why it could not be resolved, and what the user needs to provide to resolve it.
  5. Include the version bump justification (from the Semantic Version Justification pattern) as a subsection of the report so that reviewers can verify the version decision without searching the document.

### Anti-Patterns for Validation and Reporting

#### Anti-Pattern: Isolated Field Validation

- **Description:** The validator checks each field individually (is the version a valid semver string? is the date in ISO format?) without checking whether fields are consistent with each other or with the Sync Impact Report.
- **Reasons to avoid:** Individual field validity does not guarantee document consistency. A version of `1.3.0` is a valid semver string, but if the Sync Impact Report states the version changed from `1.2.0` to `2.0.0`, the document is internally contradictory. An amendment date of `2024-01-15` is a valid ISO date, but if today is `2025-07-02` and changes were made, the date is wrong. This mistake occurs when the validator treats validation as a per-field format check rather than as a relational consistency check.
- **Negative consequences:** The constitution passes validation despite containing contradictions. The Sync Impact Report states one version while the document header states another. Downstream agents that read the version from the document and agents that read it from the report get different values. Dates that do not reflect actual modification times mislead reviewers about when the constitution was last changed.
- **Correct alternative:** Apply the **Cross-Field Consistency Validation** pattern to verify relational consistency between fields, between the document and its report, and between the document and the user's requested changes.

#### Anti-Pattern: Opaque Impact Reporting

- **Description:** The Sync Impact Report summarizes changes at a high level ("3 principles updated, 2 templates modified") without specifying what changed in each principle, what was modified in each template, or what action is needed for pending items.
- **Reasons to avoid:** A high-level summary tells reviewers that changes happened but not what the changes are. Reviewers must open each modified file and compare versions manually to understand the impact — which is exactly the work the report is supposed to eliminate. This mistake occurs when the executor treats the report as a changelog entry rather than as a detailed audit trail, or when it prioritizes brevity over completeness.
- **Negative consequences:** Reviewers cannot verify the version bump without examining every changed file. Pending items are listed without actionable descriptions, so they are never resolved. The report provides a false sense of documentation — it exists, but it does not inform. Over repeated constitution updates, the accumulated reports form an uninformative history that cannot be used to trace governance evolution.
- **Correct alternative:** Apply the **Actionable Report Generation** pattern to produce a report that specifies every change, its rationale, its downstream impact, and the action taken or needed.

7. Write the completed constitution back to `.specify/memory/constitution.md` (overwrite).

8. Output a final summary to the user with:
   - New version and bump rationale.
   - Any files flagged for manual follow-up.
   - Suggested commit message (e.g., `docs: amend constitution to vX.Y.Z (principle additions + governance update)`).

Formatting & Style Requirements:

- Use Markdown headings exactly as in the template (do not demote/promote levels).
- Wrap long rationale lines to keep readability (<100 chars ideally) but do not hard enforce with awkward breaks.
- Keep a single blank line between sections.
- Avoid trailing whitespace.

If the user supplies partial updates (e.g., only one principle revision), still perform validation and version decision steps.

If critical info missing (e.g., ratification date truly unknown), insert `TODO(<FIELD_NAME>): explanation` and include in the Sync Impact Report under deferred items.

Do not create a new template; always operate on the existing `.specify/memory/constitution.md` file.

## Post-Execution Checks

**Check for extension hooks (after constitution update)**:
Check if `.specify/extensions.yml` exists in the project root.
- If it exists, read it and look for entries under the `hooks.after_constitution` key
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
