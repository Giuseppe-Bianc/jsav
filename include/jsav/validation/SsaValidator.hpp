#pragma once

#include "jsav/error/CompileError.hpp"
#include "jsav/ir/Function.hpp"

namespace jsv {

    /// @brief SSA validator: single definition and dominance properties (T038a-T038b, US1)
    class SsaValidator {
    public:
        /// Validate SSA property: single definition per value
        [[nodiscard]] static std::expected<void, std::vector<CompileError>>
        validate_single_definition(const Function &func) noexcept;

        /// Validate dominance: every use is dominated by its definition
        [[nodiscard]] static std::expected<void, std::vector<CompileError>>
        validate_dominance(const Function &func) noexcept;
    };

}  // namespace jsv
