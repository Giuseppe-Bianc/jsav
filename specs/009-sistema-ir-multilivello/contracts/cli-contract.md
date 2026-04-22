# Contract - CLI Surface for IR Pipeline

## Scope

Contract for the CLI interface used for validation, analysis, and multi-level IR transformations.

## Commands

- `jsav validate --level <hir|mir|lir> --input <file.vn>`
- `jsav lower --from <hir|mir> --to <mir|lir> --input <file.vn> --passes <list>`
- `jsav analyze --kind <dominance|rd|liveness|dependence> --level <mir|lir> --input <file.vn>`
- `jsav pipeline --input <file.vn> --plan <pipeline.yaml|json>`

## Inputs

- Input IR/program file path esistente e leggibile.
- IR/program input file path must exist and be readable.
- Levels and pass combinations must be valid according to declared preconditions.
- Deterministic mode configuration is mandatory (default ON).

## Outputs

- Success: deterministically ordered report with canonical key.
- Failure: deterministically ordered `CompileError` batch.
- No partial output if a pass fails after mutations on the working copy.

## Exit Codes

- `0`: success.
- `2`: validation/transformation error (`CompileError` batch).
- `3`: invalid configuration/pipeline error.
- `4`: I/O error.

## Non-Functional Guarantees

- Stable output across identical reruns.
- No exposure of secrets/sensitive config on stdout/stderr.
- CI pipeline compatibility for machine-readable parsing (json option).
