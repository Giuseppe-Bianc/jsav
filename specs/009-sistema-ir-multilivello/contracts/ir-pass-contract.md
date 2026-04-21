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

enum class IrGranularity {
    Module,
    Function,
};

enum class OrderingRelation {
    MustRunAfter,
    MustRunBefore,
    PreferRunAfter,
    PreferRunBefore,
    MutuallyExclusive,
};

struct OrderingConstraint {
    std::string_view otherPass;
    OrderingRelation relation;
    bool conditional;
};

using AnalysisKey = std::string_view;

enum class OutputOwnershipSemantics {
    TransferOwnership,
    DeepCopy,
};

class IPass {
public:
    virtual ~IPass() = default;

    virtual std::string_view name() const noexcept = 0;
    virtual PassKind kind() const noexcept = 0;

    virtual std::vector<std::string_view> getRequiredPasses() const {
        return {};
    }

    virtual std::vector<OrderingConstraint> getOrderingConstraints() const {
        return {};
    }

    virtual std::vector<AnalysisKey> getRequiredAnalyses() const {
        return {};
    }

    virtual std::vector<AnalysisKey> getPreservedAnalyses() const {
        return {};
    }

    virtual std::vector<AnalysisKey> getInvalidatedAnalyses() const {
        return {};
    }

    virtual void onAnalysesInvalidated(std::span<const AnalysisKey> /*invalidated*/) {
    }

    virtual void registerAnalysisResult(AnalysisKey /*key*/, const void* /*result*/) {
    }

    virtual void deregisterAnalysisResult(AnalysisKey /*key*/) {
    }

    virtual bool supportsIncremental() const noexcept {
        return false;
    }

    virtual IrGranularity granularity() const noexcept {
        return IrGranularity::Module;
    }

    virtual OutputOwnershipSemantics outputOwnershipSemantics() const noexcept {
        return OutputOwnershipSemantics::TransferOwnership;
    }

    virtual PassResult<IrUnit> deepCopyIrUnit(const IrUnit& input) const {
        return IrUnit {input};
    }

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
- `runOnWorkingCopy` restituisce sempre un risultato posseduto dal chiamante.
- `outputOwnershipSemantics()` documenta se il risultato e' trasferito come ownership unica
    (`TransferOwnership`) oppure come copia profonda (`DeepCopy`).
- `deepCopyIrUnit` definisce il meccanismo canonico per forzare una copia profonda quando
    la semantica del pass o della pipeline lo richiede.
- Commit consentito solo dopo validatePostconditions senza errori.
- In caso errore: rollback completo e ritorno batch deterministico di CompileError.
- Ordinamento errori/output: chiave canonica stabile.
- `getRequiredPasses` e `getOrderingConstraints` definiscono dipendenze e ordering
    condizionale/non-condizionale del pass nella pipeline.
- `getRequiredAnalyses`, `getPreservedAnalyses`, `getInvalidatedAnalyses` modellano la
    metadata di composizione analisi e invalidazione.
- `onAnalysesInvalidated`, `registerAnalysisResult`, `deregisterAnalysisResult` forniscono
    hook lifecycle per cache/composizione tra pass; default no-op.
- `supportsIncremental` e `granularity` dichiarano la capacita' di esecuzione incrementale
    e l'unita' di applicazione (modulo o funzione).

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
