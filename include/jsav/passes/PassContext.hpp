/*
 * Created by gbian on 26/04/2026.
 * Copyright (c) 2026 All rights reserved.
 */
// NOLINTBEGIN(*-include-cleaner)
#pragma once

#include "../ir/IrCommon.hpp"

namespace jsv {

    struct CanonicalPipelineConfig {
        bool deterministic_mode{true};
        std::string pipeline_name{"default"};
        std::vector<std::string> pass_order{};
    };

    struct PassContext {
        IrLevel level{IrLevel::Hir};
        CanonicalPipelineConfig config{};
        std::uint64_t deterministic_seed{};

        [[nodiscard]] std::string canonical_config_key() const {
            std::string key = FORMAT("{}:{}:{}", to_string(level), config.pipeline_name, deterministic_seed);
            for(const auto &pass_name : config.pass_order) { key += FORMAT(":{}", pass_name); }
            return key;
        }
    };

}  // namespace jsv

// NOLINTEND(*-include-cleaner)
