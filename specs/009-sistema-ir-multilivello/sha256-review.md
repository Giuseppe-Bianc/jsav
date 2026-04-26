# SHA-256 Review

## Scope

Review target: `include/jsavCore/util/Sha256.hpp` and `src/jsav_Core_lib/util/Sha256.cpp`.

Intended use: deterministic nominal versioning and structural hashing inside the compiler IR. This implementation is not intended for password hashing, MACs, or secret-bearing protocols.

## Security Findings

- The implementation uses fixed-size `std::uint32_t` state and round constants, which avoids width-dependent behavior.
- Padding uses an explicit 64-bit big-endian bit length, matching SHA-256 requirements for normal compiler-scale payloads.
- No global mutable state is used; the implementation is deterministic and side-effect free.
- Bitwise operations are expressed on unsigned integers only, which avoids signed-shift UB.
- The implementation is not constant-time and should not be used for secret comparisons or authentication workflows.

## Performance Findings

- The code performs a single linear preprocessing pass plus 64-round compression per block, which is appropriate for nominal versioning workloads.
- Temporary allocation is limited to the padded message vector and a fixed local schedule array per block.
- No SIMD or platform-specific acceleration is used; that keeps the implementation portable and adequate for the feature scope, but it is not tuned for bulk cryptographic throughput.

## Verdict

Approved for deterministic compiler-internal hashing under the feature constraints:

- no new dependencies
- portable C++23 implementation
- deterministic output across runs and platforms

Rejected use cases:

- password hashing
- keyed cryptography
- side-channel-sensitive workloads
