# Contract - IR Pass Interface and Validation

## Scope

Contract for analysis/transformation/optimization/lowering passes on HIR, MIR, and LIR.

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

- Each pass operates on a transactional working copy.
- `runOnWorkingCopy` always returns a result owned by the caller.
- `outputOwnershipSemantics()` documents whether the result is transferred as unique ownership
    (`TransferOwnership`) or as a deep copy (`DeepCopy`).
- `deepCopyIrUnit` defines the canonical mechanism to force a deep copy when
    pass or pipeline semantics require it.
- Commit is allowed only after validatePostconditions succeeds with no errors.
- On error: full rollback and deterministic CompileError batch return.
- Error/output ordering: stable canonical key.
- `getRequiredPasses` and `getOrderingConstraints` define pass dependencies and
    conditional/non-conditional ordering in the pipeline.
- `getRequiredAnalyses`, `getPreservedAnalyses`, `getInvalidatedAnalyses` model
    analysis composition and invalidation metadata.
- `onAnalysesInvalidated`, `registerAnalysisResult`, `deregisterAnalysisResult` provide
    lifecycle hooks for cache/composition across passes; default is no-op.
- `supportsIncremental` and `granularity` declare incremental execution capability
    and application unit (module or function).

## Error Contract

- Single error type: CompileError.
- Reporting: `std::vector<CompileError>` batch-per-pass.
- Errors must include precise location and failed-invariant context.
- Checks blocked by failed dependencies must be annotated as skipped.

## Memory Reorder Contract

- If may-alias exists between accesses, reordering is forbidden by default.
- Exception allowed only with verifiable formal proof (certificate or reproducible analytical log).
- Missing valid proof => pass fails with dedicated CompileError.

## Determinism Contract

- No intra-pass parallelism.
- Same input + same pipeline + same config => same bit-identical output.
