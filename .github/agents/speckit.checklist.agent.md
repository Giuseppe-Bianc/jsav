---
description: Generate a custom checklist for the current feature based on user requirements.
---

## Checklist Purpose: "Unit Tests for English"

**CRITICAL CONCEPT**: Checklists are **UNIT TESTS FOR REQUIREMENTS WRITING** - they validate the quality, clarity, and completeness of requirements in a given domain.

**NOT for verification/testing**:

- ❌ NOT "Verify the button clicks correctly"
- ❌ NOT "Test error handling works"
- ❌ NOT "Confirm the API returns 200"
- ❌ NOT checking if code/implementation matches the spec

**FOR requirements quality validation**:

- ✅ "Are visual hierarchy requirements defined for all card types?" (completeness)
- ✅ "Is 'prominent display' quantified with specific sizing/positioning?" (clarity)
- ✅ "Are hover state requirements consistent across all interactive elements?" (consistency)
- ✅ "Are accessibility requirements defined for keyboard navigation?" (coverage)
- ✅ "Does the spec define what happens when logo image fails to load?" (edge cases)

**Metaphor**: If your spec is code written in English, the checklist is its unit test suite. You're testing whether the requirements are well-written, complete, unambiguous, and ready for implementation - NOT whether the implementation works.

## User Input

```text
$ARGUMENTS
```

You **MUST** consider the user input before proceeding (if not empty).

## Pre-Execution Checks

**Check for extension hooks (before checklist generation)**:
- Check if `.specify/extensions.yml` exists in the project root.
- If it exists, read it and look for entries under the `hooks.before_checklist` key
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

    Wait for the result of the hook command before proceeding to the Execution Steps.
    ```
- If no hooks are registered or `.specify/extensions.yml` does not exist, skip silently

## Execution Steps

1. **Setup**: Run `pwsh -ExecutionPolicy Bypass -File .specify/scripts/powershell/check-prerequisites.ps1 -Json` from repo root and parse JSON for FEATURE_DIR and AVAILABLE_DOCS list.
   - All file paths must be absolute.
   - For single quotes in args like "I'm Groot", use escape syntax: e.g 'I'\''m Groot' (or double-quote if possible: "I'm Groot").

2. **Clarify intent (dynamic)**: Derive up to THREE initial contextual clarifying questions (no pre-baked catalog). They MUST:
   - Be generated from the user's phrasing + extracted signals from spec/plan/tasks
   - Only ask about information that materially changes checklist content
   - Be skipped individually if already unambiguous in `$ARGUMENTS`
   - Prefer precision over breadth

   Generation algorithm:
   1. Extract signals: feature domain keywords (e.g., auth, latency, UX, API), risk indicators ("critical", "must", "compliance"), stakeholder hints ("QA", "review", "security team"), and explicit deliverables ("a11y", "rollback", "contracts").
   2. Cluster signals into candidate focus areas (max 4) ranked by relevance.
   3. Identify probable audience & timing (author, reviewer, QA, release) if not explicit.
   4. Detect missing dimensions: scope breadth, depth/rigor, risk emphasis, exclusion boundaries, measurable acceptance criteria.
   5. Formulate questions chosen from these archetypes:
      - Scope refinement (e.g., "Should this include integration touchpoints with X and Y or stay limited to local module correctness?")
      - Risk prioritization (e.g., "Which of these potential risk areas should receive mandatory gating checks?")
      - Depth calibration (e.g., "Is this a lightweight pre-commit sanity list or a formal release gate?")
      - Audience framing (e.g., "Will this be used by the author only or peers during PR review?")
      - Boundary exclusion (e.g., "Should we explicitly exclude performance tuning items this round?")
      - Scenario class gap (e.g., "No recovery flows detected—are rollback / partial failure paths in scope?")

   Question formatting rules:
   - If presenting options, generate a compact table with columns: Option | Candidate | Why It Matters
   - Limit to A–E options maximum; omit table if a free-form answer is clearer
   - Never ask the user to restate what they already said
   - Avoid speculative categories (no hallucination). If uncertain, ask explicitly: "Confirm whether X belongs in scope."

   Defaults when interaction impossible:
   - Depth: Standard
   - Audience: Reviewer (PR) if code-related; Author otherwise
   - Focus: Top 2 relevance clusters

   Output the questions (label Q1/Q2/Q3). After answers: if ≥2 scenario classes (Alternate / Exception / Recovery / Non-Functional domain) remain unclear, you MAY ask up to TWO more targeted follow‑ups (Q4/Q5) with a one-line justification each (e.g., "Unresolved recovery path risk"). Do not exceed five total questions. Skip escalation if user explicitly declines more.

The intent clarification step determines the entire trajectory of the checklist. Questions that miss the user's actual concern produce a checklist focused on the wrong domain; questions that parrot back what the user already stated waste the limited budget and erode trust. The following guidance ensures questions are precisely targeted and grounded in observable signals.

### Patterns for Intent Clarification

#### Signal-Grounded Question Derivation

- **Objective:** Ensure every clarification question is triggered by a specific, identifiable signal in the user's input or the loaded feature documents, rather than drawn from a generic question bank.
- **Context of application:** During the generation algorithm in Step 2, when formulating the initial Q1/Q2/Q3 questions and any follow-up Q4/Q5 questions.
- **Key characteristics:** Each question can be traced back to a concrete keyword, risk indicator, stakeholder hint, or detected gap. The agent can articulate, for each question, the specific signal that prompted it. Questions without a traceable signal are discarded before presentation.
- **Operational guidance:**
  1. After extracting signals in sub-step 1, tag each signal with its source (user input phrase, spec section, plan dependency, task description).
  2. For each candidate question, write an internal annotation: "Triggered by signal: [exact keyword or gap]."
  3. If no specific signal supports a question, discard it — even if it seems generally useful.
  4. Prioritize questions triggered by multiple converging signals (e.g., user mentions "security" AND spec has an unresolved auth section) over questions triggered by a single weak signal.

#### Disambiguation Over Discovery

- **Objective:** Frame clarification questions to narrow existing ambiguity rather than to introduce new considerations the user has not raised.
- **Context of application:** When choosing between candidate questions that refine the user's stated intent versus questions that explore adjacent concerns the user did not mention.
- **Key characteristics:** Questions help the user choose between interpretations of what they already said, rather than asking whether they want to add scope. The question presupposes the user's domain and asks for precision within it.
- **Operational guidance:**
  1. Before presenting a question, verify it helps resolve an ambiguity in the user's `$ARGUMENTS` or the feature documents — not introduce a new topic.
  2. Prefer "Which of these interpretations matches your intent?" over "Have you considered X?"
  3. If a potential concern is entirely absent from both user input and feature documents, note it internally for the Step 7 report rather than spending a question on it.
  4. Limit follow-up questions (Q4/Q5) strictly to unresolved scenario classes, as specified — do not use them to expand scope.

### Anti-Patterns for Intent Clarification

#### Template-Driven Questioning

- **Description:** The agent selects questions from the archetype list (scope refinement, risk prioritization, depth calibration, etc.) mechanically, asking one from each archetype regardless of whether the user's input already resolves that dimension.
- **Reasons to avoid:** The archetype list is a menu of question types, not a checklist to exhaust. Users who provide clear, detailed `$ARGUMENTS` (e.g., "lightweight security review checklist for the auth module, used by author pre-commit") have already answered scope, depth, audience, and domain. Asking again wastes budget and signals that the agent did not read the input. This typically occurs when the agent treats the archetypes as mandatory categories rather than conditional tools.
- **Negative consequences:** The user receives questions they already answered, creating friction and distrust. The limited three-question budget is consumed by redundant questions, leaving genuine ambiguities (such as which scenario classes to cover) unresolved. The resulting checklist may still miss the user's actual concern because the meaningful question was never asked.
- **Correct alternative:** Apply **Signal-Grounded Question Derivation** to generate questions only when a specific signal indicates unresolved ambiguity, and skip any archetype the user's input already satisfies.

#### Scope-Expanding Interrogation

- **Description:** The agent uses clarification questions to introduce concerns the user did not raise and the feature documents do not imply, effectively expanding the checklist's scope under the guise of clarification (e.g., asking about compliance requirements when neither the user nor the spec mentions regulatory constraints).
- **Reasons to avoid:** The clarification step exists to sharpen focus, not to broaden it. Introducing new concerns shifts the cognitive burden to the user, who must now evaluate whether an unfamiliar dimension is relevant. This often occurs when the agent defaults to "what could go wrong" thinking rather than "what did the user ask for" thinking.
- **Negative consequences:** The user is pressured to make decisions about topics they did not intend to address, potentially derailing the checklist's focus. If the user accepts an agent-introduced concern to avoid friction, the resulting checklist covers a scope the user never wanted, reducing its practical value. Trust in the workflow degrades because the tool appears to impose its own agenda.
- **Correct alternative:** Apply **Disambiguation Over Discovery** to keep questions focused on refining the user's stated intent. Surface genuinely unaddressed concerns in the Step 7 completion report as recommendations, not as blocking clarification questions.

---

3. **Understand user request**: Combine `$ARGUMENTS` + clarifying answers:
   - Derive checklist theme (e.g., security, review, deploy, ux)
   - Consolidate explicit must-have items mentioned by user
   - Map focus selections to category scaffolding
   - Infer any missing context from spec/plan/tasks (do NOT hallucinate)

4. **Load feature context**: Read from FEATURE_DIR:
   - spec.md: Feature requirements and scope
   - plan.md (if exists): Technical details, dependencies
   - tasks.md (if exists): Implementation tasks

   **Context Loading Strategy**:
   - Load only necessary portions relevant to active focus areas (avoid full-file dumping)
   - Prefer summarizing long sections into concise scenario/requirement bullets
   - Use progressive disclosure: add follow-on retrieval only if gaps detected
   - If source docs are large, generate interim summary items instead of embedding raw text

The quality of the loaded context directly determines whether checklist items can be accurately anchored to spec sections or correctly flagged as gaps. Over-loading buries relevant content in noise; under-loading causes false gap assertions. The following guidance governs reliable context extraction.

### Patterns for Context Loading

#### Focus-Area Scoped Extraction

- **Objective:** Load only the spec, plan, and task content that intersects with the checklist's identified focus areas, producing concise requirement bullets rather than raw document text.
- **Context of application:** During Step 4, after the checklist theme and focus areas have been determined in Step 3.
- **Key characteristics:** The agent reads each source document through the lens of the established focus areas, extracting only sections, bullets, and statements that are relevant to the checklist domain. Irrelevant sections are noted as present but not ingested in full. Extracted content is reformulated as terse requirement statements suitable for comparison against checklist items.
- **Operational guidance:**
  1. Before loading, list the focus areas and category scaffolding established in Step 3.
  2. For each source document, scan headings and identify sections that intersect with the focus areas.
  3. Extract relevant sections as concise bullet summaries (one bullet per requirement or constraint), discarding narrative prose.
  4. For sections outside the focus areas, record only the heading and a one-line note (e.g., "Performance section present — 4 bullets — not in scope for this checklist") to support traceability without overloading context.

#### Gap-Detection Pre-Scan

- **Objective:** Before generating checklist items, identify which expected topics are absent from the loaded context so that gap markers are based on verified absences, not assumptions.
- **Context of application:** After extracting focus-area content in Step 4, before beginning checklist generation in Step 5.
- **Key characteristics:** The agent explicitly checks the loaded context against the expected content for the checklist's domain. A gap is recorded only when the agent has confirmed that no section, bullet, or passing reference in the source documents addresses the topic. This prevents false gap assertions that waste checklist slots on requirements that are actually present but located in unexpected sections.
- **Operational guidance:**
  1. For each category in the checklist scaffolding, list the topics that a well-specified feature would address.
  2. Search the loaded context for each topic — check not only expected section headings but also inline mentions, footnotes, and cross-references.
  3. Mark each topic as "present" (with location), "partially present" (with what's missing), or "absent" (confirmed gap).
  4. Carry this gap map forward into Step 5 to ensure `[Gap]` markers are grounded in verified absences.

### Anti-Patterns for Context Loading

#### Full-Document Ingestion

- **Description:** The agent loads the entirety of spec.md, plan.md, and tasks.md verbatim into its working context, regardless of the checklist's focus areas, flooding the generation step with irrelevant content.
- **Reasons to avoid:** Large feature documents often contain sections irrelevant to the checklist domain (e.g., loading the full data model when generating a UX requirements checklist). Ingesting everything dilutes attention, increases the chance of generating off-topic checklist items, and consumes context window capacity that is needed for careful item authoring. This typically occurs when the agent treats "thoroughness" as loading everything rather than loading the right things.
- **Negative consequences:** Checklist items reference spec sections outside the declared focus area, confusing the user. The agent may generate items about data modeling in a UX checklist simply because the data model section was loaded and available. Context window pressure causes the agent to truncate or rush the actual generation step.
- **Correct alternative:** Apply **Focus-Area Scoped Extraction** to load only content that intersects with the established checklist theme and focus areas.

#### Assumed-Absence Gap Marking

- **Description:** The agent marks a topic as `[Gap]` based on the absence of a dedicated section heading, without checking whether the topic is addressed inline, in a different section, or under an alternative heading.
- **Reasons to avoid:** Spec authors frequently address topics in unexpected locations — security constraints might appear in a "Constraints" section rather than a dedicated "Security" section; error handling might be documented inline within each functional requirement rather than in a standalone section. Marking these as gaps when they are actually present produces a checklist that flags phantom problems, wasting the reviewer's time and undermining trust in the checklist's accuracy.
- **Negative consequences:** The checklist reports false gaps that the spec actually addresses, causing the user to question the tool's reliability. Reviewers who follow up on false gaps waste effort re-reading the spec to confirm the requirement exists. Repeated false positives train users to ignore `[Gap]` markers, defeating the checklist's purpose.
- **Correct alternative:** Apply **Gap-Detection Pre-Scan** to verify that a topic is genuinely absent from all source documents before asserting it as a gap.

---

5. **Generate checklist** - Create "Unit Tests for Requirements":
   - Create `FEATURE_DIR/checklists/` directory if it doesn't exist
   - Generate unique checklist filename:
     - Use short, descriptive name based on domain (e.g., `ux.md`, `api.md`, `security.md`)
     - Format: `[domain].md`
   - File handling behavior:
     - If file does NOT exist: Create new file and number items starting from CHK001
     - If file exists: Append new items to existing file, continuing from the last CHK ID (e.g., if last item is CHK015, start new items at CHK016)
   - Never delete or replace existing checklist content - always preserve and append

   **CORE PRINCIPLE - Test the Requirements, Not the Implementation**:
   Every checklist item MUST evaluate the REQUIREMENTS THEMSELVES for:
   - **Completeness**: Are all necessary requirements present?
   - **Clarity**: Are requirements unambiguous and specific?
   - **Consistency**: Do requirements align with each other?
   - **Measurability**: Can requirements be objectively verified?
   - **Coverage**: Are all scenarios/edge cases addressed?

   **Category Structure** - Group items by requirement quality dimensions:
   - **Requirement Completeness** (Are all necessary requirements documented?)
   - **Requirement Clarity** (Are requirements specific and unambiguous?)
   - **Requirement Consistency** (Do requirements align without conflicts?)
   - **Acceptance Criteria Quality** (Are success criteria measurable?)
   - **Scenario Coverage** (Are all flows/cases addressed?)
   - **Edge Case Coverage** (Are boundary conditions defined?)
   - **Non-Functional Requirements** (Performance, Security, Accessibility, etc. - are they specified?)
   - **Dependencies & Assumptions** (Are they documented and validated?)
   - **Ambiguities & Conflicts** (What needs clarification?)

   **HOW TO WRITE CHECKLIST ITEMS - "Unit Tests for English"**:

   ❌ **WRONG** (Testing implementation):
   - "Verify landing page displays 3 episode cards"
   - "Test hover states work on desktop"
   - "Confirm logo click navigates home"

   ✅ **CORRECT** (Testing requirements quality):
   - "Are the exact number and layout of featured episodes specified?" [Completeness]
   - "Is 'prominent display' quantified with specific sizing/positioning?" [Clarity]
   - "Are hover state requirements consistent across all interactive elements?" [Consistency]
   - "Are keyboard navigation requirements defined for all interactive UI?" [Coverage]
   - "Is the fallback behavior specified when logo image fails to load?" [Edge Cases]
   - "Are loading states defined for asynchronous episode data?" [Completeness]
   - "Does the spec define visual hierarchy for competing UI elements?" [Clarity]

   **ITEM STRUCTURE**:
   Each item should follow this pattern:
   - Question format asking about requirement quality
   - Focus on what's WRITTEN (or not written) in the spec/plan
   - Include quality dimension in brackets [Completeness/Clarity/Consistency/etc.]
   - Reference spec section `[Spec §X.Y]` when checking existing requirements
   - Use `[Gap]` marker when checking for missing requirements

   **EXAMPLES BY QUALITY DIMENSION**:

   Completeness:
   - "Are error handling requirements defined for all API failure modes? [Gap]"
   - "Are accessibility requirements specified for all interactive elements? [Completeness]"
   - "Are mobile breakpoint requirements defined for responsive layouts? [Gap]"

   Clarity:
   - "Is 'fast loading' quantified with specific timing thresholds? [Clarity, Spec §NFR-2]"
   - "Are 'related episodes' selection criteria explicitly defined? [Clarity, Spec §FR-5]"
   - "Is 'prominent' defined with measurable visual properties? [Ambiguity, Spec §FR-4]"

   Consistency:
   - "Do navigation requirements align across all pages? [Consistency, Spec §FR-10]"
   - "Are card component requirements consistent between landing and detail pages? [Consistency]"

   Coverage:
   - "Are requirements defined for zero-state scenarios (no episodes)? [Coverage, Edge Case]"
   - "Are concurrent user interaction scenarios addressed? [Coverage, Gap]"
   - "Are requirements specified for partial data loading failures? [Coverage, Exception Flow]"

   Measurability:
   - "Are visual hierarchy requirements measurable/testable? [Acceptance Criteria, Spec §FR-1]"
   - "Can 'balanced visual weight' be objectively verified? [Measurability, Spec §FR-2]"

   **Scenario Classification & Coverage** (Requirements Quality Focus):
   - Check if requirements exist for: Primary, Alternate, Exception/Error, Recovery, Non-Functional scenarios
   - For each scenario class, ask: "Are [scenario type] requirements complete, clear, and consistent?"
   - If scenario class missing: "Are [scenario type] requirements intentionally excluded or missing? [Gap]"
   - Include resilience/rollback when state mutation occurs: "Are rollback requirements defined for migration failures? [Gap]"

   **Traceability Requirements**:
   - MINIMUM: ≥80% of items MUST include at least one traceability reference
   - Each item should reference: spec section `[Spec §X.Y]`, or use markers: `[Gap]`, `[Ambiguity]`, `[Conflict]`, `[Assumption]`
   - If no ID system exists: "Is a requirement & acceptance criteria ID scheme established? [Traceability]"

   **Surface & Resolve Issues** (Requirements Quality Problems):
   Ask questions about the requirements themselves:
   - Ambiguities: "Is the term 'fast' quantified with specific metrics? [Ambiguity, Spec §NFR-1]"
   - Conflicts: "Do navigation requirements conflict between §FR-10 and §FR-10a? [Conflict]"
   - Assumptions: "Is the assumption of 'always available podcast API' validated? [Assumption]"
   - Dependencies: "Are external podcast API requirements documented? [Dependency, Gap]"
   - Missing definitions: "Is 'visual hierarchy' defined with measurable criteria? [Gap]"

   **Content Consolidation**:
   - Soft cap: If raw candidate items > 40, prioritize by risk/impact
   - Merge near-duplicates checking the same requirement aspect
   - If >5 low-impact edge cases, create one item: "Are edge cases X, Y, Z addressed in requirements? [Coverage]"

   **🚫 ABSOLUTELY PROHIBITED** - These make it an implementation test, not a requirements test:
   - ❌ Any item starting with "Verify", "Test", "Confirm", "Check" + implementation behavior
   - ❌ References to code execution, user actions, system behavior
   - ❌ "Displays correctly", "works properly", "functions as expected"
   - ❌ "Click", "navigate", "render", "load", "execute"
   - ❌ Test cases, test plans, QA procedures
   - ❌ Implementation details (frameworks, APIs, algorithms)

   **✅ REQUIRED PATTERNS** - These test requirements quality:
   - ✅ "Are [requirement type] defined/specified/documented for [scenario]?"
   - ✅ "Is [vague term] quantified/clarified with specific criteria?"
   - ✅ "Are requirements consistent between [section A] and [section B]?"
   - ✅ "Can [requirement] be objectively measured/verified?"
   - ✅ "Are [edge cases/scenarios] addressed in requirements?"
   - ✅ "Does the spec define [missing aspect]?"

The item-level writing rules above define what a correct checklist item looks like. The following process-level guidance addresses how to generate a high-quality checklist as a whole — ensuring items are accurately grounded, appropriately distributed across quality dimensions, and collectively useful rather than individually correct but strategically redundant.

### Patterns for Checklist Generation

#### Requirement-Interrogation Framing

- **Objective:** Maintain the "unit tests for English" mindset throughout generation by treating the spec as the system under test and each checklist item as an assertion about the spec's quality, not the feature's behavior.
- **Context of application:** During the authoring of every individual checklist item in Step 5, as a persistent cognitive frame that governs phrasing, focus, and scope.
- **Key characteristics:** Each item's subject is the spec document itself — what it says, what it omits, whether its language is precise. The item never shifts its subject to the system being specified. The agent mentally substitutes "the spec" for "the system" before finalizing any item.
- **Operational guidance:**
  1. Before writing each item, complete the sentence: "This item tests whether the spec [does/does not] ___."
  2. If the completion describes system behavior (e.g., "handles errors gracefully"), reframe to spec quality (e.g., "defines error handling requirements for all identified failure modes").
  3. After drafting a batch of items, re-read each one and ask: "Could a reviewer answer this item by reading only the spec, without running any code?" If no, rephrase.
  4. When tempted to use verbs like "verify," "test," or "confirm," replace with "Are requirements defined for," "Is [term] quantified with," or "Does the spec specify."

#### Traceability-Anchored Authoring

- **Objective:** Anchor every checklist item to a verifiable location in the source documents so that reviewers can immediately assess each item without searching the entire spec.
- **Context of application:** When writing each checklist item and assigning its traceability markers (`[Spec §X.Y]`, `[Gap]`, `[Ambiguity]`, etc.).
- **Key characteristics:** Items checking existing requirements cite the specific section and, where possible, the specific bullet or requirement ID. Items asserting gaps reference the section where the content would be expected to appear, confirming its absence. No item is emitted without at least one traceability marker.
- **Operational guidance:**
  1. For items about existing requirements, locate the exact spec section during drafting and record its identifier in the item.
  2. For gap items, record where in the spec the missing content would logically belong (e.g., `[Gap — expected in §NFR]`) rather than using a bare `[Gap]` tag.
  3. Cross-reference the gap map produced during the **Gap-Detection Pre-Scan** (Step 4) to confirm the absence is verified, not assumed.
  4. After completing all items, count traceability references — if fewer than 80% of items have a reference, revisit unanchored items and either add references or justify their inclusion.

#### Quality-Dimension Distribution

- **Objective:** Distribute checklist items across multiple quality dimensions (completeness, clarity, consistency, measurability, coverage) to prevent overconcentration in a single dimension that creates blind spots in others.
- **Context of application:** After generating the initial set of candidate items, during the content consolidation phase of Step 5.
- **Key characteristics:** The agent reviews the distribution of items across quality dimensions and rebalances if any single dimension accounts for more than 40% of total items or if any relevant dimension has zero items. Rebalancing involves generating new items for underrepresented dimensions or consolidating redundant items in overrepresented ones.
- **Operational guidance:**
  1. After generating all candidate items, tally items per quality dimension using their bracketed tags.
  2. If any dimension exceeds 40% of total items, review for near-duplicates or low-impact items that can be merged or removed.
  3. If any applicable dimension has zero items, generate at least one item — reviewing the gap map and loaded context for overlooked concerns in that dimension.
  4. Prioritize coverage of dimensions that are most relevant to the checklist's declared focus areas, but do not leave any applicable dimension entirely empty.

### Anti-Patterns for Checklist Generation

#### Implementation Verification Drift

- **Description:** The agent gradually shifts from interrogating the spec's quality to describing expected system behavior, producing items that sound like test cases rather than requirements audits. Items begin with "Are requirements defined for..." early in the checklist but degrade to "Verify that the system handles..." as the agent progresses through less familiar territory.
- **Reasons to avoid:** This drift is the single most common failure mode for this workflow. It occurs because the agent's training data contains far more test cases than requirements audits, and the default generation tendency is to describe what a system should do. The drift is typically gradual — early items are correctly framed, but fatigue or domain unfamiliarity causes later items to revert to implementation-testing language.
- **Negative consequences:** The checklist becomes a hybrid document — part requirements audit, part test plan — that serves neither purpose well. Reviewers using it cannot tell whether they should be reading the spec or running the system. The core value proposition ("unit tests for English") is undermined, and the checklist is functionally indistinguishable from a generic QA checklist.
- **Correct alternative:** Apply **Requirement-Interrogation Framing** for every item, including a post-generation re-read specifically scanning for behavioral verbs and system-as-subject phrasing.

#### Unanchored Gap Assertion

- **Description:** The agent marks items with `[Gap]` based on the assumption that a topic is missing, without verifying against the loaded context that the spec genuinely does not address the topic anywhere — including in unexpected sections, inline mentions, or cross-references.
- **Reasons to avoid:** Specs are authored by humans who do not always place content under the heading an agent expects. Security constraints may appear in a "Constraints" section; error handling may be documented inline within each functional requirement rather than in a standalone section. Asserting a gap without verification produces false positives that waste reviewer time and erode trust in the checklist. This occurs when the agent relies on heading-level scanning rather than content-level scanning during context loading.
- **Negative consequences:** Reviewers investigate false gaps, find the requirement already exists, and begin distrusting all `[Gap]` markers — including the legitimate ones. The checklist's value as a diagnostic tool is destroyed because its signal-to-noise ratio is too low. Subsequent checklist runs may re-flag the same false gaps, creating a pattern of wasted effort.
- **Correct alternative:** Apply **Traceability-Anchored Authoring** and cross-reference the gap map from the **Gap-Detection Pre-Scan** to confirm every `[Gap]` marker reflects a verified absence, not a scanning failure.

#### Category Padding

- **Description:** The agent generates low-value or redundant items to ensure every quality dimension category contains at least several entries, prioritizing visual completeness of the checklist over the actual diagnostic value of each item.
- **Reasons to avoid:** A checklist with 40 items across 9 categories looks thorough but may contain 15 items that test the same underlying concern with minor phrasing variations, and 10 items about topics irrelevant to the feature's domain. This occurs when the agent treats category coverage as a goal in itself rather than as a distribution heuristic, and when it fills less relevant categories with generic items rather than leaving them appropriately sparse.
- **Negative consequences:** Reviewers experience checklist fatigue and begin checking items mechanically without engaging with them. Genuinely important items are buried among padding items. The checklist takes longer to complete without proportionally increasing the quality of the review. Over time, teams learn to skim checklists rather than use them as rigorous diagnostic tools.
- **Correct alternative:** Apply **Quality-Dimension Distribution** as a balance check, not a fill-every-slot mandate. Allow categories that are genuinely less relevant to the feature's domain to contain fewer items or to be omitted with a one-line justification (e.g., "Accessibility: not applicable — backend-only feature; no UI surface").

---

6. **Structure Reference**: Generate the checklist following the canonical template in `.specify/templates/checklist-template.md` for title, meta section, category headings, and ID formatting. If template is unavailable, use: H1 title, purpose/created meta lines, `##` category sections containing `- [ ] CHK### <requirement item>` lines with globally incrementing IDs starting at CHK001.

7. **Report**: Output full path to checklist file, item count, and summarize whether the run created a new file or appended to an existing one. Summarize:
   - Focus areas selected
   - Depth level
   - Actor/timing
   - Any explicit user-specified must-have items incorporated

**Important**: Each `/speckit.checklist` command invocation uses a short, descriptive checklist filename and either creates a new file or appends to an existing one. This allows:

- Multiple checklists of different types (e.g., `ux.md`, `test.md`, `security.md`)
- Simple, memorable filenames that indicate checklist purpose
- Easy identification and navigation in the `checklists/` folder

To avoid clutter, use descriptive types and clean up obsolete checklists when done.

## Example Checklist Types & Sample Items

**UX Requirements Quality:** `ux.md`

Sample items (testing the requirements, NOT the implementation):

- "Are visual hierarchy requirements defined with measurable criteria? [Clarity, Spec §FR-1]"
- "Is the number and positioning of UI elements explicitly specified? [Completeness, Spec §FR-1]"
- "Are interaction state requirements (hover, focus, active) consistently defined? [Consistency]"
- "Are accessibility requirements specified for all interactive elements? [Coverage, Gap]"
- "Is fallback behavior defined when images fail to load? [Edge Case, Gap]"
- "Can 'prominent display' be objectively measured? [Measurability, Spec §FR-4]"

**API Requirements Quality:** `api.md`

Sample items:

- "Are error response formats specified for all failure scenarios? [Completeness]"
- "Are rate limiting requirements quantified with specific thresholds? [Clarity]"
- "Are authentication requirements consistent across all endpoints? [Consistency]"
- "Are retry/timeout requirements defined for external dependencies? [Coverage, Gap]"
- "Is versioning strategy documented in requirements? [Gap]"

**Performance Requirements Quality:** `performance.md`

Sample items:

- "Are performance requirements quantified with specific metrics? [Clarity]"
- "Are performance targets defined for all critical user journeys? [Coverage]"
- "Are performance requirements under different load conditions specified? [Completeness]"
- "Can performance requirements be objectively measured? [Measurability]"
- "Are degradation requirements defined for high-load scenarios? [Edge Case, Gap]"

**Security Requirements Quality:** `security.md`

Sample items:

- "Are authentication requirements specified for all protected resources? [Coverage]"
- "Are data protection requirements defined for sensitive information? [Completeness]"
- "Is the threat model documented and requirements aligned to it? [Traceability]"
- "Are security requirements consistent with compliance obligations? [Consistency]"
- "Are security failure/breach response requirements defined? [Gap, Exception Flow]"

## Anti-Examples: What NOT To Do

**❌ WRONG - These test implementation, not requirements:**

```markdown
- [ ] CHK001 - Verify landing page displays 3 episode cards [Spec §FR-001]
- [ ] CHK002 - Test hover states work correctly on desktop [Spec §FR-003]
- [ ] CHK003 - Confirm logo click navigates to home page [Spec §FR-010]
- [ ] CHK004 - Check that related episodes section shows 3-5 items [Spec §FR-005]
```

**✅ CORRECT - These test requirements quality:**

```markdown
- [ ] CHK001 - Are the number and layout of featured episodes explicitly specified? [Completeness, Spec §FR-001]
- [ ] CHK002 - Are hover state requirements consistently defined for all interactive elements? [Consistency, Spec §FR-003]
- [ ] CHK003 - Are navigation requirements clear for all clickable brand elements? [Clarity, Spec §FR-010]
- [ ] CHK004 - Is the selection criteria for related episodes documented? [Gap, Spec §FR-005]
- [ ] CHK005 - Are loading state requirements defined for asynchronous episode data? [Gap]
- [ ] CHK006 - Can "visual hierarchy" requirements be objectively measured? [Measurability, Spec §FR-001]
```

**Key Differences:**

- Wrong: Tests if the system works correctly
- Correct: Tests if the requirements are written correctly
- Wrong: Verification of behavior
- Correct: Validation of requirement quality
- Wrong: "Does it do X?"
- Correct: "Is X clearly specified?"

## Post-Execution Checks

**Check for extension hooks (after checklist generation)**:
Check if `.specify/extensions.yml` exists in the project root.
- If it exists, read it and look for entries under the `hooks.after_checklist` key
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
