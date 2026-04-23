# Contract - CLI Surface for IR Pipeline

## Scope

Contract for the CLI interface used for validation, analysis, and multi-level IR transformations.

## Commands

- `jsav validate --level <hir|mir|lir> --input <file.vn>`
- `jsav lower --from <hir|mir> --to <mir|lir> --input <file.vn> --passes <list>`
- `jsav analyze --kind <dominance|rd|liveness|dependence> --level <mir|lir> --input <file.vn>`
- `jsav pipeline --input <file.vn> --plan <pipeline.yaml|json>`

## Inputs

- IR/program input file path must exist and be readable.
- Levels and pass combinations must be valid according to declared preconditions.
- Deterministic mode configuration is mandatory (default ON).

## Outputs

- Success: deterministically ordered report with canonical key.
- Failure: deterministically ordered `CompileError` batch.
- Atomicity guarantee: If any pass fails, no partial output is produced regardless of failure timing.
  - Working copy is discarded entirely on any failure
  - Only the `CompileError` batch and exit code are emitted
  - Mutations (transformations that modify IR structure) are transactional per-pass
  - Validation and analysis operations are non-mutating and cannot leave partial state

## Exit Codes

- `0`: success.
- `1`: general/unspecified error (reserved for future use, currently unused).
- `2`: validation/transformation error (`CompileError` batch).
- `3`: invalid configuration/pipeline error.
- `4`: I/O error.
- `5`: internal error (unexpected failure, assertion, panic).

## Non-Functional Guarantees

- Deterministic output: Given identical input file content, command flags, and tool version, 
  output is byte-for-byte identical regardless of:
  - File path or working directory
  - Execution timestamp
  - Machine architecture or OS (assuming same tool version compiled for that platform)
  - Environment variables (except those explicitly documented as affecting output)
  - Flag order (flags are order-independent)
- No exposure of secrets/sensitive config on stdout/stderr:
  - Never output: API keys, passwords, tokens, private keys, credentials
  - Never output: absolute file paths (use relative paths or sanitized paths)
  - Never output: environment variable values (except whitelisted safe vars)
  - Error messages must not expose: stack traces with sensitive context, internal implementation details
  - Safe to output: public API names, IR level/pass names, anonymized error categories
- CI pipeline compatibility for machine-readable parsing (json option).
