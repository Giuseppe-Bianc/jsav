// clang-format off
// NOLINTBEGIN(*-include-cleaner, *-avoid-magic-numbers, *-magic-numbers, *-owning-memory)
// clang-format on

#include "jsav/ir/PhiNode.hpp"
#include "jsav/ir/GlobalEntityId.hpp"

namespace jsv {

    // ─────────────────────────────────────────────────────────────────────────────
    // PhiNode Implementation
    // ─────────────────────────────────────────────────────────────────────────────

    PhiNode::PhiNode(GlobalEntityId target_val, std::vector<PhiIncoming> incoming) noexcept
        : target_value_(target_val), incoming_(std::move(incoming)) {
        // Generate deterministic global ID
        const std::string canonical_path = fmt::format("phi/{}", target_val.path());
        phi_id_ = GlobalEntityId::from_structural_path(canonical_path);
    }

    void PhiNode::remove_incoming(const GlobalEntityId &pred_block_id) noexcept {
        // Remove incoming edge from specified predecessor
        auto it = std::find_if(incoming_.begin(), incoming_.end(),
                               [&pred_block_id](const PhiIncoming &pi) { return pi.predecessor_block_id == pred_block_id; });

        if(it != incoming_.end()) {
            incoming_.erase(it);
        }
    }

    void PhiNode::update_incoming(const GlobalEntityId &pred_block_id, GlobalEntityId new_value_id) noexcept {
        // Update incoming value for specified predecessor
        for(auto &pi : incoming_) {
            if(pi.predecessor_block_id == pred_block_id) {
                pi.value_id = new_value_id;
                return;
            }
        }

        // If not found, add new incoming edge
        incoming_.push_back(PhiIncoming{pred_block_id, new_value_id});
    }

    std::optional<GlobalEntityId> PhiNode::try_minimize() const noexcept {
        // Try to minimize trivial PHI: all operands same?
        if(incoming_.empty()) {
            return std::nullopt;
        }

        const auto first_value = incoming_[0].value_id;
        for(const auto &pi : incoming_) {
            if(pi.value_id != first_value) {
                return std::nullopt;  // Not all the same
            }
        }

        // All incoming values are the same
        return first_value;
    }

    std::expected<void, std::vector<CompileError>>
    PhiNode::validate(const std::vector<GlobalEntityId> &current_predecessors) const noexcept {
        std::vector<CompileError> errors;

        // Check: one operand for each current predecessor
        if(incoming_.size() != current_predecessors.size()) {
            errors.emplace_back("PhiNode::validate",
                               fmt::format("PHI has {} operands but {} predecessors", incoming_.size(),
                                           current_predecessors.size()),
                               "PHI001");
        }

        // Check: all incoming values reference valid blocks
        for(const auto &pi : incoming_) {
            bool found = false;
            for(const auto &pred : current_predecessors) {
                if(pred == pi.predecessor_block_id) {
                    found = true;
                    break;
                }
            }

            if(!found) {
                errors.emplace_back("PhiNode::validate",
                                   fmt::format("Incoming edge from non-existent predecessor {}",
                                               pi.predecessor_block_id.path()),
                                   "PHI002");
            }
        }

        if(!errors.empty()) {
            return std::unexpected(errors);
        }

        return {};
    }

}  // namespace jsv

// clang-format off
// NOLINTEND(*-include-cleaner, *-avoid-magic-numbers, *-magic-numbers, *-owning-memory)
// clang-format on
