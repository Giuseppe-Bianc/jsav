---
description: Convert existing tasks into actionable, dependency-ordered GitHub issues for the feature based on available design artifacts.
tools: ['github/github-mcp-server/issue_write']
---

## User Input

```text
$ARGUMENTS
```

You **MUST** consider the user input before proceeding (if not empty).

## Pre-Execution Checks

**Check for extension hooks (before tasks-to-issues conversion)**:
- Check if `.specify/extensions.yml` exists in the project root.
- If it exists, read it and look for entries under the `hooks.before_taskstoissues` key
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

1. Run `.specify/scripts/powershell/check-prerequisites.ps1 -Json -RequireTasks -IncludeTasks` from repo root and parse FEATURE_DIR and AVAILABLE_DOCS list. All paths must be absolute. For single quotes in args like "I'm Groot", use escape syntax: e.g 'I'\''m Groot' (or double-quote if possible: "I'm Groot").
1. From the executed script, extract the path to **tasks**.
1. Get the Git remote by running:

```bash
git config --get remote.origin.url
```

> [!CAUTION]
> ONLY PROCEED TO NEXT STEPS IF THE REMOTE IS A GITHUB URL

The remote validation gate above is the single most important safety check in this workflow. A false positive — treating a non-GitHub URL as GitHub, or extracting the wrong owner/repo — leads to issues created in the wrong repository, which may be irreversible without manual cleanup. The following guidance governs how the validation must be performed and what failures look like.

### Patterns for Remote Validation

#### Canonical Remote URL Parsing

- **Objective:** Extract the repository owner and name from the Git remote URL using a strict, format-aware parser that rejects ambiguous or non-GitHub URLs rather than guessing.
- **Context of application:** Immediately after retrieving the remote URL via `git config --get remote.origin.url`, before any issue creation is attempted.
- **Key characteristics:** The parser handles both HTTPS (`https://github.com/owner/repo.git`) and SSH (`git@github.com:owner/repo.git`) formats explicitly. It rejects URLs that do not match either pattern. The extracted owner and repo values are verified to be non-empty and to contain no path separators, query strings, or fragments.
- **Operational guidance:**
  1. Match the remote URL against the HTTPS pattern (`^https://github\.com/([^/]+)/([^/]+?)(?:\.git)?$`) and the SSH pattern (`^git@github\.com:([^/]+)/([^/]+?)(?:\.git)?$`).
  2. If neither pattern matches, abort with a clear error: "Remote URL does not match a recognized GitHub format. Issue creation is restricted to GitHub repositories."
  3. After extraction, verify that both owner and repo are non-empty strings containing only alphanumeric characters, hyphens, underscores, and periods.
  4. Log the resolved `owner/repo` pair explicitly before proceeding, so the user can confirm correctness before issues are created.

#### Pre-Creation Confirmation

- **Objective:** Present the resolved target repository to the user and require implicit or explicit confirmation before creating any issues, preventing silent misdirection.
- **Context of application:** After successful remote URL parsing and before the first issue creation call in step 4.
- **Key characteristics:** The agent states the resolved repository (`owner/repo`), the number of issues it intends to create, and waits for the user to proceed. This provides a final human checkpoint against repository mismatch, especially in environments with multiple remotes or forked repositories.
- **Operational guidance:**
  1. Output a summary line: "Resolved target repository: `owner/repo`. Preparing to create N issues."
  2. If `$ARGUMENTS` contains an explicit flag to skip confirmation (e.g., "no-confirm"), proceed directly but log the skip.
  3. If the remote URL resolved to a fork or an unexpected organization, highlight this explicitly: "Note: resolved repository appears to be a fork of `upstream/repo`."
  4. Do not batch-create all issues before the user has had an opportunity to see the target repository.

### Anti-Patterns for Remote Validation

#### Substring-Based URL Matching

- **Description:** The agent checks whether the remote URL merely contains the substring "github" (or "github.com") rather than parsing it against a strict URL pattern, accepting URLs like `https://my-github-mirror.corp.com/repo`, `https://github.enterprise.acme.com/org/repo`, or malformed strings that happen to include the word "github."
- **Reasons to avoid:** Substring matching produces false positives for GitHub Enterprise instances, corporate mirrors, and coincidental string matches. The GitHub MCP server targets `github.com` — creating issues against a URL that contains "github" but points to a different host results in either API errors (best case) or issues created in the wrong repository on github.com if the owner/repo extraction happens to collide with a real github.com path (worst case). This occurs when the agent treats the CAUTION callout as a low-rigor check rather than a security-critical gate.
- **Negative consequences:** Issues are created in repositories the user does not control or does not intend to target. In the collision scenario, confidential task descriptions may be exposed as public issues on a stranger's repository. Cleanup requires manual issue deletion, which may not be possible if the user lacks write access to the misidentified repository.
- **Correct alternative:** Apply **Canonical Remote URL Parsing** to match the remote URL against exact HTTPS and SSH patterns for `github.com`, rejecting anything that does not conform.

#### Silent Repository Assumption

- **Description:** The agent extracts owner/repo from the remote URL and proceeds directly to issue creation without surfacing the resolved target to the user, assuming the extraction is correct because the URL matched a GitHub pattern.
- **Reasons to avoid:** Even with correct URL parsing, the resolved repository may not be the user's intended target. Common scenarios include: the user's local clone points to a personal fork rather than the upstream repository; the remote was recently changed and the cached URL is stale; the repository has been transferred to a different organization. Without confirmation, the agent creates issues in the wrong repository with no opportunity for the user to intervene.
- **Negative consequences:** Issues appear in a fork or archived repository where they have no visibility to the team. The user discovers the mismatch only after creation, requiring manual cleanup (deletion and re-creation in the correct repository). If the wrong repository is public, internal task descriptions may be exposed.
- **Correct alternative:** Apply **Pre-Creation Confirmation** to surface the resolved `owner/repo` to the user before any issues are created.

---

1. For each task in the list, use the GitHub MCP server to create a new issue in the repository that is representative of the Git remote.

> [!CAUTION]
> UNDER NO CIRCUMSTANCES EVER CREATE ISSUES IN REPOSITORIES THAT DO NOT MATCH THE REMOTE URL

Issue creation is where the structured task data from `tasks.md` is translated into GitHub issues. The quality of each created issue — its title, body, labels, dependency references, and ordering — determines whether the issues are usable as an implementation backlog or require immediate manual rework. The following guidance ensures the translation is faithful, safe to re-run, and preserves the relationships between tasks.

### Patterns for Issue Creation

#### Structured Task-to-Issue Mapping

- **Objective:** Translate each task's structured metadata (ID, description, phase, dependencies, parallel markers, referenced file paths) into well-formed GitHub issue fields, preserving all information that downstream consumers (developers, project boards, automation) need.
- **Context of application:** When constructing the title and body of each GitHub issue from the corresponding task entry in `tasks.md`.
- **Key characteristics:** The mapping is deterministic — the same task always produces the same issue content. Task IDs appear in the issue title or body for traceability. Phase grouping is encoded as labels or a body section. Dependencies between tasks are expressed as issue cross-references (using `#issue-number` syntax) once predecessor issues have been created. File paths referenced in the task appear in the issue body.
- **Operational guidance:**
  1. Set the issue title to include the task ID and a concise summary derived from the task description (e.g., "TASK-003: Implement user authentication middleware").
  2. Structure the issue body with sections: **Description** (from task description), **Phase** (from phase grouping), **Dependencies** (list of predecessor task IDs, updated to issue cross-references as issues are created), **Files** (referenced file paths from the task), and **Parallel** (note if the task carries a `[P]` marker).
  3. Create issues in dependency order: predecessor tasks first, so that their issue numbers are available for cross-referencing in dependent issues.
  4. After all issues are created, verify that every dependency reference in every issue body points to a valid, existing issue number.

#### Idempotent Creation Guard

- **Objective:** Detect and skip tasks that already have corresponding GitHub issues, preventing duplicate issue creation when the command is run multiple times.
- **Context of application:** Before creating each individual issue, and as a pre-scan before the creation loop begins.
- **Key characteristics:** The agent searches existing open issues in the target repository for titles or body content containing the task ID. If a matching issue is found, the task is skipped and the existing issue number is used for any dependency cross-references. The agent reports which tasks were skipped and which were created.
- **Operational guidance:**
  1. Before the creation loop, list existing open issues in the target repository (using the GitHub MCP server's read capabilities if available, or noting the limitation if not).
  2. For each task, search the existing issues for the task ID (e.g., "TASK-003") in the issue title.
  3. If a match is found, skip creation, record the existing issue number for cross-referencing, and log: "TASK-003: Skipped — existing issue #42."
  4. If no read capability is available to check existing issues, warn the user: "Unable to check for existing issues. Duplicate issues may be created if this command has been run before." Proceed only with user acknowledgment.

### Anti-Patterns for Issue Creation

#### Title-Only Issue Creation

- **Description:** The agent creates GitHub issues with only the task title (or a minimal summary), discarding the task's description, phase assignment, dependency list, parallel markers, and referenced file paths — producing issues that are structurally correct but informationally empty.
- **Reasons to avoid:** A GitHub issue serves as the implementation instruction for a developer. An issue with only a title forces the developer to navigate back to `tasks.md` to understand what the task involves, what it depends on, and what files are affected. This defeats the purpose of creating issues in the first place. This typically occurs when the agent treats issue creation as a mechanical "for each task, create issue with task name" operation rather than a translation of structured data into a usable work item.
- **Negative consequences:** Developers cannot work from the issues alone — they must cross-reference `tasks.md` constantly, creating a fragile dependency on a file that may change or become inaccessible. Project boards and sprint views show a list of cryptic titles with no context. Dependency relationships between tasks are invisible in the issue tracker, preventing effective parallelization and sequencing.
- **Correct alternative:** Apply **Structured Task-to-Issue Mapping** to translate all available task metadata into issue body sections, labels, and cross-references.

#### Unbounded Re-Run Duplication

- **Description:** The agent creates a new issue for every task on every invocation, without checking whether issues for those tasks already exist in the repository, resulting in duplicate issues that clutter the project board and confuse contributors.
- **Reasons to avoid:** Users frequently re-run commands — after a failed partial execution, after adding new tasks, or to verify the workflow. Without an idempotency check, each re-run doubles the number of issues. Duplicate issues cannot be automatically distinguished from legitimate ones without manual inspection. This occurs when the agent treats the command as stateless (no awareness of prior executions) rather than as an operation that should converge to the correct state regardless of how many times it runs.
- **Negative consequences:** The repository accumulates duplicate issues that must be manually closed or deleted. Contributors assigned to issues may work from duplicates that lack the cross-references of the original. Project metrics (issue count, burndown charts) become unreliable. In repositories with issue-triggered automation (CI pipelines, notifications), duplicates trigger redundant workflows.
- **Correct alternative:** Apply **Idempotent Creation Guard** to check for existing issues matching each task ID before creating new ones, and skip tasks that are already represented.

## Post-Execution Checks

**Check for extension hooks (after tasks-to-issues conversion)**:
Check if `.specify/extensions.yml` exists in the project root.
- If it exists, read it and look for entries under the `hooks.after_taskstoissues` key
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
