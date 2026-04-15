---
description: Identify underspecified areas in the current feature spec by asking up to 5 highly targeted clarification questions and encoding answers back into the spec.
handoffs: 
  - label: Build Technical Plan
    agent: speckit.plan
    prompt: Create a plan for the spec. I am building with...
---

## User Input

```text
$ARGUMENTS
```

You **MUST** consider the user input before proceeding (if not empty).

## Pre-Execution Checks

**Check for extension hooks (before clarification)**:
- Check if `.specify/extensions.yml` exists in the project root.
- If it exists, read it and look for entries under the `hooks.before_clarify` key
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

Goal: Detect and reduce ambiguity or missing decision points in the active feature specification and record the clarifications directly in the spec file.

Note: This clarification workflow is expected to run (and be completed) BEFORE invoking `/speckit.plan`. If the user explicitly states they are skipping clarification (e.g., exploratory spike), you may proceed, but must warn that downstream rework risk increases.

Execution steps:

1. Run `pwsh -ExecutionPolicy Bypass -File .specify/scripts/powershell/check-prerequisites.ps1 -Json -PathsOnly` from repo root **once** (combined `--json --paths-only` mode / `-Json -PathsOnly`). Parse minimal JSON payload fields:
   - `FEATURE_DIR`
   - `FEATURE_SPEC`
   - (Optionally capture `IMPL_PLAN`, `TASKS` for future chained flows.)
   - If JSON parsing fails, abort and instruct user to re-run `/speckit.specify` or verify feature branch environment.
   - For single quotes in args like "I'm Groot", use escape syntax: e.g 'I'\''m Groot' (or double-quote if possible: "I'm Groot").

2. Load the current spec file. Perform a structured ambiguity & coverage scan using this taxonomy. For each category, mark status: Clear / Partial / Missing. Produce an internal coverage map used for prioritization (do not output raw map unless no questions will be asked).

   Functional Scope & Behavior:
   - Core user goals & success criteria
   - Explicit out-of-scope declarations
   - User roles / personas differentiation

   Domain & Data Model:
   - Entities, attributes, relationships
   - Identity & uniqueness rules
   - Lifecycle/state transitions
   - Data volume / scale assumptions

   Interaction & UX Flow:
   - Critical user journeys / sequences
   - Error/empty/loading states
   - Accessibility or localization notes

   Non-Functional Quality Attributes:
   - Performance (latency, throughput targets)
   - Scalability (horizontal/vertical, limits)
   - Reliability & availability (uptime, recovery expectations)
   - Observability (logging, metrics, tracing signals)
   - Security & privacy (authN/Z, data protection, threat assumptions)
   - Compliance / regulatory constraints (if any)

   Integration & External Dependencies:
   - External services/APIs and failure modes
   - Data import/export formats
   - Protocol/versioning assumptions

   Edge Cases & Failure Handling:
   - Negative scenarios
   - Rate limiting / throttling
   - Conflict resolution (e.g., concurrent edits)

   Constraints & Tradeoffs:
   - Technical constraints (language, storage, hosting)
   - Explicit tradeoffs or rejected alternatives

   Terminology & Consistency:
   - Canonical glossary terms
   - Avoided synonyms / deprecated terms

   Completion Signals:
   - Acceptance criteria testability
   - Measurable Definition of Done style indicators

   Misc / Placeholders:
   - TODO markers / unresolved decisions
   - Ambiguous adjectives ("robust", "intuitive") lacking quantification

   For each category with Partial or Missing status, add a candidate question opportunity unless:
   - Clarification would not materially change implementation or validation strategy
   - Information is better deferred to planning phase (note internally)

The structured scan above is the foundation for every downstream decision in this workflow. The following best practices and common mistakes govern how the scan should be conducted to maximize its reliability.

### Patterns for Ambiguity Analysis

#### Taxonomy-Exhaustive Evaluation

- **Objective:** Eliminate blind spots by evaluating every taxonomy category against the spec, even categories that appear irrelevant at first glance.
- **Context of application:** During the structured ambiguity scan in Step 2, before any candidate questions are generated.
- **Key characteristics:** Every category receives an explicit status mark. No category is skipped or assumed to be clear based on project type alone. Categories that seem inapplicable are still verified against actual spec text and marked with a justification.
- **Operational guidance:**
  1. Process categories in the listed order; do not skip or reorder based on perceived relevance.
  2. For each category, cite at least one spec passage that supports the assigned status, or note the absence of any relevant content.
  3. If a category appears inapplicable to the project, mark it "Clear — N/A" with a one-line justification (e.g., "No regulatory domain identified in spec scope") rather than omitting it from the coverage map.
  4. Complete the full taxonomy before generating any candidate questions — premature question drafting biases the remaining evaluation.

#### Materiality-First Filtering

- **Objective:** Distinguish ambiguities that materially affect implementation, testing, or architecture from those that are cosmetic or safely deferrable to planning.
- **Context of application:** After marking all category statuses, when deciding which Partial or Missing categories warrant a clarification question from the limited budget.
- **Key characteristics:** Each identified gap is evaluated for downstream impact. A gap is material if resolving it would change task decomposition, data model design, API contracts, test case structure, or operational configuration. Gaps that affect only prose quality or presentation are deprioritized.
- **Operational guidance:**
  1. For each Partial or Missing category, ask: "If this remained unresolved, what specific implementation decision would be blocked or incorrect?"
  2. If the answer is "none" or "only cosmetic," mark the gap as low-priority and do not allocate a question to it.
  3. If the answer identifies a concrete decision (e.g., "We would not know whether to use polling or webhooks"), mark it as high-priority.
  4. Document the impact assessment internally to justify question prioritization in Step 3.

### Anti-Patterns for Ambiguity Analysis

#### Familiarity Bias Skipping

- **Description:** The agent skips taxonomy categories it assumes are well-covered based on the project type or domain familiarity (e.g., "web apps always handle auth"), without verifying against the actual spec content.
- **Reasons to avoid:** Assumptions based on project type are frequently wrong for specific specs. A web application spec may omit authentication entirely because it sits behind an API gateway, or it may define a novel auth scheme that contradicts the agent's default assumption. Skipping categories creates undetected gaps that surface as rework during implementation.
- **Negative consequences:** Ambiguities in skipped categories remain invisible. The coverage map reports false confidence, and downstream steps (question generation, planning) operate on incomplete data. The clarification step fails to catch the very gaps it exists to find.
- **Correct alternative:** Apply **Taxonomy-Exhaustive Evaluation** to verify every category against actual spec content before assigning any status.

#### Deferred-by-Default Avoidance

- **Description:** The agent marks ambiguities as "deferred to planning phase" to avoid consuming question budget, even when the ambiguity would materially affect the plan itself.
- **Reasons to avoid:** Deferring a question that affects architecture, data modeling, or acceptance criteria means the planning agent will build on an unresolved assumption. The planning agent cannot correct what the clarification agent chose not to surface. This typically occurs when the agent conflates "complex to ask" with "better handled later."
- **Negative consequences:** The plan contains implicit assumptions that may be wrong. Rework cascades when the unresolved assumption surfaces during implementation or review. The clarification step fails its primary purpose of reducing downstream ambiguity.
- **Correct alternative:** Apply **Materiality-First Filtering** and defer only gaps whose resolution genuinely cannot affect planning decisions — not gaps that are merely uncomfortable to ask about within a constrained question format.

---

3. Generate (internally) a prioritized queue of candidate clarification questions (maximum 5). Do NOT output them all at once. Apply these constraints:
    - Maximum of 5 total questions across the whole session.
    - Each question must be answerable with EITHER:
       - A short multiple‑choice selection (2–5 distinct, mutually exclusive options), OR
       - A one-word / short‑phrase answer (explicitly constrain: "Answer in <=5 words").
    - Only include questions whose answers materially impact architecture, data modeling, task decomposition, test design, UX behavior, operational readiness, or compliance validation.
    - Ensure category coverage balance: attempt to cover the highest impact unresolved categories first; avoid asking two low-impact questions when a single high-impact area (e.g., security posture) is unresolved.
    - Exclude questions already answered, trivial stylistic preferences, or plan-level execution details (unless blocking correctness).
    - Favor clarifications that reduce downstream rework risk or prevent misaligned acceptance tests.
    - If more than 5 categories remain unresolved, select the top 5 by (Impact * Uncertainty) heuristic.

4. Sequential questioning loop (interactive):
    - Present EXACTLY ONE question at a time.
    - For multiple‑choice questions:
       - **Analyze all options** and determine the **most suitable option** based on:
          - Best practices for the project type
          - Common patterns in similar implementations
          - Risk reduction (security, performance, maintainability)
          - Alignment with any explicit project goals or constraints visible in the spec
       - Present your **recommended option prominently** at the top with clear reasoning (1-2 sentences explaining why this is the best choice).
       - Format as: `**Recommended:** Option [X] - <reasoning>`
       - Then render all options as a Markdown table:

       | Option | Description                                                                                         |
       | ------ | --------------------------------------------------------------------------------------------------- |
       | A      | <Option A description>                                                                              |
       | B      | <Option B description>                                                                              |
       | C      | <Option C description> (add D/E as needed up to 5)                                                  |
       | Short  | Provide a different short answer (<=5 words) (Include only if free-form alternative is appropriate) |

       - After the table, add: `You can reply with the option letter (e.g., "A"), accept the recommendation by saying "yes" or "recommended", or provide your own short answer.`
    - For short‑answer style (no meaningful discrete options):
       - Provide your **suggested answer** based on best practices and context.
       - Format as: `**Suggested:** <your proposed answer> - <brief reasoning>`
       - Then output: `Format: Short answer (<=5 words). You can accept the suggestion by saying "yes" or "suggested", or provide your own answer.`
    - After the user answers:
       - If the user replies with "yes", "recommended", or "suggested", use your previously stated recommendation/suggestion as the answer.
       - Otherwise, validate the answer maps to one option or fits the <=5 word constraint.
       - If ambiguous, ask for a quick disambiguation (count still belongs to same question; do not advance).
       - Once satisfactory, record it in working memory (do not yet write to disk) and move to the next queued question.
    - Stop asking further questions when:
       - All critical ambiguities resolved early (remaining queued items become unnecessary), OR
       - User signals completion ("done", "good", "no more"), OR
       - You reach 5 asked questions.
    - Never reveal future queued questions in advance.
    - If no valid questions exist at start, immediately report no critical ambiguities.

The question generation and interactive questioning phases are where the limited five-question budget is spent. The following guidance ensures each question maximizes the information gained per question asked, and highlights the most common ways budget is wasted.

### Patterns for Clarification Questioning

#### Single-Decision Scoping

- **Objective:** Ensure each question targets exactly one decision point, producing an answer that maps to a single, unambiguous spec update.
- **Context of application:** When formulating each candidate question before presenting it to the user, both during initial queue generation in Step 3 and dynamically during the loop in Step 4.
- **Key characteristics:** The question contains one interrogative clause. The answer space is constrained to mutually exclusive options or a short phrase. The agent can predict exactly which spec section and line will be updated based on any valid answer, before asking the question.
- **Operational guidance:**
  1. Before presenting a question, write (internally) a spec update template: "If the user answers X, I will update section Y with statement Z."
  2. If the template requires conditional branching across multiple sections for a single answer, the question is too broad — split it into two and select the higher-impact half.
  3. Verify that no two answer options would produce the same spec update; if they would, merge those options into one.
  4. Confirm the question cannot be interpreted as asking about two separate concerns (e.g., "Should we support OAuth and what are the rate limits?" is two questions, not one).

#### Budget-Aware Category Distribution

- **Objective:** Allocate the five-question budget across distinct high-impact taxonomy categories rather than concentrating multiple questions on a single category.
- **Context of application:** When ordering the prioritized question queue in Step 3, and when re-evaluating queued questions after each accepted answer in Step 4.
- **Key characteristics:** The agent tracks how many questions have been allocated to each taxonomy category. No single category receives more than two questions unless all other unresolved categories are demonstrably lower impact. After each accepted answer, the agent re-evaluates whether remaining queued questions are still necessary or whether the resolved ambiguity renders them redundant.
- **Operational guidance:**
  1. After generating the candidate queue, tag each question with its primary taxonomy category.
  2. If two or more questions share the same category, evaluate whether one question could subsume the other or whether the second question's impact justifies the budget cost.
  3. Prioritize coverage breadth: one question each across five categories is generally more valuable than three questions concentrated in one category.
  4. After each accepted answer, reassess queued questions — a resolved ambiguity in one category may make a question in a related category unnecessary, freeing budget for a different high-impact area.

### Anti-Patterns for Clarification Questioning

#### Compound Question Bundling

- **Description:** The agent combines multiple ambiguities into a single question (e.g., "What authentication method should we use and what are the rate-limiting thresholds?"), producing answers that partially address several issues but fully resolve none.
- **Reasons to avoid:** Users tend to answer the most salient part of a compound question and skip or abbreviate the rest. The constrained answer format — multiple choice or five words maximum — physically cannot accommodate multi-part responses. Integration becomes ambiguous because the answer maps to multiple spec sections with unclear boundaries.
- **Negative consequences:** Partial answers that leave residual ambiguity in one or more of the bundled concerns. A wasted question slot (the compound question counts as one question consumed, but resolved less than one complete issue). Integration errors from attempting to map a partial answer to multiple spec locations simultaneously.
- **Correct alternative:** Apply **Single-Decision Scoping** to ensure each question isolates exactly one decision point with a predictable spec update target.

#### Scope-Creep Questioning

- **Description:** The agent asks questions that introduce new requirements, features, or considerations not implied by the existing spec, expanding project scope under the guise of clarification.
- **Reasons to avoid:** The clarification agent's purpose is to resolve ambiguities in the current spec, not to perform requirements elicitation or feature ideation. Scope expansion creates obligations the user did not intend and can invalidate the spec's existing out-of-scope declarations. This typically happens when the agent identifies something the spec *could* address and mistakes that potential for an *obligation* to address it.
- **Negative consequences:** The spec grows in scope without a deliberate stakeholder decision. New requirements introduced via clarification bypass prioritization and feasibility analysis. The user loses trust in the clarification process because it adds work rather than reducing ambiguity.
- **Correct alternative:** Apply **Budget-Aware Category Distribution** and constrain every question to ambiguities that already exist within the spec's declared scope. If a potential gap implies an entirely new feature, note it as a recommendation in the Step 8 completion report rather than spending a question on it.

---

5. Integration after EACH accepted answer (incremental update approach):
    - Maintain in-memory representation of the spec (loaded once at start) plus the raw file contents.
    - For the first integrated answer in this session:
       - Ensure a `## Clarifications` section exists (create it just after the highest-level contextual/overview section per the spec template if missing).
       - Under it, create (if not present) a `### Session YYYY-MM-DD` subheading for today.
    - Append a bullet line immediately after acceptance: `- Q: <question> → A: <final answer>`.
    - Then immediately apply the clarification to the most appropriate section(s):
       - Functional ambiguity → Update or add a bullet in Functional Requirements.
       - User interaction / actor distinction → Update User Stories or Actors subsection (if present) with clarified role, constraint, or scenario.
       - Data shape / entities → Update Data Model (add fields, types, relationships) preserving ordering; note added constraints succinctly.
       - Non-functional constraint → Add/modify measurable criteria in Success Criteria > Measurable Outcomes (convert vague adjective to metric or explicit target).
       - Edge case / negative flow → Add a new bullet under Edge Cases / Error Handling (or create such subsection if template provides placeholder for it).
       - Terminology conflict → Normalize term across spec; retain original only if necessary by adding `(formerly referred to as "X")` once.
    - If the clarification invalidates an earlier ambiguous statement, replace that statement instead of duplicating; leave no obsolete contradictory text.
    - Save the spec file AFTER each integration to minimize risk of context loss (atomic overwrite).
    - Preserve formatting: do not reorder unrelated sections; keep heading hierarchy intact.
    - Keep each inserted clarification minimal and testable (avoid narrative drift).

6. Validation (performed after EACH write plus final pass):
   - Clarifications session contains exactly one bullet per accepted answer (no duplicates).
   - Total asked (accepted) questions ≤ 5.
   - Updated sections contain no lingering vague placeholders the new answer was meant to resolve.
   - No contradictory earlier statement remains (scan for now-invalid alternative choices removed).
   - Markdown structure valid; only allowed new headings: `## Clarifications`, `### Session YYYY-MM-DD`.
   - Terminology consistency: same canonical term used across all updated sections.

The integration and validation steps are where clarification answers become permanent spec content. Errors introduced here persist into planning and implementation. The following guidance ensures edits are precise and consistent, and highlights mistakes that silently degrade spec quality.

### Patterns for Spec Integration

#### Minimal-Diff Insertion

- **Objective:** Apply the smallest possible edit to encode a clarification, modifying only the specific statement or section affected, to minimize the risk of unintended side effects.
- **Context of application:** When writing each clarification answer into the spec file during Step 5, and when replacing ambiguous statements with clarified versions.
- **Key characteristics:** Each edit is scoped to the exact location of the ambiguity. Surrounding content is not reformatted, reworded, or reordered. The difference between the old and new spec versions shows only the lines directly affected by the clarification.
- **Operational guidance:**
  1. Before editing, identify the exact line or bullet that contains the ambiguous content to be resolved.
  2. Replace or augment only that line. Do not "improve" adjacent content while making the edit.
  3. If the clarification requires a new bullet, insert it in the logical position within the existing list without moving other bullets.
  4. After editing, mentally compare the old and new versions; if any change is not directly justified by the clarification answer, revert it.

#### Post-Write Consistency Sweep

- **Objective:** After each spec write, verify that the newly integrated clarification is consistent with all other sections and that no residual contradictions or terminology mismatches remain.
- **Context of application:** During Step 6 validation, performed after each atomic write operation — not only at the end of the session.
- **Key characteristics:** The sweep covers terminology consistency, cross-section agreement, and removal of invalidated content. It is brief and targeted, focused on the areas potentially affected by the most recent edit rather than a full document re-read.
- **Operational guidance:**
  1. After each write, search the spec for any other occurrence of the term or concept just clarified.
  2. Verify that all occurrences now agree with the clarified version. Update any that do not.
  3. Check whether the clarification invalidates any existing out-of-scope declaration, assumption, or edge case description elsewhere in the spec.
  4. Confirm the Clarifications session log entry matches what was actually written into the body sections — no log entry should exist without a corresponding body update, and no body update should exist without a log entry.

### Anti-Patterns for Spec Integration

#### Additive-Only Editing

- **Description:** The agent appends the clarification as new content without updating or removing the original ambiguous statement, leaving both the old vague text and the new precise text in the spec simultaneously.
- **Reasons to avoid:** Retaining the original ambiguous text alongside the clarification creates contradictory or redundant statements. Downstream consumers of the spec (planning agents, developers, testers) may reference the original text instead of the clarification, especially if the original text appears first or in a more prominent section. This typically occurs when the agent treats integration as "adding information" rather than "resolving ambiguity."
- **Negative consequences:** The spec contains conflicting guidance on the same topic. Downstream agents may parse the original statement and ignore the clarification, perpetuating the very ambiguity the process was meant to resolve. The spec grows in size without growing in clarity, making future clarification passes harder because the agent must distinguish between authoritative and obsolete statements.
- **Correct alternative:** Apply **Minimal-Diff Insertion** and replace the ambiguous statement with the clarified version rather than appending the clarification alongside it.

#### Narrative Drift

- **Description:** The agent over-expands a clarification into explanatory prose, rationale, or implementation commentary that exceeds the scope of the original question, introducing new unvalidated assumptions or vague language into the spec.
- **Reasons to avoid:** A clarification should encode a decision, not a discussion. Narrative expansion introduces language that has not been validated by the user and may contain the agent's assumptions rather than the user's intent. It also makes the spec harder to maintain because prose paragraphs are more difficult to update atomically than bullet-point decisions. This typically occurs when the agent attempts to be "helpful" by providing context the user did not request.
- **Negative consequences:** The spec accumulates agent-generated prose that the user never explicitly approved. Vague language re-enters the spec through the back door (e.g., "this should be robust and scalable" appended as contextual framing around a clarification about database choice). Future clarification passes may flag the agent's own additions as new ambiguities, creating a self-reinforcing cycle of unnecessary clarification.
- **Correct alternative:** Apply **Minimal-Diff Insertion** and keep each clarification to a single declarative statement or constrained bullet point. If contextual framing is needed, limit it to one parenthetical clause that uses only terms the user provided in their answer.

---

7. Write the updated spec back to `FEATURE_SPEC`.

8. Report completion (after questioning loop ends or early termination):
   - Number of questions asked & answered.
   - Path to updated spec.
   - Sections touched (list names).
   - Coverage summary table listing each taxonomy category with Status: Resolved (was Partial/Missing and addressed), Deferred (exceeds question quota or better suited for planning), Clear (already sufficient), Outstanding (still Partial/Missing but low impact).
   - If any Outstanding or Deferred remain, recommend whether to proceed to `/speckit.plan` or run `/speckit.clarify` again later post-plan.
   - Suggested next command.

Behavior rules:

- If no meaningful ambiguities found (or all potential questions would be low-impact), respond: "No critical ambiguities detected worth formal clarification." and suggest proceeding.
- If spec file missing, instruct user to run `/speckit.specify` first (do not create a new spec here).
- Never exceed 5 total asked questions (clarification retries for a single question do not count as new questions).
- Avoid speculative tech stack questions unless the absence blocks functional clarity.
- Respect user early termination signals ("stop", "done", "proceed").
- If no questions asked due to full coverage, output a compact coverage summary (all categories Clear) then suggest advancing.
- If quota reached with unresolved high-impact categories remaining, explicitly flag them under Deferred with rationale.

Context for prioritization: $ARGUMENTS

## Post-Execution Checks

**Check for extension hooks (after clarification)**:
Check if `.specify/extensions.yml` exists in the project root.
- If it exists, read it and look for entries under the `hooks.after_clarify` key
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
