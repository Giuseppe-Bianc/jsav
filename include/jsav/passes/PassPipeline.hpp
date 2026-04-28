#pragma once

#include "jsav/error/CompileError.hpp"
#include "jsav/ir/Function.hpp"

namespace jsv {

    /// @brief Pass pipeline: orchestrates passes and post-pass validation (T039, US1)
    class PassPipeline {
    public:
        /// Integrate post-pass validation orchestration with CompileError batch
        [[nodiscard]] static std::expected<void, std::vector<CompileError>>
        validate_post_pass(const Function &func) noexcept;
    };

}  // namespace jsv
