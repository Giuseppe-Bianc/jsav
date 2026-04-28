#pragma once

#include "jsav/error/CompileError.hpp"
#include "jsav/ir/Function.hpp"

namespace jsv {

    /// @brief Use-def validator: checks definition-use relationships (T037-T038, US1)
    class UseDefValidator {
    public:
        /// Validate use-def chain: every use has a reachable definition
        [[nodiscard]] static std::expected<void, std::vector<CompileError>>
        validate_use_def_chain(const Function &func) noexcept;
    };

}  // namespace jsv
