/*
 * Created by gbian on 26/04/2026.
 * Copyright (c) 2026 All rights reserved.
 */
// NOLINTBEGIN(*-include-cleaner)
#pragma once

#include "../ir/IrCommon.hpp"
#include "PassResult.hpp"

namespace jsv {

    struct PassContext;

    struct PassInvariantReport {
        bool cfg_valid{true};
        bool ssa_valid{true};
        bool phi_valid{true};
        bool types_valid{true};
        bool memory_valid{true};

        [[nodiscard]] bool all_valid() const noexcept { return cfg_valid && ssa_valid && phi_valid && types_valid && memory_valid; }

        [[nodiscard]] static constexpr PassInvariantReport all_passed() noexcept { return {}; }
    };

    class IPass {
    public:
        virtual ~IPass() = default;

        [[nodiscard]] virtual std::string_view name() const noexcept = 0;
        [[nodiscard]] virtual PassKind kind() const noexcept = 0;

        [[nodiscard]] virtual PassResult<void> validate_preconditions(const IrUnit &input, const PassContext &context) const = 0;

        [[nodiscard]] virtual PassResult<IrUnit> run_on_working_copy(const IrUnit &input, const PassContext &context) const = 0;

        [[nodiscard]] virtual PassResult<PassInvariantReport> validate_postconditions(const IrUnit &output,
                                                                                      const PassContext &context) const = 0;
    };

}  // namespace jsv

// NOLINTEND(*-include-cleaner)
