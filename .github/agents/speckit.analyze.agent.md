---
description: Perform a non-destructive cross-artifact consistency and quality analysis across spec.md, plan.md, and tasks.md after task generation.
---

## User Input

```text
$ARGUMENTS
```

You **MUST** consider the user input before proceeding (if not empty).

## Pre-Execution Checks

**Check for extension hooks (before analysis)**:
- Check if `.specify/extensions.yml` exists in the project root.
- If it exists, read it and look for entries under the `hooks.before_analyze` key
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

    Wait for the result of the hook command before proceeding to the Goal.
    ```

- If no hooks are registered or `.specify/extensions.yml` does not exist, skip silently

## Goal

Identify inconsistencies, duplications, ambiguities, and underspecified items across the three core artifacts (`spec.md`, `plan.md`, `tasks.md`) before implementation. This command MUST run only after `/speckit.tasks` has successfully produced a complete `tasks.md`.

## Operating Constraints

**STRICTLY READ-ONLY**: Do **not** modify any files. Output a structured analysis report. Offer an optional remediation plan (user must explicitly approve before any follow-up editing commands would be invoked manually).

**Constitution Authority**: The project constitution (`.specify/memory/constitution.md`) is **non-negotiable** within this analysis scope. Constitution conflicts are automatically CRITICAL and require adjustment of the spec, plan, or tasks—not dilution, reinterpretation, or silent ignoring of the principle. If a principle itself needs to change, that must occur in a separate, explicit constitution update outside `/speckit.analyze`.

## Execution Steps

### 1. Initialize Analysis Context

Run `pwsh -ExecutionPolicy Bypass -File .specify/scripts/powershell/check-prerequisites.ps1 -Json -RequireTasks -IncludeTasks` once from repo root and parse JSON for FEATURE_DIR and AVAILABLE_DOCS. Derive absolute paths:

- SPEC = FEATURE_DIR/spec.md
- PLAN = FEATURE_DIR/plan.md
- TASKS = FEATURE_DIR/tasks.md

Abort with an error message if any required file is missing (instruct the user to run missing prerequisite command).
For single quotes in args like "I'm Groot", use escape syntax: e.g 'I'\''m Groot' (or double-quote if possible: "I'm Groot").

### 2. Load Artifacts (Progressive Disclosure)

Load only the minimal necessary context from each artifact:

**From spec.md:**

- Overview/Context
- Functional Requirements
- Success Criteria (measurable outcomes — e.g., performance, security, availability, user success, business impact)
- User Stories
- Edge Cases (if present)

**From plan.md:**

- Architecture/stack choices
- Data Model references
- Phases
- Technical constraints

**From tasks.md:**

- Task IDs
- Descriptions
- Phase grouping
- Parallel markers [P]
- Referenced file paths

**From constitution:**

- Load `.specify/memory/constitution.md` for principle validation

### 3. Build Semantic Models

Create internal representations (do not include raw artifacts in output):

- **Requirements inventory**: For each Functional Requirement (FR-###) and Success Criterion (SC-###), record a stable key. Use the explicit FR-/SC- identifier as the primary key when present, and optionally also derive an imperative-phrase slug for readability (e.g., "User can upload file" → `user-can-upload-file`). Include only Success Criteria items that require buildable work (e.g., load-testing infrastructure, security audit tooling), and exclude post-launch outcome metrics and business KPIs (e.g., "Reduce support tickets by 50%").
- **User story/action inventory**: Discrete user actions with acceptance criteria
- **Task coverage mapping**: Map each task to one or more requirements or stories (inference by keyword / explicit reference patterns like IDs or key phrases)
- **Constitution rule set**: Extract principle names and MUST/SHOULD normative statements

The semantic models built in this step serve as the analytical foundation for each of the six detection passes. These models typically encode entities, their normalized representations, and the mappings that link tasks to their corresponding requirements. Errors in key derivation, entity normalization, or task-to-requirement mapping propagate silently, resulting in false positives, coverage gaps, and unreliable downstream metrics. This propagation occurs because each detection pass depends on the integrity of prior model outputs, causing inaccuracies to accumulate rather than remain isolated. The following guidance is designed to ensure that model construction is both accurate and reproducible. Accuracy requires precise key generation, consistent entity normalization, and verifiable mapping logic. Reproducibility requires that all transformations are deterministic, documented, and traceable across processing stages. The guidance therefore outlines validation procedures, normalization standards, and mapping verification steps necessary to maintain model integrity.

#### Patterns for Semantic Model Construction

##### Deterministic Key Derivation

- **Objective:** Produce stable, reproducible requirement keys so that findings remain consistent across reruns and can be tracked over time.
- **Context of application:** When generating the slug-based stable key for each requirement in the requirements inventory (e.g., "User can upload file" → `user-can-upload-file`).
- **Key characteristics:** The key derivation algorithm is purely mechanical — based on the requirement's imperative phrase text, not on its position in the document, surrounding context, or the agent's interpretation of intent. Two runs against the same unchanged spec produce identical keys in the same order.
- **Operational guidance:**
  1. Extract the imperative phrase verbatim from the requirement text before transforming it.
  2. Apply a fixed normalization sequence: lowercase → strip leading articles ("a", "an", "the") → replace whitespace and punctuation with hyphens → collapse consecutive hyphens.
  3. If two requirements produce the same key after normalization, append a numeric suffix (`-2`, `-3`) based on order of appearance — never based on semantic judgment.
  4. After generating all keys, verify uniqueness: no two requirements share a key. If duplicates exist, flag them as candidate findings for Duplication Detection (Step 4A) rather than silently disambiguating.

##### Inference-Bounded Coverage Mapping

- **Objective:** Map tasks to requirements using observable textual evidence while explicitly marking the confidence level of each mapping, so that coverage metrics reflect actual traceability rather than speculative inference.
- **Context of application:** When building the task coverage mapping — linking each task in tasks.md to one or more requirements or user stories from spec.md.
- **Key characteristics:** Each mapping is grounded in a specific textual signal: an explicit requirement ID reference, a shared key phrase, or a named entity match. Mappings based on thematic similarity alone are flagged as low-confidence and excluded from coverage percentage calculations. The agent records what evidence supports each mapping.
- **Operational guidance:**
  1. First pass — exact matches: link tasks that explicitly reference requirement IDs, user story identifiers, or quoted requirement phrases.
  2. Second pass — key phrase matches: link tasks whose descriptions contain distinctive multi-word phrases (≥3 content words) that appear verbatim in a requirement.
  3. Third pass — entity matches: link tasks that reference the same named data entity, API endpoint, or component as a requirement, where the entity name is sufficiently specific (not generic terms like "user," "data," "system").
  4. For any remaining unmapped tasks or requirements, record them as unmapped rather than forcing a speculative connection. These feed directly into Coverage Gaps detection (Step 4E).

#### Anti-Patterns for Semantic Model Construction

##### Volatile Key Generation

- **Description:** The agent generates requirement keys using features that vary across runs — such as document position, surrounding context, inferred synonyms, or the agent's paraphrased summary of the requirement rather than its verbatim text.
- **Reasons to avoid:** Volatile keys violate the operating principle that "rerunning without changes should produce consistent IDs and counts." When keys change between runs, findings from a previous analysis cannot be compared to findings from a current one. Coverage metrics become non-reproducible, and users cannot track whether a previously flagged issue has been resolved. This typically occurs when the agent treats key generation as a semantic task (understanding the requirement) rather than a mechanical task (transforming fixed text).
- **Negative consequences:** Finding IDs shift between runs, making the report unreliable for tracking remediation. Coverage percentages fluctuate without any actual change to artifacts. Users lose the ability to diff analysis reports across iterations, which is the primary way they verify that fixes addressed the right issues.
- **Correct alternative:** Apply **Deterministic Key Derivation** to produce keys from a fixed, mechanical transformation of verbatim requirement text.

##### Uncalibrated Coverage Inference

- **Description:** The agent maps tasks to requirements based on broad thematic similarity or domain-level association (e.g., mapping a "set up database" task to a "user can search products" requirement because both involve data), without requiring specific textual evidence for the connection.
- **Reasons to avoid:** Over-inferred mappings inflate coverage metrics, masking genuine gaps. Under-inferred mappings (the opposite failure — requiring exact ID matches only) deflate coverage and generate false gap findings. Both failures stem from the same root cause: the agent does not calibrate its inference threshold or distinguish between evidence-based and speculative connections. Over-inference is more common because the agent's training biases it toward finding connections rather than reporting absences.
- **Negative consequences:** Inflated coverage gives false confidence that all requirements are addressed by tasks, allowing gaps to reach implementation undetected. Deflated coverage floods the report with spurious gap findings, causing the user to distrust or ignore legitimate gaps. In both cases, the coverage percentage — a key metric in the report — becomes unreliable.
- **Correct alternative:** Apply **Inference-Bounded Coverage Mapping** with explicit confidence tiers and exclude low-confidence mappings from coverage percentage calculations.

### 4. Detection Passes (Token-Efficient Analysis)

Prioritize findings with the highest informational relevance and evidential strength. Limit the output to a maximum of 50 findings. Any additional findings must be consolidated into a clearly labeled overflow summary that preserves their meaning without full elaboration. Define high-signal findings as those that are directly relevant to the analytical objective, supported by evidence, and materially significant to interpretation or decision-making. The overflow summary must contain all excluded findings organized in a structured form that allows later retrieval or review.

#### A. Duplication Detection

- Identify near-duplicate requirements
- Mark lower-quality phrasing for consolidation

#### B. Ambiguity Detection

- Flag vague adjectives lacking measurable criteria, including but not limited to:
    - **Performance**: fast, scalable, efficient, performant, responsive, optimized, lightweight, low-latency, real-time
    - **Reliability**: robust, reliable, stable, resilient, fault-tolerant, highly-available
    - **Security**: secure, safe, protected, hardened
    - **Usability**: intuitive, user-friendly, easy-to-use, simple, clean, seamless, polished
    - **Maintainability**: maintainable, extensible, flexible, modular, well-structured
    - **Quality**: high-quality, production-ready, enterprise-grade, world-class, best-in-class
    - **Comparatives without baseline**: better, faster, improved, enhanced, superior
    - Require numeric thresholds, SLOs, or testable acceptance criteria for each flagged term

- Flag unresolved placeholders (case-insensitive detection): BUG, FIXME, HACK, NOTE, OPTIMIZE, TODO, TBD, TKTK, WIP, XXX, ???, `<placeholder>`, etc.

#### C. Underspecification

- Requirements with verbs but missing object or measurable outcome
- User stories missing acceptance criteria alignment
- Tasks referencing files or components not defined in spec/plan

#### D. Constitution Alignment

- Any requirement or plan element conflicting with a MUST principle
- Missing mandated sections or quality gates from constitution

#### E. Coverage Gaps

- Requirements with zero associated tasks
- Tasks with no mapped requirement/story
- Success Criteria requiring buildable work (performance, security, availability) not reflected in tasks

#### F. Inconsistency

- Terminology drift (same concept named differently across files)
- Data entities referenced in plan but absent in spec (or vice versa)
- Task ordering contradictions (e.g., integration tasks before foundational setup tasks without dependency note)
- Conflicting requirements (e.g., one requires Next.js while other specifies Vue)

The six detection passes above define *what* to look for. The following guidance governs *how* to conduct each pass so that findings are evidence-based, non-redundant, and efficiently allocated within the 50-finding cap.

#### Patterns for Detection Pass Execution

##### Evidence-Cited Findings

- **Objective:** Ensure every reported finding references specific, verifiable text in a specific artifact location, so that the user can confirm the finding without re-reading entire documents.
- **Context of application:** When formulating each individual finding during any of the six detection passes (A through F).
- **Key characteristics:** Each finding includes the artifact filename, the line range or section heading where the issue occurs, and a direct quotation or precise paraphrase of the problematic text. No finding relies on the agent's general impression of the artifact. If a finding cannot cite specific text, it is discarded.
- **Operational guidance:**
  1. For each candidate finding, record the exact source location (file and line range or section identifier) before drafting the summary.
  2. Include a brief quotation (≤15 words) of the specific text that triggers the finding — for ambiguity findings, quote the vague term in context; for inconsistency findings, quote both conflicting statements.
  3. If a finding involves an absence (e.g., a coverage gap), cite the section where the content would be expected and confirm its absence explicitly (e.g., "spec.md §Non-Functional Requirements contains no performance targets").
  4. Before finalizing the findings list, discard any finding whose "Location(s)" cell would require "general" or "throughout" rather than a specific reference.

##### Cross-Pass Deduplication

- **Objective:** After completing six predefined detection passes, merge findings that refer to the same underlying issue based on defined equivalence criteria and that originate from distinct analytical perspectives. Then identify compound issues as higher-order aggregated patterns that emerge across multiple detection passes, where a single root cause produces linked or repeated findings across different analytical dimensions.
- **Context of application:** As a consolidation step after all individual passes (A through F) have generated their candidate findings, before applying the 50-finding cap.
- **Key characteristics:** Findings from different passes that reference the same artifact location or the same requirement key are compared. If they describe the same root issue (e.g., an ambiguous requirement flagged by both Ambiguity Detection and Underspecification), they are merged into a single finding with the higher severity and references to both detection categories. Compound issues that only become visible through cross-pass correlation (e.g., a terminology drift that causes a false coverage gap) are surfaced as new findings.
- **Operational guidance:**
  1. After all passes complete, sort all candidate findings by artifact location.
  2. For adjacent findings referencing the same location or requirement key, determine whether they describe the same root issue. If yes, merge into one finding and list both detection categories in the "Category" column (e.g., "Ambiguity / Underspecification").
  3. Scan for correlation patterns: does a terminology drift finding (F) explain a coverage gap finding (E)? Does a duplication finding (A) relate to an inconsistency finding (F)? Promote these correlations to compound findings.
  4. After deduplication, if remaining findings exceed 50, rank by severity and then by impact breadth (findings affecting multiple artifacts rank higher). Aggregate overflow findings into a summary count by category.

#### Anti-Patterns for Detection Pass Execution

##### Fabricated Finding Insertion

- **Description:** The agent reports issues that do not actually exist in the loaded artifacts — inventing plausible-sounding line references, asserting terminology conflicts between terms that are not used in the documents, or flagging ambiguity in language that is already quantified with specific criteria.
- **Reasons to avoid:** This is the highest-risk failure mode for an LLM-based analysis agent. The agent's generative capability allows it to produce findings that are structurally well-formed and categorically appropriate but factually wrong. Users initially trust the report format and may act on fabricated findings, introducing unnecessary changes. This typically occurs when the agent has incomplete context (due to progressive disclosure loading) and fills gaps with plausible inference rather than reporting uncertainty.
- **Negative consequences:** Users modify artifacts to fix non-existent problems, potentially introducing real issues. Trust in the analysis workflow collapses once a fabricated finding is discovered — subsequent legitimate findings are doubted. The read-only safety guarantee is preserved (the agent doesn't modify files), but the remediation actions the user takes based on false findings are effectively agent-induced damage.
- **Correct alternative:** Apply **Evidence-Cited Findings** and discard any finding that cannot be anchored to a specific, quotable passage in a loaded artifact. When context is insufficient to confirm an issue, report it as a "potential finding — requires manual verification" rather than asserting it as fact.

##### Cap-Blind Enumeration

- **Description:** The agent generates findings without managing the 50-finding budget, producing many low-severity or redundant items that consume cap space and displace higher-impact findings. Alternatively, the agent treats the cap as a target to fill rather than a ceiling, padding the list with marginal findings to reach 50.
- **Reasons to avoid:** The 50-finding cap exists to keep the report actionable. If the cap is consumed by a flood of LOW-severity wording suggestions or by duplicate findings from overlapping detection passes, CRITICAL and HIGH findings may be pushed into the overflow summary where they receive less attention. This occurs when the agent runs each pass independently without tracking cumulative count, or when it prioritizes completeness over signal quality.
- **Negative consequences:** The findings table becomes a wall of low-value items that the user must triage manually, defeating the purpose of automated severity assignment. Critical issues may appear only in the overflow summary, which users typically skim. The report feels exhaustive but is not actionable — the user cannot distinguish the three issues that truly matter from the 47 that do not.
- **Correct alternative:** Apply **Cross-Pass Deduplication** to merge redundant findings before applying the cap, and rank all candidates by severity and impact breadth so that the 50 slots contain the highest-signal findings.

### 5. Severity Assignment

Use this heuristic to prioritize findings:

- **CRITICAL**: Violates a mandatory constitution rule (MUST), is missing a core spec artifact, or has a requirement with zero coverage that blocks baseline functionality.
- **HIGH**: Duplicate or conflicting requirement, ambiguous security/performance attribute, untestable acceptance criterion
- **MEDIUM**: Terminology drift, missing non-functional task coverage, underspecified edge case
- **LOW**: Style/wording improvements, minor redundancy not affecting execution order

### 6. Produce Compact Analysis Report

Output a Markdown report (no file writes) with the following structure:

## Specification Analysis Report

| ID |   Category  | Severity |    Location(s)   |            Summary           |            Recommendation            |
|----|-------------|----------|------------------|------------------------------|--------------------------------------|
| A1 | Duplication |   HIGH   | spec.md:L120-134 | Two similar requirements ... | Merge phrasing; keep clearer version |

(Add one row per finding; generate stable IDs prefixed by category initial.)

**Coverage Summary Table:**

| Requirement Key | Has Task? | Task IDs | Notes |
|-----------------|-----------|----------|-------|

**Constitution Alignment Issues:** (if any)

**Unmapped Tasks:** (if any)

**Metrics:**

- Total Requirements
- Total Tasks
- Coverage % (requirements with ≥1 task / total requirements)
    - Formula: `(count of requirements with at least one mapped task) / (count of all requirements)` × 100
    - Includes both functional and non-functional requirements in denominator
    - Format: Percentage rounded to 1 decimal place (e.g., `75.0%`)
    - Edge case: If total requirements = 0, display `N/A` and flag as CRITICAL issue
    - Must match count from Coverage Summary Table where "Has Task?" = YES
- Ambiguity Count
- Duplication Count
- Critical Issues Count

The report is the sole deliverable of this workflow. Its utility depends on whether severity assignments accurately reflect the documented heuristic and whether recommendations are specific enough for the user to act on without re-reading the artifacts or re-running the analysis. The following guidance addresses severity calibration and recommendation quality.

#### Patterns for Report Quality

##### Definition-Anchored Severity Classification

- **Objective:** Assign severity to each finding by mechanically matching it against the documented severity definitions in Step 5, rather than relying on intuitive judgment about how "bad" an issue feels.
- **Context of application:** When assigning the Severity value for each finding row in the analysis report.
- **Key characteristics:** Each severity assignment can be justified by citing the specific clause in the severity heuristic that applies. The agent does not interpolate between severity levels or create hybrid justifications. If a finding does not clearly match any severity definition, it defaults to the lowest applicable level rather than being rounded up.
- **Operational guidance:**
  1. For each finding, compare it against severity definitions starting from CRITICAL and working down.
  2. Assign the first level whose definition the finding unambiguously matches. Record internally which clause matched (e.g., "CRITICAL — requirement with zero coverage that blocks baseline functionality").
  3. If the finding partially matches two levels (e.g., it involves terminology drift — MEDIUM — but the drift causes a conflicting requirement — HIGH), assign the higher level and note the compounding factor in the Summary column.
  4. After assigning all severities, review the distribution. If more than 50% of findings are CRITICAL or HIGH, re-examine each one against the definitions — over-concentration at the top usually indicates the agent is interpreting definitions too broadly.

##### Actionable Recommendation Specificity

- **Objective:** Ensure each recommendation in the report identifies the specific artifact, section, and change needed so the user can act on it directly.
- **Context of application:** When writing the Recommendation column for each finding row.
- **Key characteristics:** Every recommendation names the file to edit, the section or line to change, and the nature of the change (add, remove, replace, merge, quantify). The user can evaluate whether a recommendation has been implemented by re-reading only the referenced location, without scanning the full artifact.
- **Operational guidance:**
  1. Structure each recommendation as: "[Action verb] in [file]:[section/line] — [specific change]" (e.g., "Replace 'fast' with a latency target in spec.md §NFR-2: specify p95 response time threshold").
  2. For duplication findings, identify which version to keep and which to remove, citing the criterion (e.g., "Keep spec.md:L45 (includes measurable criteria); remove spec.md:L120 (vague restatement)").
  3. For gap findings, specify what content to add and where (e.g., "Add performance task to tasks.md Phase 2 covering spec requirement `user-search-returns-results`").
  4. Avoid open-ended directives: never write "consider improving," "review for clarity," or "ensure alignment" without specifying exactly what to change.

#### Anti-Patterns for Report Quality

##### Severity Inflation

- **Description:** The agent assigns CRITICAL or HIGH severity to the majority of findings, either to signal thoroughness, because it conflates "could cause a problem" with "blocks baseline functionality," or because it defaults to the highest applicable level when a finding sits between two definitions.
- **Reasons to avoid:** Severity inflation destroys the prioritization signal that is the report's primary value. When everything is critical, nothing is — the user cannot distinguish the three findings that genuinely block implementation from the twelve that are merely suboptimal. This typically occurs when the agent interprets severity definitions expansively (e.g., treating any ambiguous term as "blocking baseline functionality") or when it feels pressure to justify the analysis effort by elevating findings.
- **Negative consequences:** Users either treat the entire report as alarmist and deprioritize remediation, or they attempt to fix all CRITICAL/HIGH findings before proceeding and waste effort on issues that do not actually block implementation. The severity distribution loses its diagnostic value — a healthy report has a pyramid shape (few CRITICAL, more LOW), not a top-heavy bar.
- **Correct alternative:** Apply **Definition-Anchored Severity Classification** and assign severity by mechanical matching against the documented heuristic, defaulting to the lower level when a finding does not unambiguously match the higher one.

##### Opaque Recommendation Phrasing

- **Description:** The agent writes recommendations using vague directives such as "improve clarity," "ensure consistency," "consider refining," or "review and update" without specifying which artifact, which section, or what specific change to make.
- **Reasons to avoid:** Vague recommendations transfer the analytical burden back to the user, who must re-read the artifacts to determine what "improve clarity" means in context. The agent has already performed this analysis during the detection pass — failing to encode the result in the recommendation wastes the analytical work and forces the user to repeat it. This occurs when the agent treats the recommendation column as a summary of the problem rather than a prescription for the solution.
- **Negative consequences:** Users cannot act on the report without significant additional effort. Remediation becomes a second analysis exercise rather than a targeted editing task. The report functions as a problem list rather than an action plan, reducing its practical value. Users who lack domain expertise may implement incorrect fixes because the recommendation did not specify the correct change.
- **Correct alternative:** Apply **Actionable Recommendation Specificity** to ensure every recommendation identifies the exact file, location, and change needed.

### 7. Provide Next Actions

At end of report, output a concise Next Actions block:

- If CRITICAL issues exist: Recommend resolving before `/speckit.implement`
- If only LOW/MEDIUM: User may proceed, but provide improvement suggestions
- Provide explicit command suggestions: e.g., "Run /speckit.specify with refinement", "Run /speckit.plan to adjust architecture", "Manually edit tasks.md to add coverage for 'performance-metrics'"

### 8. Offer Remediation

Ask the user: "Would you like me to suggest concrete remediation edits for the top N issues?" (Do NOT apply them automatically.)

### 9. Check for extension hooks

After reporting, check if `.specify/extensions.yml` exists in the project root.
- If it exists, read it and look for entries under the `hooks.after_analyze` key
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

## Operating Principles

### Context Efficiency

- **Minimal high-signal tokens**: Focus on actionable findings, not exhaustive documentation
- **Progressive disclosure**: Load artifacts incrementally; don't dump all content into analysis
- **Token-efficient output**: Limit findings table to 50 rows; summarize overflow
- **Deterministic results**: Rerunning without changes should produce consistent IDs and counts

### Analysis Guidelines

- **NEVER modify files** (this is read-only analysis)
- **NEVER hallucinate missing sections** (if absent, report them accurately)
- **Prioritize constitution violations** (these are always CRITICAL)
- **Use examples over exhaustive rules** (cite specific instances, not generic patterns)
- **Report zero issues gracefully** (emit success report with coverage statistics)

## Context

$ARGUMENTS
