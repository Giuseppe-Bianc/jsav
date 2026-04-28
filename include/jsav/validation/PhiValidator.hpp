#pragma once

#include "jsav/error/CompileError.hpp"
#include "jsav/ir/Function.hpp"

namespace jsv {

    /// @brief PHI validator: PHI node correctness (T038c-T038d, US1)
    class PhiValidator {
    public:
        /// Validate PHI nodes: one operand per predecessor
        [[nodiscard]] static std::expected<void, std::vector<CompileError>>
        validate_phi_nodes(const Function &func) noexcept;
    };

}  // namespace jsv
