# Contract - CLI Surface for IR Pipeline

## Scope
Contratto dell'interfaccia CLI per validazione, analisi e trasformazioni IR multi-livello.

## Commands
- `jsav validate --level <hir|mir|lir> --input <file.vn>`
- `jsav lower --from <hir|mir> --to <mir|lir> --input <file.vn> --passes <list>`
- `jsav analyze --kind <dominance|rd|liveness|dependence> --level <mir|lir> --input <file.vn>`
- `jsav pipeline --input <file.vn> --plan <pipeline.yaml|json>`

## Inputs
- Input IR/program file path esistente e leggibile.
- Livelli e combinazioni pass validi secondo precondizioni dichiarate.
- Config deterministic mode obbligatoria (default ON).

## Outputs
- Successo: report ordinato deterministicamente con chiave canonica.
- Fallimento: batch errori `CompileError` ordinati deterministicamente.
- Nessun output parziale se pass fallisce dopo mutazioni su working copy.

## Exit Codes
- `0`: successo.
- `2`: errore di validazione/trasformazione (CompileError batch).
- `3`: errore configurazione/pipeline invalida.
- `4`: errore I/O.

## Non-Functional Guarantees
- Stabilita output su riesecuzioni identiche.
- Nessuna esposizione segreti/config sensibili su stdout/stderr.
- Compatibilita con pipeline CI per parsing machine-readable (json option).
