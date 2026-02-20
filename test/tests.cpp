#include <catch2/catch_test_macros.hpp>
// clang-format off
// NOLINTBEGIN(*-include-cleaner, *-avoid-magic-numbers, *-magic-numbers, *-unchecked-optional-access, *-avoid-do-while, *-use-anonymous-namespace, *-qualified-auto, *-suspicious-stringview-data-usage, *-err58-cpp, *-function-cognitive-complexity, *-macro-usage, *-unnecessary-copy-initialization)
// clang-format on
#include <future>

#include "testsConstanst.hpp"

#define REQ_FORMAT(type, string) REQUIRE(FORMAT("{}", type) == (string));
#define REQ_FORMAT_COMPTOK(type, string) REQUIRE(FORMAT("{}", comp_tokType(type)) == (string));
#define MSG_FORMAT(...) Message(FORMAT(__VA_ARGS__))

static fs::path createTestFolderStructure() {
    fs::path testFolder = fs::temp_directory_path() / "test_folder_deletion";
    if(fs::exists(testFolder)) { fs::remove_all(testFolder); }

    fs::create_directories(testFolder / "subfolder1");
    fs::create_directories(testFolder / "subfolder2" / "nested");

    std::ofstream(testFolder / "file1.txt") << "File 1 content";
    std::ofstream(testFolder / "subfolder1" / "file2.txt") << "File 2 content";
    std::ofstream(testFolder / "subfolder2" / "nested" / "file3.txt") << "File 3 content";

    return testFolder;
}

TEST_CASE("Logger setup", "[setup_logger]") {
    SECTION("Default setup") { REQUIRE_NOTHROW(setup_logger()); }
    SECTION("Logger sinks") {
        setup_logger();
        auto logger = spdlog::default_logger();
        REQUIRE(logger->sinks().size() == 1);
    }
}

TEST_CASE("TimeValues initialization", "[TimeValues]") {
    using vnd::TimeValues;

    SECTION("Default Constructor") {
        const TimeValues time;
        REQUIRE(time.get_seconds() == 0.0L);
        REQUIRE(time.get_millis() == 0.0L);
        REQUIRE(time.get_micro() == 0.0L);
        REQUIRE(time.get_nano() == 0.0L);
    }

    SECTION("Initialization with nanoseconds") {
        const TimeValues time(1'000'000.0L);  // 1 millisecond in nanoseconds
        REQUIRE(time.get_seconds() == 0.001L);
        REQUIRE(time.get_millis() == 1.0L);
        REQUIRE(time.get_micro() == 1000.0L);
        REQUIRE(time.get_nano() == 1'000'000.0L);
    }

    SECTION("Initialization with individual time units") {
        const TimeValues time(1.0L, 1000.0L, 1'000'000.0L, 1'000'000'000.0L);  // 1 second
        REQUIRE(time.get_seconds() == 1.0L);
        REQUIRE(time.get_millis() == 1000.0L);
        REQUIRE(time.get_micro() == 1'000'000.0L);
        REQUIRE(time.get_nano() == 1'000'000'000.0L);
    }
}

TEST_CASE("ValueLabel functionality", "[ValueLabel]") {
    using vnd::ValueLabel;

    SECTION("Transform time in microseconds") {
        const ValueLabel value(time_val_micro, "us");
        REQUIRE(value.transformTimeMicro(time_val_micro) == "1500us,0ns");

        const ValueLabel valueNonExact(time_val_micro2, "us");
        REQUIRE(valueNonExact.transformTimeMicro(time_val_micro2) == "1500us,500ns");
    }

    SECTION("Transform time in milliseconds") {
        const ValueLabel value(time_val_milli, "ms");
        REQUIRE(value.transformTimeMilli(time_val_milli) == "2ms,500us,0ns");

        const ValueLabel valueNonExact(time_val_milli2, "ms");
        REQUIRE(valueNonExact.transformTimeMilli(time_val_milli2) == "2ms,505us,0ns");
    }

    SECTION("Transform time in seconds") {
        const ValueLabel value(time_val_second, "s");
        REQUIRE(value.transformTimeSeconds(time_val_second) == "1s,0ms,0us,0ns");

        const ValueLabel valueNonExact(time_val_second2, "s");
        REQUIRE(valueNonExact.transformTimeSeconds(time_val_second2) == "1s,5ms,1us,0ns");
    }

    SECTION("ToString based on time label") {
        const ValueLabel secondsVal(2.0L, "s");
        REQUIRE(secondsVal.toString() == "2s,0ms,0us,0ns");

        const ValueLabel millisVal(2500.0L, "ms");
        REQUIRE(millisVal.toString() == "2500ms,0us,0ns");

        const ValueLabel microsVal(1500.0L, "us");
        REQUIRE(microsVal.toString() == "1500us,0ns");

        const ValueLabel unknownVal(3.0L, "unknown");
        REQUIRE(unknownVal.toString() == "3 unknown");
    }
}

TEST_CASE("Times functionality for  nano seconds", "[Times]") {
    const vnd::Times time(10.0L);  // 1 millisecond
    REQUIRE(time.getRelevantTimeframe().toString() == "10 ns");
}

TEST_CASE("Times functionality", "[Times]") {
    using vnd::Times;
    using vnd::TimeValues;
    using vnd::ValueLabel;

    SECTION("Initialization with nanoseconds") {
        const Times time(1'000'000.0L);  // 1 millisecond
        const ValueLabel relevantTime = time.getRelevantTimeframe();
        REQUIRE(relevantTime.toString() == "1000us,0ns");
    }

    SECTION("Initialization with TimeValues and custom labels") {
        const TimeValues timeVals(0.5L, 500.0L, 500'000.0L, 500'000'000.0L);  // 0.5 seconds
        const Times time(timeVals, "seconds", "milliseconds", "microseconds", "nanoseconds");

        const ValueLabel relevantTime = time.getRelevantTimeframe();
        REQUIRE(relevantTime.toString() == "500 milliseconds");
    }

    SECTION("Switch between time units") {
        const TimeValues timeVals(0.001L, 1.0L, 1000.0L, 1'000'000.0L);  // 1 millisecond
        const Times time(timeVals);

        const ValueLabel relevantTime = time.getRelevantTimeframe();
        REQUIRE(relevantTime.toString() == "1000us,0ns");
    }

    SECTION("Very small nanoseconds") {
        const TimeValues timeVals(0.000001L, 0.001L, 1.0L, 1'000.0L);  // 1 microsecond
        const Times time(timeVals);

        const ValueLabel relevantTime = time.getRelevantTimeframe();
        REQUIRE(relevantTime.toString() == "1000 ns");
    }
}

TEST_CASE("Corner cases for TimeValues and Times", "[TimeValues][Times][CornerCases]") {
    using vnd::Times;
    using vnd::TimeValues;
    using vnd::ValueLabel;

    SECTION("Negative values") {
        const TimeValues negativeTime(-1000000.0L);  // -1 millisecond
        const Times time(negativeTime);

        const ValueLabel relevantTime = time.getRelevantTimeframe();
        REQUIRE(relevantTime.toString() == "-1000000 ns");
    }

    SECTION("Zero values") {
        const TimeValues zeroTime(0.0L);  // Zero nanoseconds
        const Times time(zeroTime);

        const ValueLabel relevantTime = time.getRelevantTimeframe();
        REQUIRE(relevantTime.toString() == "0 ns");
    }

    SECTION("Large values") {
        const long double largeValue = 1'000'000'000'000.0L;  // 1 second in nanoseconds
        const TimeValues largeTime(largeValue);               // 1 second
        const Times time(largeTime);

        const ValueLabel relevantTime = time.getRelevantTimeframe();
        REQUIRE(relevantTime.toString() == "1000s,0ms,0us,0ns");
    }
}

TEST_CASE("get_current_timestamp() tests", "[timestamp]") {
    SECTION("Basic test") {
        auto timestamp = get_current_timestamp();
        REQUIRE(timestamp.size() >= timestampSize);
    }

    SECTION("Repeatability test") {
        auto timestamp1 = get_current_timestamp();
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        auto timestamp2 = get_current_timestamp();
        REQUIRE(timestamp1 != timestamp2);
    }

    SECTION("Concurrency test") {
        constexpr int num_threads = 4;
        std::vector<std::future<std::string>> futures;
        for(int i = 0; i < num_threads; ++i) {
            // NOLINTNEXTLINE(*-inefficient-vector-operation)
            futures.emplace_back(std::async(std::launch::async, []() { return get_current_timestamp(); }));
        }
        for(auto &future : futures) {
            auto timestamp = future.get();
            REQUIRE(timestamp.size() >= timestampSize);
        }
    }
}

TEST_CASE("createFile: Successfully create a file with content", "[FileCreationResult]") {
    const fs::path testDir = fs::temp_directory_path() / "test_file_creation";
    fs::create_directories(testDir);

    const std::string fileName = "test_file.txt";
    std::stringstream content;
    content << "Hello, this is a test file.";

    auto result = vnd::FileCreationResult::createFile(testDir, fileName, content);

    const fs::path createdFilePath = testDir / fileName;
    REQUIRE(result.success());
    REQUIRE(fs::exists(createdFilePath));

    const std::string filecontent = vnd::readFromFile(createdFilePath.string());

    REQUIRE(filecontent == content.str());

    // Cleanup
    fs::remove_all(testDir);
}

TEST_CASE("createFile: Attempt to create a file in a non-existent directory", "[FileCreationResult]") {
    const fs::path nonExistentDir = fs::temp_directory_path() / "non_existent_directory";
    const std::string fileName = "test_file.txt";
    std::stringstream content;
    content << "Content for non-existent directory test.";

    const auto result = vnd::FileCreationResult::createFile(nonExistentDir, fileName, content);

    REQUIRE_FALSE(result.success());
    REQUIRE(!fs::exists(nonExistentDir / fileName));
}

TEST_CASE("createFile: Handle file creation when file already exists", "[FileCreationResult]") {
    const fs::path testDir = fs::temp_directory_path() / "test_file_creation_existing";
    fs::create_directories(testDir);

    const std::string fileName = "existing_file.txt";
    std::stringstream initialContent;
    initialContent << "Initial content.";

    const fs::path existingFilePath = testDir / fileName;
    std::ofstream outfile(existingFilePath);
    outfile << initialContent.rdbuf();
    outfile.close();

    REQUIRE(fs::exists(existingFilePath));

    std::stringstream newContent;
    newContent << "New content that overwrites.";

    auto result = vnd::FileCreationResult::createFile(testDir, fileName, newContent);

    REQUIRE(result.success());
    REQUIRE(fs::exists(existingFilePath));

    const std::string filecontent = vnd::readFromFile(existingFilePath.string());

    REQUIRE(filecontent == newContent.str());

    // Cleanup
    fs::remove_all(testDir);
}
TEST_CASE("createFile: Attempt to create a file with empty content", "[FileCreationResult]") {
    const fs::path testDir = fs::temp_directory_path() / "test_empty_content";
    fs::create_directories(testDir);

    const std::string fileName = "empty_content_file.txt";
    const std::stringstream emptyContent;

    auto result = vnd::FileCreationResult::createFile(testDir, fileName, emptyContent);

    const fs::path createdFilePath = testDir / fileName;
    REQUIRE(result.success());
    REQUIRE(fs::exists(createdFilePath));

    const std::string filecontent = vnd::readFromFile(createdFilePath.string());

    REQUIRE(filecontent.empty());

    // Cleanup
    fs::remove_all(testDir);
}

TEST_CASE("deleteFile: Successfully delete an existing file", "[FileDelitionResult]") {
    const fs::path testFile = fs::temp_directory_path() / "test_file_to_delete.txt";

    // Create the test file
    std::ofstream(testFile) << "Sample content for deletion test";
    REQUIRE(fs::exists(testFile));

    const auto result = vnd::FileDelitionResult::deleteFile(testFile);

    REQUIRE(result.success());
    REQUIRE(!fs::exists(testFile));
}

TEST_CASE("deleteFile: Attempt to delete a non-existent file", "[FileDelitionResult]") {
    const fs::path nonExistentFile = fs::temp_directory_path() / "non_existent_file.txt";

    REQUIRE(!fs::exists(nonExistentFile));

    const auto result = vnd::FileDelitionResult::deleteFile(nonExistentFile);

    REQUIRE_FALSE(result.success());
}

TEST_CASE("deleteFile: Attempt to delete a directory instead of a file", "[FileDelitionResult]") {
    const fs::path testDirectory = fs::temp_directory_path() / "test_directory";
    fs::create_directories(testDirectory);

    REQUIRE(fs::exists(testDirectory));
    REQUIRE(fs::is_directory(testDirectory));

    const auto result = vnd::FileDelitionResult::deleteFile(testDirectory);

    REQUIRE_FALSE(result.success());
    REQUIRE(fs::exists(testDirectory));  // Ensure the directory is not accidentally deleted

    // Cleanup
    fs::remove_all(testDirectory);
}

TEST_CASE("deleteFile: Handle exceptions gracefully", "[FileDelitionResult]") {
    const fs::path invalidPath;

    const auto result = vnd::FileDelitionResult::deleteFile(invalidPath);

    REQUIRE_FALSE(result.success());
}

TEST_CASE("deleteFolder: Successfully delete an existing folder structure", "[FolderDeletionResult]") {
    const fs::path testFolder = createTestFolderStructure();
    REQUIRE(fs::exists(testFolder));

    const auto result = vnd::FolderDeletionResult::deleteFolder(testFolder);

    REQUIRE(result.success());
    REQUIRE(!fs::exists(testFolder));
}

TEST_CASE("deleteFolder: Attempt to delete a non-existent folder", "[FolderDeletionResult]") {
    const fs::path nonExistentFolder = fs::temp_directory_path() / "non_existent_folder";
    REQUIRE(!fs::exists(nonExistentFolder));

    const auto result = vnd::FolderDeletionResult::deleteFolder(nonExistentFolder);

    REQUIRE_FALSE(result.success());
}

TEST_CASE("deleteFolder: Attempt to delete a file path instead of a folder", "[FolderDeletionResult]") {
    const fs::path testFile = fs::temp_directory_path() / "test_file.txt";

    // Create the test file
    std::ofstream(testFile) << "Test content";
    REQUIRE(fs::exists(testFile));

    const auto result = vnd::FolderDeletionResult::deleteFolder(testFile);

    REQUIRE_FALSE(result.success());
    REQUIRE(fs::exists(testFile));  // Ensure the file is not accidentally deleted

    // Cleanup
    fs::remove(testFile);
}

TEST_CASE("deleteFolder: Folder with nested subfolders and files", "[FolderDeletionResult]") {
    const fs::path testFolder = createTestFolderStructure();

    REQUIRE(fs::exists(testFolder));
    REQUIRE(fs::exists(testFolder / "subfolder1"));
    REQUIRE(fs::exists(testFolder / "subfolder2" / "nested" / "file3.txt"));

    auto result = vnd::FolderDeletionResult::deleteFolder(testFolder);

    REQUIRE(result.success());
    REQUIRE(!fs::exists(testFolder));
}

TEST_CASE("deleteFolder: Handle exceptions gracefully", "[FolderDeletionResult]") {
    const fs::path invalidPath;

    const auto result = vnd::FolderDeletionResult::deleteFolder(invalidPath);

    REQUIRE_FALSE(result.success());
}
// clang-format off
// NOLINTEND(*-include-cleaner, *-avoid-magic-numbers, *-magic-numbers, *-unchecked-optional-access, *-avoid-do-while, *-use-anonymous-namespace, *-qualified-auto, *-suspicious-stringview-data-usage, *-err58-cpp, *-function-cognitive-complexity, *-macro-usage, *-unnecessary-copy-initialization)
// clang-format on
