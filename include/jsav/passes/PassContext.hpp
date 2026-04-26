/*
 * Created by gbian on 26/04/2026.
 * Copyright (c) 2026 All rights reserved.
 */
// NOLINTBEGIN(*-include-cleaner)
#pragma once

#include "../ir/IrCommon.hpp"

namespace jsv {

    /// Deterministic seed for reproducible pass execution
    struct DeterministicSeed {
        std::uint64_t value;

        bool operator==(const DeterministicSeed& other) const noexcept {
            return value == other.value;
        }
    };

    /// Canonical pipeline configuration for deterministic execution
    struct CanonicalPipelineConfig {
        bool enableOptimizations;
        bool enableAnalyses;
        bool enableValidation;
        std::uint64_t maxIterations;
        std::uint64_t timeoutMs;

        bool operator==(const CanonicalPipelineConfig& other) const noexcept {
            return enableOptimizations == other.enableOptimizations &&
                   enableAnalyses == other.enableAnalyses &&
                   enableValidation == other.enableValidation &&
                   maxIterations == other.maxIterations &&
                   timeoutMs == other.timeoutMs;
        }

        /// Create default canonical configuration
        static CanonicalPipelineConfig canonical() noexcept {
            return {
                true,   // enableOptimizations
                true,   // enableAnalyses
                true,   // enableValidation
                1000,   // maxIterations
                0       // timeoutMs (no timeout)
            };
        }
    };

    /// Context for pass execution - deterministic and reproducible
    struct PassContext {
        IrLevel level;
        CanonicalPipelineConfig config;
        DeterministicSeed seed;
        std::string_view pass_name;
        std::uint32_t pass_index;

        /// Check configuration equality for determinism verification
        bool operator==(const PassContext& other) const noexcept {
            return level == other.level &&
                   config == other.config &&
                   seed == other.seed &&
                   pass_name == other.pass_name &&
                   pass_index == other.pass_index;
        }
    };

}  // namespace jsv

// NOLINTEND(*-include-cleaner)
