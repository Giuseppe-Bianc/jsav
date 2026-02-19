// NOLINTBEGIN(*-include-cleaner)
#pragma once

#include "../headers.hpp"

namespace vnd {
    constexpr std::hash<bool> bool_hasher;
#ifdef __llvm__
    constexpr std::hash<std::string> string_hasher;
#else
    constexpr std::hash<fs::path> path_hasher;
#endif
    /**
     * @brief Computes the build folder path relative to the given parent directory.
     * @param parentDir The source directory path (e.g., project source root).
     * @return Path to the build folder (sibling to parentDir named VANDIOR_BUILDFOLDER).
     * @pre parentDir must not be empty or a root path.
     */
    inline auto GetBuildFolder(const fs::path &parentDir) -> fs::path {
        fs::path parent = parentDir.lexically_normal();
        if(parent.filename() == "") { parent = parent.parent_path(); }
        if(parent == "..") { return parent / VANDIOR_BUILDFOLDER; }
        return parent.parent_path() / VANDIOR_BUILDFOLDER;
    }

}  // namespace vnd

// NOLINTEND(*-include-cleaner)