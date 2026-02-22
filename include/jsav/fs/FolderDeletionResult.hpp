// NOLINTBEGIN(*-include-cleaner)
#pragma once

#include "FileDeletionResult.hpp"  //rename to FileDeletionResult.hpp
#include "OSOperationResult.hpp"

DISABLE_WARNINGS_PUSH(4820)

namespace vnd {
    class FolderDeletionResult : public OSOperationResult {
    public:
        using OSOperationResult::OSOperationResult;

        /**
         * @brief Deletes a folder at the specified path, including its contents.
         * @param folderPath The path of the folder to delete.
         * @return A FolderDeletionResult object indicating the result of the operation.
         */
        [[nodiscard]] static auto deleteFolder(const fs::path &folderPath) -> FolderDeletionResult {  // NOLINT(*-no-recursion)
            try {
                if(!fs::exists(folderPath)) {
                    LERROR("Folder '{}' does not exist.", folderPath);
                    return {false, folderPath};
                }
                if(!fs::is_directory(folderPath)) {
                    LERROR("The path '{}' is not a folder.", folderPath);
                    return {false, folderPath};
                }

                // Recursively delete folder contents
                for(const auto &entry : fs::directory_iterator(folderPath)) {
                    // Handle symlinks first - remove the link itself, don't follow it
                    if(fs::is_symlink(entry)) {
                        std::error_code ec;
                        fs::remove(entry.path(), ec);
                        if(ec) {
                            LERROR("Failed to remove symlink '{}': {}", entry.path(), ec.message());
                            return {false, folderPath};
                        }
                        continue;
                    }
                    if(fs::is_directory(entry)) {
                        if(auto result = deleteFolder(entry.path()); !result.success()) {
                            LERROR("Failed to delete subfolder '{}'.", entry.path());
                            return {false, folderPath};
                        }
                    } else {
                        if(auto fileResult = FileDelitionResult::deleteFile(entry.path()); !fileResult.success()) {
                            LERROR("Failed to delete file '{}'.", entry.path());
                            return {false, folderPath};
                        }
                    }
                }

                // Remove the folder itself
                std::error_code ec;
                fs::remove(folderPath, ec);
                if(ec) {
                    LERROR("Failed to remove folder '{}': {}", folderPath, ec.message());
                    return {false, folderPath};
                }
#ifdef INDEPT
                LINFO("Folder '{}' deleted successfully.", folderPath);
#endif
                return {true, folderPath};
            } catch(const fs::filesystem_error &e) {
                LERROR("Filesystem error while deleting folder '{}': {}", folderPath, e.what());
                return {false, folderPath};
            } catch(const std::exception &e) {
                LERROR("Exception while deleting folder '{}': {}", folderPath, e.what());
                return {false, folderPath};
            } catch(...) {
                LERROR("Unknown error occurred while deleting folder '{}'.", folderPath);
                return {false, folderPath};
            }
        }
    };

}  // namespace vnd

DISABLE_WARNINGS_POP()

// NOLINTEND(*-include-cleaner)