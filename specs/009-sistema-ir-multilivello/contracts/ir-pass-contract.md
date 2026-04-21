# Contract - IR Pass Interface and Validation

## Scope
Contratto per pass di analisi/trasformazione/ottimizzazione/lowering su HIR, MIR, LIR.

## Canonical C++ Interface (design-level)

```cpp
using ErrorBatch = std::vector<CompileError>;

template <typename TResult>
using PassResult = std::expected<TResult, ErrorBatch>;

struct PassContext {
    IrLevel level;
    CanonicalPipelineConfig config;
    DeterministicSeed seed;
};

struct PassInvariantReport {
    bool cfgValid;
    bool ssaValid;
    bool phiValid;
    bool typesValid;
    bool memoryValid;
};

class IPass {
public:
    virtual ~IPass() = default;

    virtual std::string_view name() const noexcept = 0;
    virtual PassKind kind() const noexcept = 0;

    virtual PassResult<void> validatePreconditions(const IrUnit& input,
                                                   const PassContext& ctx) const = 0;

    virtual PassResult<IrUnit> runOnWorkingCopy(const IrUnit& input,
                                                const PassContext& ctx) const = 0;

    virtual PassResult<PassInvariantReport> validatePostconditions(const IrUnit& output,
                                                                   const PassContext& ctx) const = 0;
};
```

## Behavioral Requirements
- Ogni pass opera su working copy transazionale.
- Commit consentito solo dopo validatePostconditions senza errori.
- In caso errore: rollback completo e ritorno batch deterministico di CompileError.
- Ordinamento errori/output: chiave canonica stabile.

## Error Contract
- Tipo errore unico: CompileError.
- Reporting: `std::vector<CompileError>` batch-per-pass.
- Errori devono includere localizzazione precisa e contesto invarianti falliti.
- Verifiche bloccate da dipendenze fallite devono essere annotate come skipped.

## Memory Reorder Contract
- Se esiste may-alias tra accessi, riordino vietato per default.
- Eccezione consentita solo con prova formale verificabile (certificato o log analitico riproducibile).
- Assenza di prova valida => pass fallito con CompileError dedicato.

## Determinism Contract
- Nessun parallelismo intra-pass.
- Stesso input + stessa pipeline + stessa config => stesso output bit-identico.
