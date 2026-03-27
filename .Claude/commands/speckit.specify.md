---
description: Create or update the feature specification from a natural language feature description.
handoffs: 
  - label: Build Technical Plan
    agent: speckit.plan
    prompt: Create a plan for the spec. I am building with...
  - label: Clarify Spec Requirements
    agent: speckit.clarify
    prompt: Clarify specification requirements
    send: true
---

## User Input

```text
$ARGUMENTS
```

You **MUST** consider the user input before proceeding (if not empty).

## Pre-Execution Checks

**Check for extension hooks (before specification)**:
- Check if `.specify/extensions.yml` exists in the project root.
- If it exists, read it and look for entries under the `hooks.before_specify` key
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

The text the user typed after `/speckit.specify` in the triggering message **is** the feature description. Assume you always have it available in this conversation even if `$ARGUMENTS` appears literally below. Do not ask the user to repeat it unless they provided an empty command.

Given that feature description, do this:

1. **Generate a concise short name** (2-4 words) for the branch:
   - Analyze the feature description and extract the most meaningful keywords
   - Create a 2-4 word short name that captures the essence of the feature
   - Use action-noun format when possible (e.g., "add-user-auth", "fix-payment-bug")
   - Preserve technical terms and acronyms (OAuth2, API, JWT, etc.)
   - Keep it concise but descriptive enough to understand the feature at a glance
   - Examples:
     - "I want to add user authentication" → "user-auth"
     - "Implement OAuth2 integration for the API" → "oauth2-api-integration"
     - "Create a dashboard for analytics" → "analytics-dashboard"
     - "Fix payment processing timeout bug" → "fix-payment-timeout"

2. **Create the feature branch** by running the script with `--short-name` (and `--json`). In sequential mode, do NOT pass `--number` — the script auto-detects the next available number. In timestamp mode, the script generates a `YYYYMMDD-HHMMSS` prefix automatically:

   **Branch numbering mode**: Before running the script, check if `.specify/init-options.json` exists and read the `branch_numbering` value.
   - If `"timestamp"`, add `--timestamp` (Bash) or `-Timestamp` (PowerShell) to the script invocation
   - If `"sequential"` or absent, do not add any extra flag (default behavior)

   - Bash example: `.specify/scripts/powershell/create-new-feature.ps1 "$ARGUMENTS" --json --short-name "user-auth" "Add user authentication"`
   - Bash (timestamp): `.specify/scripts/powershell/create-new-feature.ps1 "$ARGUMENTS" --json --timestamp --short-name "user-auth" "Add user authentication"`
   - PowerShell example: `pwsh -ExecutionPolicy Bypass -File .specify/scripts/powershell/create-new-feature.ps1 "$ARGUMENTS" -Json -ShortName "user-auth" "Add user authentication"`
   - PowerShell (timestamp): `pwsh -ExecutionPolicy Bypass -File .specify/scripts/powershell/create-new-feature.ps1 "$ARGUMENTS" -Json -Timestamp -ShortName "user-auth" "Add user authentication"`

   **IMPORTANT**:
   - Do NOT pass `--number` — the script determines the correct next number automatically
   - Always include the JSON flag (`--json` for Bash, `-Json` for PowerShell) so the output can be parsed reliably
   - You must only ever run this script once per feature
   - The JSON is provided in the terminal as output - always refer to it to get the actual content you're looking for
   - The JSON output will contain BRANCH_NAME and SPEC_FILE paths
   - For single quotes in args like "I'm Groot", use escape syntax: e.g 'I'\''m Groot' (or double-quote if possible: "I'm Groot")

3. Load `.specify/templates/spec-template.md` to understand required sections.

4. Follow this execution flow:

    1. Parse user description from Input
       If empty: ERROR "No feature description provided"
    2. Extract key concepts from description
       Identify: actors, actions, data, constraints
    3. For unclear aspects:
       - Make informed guesses based on context and industry standards
       - Only mark with [NEEDS CLARIFICATION: specific question] if:
         - The choice significantly impacts feature scope or user experience
         - Multiple reasonable interpretations exist with different implications
         - No reasonable default exists
       - **LIMIT: Maximum 3 [NEEDS CLARIFICATION] markers total**
       - Prioritize clarifications by impact: scope > security/privacy > user experience > technical details
    4. Fill User Scenarios & Testing section
       If no clear user flow: ERROR "Cannot determine user scenarios"
    5. Generate Functional Requirements
       Each requirement must be testable
       Use reasonable defaults for unspecified details (document assumptions in Assumptions section)
    6. Define Success Criteria
       Create measurable, technology-agnostic outcomes
       Include both quantitative metrics (time, performance, volume) and qualitative measures (user satisfaction, task completion)
       Each criterion must be verifiable without implementation details
    7. Identify Key Entities (if data involved)
    8. Return: SUCCESS (spec ready for planning)

5. Write the specification to SPEC_FILE using the template structure, replacing placeholders with concrete details derived from the feature description (arguments) while preserving section order and headings.

6. **Specification Quality Validation**: After writing the initial spec, validate it against quality criteria:

   a. **Create Spec Quality Checklist**: Generate a checklist file at `FEATURE_DIR/checklists/requirements.md` using the checklist template structure with these validation items:

      ```markdown
      # Specification Quality Checklist: [FEATURE NAME]
      
      **Purpose**: Validate specification completeness and quality before proceeding to planning
      **Created**: [DATE]
      **Feature**: [Link to spec.md]
      
      ## Content Quality
      
      - [ ] No implementation details (languages, frameworks, APIs)
      - [ ] Focused on user value and business needs
      - [ ] Written for non-technical stakeholders
      - [ ] All mandatory sections completed
      
      ## Requirement Completeness
      
      - [ ] No [NEEDS CLARIFICATION] markers remain
      - [ ] Requirements are testable and unambiguous
      - [ ] Success criteria are measurable
      - [ ] Success criteria are technology-agnostic (no implementation details)
      - [ ] All acceptance scenarios are defined
      - [ ] Edge cases are identified
      - [ ] Scope is clearly bounded
      - [ ] Dependencies and assumptions identified
      
      ## Feature Readiness
      
      - [ ] All functional requirements have clear acceptance criteria
      - [ ] User scenarios cover primary flows
      - [ ] Feature meets measurable outcomes defined in Success Criteria
      - [ ] No implementation details leak into specification
      
      ## Notes
      
      - Items marked incomplete require spec updates before `/speckit.clarify` or `/speckit.plan`
      ```

   b. **Run Validation Check**: Review the spec against each checklist item:
      - For each item, determine if it passes or fails
      - Document specific issues found (quote relevant spec sections)

   c. **Handle Validation Results**:

      - **If all items pass**: Mark checklist complete and proceed to step 7

      - **If items fail (excluding [NEEDS CLARIFICATION])**:
        1. List the failing items and specific issues
        2. Update the spec to address each issue
        3. Re-run validation until all items pass (max 3 iterations)
        4. If still failing after 3 iterations, document remaining issues in checklist notes and warn user

      - **If [NEEDS CLARIFICATION] markers remain**:
        1. Extract all [NEEDS CLARIFICATION: ...] markers from the spec
        2. **LIMIT CHECK**: If more than 3 markers exist, keep only the 3 most critical (by scope/security/UX impact) and make informed guesses for the rest
        3. For each clarification needed (max 3), present options to user in this format:

           ```markdown
           ## Question [N]: [Topic]
           
           **Context**: [Quote relevant spec section]
           
           **What we need to know**: [Specific question from NEEDS CLARIFICATION marker]
           
           **Suggested Answers**:
           
           | Option | Answer | Implications |
           |--------|--------|--------------|
           | A      | [First suggested answer] | [What this means for the feature] |
           | B      | [Second suggested answer] | [What this means for the feature] |
           | C      | [Third suggested answer] | [What this means for the feature] |
           | Custom | Provide your own answer | [Explain how to provide custom input] |
           
           **Your choice**: _[Wait for user response]_
           ```

        4. **CRITICAL - Table Formatting**: Ensure markdown tables are properly formatted:
           - Use consistent spacing with pipes aligned
           - Each cell should have spaces around content: `| Content |` not `|Content|`
           - Header separator must have at least 3 dashes: `|--------|`
           - Test that the table renders correctly in markdown preview
        5. Number questions sequentially (Q1, Q2, Q3 - max 3 total)
        6. Present all questions together before waiting for responses
        7. Wait for user to respond with their choices for all questions (e.g., "Q1: A, Q2: Custom - [details], Q3: B")
        8. Update the spec by replacing each [NEEDS CLARIFICATION] marker with the user's selected or provided answer
        9. Re-run validation after all clarifications are resolved

   d. **Update Checklist**: After each validation iteration, update the checklist file with current pass/fail status

7. Report completion with branch name, spec file path, checklist results, and readiness for the next phase (`/speckit.clarify` or `/speckit.plan`).

8. **Check for extension hooks**: After reporting completion, check if `.specify/extensions.yml` exists in the project root.
   - If it exists, read it and look for entries under the `hooks.after_specify` key
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

**NOTE:** The script creates and checks out the new branch and initializes the spec file before writing.

The specification workflow above involves script execution, document generation, validation, and clarification handling across multiple stages. The following patterns and anti-patterns guide reliable execution of this workflow.

## Patterns for the Specification Workflow

### Pattern: Clarification Triage

- **Objective:** Ensure that [NEEDS CLARIFICATION] markers are reserved for genuinely ambiguous decisions while all other gaps are resolved through informed defaults, keeping the workflow fast and reducing round-trips with the user.
- **Context of application:** Apply during step 4.3 of the Outline when deciding whether an unclear aspect warrants a [NEEDS CLARIFICATION] marker or should be resolved with a reasonable default.
- **Key characteristics:** Every potential ambiguity passes through a three-gate filter before receiving a marker: (1) Can the ambiguity be resolved from hints in the user's own description? (2) Does an industry convention or domain norm provide a default? (3) Does the wrong guess significantly impact scope, security, or user experience? Only ambiguities that fail all three gates receive a marker. The generator actively searches for resolution before resorting to a marker.
- **Operational guidance:**
  1. When encountering an unclear aspect, re-read the user's feature description for implicit hints. A request for "user authentication" in a web application context implies standard session or token-based auth — this is not a clarification-worthy ambiguity.
  2. If the description provides no hints, check whether the domain has a dominant convention. Consult the "Examples of reasonable defaults" list in the For AI Generation section.
  3. If a reasonable default exists, adopt it and record it explicitly in the Assumptions section of the spec.
  4. If no default exists and the wrong guess would significantly change scope, introduce security risk, or alter user experience, create a [NEEDS CLARIFICATION] marker. Structure it with a specific question and at least three options with implications (as specified in step 6c).
  5. Before finalizing, count all markers. If the count exceeds 3, rank them by impact (scope > security/privacy > user experience > technical details) and resolve the lowest-impact ones with informed defaults.

### Pattern: Single-Pass Branch Initialization

- **Objective:** Execute the feature branch creation script exactly once per feature, preventing duplicate branches, conflicting spec directories, or feature-number collisions.
- **Context of application:** Apply during step 2 of the Outline when constructing and executing the `create-new-feature.ps1` command.
- **Key characteristics:** Branch creation is treated as an irreversible operation. The generator validates the short name and command construction before execution and never retries with modified parameters. The JSON output is parsed immediately and its values (BRANCH_NAME, SPEC_FILE) are stored for use in all subsequent steps. If the script fails, the error is surfaced to the user rather than masked by a second attempt.
- **Operational guidance:**
  1. Generate the short name (step 1) and validate it against naming conventions before constructing the command: 2–4 words, lowercase, hyphen-separated, action-noun format where possible.
  2. Construct the full command with the `--json` and `--short-name` flags. Confirm that `--number` is not included.
  3. Handle special characters in the feature description (single quotes, double quotes) using the escape syntax documented in step 2.
  4. Execute the command exactly once. Parse the JSON output and extract BRANCH_NAME and SPEC_FILE.
  5. If the command returns an error, report the exact error message to the user and stop. Do not re-run the script with a different short name, a manually assigned number, or altered arguments.

## Anti-Patterns for the Specification Workflow

### Anti-Pattern: Over-Clarification

- **Description:** The generator inserts [NEEDS CLARIFICATION] markers for aspects that have obvious defaults or can be reasonably inferred from context — for example, asking about data retention policy for a simple to-do app, querying authentication method when standard web conventions apply, or requesting explicit error-handling preferences when "user-friendly messages with appropriate fallbacks" is the universal norm.
- **Reasons to avoid:** Each marker triggers a round-trip with the user (step 6c), turning what should be a single-pass spec generation into an extended interview. Over-clarification signals that the generator lacks domain knowledge or is deferring decisions it should be equipped to make. This typically happens when the generator treats every unspecified detail as equally important rather than filtering by impact. Under time pressure, users receiving four or five clarification questions may abandon the workflow or provide rushed, low-quality answers.
- **Negative consequences:** The 3-marker limit is consumed by low-impact questions, leaving no capacity for genuinely critical ambiguities. The spec generation feels burdensome rather than assistive. Users who encounter excessive questions in their first interaction are less likely to trust or re-use the workflow. The Assumptions section — which should capture the generator's informed defaults — remains empty, making the spec harder to review.
- **Correct alternative:** Apply the **Clarification Triage** pattern to resolve low-impact ambiguities with informed defaults and reserve markers for decisions where the wrong guess would materially alter scope, security, or user experience.

### Anti-Pattern: Repeated Branch Creation

- **Description:** The generator executes the `create-new-feature.ps1` script more than once — either because it misinterprets a script error as a transient failure, because it wants to change the short name after initial creation, or because it loses track of the BRANCH_NAME and SPEC_FILE values and re-runs the script to re-derive them.
- **Reasons to avoid:** The script auto-assigns the next available feature number. A second execution creates a second feature directory with a new number, leaving the first as an orphan. If the first execution succeeded and checked out the branch, a second execution may fail in a confusing state — or worse, succeed and create a parallel branch that diverges from the first. This mistake typically occurs when the generator does not store the script's output or when it treats branch creation as an idempotent operation.
- **Negative consequences:** The repository accumulates orphan branches and spec directories that must be manually cleaned up. The SPEC_FILE path used in subsequent steps may not match the branch that is actually checked out, causing the spec to be written to the wrong location. Feature numbers become non-sequential, confusing teams that use them for ordering or reference.
- **Correct alternative:** Apply the **Single-Pass Branch Initialization** pattern to execute the script once, parse and store its output immediately, and report errors rather than retrying.

## Quick Guidelines

- Focus on **WHAT** users need and **WHY**.
- Avoid HOW to implement (no tech stack, APIs, code structure).
- Written for business stakeholders, not developers.
- DO NOT create any checklists that are embedded in the spec. That will be a separate command.

### Section Requirements

- **Mandatory sections**: Must be completed for every feature
- **Optional sections**: Include only when relevant to the feature
- When a section doesn't apply, remove it entirely (don't leave as "N/A")

### For AI Generation

When creating this spec from a user prompt:

1. **Make informed guesses**: Use context, industry standards, and common patterns to fill gaps
2. **Document assumptions**: Record reasonable defaults in the Assumptions section
3. **Limit clarifications**: Maximum 3 [NEEDS CLARIFICATION] markers - use only for critical decisions that:
   - Significantly impact feature scope or user experience
   - Have multiple reasonable interpretations with different implications
   - Lack any reasonable default
4. **Prioritize clarifications**: scope > security/privacy > user experience > technical details
5. **Think like a tester**: Every vague requirement should fail the "testable and unambiguous" checklist item
6. **Common areas needing clarification** (only if no reasonable default exists):
   - Feature scope and boundaries (include/exclude specific use cases)
   - User types and permissions (if multiple conflicting interpretations possible)
   - Security/compliance requirements (when legally/financially significant)

**Examples of reasonable defaults** (don't ask about these):

- Data retention: Industry-standard practices for the domain
- Performance targets: Standard web/mobile app expectations unless specified
- Error handling: User-friendly messages with appropriate fallbacks
- Authentication method: Standard session-based or OAuth2 for web apps
- Integration patterns: Use project-appropriate patterns (REST/GraphQL for web services, function calls for libraries, CLI args for tools, etc.)

### Success Criteria Guidelines

Success criteria must be:

1. **Measurable**: Include specific metrics (time, percentage, count, rate)
2. **Technology-agnostic**: No mention of frameworks, languages, databases, or tools
3. **User-focused**: Describe outcomes from user/business perspective, not system internals
4. **Verifiable**: Can be tested/validated without knowing implementation details

**Good examples**:

- "Users can complete checkout in under 3 minutes"
- "System supports 10,000 concurrent users"
- "95% of searches return results in under 1 second"
- "Task completion rate improves by 40%"

**Bad examples** (implementation-focused):

- "API response time is under 200ms" (too technical, use "Users see results instantly")
- "Database can handle 1000 TPS" (implementation detail, use user-facing metric)
- "React components render efficiently" (framework-specific)
- "Redis cache hit rate above 80%" (technology-specific)

The guidelines above establish the standards for specification content. The following patterns and anti-patterns provide deeper guidance on producing requirements and success criteria that meet those standards.

## Patterns for Specification Content Quality

### Pattern: Scope Boundary Enforcement

- **Objective:** Produce a specification that explicitly defines what the feature includes and excludes, preventing scope creep during planning and implementation and satisfying the "Scope is clearly bounded" validation item.
- **Context of application:** Apply during step 4.2 (Extract key concepts) and step 4.5 (Generate Functional Requirements) when determining the feature's boundaries, and during step 6b (Run Validation Check) when verifying the scope checklist item.
- **Key characteristics:** The spec contains explicit inclusion and exclusion statements. Exclusions are derived from the feature description — if the user mentions authentication but not authorization, the spec states that role-based access control is out of scope. Boundary statements are specific and testable ("This feature does not include bulk user import") rather than vague ("This feature has limited scope"). Adjacent functionality that a reader might assume is included is called out explicitly.
- **Operational guidance:**
  1. After extracting key concepts from the user's description, list all adjacent functionality areas. For a "user registration" feature, adjacent areas include password reset, profile editing, account deletion, and social login.
  2. For each adjacent area, determine whether the user's description implies it is in scope. If it does, include it. If it does not mention it, declare it out of scope.
  3. Write boundary statements as testable assertions: "The feature includes [X]" and "The feature does not include [Y]."
  4. During validation (step 6b), verify that no functional requirement references functionality declared out of scope. If it does, either expand the scope boundary or remove the requirement.
  5. If the user's description is broad enough that boundaries cannot be set without significant guessing, use a [NEEDS CLARIFICATION] marker — scope ambiguity is one of the highest-impact clarification categories.

### Pattern: Behavioral Requirement Specification

- **Objective:** Write every functional requirement as an observable behavior with a clear trigger, action, and expected outcome, so that each requirement is directly convertible into a test case.
- **Context of application:** Apply during step 4.5 (Generate Functional Requirements) when translating extracted concepts into individual requirements.
- **Key characteristics:** Each requirement follows an implicit "When [trigger], the system [action], resulting in [outcome]" structure. Requirements avoid capability language ("supports," "handles," "manages") that describes what the system can do without specifying what happens. Every requirement can be evaluated with a binary pass/fail test by a person who has never seen the implementation.
- **Operational guidance:**
  1. For each functional requirement, identify three elements: the trigger (a user action or system event), the expected system behavior, and the observable outcome.
  2. If you cannot identify all three elements, the requirement is too vague. Refine it until all three are present — for example, replace "The system supports file uploads" with "When a user selects a file and clicks Upload, the system stores the file and displays a confirmation showing the file name and size."
  3. Eliminate hedge words ("should," "may," "could") from requirements. Each requirement states what the system does, not what it might do.
  4. Verify each requirement against the testability criterion: could a tester write a pass/fail test from this requirement alone, without asking clarifying questions? If not, add the missing specificity.
  5. Group related requirements under the user scenario they support, maintaining traceability from scenario to requirement to (eventually) implementation task.

### Pattern: Measurable Outcome Definition

- **Objective:** Ensure every success criterion contains a specific, numeric threshold tied to a user-visible outcome, making it objectively verifiable without reference to implementation details.
- **Context of application:** Apply during step 4.6 (Define Success Criteria) when translating feature goals into measurable outcomes.
- **Key characteristics:** Each criterion includes a number — a time limit, a percentage, a count, or a rate. The metric is expressed in user-facing terms (task completion time, error rate seen by users, concurrent user capacity) rather than system internals (API latency, database throughput, cache hit ratio). Criteria are technology-agnostic and stakeholder-readable.
- **Operational guidance:**
  1. For each success criterion, identify the user-visible outcome it measures (e.g., "users complete checkout quickly" → time to complete checkout).
  2. Assign a specific numeric threshold based on domain conventions. If the feature description provides no performance expectations, use industry-standard defaults (e.g., web page loads under 3 seconds, form completion under 2 minutes, system availability above 99.5%) and document these in the Assumptions section.
  3. Verify the metric is technology-agnostic: if the criterion mentions a framework, database, API, or infrastructure component, rewrite it using the user-facing equivalent (e.g., "API response under 200ms" becomes "Users see results within 1 second of submitting a request").
  4. Include both quantitative measures (time, count, rate) and qualitative measures expressed in measurable terms (e.g., "task completion rate" rather than "users find it easy").
  5. Test each criterion by asking: "Could a non-technical stakeholder read this, understand it, and confirm whether the feature meets it?" If the answer is no, rephrase.

## Anti-Patterns for Specification Content Quality

### Anti-Pattern: Unbounded Scope

- **Description:** The specification describes what the feature does but never states what it does not do, leaving the feature's boundaries implicit and open to interpretation by downstream agents and stakeholders.
- **Reasons to avoid:** Without explicit exclusions, every adjacent piece of functionality is implicitly "maybe in scope." A user registration spec without scope boundaries might be interpreted by the planning agent as including password reset, social login, profile management, and account deletion — quadrupling the implementation effort. This typically occurs when the generator focuses exclusively on what the user asked for without considering what the user did not ask for but might be assumed. The "Scope is clearly bounded" validation item will fail, but the generator may not recognize why if it equates "scope" with "feature description."
- **Negative consequences:** The planning phase (speckit.plan) produces a technical plan that is far larger than intended, because it accounts for functionality the user never requested. Implementation estimates are inflated. The spec passes internal consistency checks (all described requirements are coherent) but fails scope validation because it never establishes boundaries. Stakeholders reviewing the spec approve it without realizing that adjacent functionality is neither included nor excluded.
- **Correct alternative:** Apply the **Scope Boundary Enforcement** pattern to explicitly declare what is in scope and what is out of scope, derived from the user's feature description and its adjacent functionality areas.

### Anti-Pattern: Capability Enumeration

- **Description:** Functional requirements are written as a list of system capabilities — "supports file upload," "handles multiple user roles," "manages notifications" — without specifying triggers, behaviors, or expected outcomes.
- **Reasons to avoid:** Capability language sounds authoritative but is fundamentally untestable. "Supports file upload" does not specify what file types are accepted, what size limits apply, what feedback the user receives on success or failure, or what happens when storage is full. This style emerges naturally because it mirrors how features are discussed in conversation ("we need to support file uploads"), and the generator may reproduce conversational phrasing without translating it into specification-grade precision. The validation item "Requirements are testable and unambiguous" will fail for every capability-style requirement.
- **Negative consequences:** Downstream agents (speckit.plan, speckit.implement) must independently decide what "supports file upload" means, introducing inconsistencies between the spec, the plan, and the implementation. Testers cannot write test cases because there are no expected outcomes to verify. The spec gives an illusion of completeness — it lists many capabilities — while actually deferring all meaningful decisions to later phases. Review cycles increase because stakeholders request clarification on every vague requirement.
- **Correct alternative:** Apply the **Behavioral Requirement Specification** pattern to rewrite each requirement with an explicit trigger, system behavior, and observable outcome.

### Anti-Pattern: Metric-Free Success Criteria

- **Description:** Success criteria use qualitative language without measurable thresholds — "the system performs well," "users find the checkout process intuitive," "the feature is reliable and fast."
- **Reasons to avoid:** Qualitative criteria cannot be objectively verified. "The system performs well" passes or fails based on whoever is evaluating it, not on observable measurement. This mistake occurs when the generator avoids committing to specific numbers — either because the user did not provide performance expectations, or because the generator is uncertain what thresholds are realistic. Vague language feels safe because it is difficult to fail, but that is precisely why it is useless: a criterion that everything passes is a criterion that validates nothing.
- **Negative consequences:** The validation item "Success criteria are measurable" fails. Planning and implementation agents have no performance targets to design against. Acceptance testing becomes a matter of subjective opinion rather than objective observation. Stakeholders cannot determine whether the delivered feature meets expectations because expectations were never quantified. The specification loses its function as a contract between the product vision and the implementation.
- **Correct alternative:** Apply the **Measurable Outcome Definition** pattern to replace every qualitative criterion with a specific, numeric, user-facing threshold that can be objectively verified.
