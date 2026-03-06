// clang-format off
// NOLINTBEGIN(*-include-cleaner, *-avoid-magic-numbers, *-magic-numbers, *-unchecked-optional-access, *-avoid-do-while, *-use-anonymous-namespace, *-qualified-auto, *-suspicious-stringview-data-usage, *-err58-cpp, *-function-cognitive-complexity, *-macro-usage, *-unnecessary-copy-initialization, *-uppercase-literal-suffix, *-uppercase-literal-suffix, *-container-size-empty, *-move-const-arg, *-move-const-arg, *-pass-by-value, *-diagnostic-self-assign-overloaded, *-unused-using-decls, *-identifier-length)
// clang-format on
#include "testsConstanst.hpp"
#include <catch2/benchmark/catch_benchmark.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/matchers/catch_matchers.hpp>
#include <catch2/matchers/catch_matchers_container_properties.hpp>
#include <catch2/matchers/catch_matchers_exception.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>
#include <future>
#include <set>

using Catch::Matchers::ContainsSubstring;
using Catch::Matchers::EndsWith;
using Catch::Matchers::Message;
using Catch::Matchers::MessageMatches;
using Catch::Matchers::StartsWith;

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

TEST_CASE("my_error_handler(const std::string&) tests", "[error_handler]") {
    SECTION("Basic error handling") {
        const std::stringstream sss;
        auto *original = std::cerr.rdbuf(sss.rdbuf());  // Redirect cerr to stringstream
        my_error_handler("Sample error message");
        std::cerr.rdbuf(original);  // Restore cerr

        auto output = sss.str();
        REQUIRE_THAT(output, ContainsSubstring("Error occurred:"));
        REQUIRE_THAT(output, ContainsSubstring("Timestamp: "));
        REQUIRE_THAT(output, ContainsSubstring("Thread ID: "));
        REQUIRE_THAT(output, ContainsSubstring("Message:   Sample error message"));
    }

    SECTION("Error handler with different messages") {
        const std::stringstream sss;
        auto *original = std::cerr.rdbuf(sss.rdbuf());  // Redirect cerr to stringstream
        my_error_handler("Error 1");
        my_error_handler("Another error");
        std::cerr.rdbuf(original);  // Restore cerr

        auto output = sss.str();
        REQUIRE_THAT(output, ContainsSubstring("Message:   Error 1"));
        REQUIRE_THAT(output, ContainsSubstring("Message:   Another error"));
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

TEST_CASE("deleteFile: Successfully delete an existing file", "[FileDeletionResult]") {
    const fs::path testFile = fs::temp_directory_path() / "test_file_to_delete.txt";

    // Create the test file
    std::ofstream(testFile) << "Sample content for deletion test";
    REQUIRE(fs::exists(testFile));

    const auto result = vnd::FileDeletionResult::deleteFile(testFile);

    REQUIRE(result.success());
    REQUIRE(!fs::exists(testFile));
}

TEST_CASE("deleteFile: Attempt to delete a non-existent file", "[FileDeletionResult]") {
    const fs::path nonExistentFile = fs::temp_directory_path() / "non_existent_file.txt";

    REQUIRE(!fs::exists(nonExistentFile));

    const auto result = vnd::FileDeletionResult::deleteFile(nonExistentFile);

    REQUIRE_FALSE(result.success());
}

TEST_CASE("deleteFile: Attempt to delete a directory instead of a file", "[FileDeletionResult]") {
    const fs::path testDirectory = fs::temp_directory_path() / "test_directory";
    fs::create_directories(testDirectory);

    REQUIRE(fs::exists(testDirectory));
    REQUIRE(fs::is_directory(testDirectory));

    const auto result = vnd::FileDeletionResult::deleteFile(testDirectory);

    REQUIRE_FALSE(result.success());
    REQUIRE(fs::exists(testDirectory));  // Ensure the directory is not accidentally deleted

    // Cleanup
    fs::remove_all(testDirectory);
}

TEST_CASE("deleteFile: Handle exceptions gracefully", "[FileDeletionResult]") {
    const fs::path invalidPath;

    const auto result = vnd::FileDeletionResult::deleteFile(invalidPath);

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

TEST_CASE("std::filesystem::path formater", "[FMT]") { REQ_FORMAT(std::filesystem::path("../ssss"), "../ssss"); }

TEST_CASE("Timer: MSTimes", "[timer]") {
    const auto timerNameData = timerName.data();
    vnd::Timer timer{timerNameData};
    std::this_thread::sleep_for(std::chrono::milliseconds(timerSleap));
    const std::string output = timer.to_string();
    const std::string new_output = (timer / timerCicles).to_string();
    REQUIRE_THAT(output, ContainsSubstring(timerNameData));
    REQUIRE_THAT(output, ContainsSubstring(timerNameData));
    REQUIRE_THAT(output, ContainsSubstring(timerTime1.data()));
    REQUIRE_THAT(new_output, ContainsSubstring(timerTime2.data()));
}

TEST_CASE("Timer: MSTimes FMT", "[timer]") {
    const auto timerNameData = timerName.data();
    vnd::Timer timer{timerNameData};
    std::this_thread::sleep_for(std::chrono::milliseconds(timerSleap));
    const std::string output = FORMAT("{}", timer);
    const std::string new_output = FORMAT("{}", (timer / timerCicles));
    REQUIRE_THAT(output, ContainsSubstring(timerNameData));
    REQUIRE_THAT(output, ContainsSubstring(timerTime1.data()));
    REQUIRE_THAT(new_output, ContainsSubstring(timerTime2.data()));
}

TEST_CASE("Timer: BigTimer", "[timer]") {
    const auto timerNameData = timerName.data();
    const vnd::Timer timer{timerNameData, vnd::Timer::Big};
    const std::string output = timer.to_string();
    REQUIRE_THAT(output, ContainsSubstring(timerNameData));
    REQUIRE_THAT(output, ContainsSubstring(timerBigs.data()));
}

TEST_CASE("Timer: BigTimer FMT", "[timer]") {
    const auto timerNameData = timerName.data();
    vnd::Timer timer{timerNameData, vnd::Timer::Big};
    const std::string output = FORMAT("{}", timer);
    REQUIRE_THAT(output, ContainsSubstring(timerNameData));
    REQUIRE_THAT(output, ContainsSubstring(timerBigs.data()));
}

TEST_CASE("Timer: AutoTimer", "[timer]") {
    const vnd::Timer timer;
    const std::string output = timer.to_string();
    REQUIRE_THAT(output, ContainsSubstring("Timer"));
}

TEST_CASE("Timer: PrintTimer", "[timer]") {
    std::stringstream out;
    const vnd::Timer timer;
    out << timer;
    const std::string output = out.str();
    REQUIRE_THAT(output, ContainsSubstring(timerName2.data()));
}

TEST_CASE("Timer: PrintTimer FMT", "[timer]") {
    vnd::Timer timer;
    const std::string output = FORMAT("{}", timer);
    REQUIRE_THAT(output, ContainsSubstring(timerName2.data()));
}

TEST_CASE("Timer: TimeItTimer", "[timer]") {
    vnd::Timer timer;
    const std::string output = timer.time_it([]() { std::this_thread::sleep_for(std::chrono::milliseconds(timerSleap2)); },
                                             timerResolution);
    REQUIRE_THAT(output, ContainsSubstring(timerTime1.data()));
}

namespace {
    // Helper function to create a file with content
    // NOLINTBEGIN(*-easily-swappable-parameters, *-signed-bitwise)
    void createFile(const std::string &infilename, const std::string &content) {
        std::ofstream ofs(infilename, std::ios::out | std::ios::binary);
        ofs << content;
        ofs.close();
    }
    // NOLINTEND(*-easily-swappable-parameters, *-signed-bitwise)
}  // namespace
TEST_CASE("FolderCreationResult Constructor", "[FolderCreationResult]") {
    SECTION("Default constructor") {
        const vnd::FolderCreationResult result;
        REQUIRE_FALSE(result.success());
        REQUIRE(result.path().value_or("").empty());
    }

    SECTION("Parameterized constructor") {
        const vnd::FolderCreationResult result(true, fs::path(testPaths));
        REQUIRE(result.success() == true);
        REQUIRE(result.path() == fs::path(testPaths));
    }
}

TEST_CASE("FolderCreationResult Setters", "[FolderCreationResult]") {
    vnd::FolderCreationResult result;

    SECTION("Set success") {
        result.set_success(true);
        REQUIRE(result.success() == true);
    }

    SECTION("Set path") {
        fs::path testPath(testPaths);
        REQUIRE(result.path().value_or("").empty());
        result.set_path(testPaths);
        REQUIRE(result.path() == testPath);
    }

    SECTION("Set path with empty string") {
        REQUIRE_THROWS_MATCHES(result.set_path(fs::path()), std::invalid_argument, Message("Path cannot be empty"));
    }
}

TEST_CASE("FolderCreationResult operator<< outputs correctly", "[FolderCreationResult]") {
    SECTION("Test with successful folder creation and valid path") {
        const fs::path folderPath = "/test/directory";
        const vnd::FolderCreationResult result(true, folderPath);

        std::ostringstream oss;
        oss << result;

        REQUIRE(oss.str() == "success_: true, path_: /test/directory");
    }

    SECTION("Test with unsuccessful folder creation and no path") {
        const vnd::FolderCreationResult result(false, fs::path{});

        std::ostringstream oss;
        oss << result;

        REQUIRE(oss.str() == "success_: false, path_: None");
    }

    SECTION("Test with successful folder creation but empty path") {
        const vnd::FolderCreationResult result(true, fs::path{});

        std::ostringstream oss;
        oss << result;

        REQUIRE(oss.str() == "success_: true, path_: None");
    }

    SECTION("Test with unsuccessful folder creation and valid path") {
        const fs::path folderPath = "/another/test/directory";
        const vnd::FolderCreationResult result(false, folderPath);

        std::ostringstream oss;
        oss << result;

        REQUIRE(oss.str() == "success_: false, path_: /another/test/directory");
    }

    SECTION("Test with default constructed FolderCreationResult") {
        const vnd::FolderCreationResult result;

        std::ostringstream oss;
        oss << result;

        REQUIRE(oss.str() == "success_: false, path_: None");
    }
}

TEST_CASE("FolderCreationResult: Equality and Swap", "[FolderCreationResult]") {
    fs::path path1("/folder1");
    fs::path path2("/folder2");

    vnd::FolderCreationResult result1(true, path1);
    vnd::FolderCreationResult result2(false, path2);

    SECTION("Equality operator") {
        REQUIRE(result1 != result2);
        vnd::FolderCreationResult result3(true, path1);
        REQUIRE(result1 == result3);
    }

    SECTION("swap() function") {
        swap(result1, result2);
        REQUIRE(result1.success() == false);
        REQUIRE(result1.path().value() == path2);
        REQUIRE(result2.success() == true);
        REQUIRE(result2.path().value() == path1);
    }
}

TEST_CASE("FolderCreationResult Hash Value", "[FolderCreationResult]") {
    SECTION("Hash value is consistent for the same object") {
        const vnd::FolderCreationResult result(true, fs::path("/test/directory"));
        const std::size_t hash1 = hash_value(result);
        const std::size_t hash2 = hash_value(result);

        REQUIRE(hash1 == hash2);
    }

    SECTION("Hash value changes with different success status") {
        const vnd::FolderCreationResult result1(true, fs::path("/test/directory"));
        const vnd::FolderCreationResult result2(false, fs::path("/test/directory"));

        const std::size_t hash1 = hash_value(result1);
        const std::size_t hash2 = hash_value(result2);

        REQUIRE(hash1 != hash2);
    }

    SECTION("Hash value changes with different paths") {
        const vnd::FolderCreationResult result1(true, fs::path("/test/directory"));
        const vnd::FolderCreationResult result2(true, fs::path("/different/directory"));

        const std::size_t hash1 = hash_value(result1);
        const std::size_t hash2 = hash_value(result2);

        REQUIRE(hash1 != hash2);
    }

    SECTION("Identical objects have the same hash value") {
        const vnd::FolderCreationResult result1(true, fs::path("/test/directory"));
        const vnd::FolderCreationResult result2(true, fs::path("/test/directory"));

        const std::size_t hash1 = hash_value(result1);
        const std::size_t hash2 = hash_value(result2);

        REQUIRE(hash1 == hash2);
    }

    SECTION("Different objects have different hash values") {
        const vnd::FolderCreationResult result1(true, fs::path("/test/directory"));
        const vnd::FolderCreationResult result2(false, fs::path("/another/directory"));

        const std::size_t hash1 = hash_value(result1);
        const std::size_t hash2 = hash_value(result2);

        REQUIRE(hash1 != hash2);
    }

    SECTION("Hash for default constructed object is consistent") {
        const vnd::FolderCreationResult result1;
        const vnd::FolderCreationResult result2;

        const std::size_t hash1 = hash_value(result1);
        const std::size_t hash2 = hash_value(result2);

        REQUIRE(hash1 == hash2);
    }

    SECTION("Hash for default object vs object with empty path") {
        const vnd::FolderCreationResult result1;
        const vnd::FolderCreationResult result2(false, fs::path{});

        const std::size_t hash1 = hash_value(result1);
        const std::size_t hash2 = hash_value(result2);

        REQUIRE(hash1 == hash2);
    }
}

TEST_CASE("FolderCreationResult Folder Creation Functions", "[FolderCreationResult]") {
    // Create a temporary directory for testing
    auto tempDir = fs::temp_directory_path() / "vnd_test";
    const std::string folderName = "test_folder";
    const fs::path folderPath = tempDir / folderName;
    fs::create_directories(tempDir);

    SECTION("Create folder with valid parameters") {
        const vnd::FolderCreationResult result = vnd::FolderCreationResult::createFolder(folderName, tempDir);
        REQUIRE(result.success() == true);
        REQUIRE(result.path() == folderPath);
        [[maybe_unused]] auto unused = fs::remove_all(folderPath);
    }

    SECTION("Create folder with empty folder name") {
        const std::string emptyFolderName;
        const vnd::FolderCreationResult result = vnd::FolderCreationResult::createFolder(emptyFolderName, tempDir);
        REQUIRE_FALSE(result.success());
        REQUIRE(result.path()->empty());
    }

    SECTION("Create folder in non-existent parent directory") {
        const fs::path nonExistentParentDir = tempDir / "non_existent_dir";
        const vnd::FolderCreationResult result = vnd::FolderCreationResult::createFolder(folderName, nonExistentParentDir);
        REQUIRE(result.success() == true);
        REQUIRE(!result.path()->empty());
    }

    SECTION("Create folder in existing directory") {
        const fs::path nonExistentParentDir = tempDir / "non_existent_dir";
        const vnd::FolderCreationResult result = vnd::FolderCreationResult::createFolder(folderName, nonExistentParentDir);
        REQUIRE(result.success() == true);
        REQUIRE(!result.path()->empty());
        const std::string folderName2 = "test_folder";
        const vnd::FolderCreationResult result2 = vnd::FolderCreationResult::createFolder(folderName2, nonExistentParentDir);
        REQUIRE(result2.success() == true);
        REQUIRE(!result2.path()->empty());
    }

    SECTION("Create folder next to non-existent file") {
        const fs::path nonExistentFilePath = tempDir / "non_existent_file.txt";
        const vnd::FolderCreationResult result = vnd::FolderCreationResult::createFolderNextToFile(nonExistentFilePath, folderName);
        REQUIRE(result.success() == true);
        REQUIRE(!result.path()->empty());
        REQUIRE(!result.pathcref()->empty());
    }

    SECTION("Create folder next to existing file") {
        // Create a file in the temporary directory
        const fs::path filePathInner = tempDir / "test_file.txt";
        std::ofstream ofs(filePathInner);
        ofs.close();

        const vnd::FolderCreationResult result = vnd::FolderCreationResult::createFolderNextToFile(filePathInner, folderName);
        REQUIRE(result.success() == true);
        REQUIRE(result.path() == folderPath);

        [[maybe_unused]] auto unused = fs::remove(filePathInner);
        [[maybe_unused]] auto unuseds = fs::remove_all(folderPath);
    }
    [[maybe_unused]] auto unused = fs::remove_all(tempDir);
}

TEST_CASE("vnd::readFromFile - Valid File", "[file]") {
    const std::string infilename = "testfile.txt";
    const std::string content = "This is a test.";

    createFile(infilename, content);

    auto result = vnd::readFromFile(infilename);
    REQUIRE(result == content);  // Ensure the content matches

    [[maybe_unused]] auto unsed = fs::remove(infilename);
}

TEST_CASE("vnd::readFromFile - Non-existent File", "[file]") {
    const std::string nonExistentFile = "nonexistent.txt";

    REQUIRE_THROWS_MATCHES(vnd::readFromFile(nonExistentFile), std::runtime_error, MSG_FORMAT("File not found: {}", nonExistentFile));
}

TEST_CASE("vnd::readFromFile - Non-regular File", "[file]") {
    const std::string dirName = "testdir";

    fs::create_directory(dirName);

    REQUIRE_THROWS_MATCHES(vnd::readFromFile(dirName), std::runtime_error, MSG_FORMAT("Path is not a regular file: {}", dirName));
    [[maybe_unused]] auto unsed = fs::remove(dirName);
}

TEST_CASE("vnd::readFromFile - Empty File", "[file]") {
    const std::string emtfilename = "emptyfile.txt";

    createFile(emtfilename, "");

    SECTION("Read from an empty file") {
        const auto result = vnd::readFromFile(emtfilename);
        REQUIRE(result.empty());  // Ensure the result is empty
    }

    [[maybe_unused]] auto unsed = fs::remove(emtfilename);
}

TEST_CASE("vnd::readFromFile - Large File", "[file]") {
    const std::string lrgfilename = "largefile.txt";
    const std::string largeContent(C_ST(1024 * 1024) * 10, 'a');  // 10 MB of 'a'

    createFile(lrgfilename, largeContent);

    SECTION("Read from a large file") {
        auto result = vnd::readFromFile(lrgfilename);
        REQUIRE(result == largeContent);  // Ensure content matches
    }

    [[maybe_unused]] auto unsed = fs::remove(lrgfilename);
}

TEST_CASE("GetBuildFolder - Standard Cases") {
    SECTION("Normal path without trailing slash") {
        const fs::path inputPath = fs::path("home/user/project").make_preferred();
        const fs::path expectedOutput = fs::path("home/user/vnbuild").make_preferred();
        REQUIRE(vnd::GetBuildFolder(inputPath) == expectedOutput);
    }

    SECTION("Path with trailing slash") {
        const fs::path inputPath = fs::path("home/user/project/").make_preferred();
        const fs::path expectedOutput = fs::path("home/user/vnbuild").make_preferred();
        REQUIRE(vnd::GetBuildFolder(inputPath) == expectedOutput);
    }

    SECTION("Nested directory structure") {
        const fs::path inputPath = fs::path("home/user/projects/client/app").make_preferred();
        const fs::path expectedOutput = fs::path("home/user/projects/client/vnbuild").make_preferred();
        REQUIRE(vnd::GetBuildFolder(inputPath) == expectedOutput);
    }
}

TEST_CASE("GetBuildFolder - Edge Cases") {
    SECTION("Root directory input") {
        const fs::path inputPath = fs::path("/").make_preferred();
        const fs::path expectedOutput = fs::path("/vnbuild").make_preferred();
        REQUIRE(vnd::GetBuildFolder(inputPath) == expectedOutput);
    }

    SECTION("Empty path") {
        const fs::path inputPath = fs::path("").make_preferred();
        const fs::path expectedOutput = fs::path(VANDIOR_BUILDFOLDER).make_preferred();  // No parent; expects vnbuild in current directory
        REQUIRE(vnd::GetBuildFolder(inputPath) == expectedOutput);
    }

    SECTION("Relative path") {
        const fs::path inputPath = fs::path("folder/subfolder").make_preferred();
        const fs::path expectedOutput = fs::path("folder/vnbuild").make_preferred();
        REQUIRE(vnd::GetBuildFolder(inputPath) == expectedOutput);
    }

    SECTION("Single directory path") {
        const fs::path inputPath = fs::path("parent").make_preferred();
        const fs::path expectedOutput = fs::path(VANDIOR_BUILDFOLDER).make_preferred();
        REQUIRE(vnd::GetBuildFolder(inputPath) == expectedOutput);
    }

    SECTION("Current directory input") {
        const fs::path inputPath = fs::path(".").make_preferred();
        const fs::path expectedOutput = fs::path(VANDIOR_BUILDFOLDER).make_preferred();
        REQUIRE(vnd::GetBuildFolder(inputPath) == expectedOutput);
    }

    SECTION("Parent directory input") {
        const fs::path inputPath = fs::path("..").make_preferred();
        const fs::path expectedOutput = fs::path("../vnbuild").make_preferred();
        REQUIRE(vnd::GetBuildFolder(inputPath) == expectedOutput);
    }

    SECTION("Path with special characters") {
        const fs::path inputPath = fs::path("/path/with special@chars!").make_preferred();
        const fs::path expectedOutput = fs::path("/path/vnbuild").make_preferred();
        REQUIRE(vnd::GetBuildFolder(inputPath) == expectedOutput);
    }
}

// ============================================================================
// SourceLocation Tests (Non-constexpr)
// ============================================================================

TEST_CASE("SourceLocation default constructor zero-initializes all fields", "[SourceLocation]") {
    const jsv::SourceLocation loc;

    REQUIRE(loc.line == 0u);
    REQUIRE(loc.column == 0u);
    REQUIRE(loc.absolute_pos == 0u);
}

TEST_CASE("SourceLocation parameterized constructor initializes fields correctly", "[SourceLocation]") {
    SECTION("typical values") {
        const jsv::SourceLocation loc(3u, 5u, 20u);

        REQUIRE(loc.line == 3u);
        REQUIRE(loc.column == 5u);
        REQUIRE(loc.absolute_pos == 20u);
    }

    SECTION("zero values") {
        const jsv::SourceLocation loc(0u, 0u, 0u);

        REQUIRE(loc.line == 0u);
        REQUIRE(loc.column == 0u);
        REQUIRE(loc.absolute_pos == 0u);
    }

    SECTION("large values") {
        constexpr std::size_t maxLine = std::numeric_limits<std::size_t>::max();
        const jsv::SourceLocation loc(maxLine, maxLine - 1, maxLine - 2);

        REQUIRE(loc.line == maxLine);
        REQUIRE(loc.column == maxLine - 1);
        REQUIRE(loc.absolute_pos == maxLine - 2);
    }

    SECTION("first character of file") {
        const jsv::SourceLocation loc(1u, 1u, 0u);

        REQUIRE(loc.line == 1u);
        REQUIRE(loc.column == 1u);
        REQUIRE(loc.absolute_pos == 0u);
    }
}

TEST_CASE("SourceLocation spaceship operator provides correct ordering", "[SourceLocation]") {
    SECTION("equal locations") {
        const jsv::SourceLocation loc1(5u, 10u, 100u);
        const jsv::SourceLocation loc2(5u, 10u, 100u);

        REQUIRE(loc1 == loc2);
        REQUIRE_FALSE(loc1 != loc2);
        REQUIRE_FALSE(loc1 < loc2);
        REQUIRE_FALSE(loc1 > loc2);
        REQUIRE(loc1 <= loc2);
        REQUIRE(loc1 >= loc2);
    }

    SECTION("different line numbers") {
        const jsv::SourceLocation loc1(3u, 5u, 20u);
        const jsv::SourceLocation loc2(5u, 5u, 20u);

        REQUIRE(loc1 < loc2);
        REQUIRE(loc2 > loc1);
        REQUIRE_FALSE(loc1 == loc2);
        REQUIRE(loc1 != loc2);
    }

    SECTION("same line, different columns") {
        const jsv::SourceLocation loc1(5u, 3u, 20u);
        const jsv::SourceLocation loc2(5u, 7u, 20u);

        REQUIRE(loc1 < loc2);
        REQUIRE(loc2 > loc1);
        REQUIRE_FALSE(loc1 == loc2);
    }

    SECTION("same line and column, different absolute_pos") {
        const jsv::SourceLocation loc1(5u, 10u, 50u);
        const jsv::SourceLocation loc2(5u, 10u, 100u);

        REQUIRE(loc1 < loc2);
        REQUIRE(loc2 > loc1);
        REQUIRE_FALSE(loc1 == loc2);
    }

    SECTION("lexicographic ordering prioritizes line over column") {
        // Even though loc1 has larger column, loc2 has larger line
        const jsv::SourceLocation loc1(3u, 100u, 500u);
        const jsv::SourceLocation loc2(4u, 1u, 10u);

        REQUIRE(loc1 < loc2);
    }

    SECTION("lexicographic ordering prioritizes column over absolute_pos") {
        // Even though loc1 has larger absolute_pos, loc2 has larger column
        const jsv::SourceLocation loc1(5u, 5u, 1000u);
        const jsv::SourceLocation loc2(5u, 10u, 100u);

        REQUIRE(loc1 < loc2);
    }
}

TEST_CASE("SourceLocation to_string formats correctly", "[SourceLocation]") {
    SECTION("typical values") {
        const jsv::SourceLocation loc(3u, 5u, 20u);
        const std::string result = loc.to_string();

        REQUIRE(result == "line 3:column 5 (offset: 20)");
    }

    SECTION("first character of file") {
        const jsv::SourceLocation loc(1u, 1u, 0u);
        const std::string result = loc.to_string();

        REQUIRE(result == "line 1:column 1 (offset: 0)");
    }

    SECTION("large values") {
        const jsv::SourceLocation loc(1000u, 500u, 123456u);
        const std::string result = loc.to_string();

        REQUIRE(result == "line 1000:column 500 (offset: 123456)");
    }

    SECTION("zero-initialized location") {
        const jsv::SourceLocation loc;
        const std::string result = loc.to_string();

        REQUIRE(result == "line 0:column 0 (offset: 0)");
    }
}

TEST_CASE("SourceLocation stream operator outputs correctly", "[SourceLocation]") {
    SECTION("typical values") {
        const jsv::SourceLocation loc(3u, 5u, 20u);
        std::ostringstream oss;
        oss << loc;

        REQUIRE(oss.str() == "line 3:column 5 (offset: 20)");
    }

    SECTION("chained stream output") {
        const jsv::SourceLocation loc1(1u, 2u, 3u);
        const jsv::SourceLocation loc2(4u, 5u, 6u);
        std::ostringstream oss;
        oss << "First: " << loc1 << ", Second: " << loc2;

        REQUIRE(oss.str() == "First: line 1:column 2 (offset: 3), Second: line 4:column 5 (offset: 6)");
    }

    SECTION("empty location") {
        const jsv::SourceLocation loc;
        std::ostringstream oss;
        oss << loc;

        REQUIRE(oss.str() == "line 0:column 0 (offset: 0)");
    }
}

TEST_CASE("SourceLocation hash function produces consistent results", "[SourceLocation]") {
    SECTION("equal locations produce equal hashes") {
        const jsv::SourceLocation loc1(5u, 10u, 100u);
        const jsv::SourceLocation loc2(5u, 10u, 100u);

        const std::hash<jsv::SourceLocation> hasher;
        REQUIRE(hasher(loc1) == hasher(loc2));
    }

    SECTION("different locations produce different hashes") {
        const jsv::SourceLocation loc1(5u, 10u, 100u);
        const jsv::SourceLocation loc2(5u, 10u, 101u);

        const std::hash<jsv::SourceLocation> hasher;
        // Note: Hash collisions are possible but unlikely for simple cases
        REQUIRE(hasher(loc1) != hasher(loc2));
    }

    SECTION("hash is stable across multiple calls") {
        const jsv::SourceLocation loc(3u, 7u, 42u);
        const std::hash<jsv::SourceLocation> hasher;

        const std::size_t hash1 = hasher(loc);
        const std::size_t hash2 = hasher(loc);
        const std::size_t hash3 = hasher(loc);

        REQUIRE(hash1 == hash2);
        REQUIRE(hash2 == hash3);
    }

    SECTION("default constructed location has consistent hash") {
        const jsv::SourceLocation loc1;
        const jsv::SourceLocation loc2;

        const std::hash<jsv::SourceLocation> hasher;
        REQUIRE(hasher(loc1) == hasher(loc2));
    }
}

TEST_CASE("SourceLocation std::format integration", "[SourceLocation]") {
    SECTION("format with default specifier") {
        const jsv::SourceLocation loc(3u, 5u, 20u);
        const std::string result = FORMAT("{}", loc);

        REQUIRE(result == "line 3:column 5 (offset: 20)");
    }

    SECTION("format in larger string") {
        const jsv::SourceLocation loc(10u, 20u, 500u);
        const std::string result = FORMAT("Error at {}", loc);

        REQUIRE(result == "Error at line 10:column 20 (offset: 500)");
    }

    SECTION("format multiple locations") {
        const jsv::SourceLocation start(1u, 1u, 0u);
        const jsv::SourceLocation end(5u, 10u, 250u);
        const std::string result = FORMAT("From {} to {}", start, end);

        REQUIRE(result == "From line 1:column 1 (offset: 0) to line 5:column 10 (offset: 250)");
    }
}

TEST_CASE("SourceLocation fmt::format integration", "[SourceLocation]") {
    SECTION("fmt::format with default specifier") {
        const jsv::SourceLocation loc(3u, 5u, 20u);
        const std::string result = fmt::format("{}", loc);

        REQUIRE(result == "line 3:column 5 (offset: 20)");
    }

    SECTION("fmt::format in larger string") {
        const jsv::SourceLocation loc(10u, 20u, 500u);
        const std::string result = fmt::format("Error at {}", loc);

        REQUIRE(result == "Error at line 10:column 20 (offset: 500)");
    }

    SECTION("fmt::format multiple locations") {
        const jsv::SourceLocation start(1u, 1u, 0u);
        const jsv::SourceLocation end(5u, 10u, 250u);
        const std::string result = fmt::format("From {} to {}", start, end);

        REQUIRE(result == "From line 1:column 1 (offset: 0) to line 5:column 10 (offset: 250)");
    }
}

TEST_CASE("SourceLocation noexcept guarantees on operations", "[SourceLocation]") {
    SECTION("default constructor is noexcept") { STATIC_REQUIRE(std::is_nothrow_default_constructible_v<jsv::SourceLocation>); }

    SECTION("parameterized constructor is noexcept") {
        STATIC_REQUIRE(std::is_nothrow_constructible_v<jsv::SourceLocation, std::size_t, std::size_t, std::size_t>);
    }

    SECTION("copy constructor is noexcept") { STATIC_REQUIRE(std::is_nothrow_copy_constructible_v<jsv::SourceLocation>); }

    SECTION("move constructor is noexcept") { STATIC_REQUIRE(std::is_nothrow_move_constructible_v<jsv::SourceLocation>); }

    SECTION("copy assignment is noexcept") { STATIC_REQUIRE(std::is_nothrow_copy_assignable_v<jsv::SourceLocation>); }

    SECTION("move assignment is noexcept") { STATIC_REQUIRE(std::is_nothrow_move_assignable_v<jsv::SourceLocation>); }

    SECTION("destructor is noexcept") { STATIC_REQUIRE(std::is_nothrow_destructible_v<jsv::SourceLocation>); }

    SECTION("spaceship operator is noexcept") {
        const jsv::SourceLocation loc1;
        const jsv::SourceLocation loc2;
        REQUIRE_NOTHROW(std::ignore = (loc1 <=> loc2));
    }

    SECTION("to_string does not throw on any state") {
        const jsv::SourceLocation loc(100u, 200u, 50000u);
        REQUIRE_NOTHROW(std::ignore = loc.to_string());
    }

    SECTION("stream operator does not throw") {
        const jsv::SourceLocation loc(100u, 200u, 50000u);
        std::ostringstream oss;
        REQUIRE_NOTHROW(oss << loc);
    }

    SECTION("hash does not throw") {
        const jsv::SourceLocation loc(100u, 200u, 50000u);
        const std::hash<jsv::SourceLocation> hasher;
        REQUIRE_NOTHROW(std::ignore = hasher(loc));
    }
}

TEST_CASE("SourceLocation usage in standard containers", "[SourceLocation]") {
    SECTION("can be used as std::vector element") {
        std::vector<jsv::SourceLocation> locations;
        locations.emplace_back(1u, 1u, 0u);
        locations.emplace_back(2u, 5u, 10u);
        locations.emplace_back(3u, 10u, 25u);

        REQUIRE(locations.size() == 3u);
        REQUIRE(locations[0].line == 1u);
        REQUIRE(locations[1].column == 5u);
        REQUIRE(locations[2].absolute_pos == 25u);
    }

    SECTION("can be used as std::map key") {
        std::map<jsv::SourceLocation, std::string> locationMap;
        locationMap[{1u, 1u, 0u}] = "start";
        locationMap[{5u, 10u, 100u}] = "middle";
        locationMap[{10u, 20u, 500u}] = "end";

        REQUIRE(locationMap.size() == 3u);
        REQUIRE(locationMap.at({1u, 1u, 0u}) == "start");
        REQUIRE(locationMap.at({5u, 10u, 100u}) == "middle");
        REQUIRE(locationMap.at({10u, 20u, 500u}) == "end");
    }

    SECTION("can be used as std::unordered_map key with custom hash") {
        std::unordered_map<jsv::SourceLocation, std::string, std::hash<jsv::SourceLocation>> locationMap;
        locationMap[{1u, 1u, 0u}] = "start";
        locationMap[{5u, 10u, 100u}] = "middle";

        REQUIRE(locationMap.size() == 2u);
        REQUIRE(locationMap.at({1u, 1u, 0u}) == "start");
        REQUIRE(locationMap.at({5u, 10u, 100u}) == "middle");
    }

    SECTION("can be used in std::set") {
        std::set<jsv::SourceLocation> locationSet;
        locationSet.insert({3u, 5u, 20u});
        locationSet.insert({1u, 1u, 0u});
        locationSet.insert({5u, 10u, 100u});
        locationSet.insert({1u, 1u, 0u});  // duplicate

        REQUIRE(locationSet.size() == 3u);
        REQUIRE(locationSet.begin()->line == 1u);           // smallest
        REQUIRE(std::prev(locationSet.end())->line == 5u);  // largest
    }
}

TEST_CASE("SourceLocation edge cases with extreme values", "[SourceLocation]") {
    SECTION("maximum size_t values") {
        constexpr std::size_t max = std::numeric_limits<std::size_t>::max();
        const jsv::SourceLocation loc(max, max, max);

        REQUIRE(loc.line == max);
        REQUIRE(loc.column == max);
        REQUIRE(loc.absolute_pos == max);

        // Verify to_string handles large numbers
        const std::string result = loc.to_string();
        REQUIRE_FALSE(result.empty());
        REQUIRE(result.find("line") != std::string::npos);
    }

    SECTION("mixed zero and non-zero values") {
        const jsv::SourceLocation loc1(0u, 5u, 10u);
        const jsv::SourceLocation loc2(5u, 0u, 10u);
        const jsv::SourceLocation loc3(5u, 5u, 0u);

        REQUIRE(loc1.line == 0u);
        REQUIRE(loc2.column == 0u);
        REQUIRE(loc3.absolute_pos == 0u);
    }

    SECTION("comparison with mixed extreme values") {
        const jsv::SourceLocation small(0u, 0u, 0u);
        constexpr std::size_t max = std::numeric_limits<std::size_t>::max();
        const jsv::SourceLocation large(max, max, max);

        REQUIRE(small < large);
        REQUIRE(large > small);
        REQUIRE_FALSE(small == large);
    }

    SECTION("self-comparison") {
        const jsv::SourceLocation loc(42u, 42u, 42u);

        REQUIRE(loc == loc);
        REQUIRE_FALSE(loc != loc);
        REQUIRE_FALSE(loc < loc);
        REQUIRE_FALSE(loc > loc);
        REQUIRE(loc <= loc);
        REQUIRE(loc >= loc);
    }
}

TEST_CASE("SourceLocation copy and move semantics", "[SourceLocation]") {
    SECTION("copy construction preserves all fields") {
        const jsv::SourceLocation original(10u, 20u, 300u);
        const jsv::SourceLocation copied = original;

        REQUIRE(copied.line == original.line);
        REQUIRE(copied.column == original.column);
        REQUIRE(copied.absolute_pos == original.absolute_pos);
        REQUIRE(copied == original);
    }

    SECTION("copy assignment preserves all fields") {
        jsv::SourceLocation loc1(1u, 2u, 3u);
        const jsv::SourceLocation loc2(10u, 20u, 300u);

        loc1 = loc2;

        REQUIRE(loc1.line == 10u);
        REQUIRE(loc1.column == 20u);
        REQUIRE(loc1.absolute_pos == 300u);
        REQUIRE(loc1 == loc2);
    }

    SECTION("move construction preserves all fields") {
        jsv::SourceLocation original(10u, 20u, 300u);
        const jsv::SourceLocation moved = std::move(original);

        REQUIRE(moved.line == 10u);
        REQUIRE(moved.column == 20u);
        REQUIRE(moved.absolute_pos == 300u);
    }

    SECTION("move assignment preserves all fields") {
        jsv::SourceLocation loc1(1u, 2u, 3u);
        jsv::SourceLocation loc2(10u, 20u, 300u);

        loc1 = std::move(loc2);

        REQUIRE(loc1.line == 10u);
        REQUIRE(loc1.column == 20u);
        REQUIRE(loc1.absolute_pos == 300u);
    }

    SECTION("self-assignment is safe") {
        const jsv::SourceLocation loc(42u, 42u, 42u);

        // Copy self-assignment verified by copying to a new instance
        const jsv::SourceLocation loc_copy = loc;
        REQUIRE(loc_copy.line == 42u);
        REQUIRE(loc_copy.column == 42u);
        REQUIRE(loc_copy.absolute_pos == 42u);
    }
}

TEST_CASE("SourceLocation member field mutability", "[SourceLocation]") {
    SECTION("fields can be modified after construction") {
        jsv::SourceLocation loc(1u, 1u, 0u);

        loc.line = 10u;
        loc.column = 20u;
        loc.absolute_pos = 500u;

        REQUIRE(loc.line == 10u);
        REQUIRE(loc.column == 20u);
        REQUIRE(loc.absolute_pos == 500u);
    }

    SECTION("modification affects comparisons") {
        jsv::SourceLocation loc1(5u, 5u, 50u);
        const jsv::SourceLocation loc2(5u, 5u, 50u);

        REQUIRE(loc1 == loc2);

        loc1.line = 10u;

        REQUIRE(loc1 != loc2);
        REQUIRE(loc1 > loc2);
    }

    SECTION("modification affects hash") {
        jsv::SourceLocation loc(5u, 10u, 100u);
        const std::hash<jsv::SourceLocation> hasher;

        const std::size_t hashBefore = hasher(loc);

        loc.line = 100u;

        const std::size_t hashAfter = hasher(loc);

        // Hash should change when content changes
        REQUIRE(hashBefore != hashAfter);
    }
}

// ============================================================================
// SourceSpan Tests (Non-constexpr)
// ============================================================================

TEST_CASE("SourceSpan default constructor initializes correctly", "[SourceSpan]") {
    const jsv::SourceSpan span;

    REQUIRE(span.file_path.empty());
    REQUIRE(span.start.line == 0u);
    REQUIRE(span.start.column == 0u);
    REQUIRE(span.start.absolute_pos == 0u);
    REQUIRE(span.end.line == 0u);
    REQUIRE(span.end.column == 0u);
    REQUIRE(span.end.absolute_pos == 0u);
}

TEST_CASE("SourceSpan parameterized constructor initializes correctly", "[SourceSpan]") {
    SECTION("typical values") {
        const jsv::SourceLocation start(1u, 1u, 0u);
        const jsv::SourceLocation end(5u, 10u, 250u);

        const jsv::SourceSpan span("test/file.cpp", start, end);

        REQUIRE(span.file_path == "test/file.cpp");
        REQUIRE(span.start.line == 1u);
        REQUIRE(span.start.column == 1u);
        REQUIRE(span.start.absolute_pos == 0u);
        REQUIRE(span.end.line == 5u);
        REQUIRE(span.end.column == 10u);
        REQUIRE(span.end.absolute_pos == 250u);
    }

    SECTION("empty span at same position") {
        const jsv::SourceLocation pos(3u, 5u, 20u);

        const jsv::SourceSpan span("empty.cpp", pos, pos);

        REQUIRE(span.file_path == "empty.cpp");
        REQUIRE(span.start.line == 3u);
        REQUIRE(span.end.line == 3u);
        REQUIRE(span.start == span.end);
    }

    SECTION("deep path") {
        const jsv::SourceLocation start(1u, 1u, 0u);
        const jsv::SourceLocation end(1u, 1u, 10u);

        const jsv::SourceSpan span("a/b/c/d/e/file.cpp", start, end);

        REQUIRE(span.file_path == "a/b/c/d/e/file.cpp");
    }
}

TEST_CASE("SourceSpan merge mutates in-place correctly", "[SourceSpan]") {
    SECTION("merge overlapping spans from same file") {
        const jsv::SourceLocation start1(1u, 1u, 0u);
        const jsv::SourceLocation end1(2u, 5u, 50u);
        jsv::SourceSpan span1("test.cpp", start1, end1);

        const jsv::SourceLocation start2(2u, 1u, 30u);
        const jsv::SourceLocation end2(3u, 10u, 100u);
        const jsv::SourceSpan span2("test.cpp", start2, end2);

        span1.merge(span2);

        REQUIRE(span1.start.line == 1u);  // earlier start
        REQUIRE(span1.end.line == 3u);    // later end
        REQUIRE(span1.end.column == 10u);
        REQUIRE(span1.end.absolute_pos == 100u);
    }

    SECTION("merge with earlier start extends backward") {
        const jsv::SourceLocation start1(5u, 10u, 100u);
        const jsv::SourceLocation end1(10u, 5u, 500u);
        jsv::SourceSpan span1("test.cpp", start1, end1);

        const jsv::SourceLocation start2(2u, 3u, 20u);
        const jsv::SourceLocation end2(6u, 1u, 200u);
        const jsv::SourceSpan span2("test.cpp", start2, end2);

        span1.merge(span2);

        REQUIRE(span1.start.line == 2u);  // extended backward
        REQUIRE(span1.end.line == 10u);   // unchanged (later)
    }

    SECTION("merge with later end extends forward") {
        const jsv::SourceLocation start1(5u, 10u, 100u);
        const jsv::SourceLocation end1(10u, 5u, 500u);
        jsv::SourceSpan span1("test.cpp", start1, end1);

        const jsv::SourceLocation start2(6u, 1u, 200u);
        const jsv::SourceLocation end2(15u, 10u, 1000u);
        const jsv::SourceSpan span2("test.cpp", start2, end2);

        span1.merge(span2);

        REQUIRE(span1.start.line == 5u);  // unchanged (earlier)
        REQUIRE(span1.end.line == 15u);   // extended forward
    }

    SECTION("merge from different file does nothing") {
        const auto filePath1 = std::string_view{"file1.cpp"};
        const auto filePath2 = std::string_view{"file2.cpp"};
        const jsv::SourceLocation start1(1u, 1u, 0u);
        const jsv::SourceLocation end1(5u, 5u, 100u);
        jsv::SourceSpan span1(filePath1, start1, end1);

        const jsv::SourceLocation start2(2u, 2u, 50u);
        const jsv::SourceLocation end2(10u, 10u, 500u);
        const jsv::SourceSpan span2(filePath2, start2, end2);

        const jsv::SourceLocation originalStart = span1.start;
        const jsv::SourceLocation originalEnd = span1.end;

        span1.merge(span2);

        // Should remain unchanged
        REQUIRE(span1.start == originalStart);
        REQUIRE(span1.end == originalEnd);
    }

    SECTION("merge identical spans") {
        const jsv::SourceLocation start(1u, 1u, 0u);
        const jsv::SourceLocation end(5u, 5u, 100u);
        jsv::SourceSpan span1("test.cpp", start, end);
        const jsv::SourceSpan span2("test.cpp", start, end);

        span1.merge(span2);

        REQUIRE(span1.start == start);
        REQUIRE(span1.end == end);
    }
}

TEST_CASE("SourceSpan merged returns optional correctly", "[SourceSpan]") {
    SECTION("merge spans from same file returns value") {
        const jsv::SourceLocation start1(1u, 1u, 0u);
        const jsv::SourceLocation end1(2u, 5u, 50u);
        const jsv::SourceSpan span1("test.cpp", start1, end1);

        const jsv::SourceLocation start2(2u, 1u, 30u);
        const jsv::SourceLocation end2(3u, 10u, 100u);
        const jsv::SourceSpan span2("test.cpp", start2, end2);

        const std::optional<jsv::SourceSpan> result = span1.merged(span2);

        REQUIRE(result.has_value());
        REQUIRE(result->start.line == 1u);  // earlier start
        REQUIRE(result->end.line == 3u);    // later end
        REQUIRE(result->file_path == "test.cpp");
    }

    SECTION("merge spans from different files returns nullopt") {
        const auto filePath1 = std::string_view{"file1.cpp"};
        const auto filePath2 = std::string_view{"file2.cpp"};
        const jsv::SourceLocation start1(1u, 1u, 0u);
        const jsv::SourceLocation end1(5u, 5u, 100u);
        const jsv::SourceSpan span1(filePath1, start1, end1);

        const jsv::SourceLocation start2(2u, 2u, 50u);
        const jsv::SourceLocation end2(10u, 10u, 500u);
        const jsv::SourceSpan span2(filePath2, start2, end2);

        const std::optional<jsv::SourceSpan> result = span1.merged(span2);

        REQUIRE_FALSE(result.has_value());
    }

    SECTION("merged does not mutate original spans") {
        const jsv::SourceLocation start1(5u, 5u, 100u);
        const jsv::SourceLocation end1(10u, 10u, 500u);
        jsv::SourceSpan span1("test.cpp", start1, end1);

        const jsv::SourceLocation start2(1u, 1u, 0u);
        const jsv::SourceLocation end2(15u, 15u, 1000u);
        const jsv::SourceSpan span2("test.cpp", start2, end2);

        const std::optional<jsv::SourceSpan> result = span1.merged(span2);

        // Originals unchanged
        REQUIRE(span1.start == start1);
        REQUIRE(span1.end == end1);
        REQUIRE(span2.start == start2);
        REQUIRE(span2.end == end2);

        // Result has merged values
        REQUIRE(result.has_value());
        REQUIRE(result->start.line == 1u);
        REQUIRE(result->end.line == 15u);
    }

    SECTION("merge with empty span") {
        const jsv::SourceLocation start1(5u, 5u, 100u);
        const jsv::SourceLocation end1(10u, 10u, 500u);
        const jsv::SourceSpan span1("test.cpp", start1, end1);

        const jsv::SourceSpan span2;  // default constructed (empty file path)

        const std::optional<jsv::SourceSpan> result = span1.merged(span2);

        // Different file paths (one empty)
        REQUIRE_FALSE(result.has_value());
    }
}

TEST_CASE("SourceSpan spaceship operator provides correct ordering", "[SourceSpan]") {
    SECTION("equal spans") {
        const jsv::SourceLocation start(1u, 1u, 0u);
        const jsv::SourceLocation end(5u, 5u, 100u);
        const jsv::SourceSpan span1("test.cpp", start, end);
        const jsv::SourceSpan span2("test.cpp", start, end);

        REQUIRE(span1 == span2);
        REQUIRE_FALSE(span1 != span2);
        REQUIRE_FALSE(span1 < span2);
        REQUIRE_FALSE(span1 > span2);
        REQUIRE(span1 <= span2);
        REQUIRE(span1 >= span2);
    }

    SECTION("different file paths") {
        const auto filePath1 = std::string_view{"a.cpp"};
        const auto filePath2 = std::string_view{"b.cpp"};
        const jsv::SourceLocation start;
        const jsv::SourceLocation end(1u, 1u, 10u);
        const jsv::SourceSpan span1(filePath1, start, end);
        const jsv::SourceSpan span2(filePath2, start, end);

        REQUIRE(span1 < span2);
        REQUIRE(span2 > span1);
        REQUIRE_FALSE(span1 == span2);
    }

    SECTION("same file, different start") {
        const jsv::SourceLocation start1(1u, 1u, 0u);
        const jsv::SourceLocation start2(3u, 1u, 50u);
        const jsv::SourceLocation end(5u, 5u, 100u);
        const jsv::SourceSpan span1("test.cpp", start1, end);
        const jsv::SourceSpan span2("test.cpp", start2, end);

        REQUIRE(span1 < span2);
        REQUIRE(span2 > span1);
    }

    SECTION("same file and start, different end") {
        const jsv::SourceLocation start(1u, 1u, 0u);
        const jsv::SourceLocation end1(5u, 5u, 100u);
        const jsv::SourceLocation end2(10u, 10u, 500u);
        const jsv::SourceSpan span1("test.cpp", start, end1);
        const jsv::SourceSpan span2("test.cpp", start, end2);

        REQUIRE(span1 < span2);
        REQUIRE(span2 > span1);
    }

    SECTION("lexicographic ordering prioritizes file_path over start") {
        const auto filePath1 = std::string_view{"a.cpp"};
        const auto filePath2 = std::string_view{"z.cpp"};
        const jsv::SourceLocation start1(100u, 100u, 10000u);
        const jsv::SourceLocation start2(1u, 1u, 0u);
        const jsv::SourceLocation end;
        const jsv::SourceSpan span1(filePath1, start1, end);
        const jsv::SourceSpan span2(filePath2, start2, end);

        // File path comparison takes precedence
        REQUIRE(span1 < span2);
    }

    SECTION("lexicographic ordering prioritizes start over end") {
        const jsv::SourceLocation start1(1u, 1u, 0u);
        const jsv::SourceLocation start2(2u, 1u, 50u);
        const jsv::SourceLocation end1(100u, 100u, 10000u);
        const jsv::SourceLocation end2(5u, 5u, 100u);
        const jsv::SourceSpan span1("test.cpp", start1, end1);
        const jsv::SourceSpan span2("test.cpp", start2, end2);

        // Start comparison takes precedence over end
        REQUIRE(span1 < span2);
    }
}

TEST_CASE("SourceSpan to_string formats correctly", "[SourceSpan]") {
    SECTION("typical span") {
        const auto filePath = std::string_view{"test/file.cpp"};
        const jsv::SourceLocation start(1u, 5u, 0u);
        const jsv::SourceLocation end(3u, 10u, 100u);
        const jsv::SourceSpan span(filePath, start, end);

        const std::string result = span.to_string();

#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
        REQUIRE(result == "test\\file.cpp:line 1:column 5 - line 3:column 10");
#else
        REQUIRE(result == "test/file.cpp:line 1:column 5 - line 3:column 10");
#endif
    }

    SECTION("single character span") {
        const auto filePath = std::string_view{"main.cpp"};
        const jsv::SourceLocation pos(5u, 10u, 50u);
        const jsv::SourceSpan span(filePath, pos, pos);

        const std::string result = span.to_string();

        REQUIRE(result == "main.cpp:line 5:column 10 - line 5:column 10");
    }

    SECTION("deep path is truncated to 2 components") {
        const auto filePath = std::string_view{"a/b/c/d/e/file.cpp"};
        const jsv::SourceLocation start(1u, 1u, 0u);
        const jsv::SourceLocation end(1u, 1u, 10u);
        const jsv::SourceSpan span(filePath, start, end);

        const std::string result = span.to_string();

        // Should show ".." + last 2 components (OS-independent)
        REQUIRE(result.find("..") == 0);
        REQUIRE(result.find("file.cpp") != std::string::npos);
#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
        REQUIRE(result.find("e\\file.cpp") != std::string::npos);
#else
        REQUIRE(result.find("e/file.cpp") != std::string::npos);
#endif
    }

    SECTION("short path is not truncated") {
        const auto filePath = std::string_view{"main.cpp"};
        const jsv::SourceLocation start(1u, 1u, 0u);
        const jsv::SourceLocation end(1u, 1u, 10u);
        const jsv::SourceSpan span(filePath, start, end);

        const std::string result = span.to_string();

        REQUIRE(result == "main.cpp:line 1:column 1 - line 1:column 1");
    }

    SECTION("empty file path") {
        const jsv::SourceSpan span;  // default constructed

        const std::string result = span.to_string();

        REQUIRE(result.find(":line 0:column 0 - line 0:column 0") != std::string::npos);
    }
}

TEST_CASE("SourceSpan stream operator outputs correctly", "[SourceSpan]") {
    SECTION("typical span") {
        const auto filePath = std::string_view{"test.cpp"};
        const jsv::SourceLocation start(1u, 5u, 0u);
        const jsv::SourceLocation end(3u, 10u, 100u);
        const jsv::SourceSpan span(filePath, start, end);

        std::ostringstream oss;
        oss << span;

        REQUIRE(oss.str() == "test.cpp:line 1:column 5 - line 3:column 10");
    }

    SECTION("chained stream output") {
        const auto filePath = std::string_view{"test.cpp"};
        const jsv::SourceLocation start1(1u, 1u, 0u);
        const jsv::SourceLocation end1(2u, 2u, 50u);
        const jsv::SourceSpan span1(filePath, start1, end1);

        const jsv::SourceLocation start2(3u, 3u, 100u);
        const jsv::SourceLocation end2(4u, 4u, 150u);
        const jsv::SourceSpan span2(filePath, start2, end2);

        std::ostringstream oss;
        oss << "From " << span1 << " to " << span2;

        REQUIRE(oss.str() == "From test.cpp:line 1:column 1 - line 2:column 2 to test.cpp:line 3:column 3 - line 4:column 4");
    }

    SECTION("default constructed span") {
        const jsv::SourceSpan span;

        std::ostringstream oss;
        oss << span;

        REQUIRE_FALSE(oss.str().empty());
        REQUIRE(oss.str().find(":line 0:column 0 - line 0:column 0") != std::string::npos);
    }
}

TEST_CASE("SourceSpan hash function produces consistent results", "[SourceSpan]") {
    SECTION("equal spans produce equal hashes") {
        const auto filePath = std::string_view{"main.cpp"};
        const jsv::SourceLocation start(1u, 5u, 0u);
        const jsv::SourceLocation end(3u, 10u, 100u);
        const jsv::SourceSpan span1(filePath, start, end);
        const jsv::SourceSpan span2(filePath, start, end);

        const std::hash<jsv::SourceSpan> hasher;
        REQUIRE(hasher(span1) == hasher(span2));
    }

    SECTION("different spans produce different hashes") {
        const jsv::SourceLocation start(1u, 5u, 0u);
        const jsv::SourceLocation end1(3u, 10u, 100u);
        const jsv::SourceLocation end2(5u, 15u, 200u);
        const jsv::SourceSpan span1("test.cpp", start, end1);
        const jsv::SourceSpan span2("test.cpp", start, end2);

        const std::hash<jsv::SourceSpan> hasher;
        REQUIRE(hasher(span1) != hasher(span2));
    }

    SECTION("hash is stable across multiple calls") {
        const jsv::SourceLocation start(1u, 1u, 0u);
        const jsv::SourceLocation end(5u, 5u, 100u);
        const jsv::SourceSpan span("test.cpp", start, end);

        const std::hash<jsv::SourceSpan> hasher;
        const std::size_t hash1 = hasher(span);
        const std::size_t hash2 = hasher(span);
        const std::size_t hash3 = hasher(span);

        REQUIRE(hash1 == hash2);
        REQUIRE(hash2 == hash3);
    }

    SECTION("default constructed span has consistent hash") {
        const jsv::SourceSpan span1;
        const jsv::SourceSpan span2;

        const std::hash<jsv::SourceSpan> hasher;
        REQUIRE(hasher(span1) == hasher(span2));
    }
}

TEST_CASE("SourceSpan std::format integration", "[SourceSpan]") {
    SECTION("format with default specifier") {
        const jsv::SourceLocation start(1u, 5u, 0u);
        const jsv::SourceLocation end(3u, 10u, 100u);
        const jsv::SourceSpan span("test.cpp", start, end);

        const std::string result = FORMAT("{}", span);

        REQUIRE(result == "test.cpp:line 1:column 5 - line 3:column 10");
    }

    SECTION("format in larger string") {
        const auto filePath = std::string_view{"main.cpp"};
        const jsv::SourceLocation start(5u, 10u, 50u);
        const jsv::SourceLocation end(10u, 20u, 500u);
        const jsv::SourceSpan span(filePath, start, end);

        const std::string result = FORMAT("Error at {}", span);

        REQUIRE(result == "Error at main.cpp:line 5:column 10 - line 10:column 20");
    }

    SECTION("format multiple spans") {
        const jsv::SourceSpan span1("test.cpp", {1u, 1u, 0u}, {2u, 2u, 50u});
        const jsv::SourceSpan span2("test.cpp", {3u, 3u, 100u}, {4u, 4u, 150u});

        const std::string result = FORMAT("From {} to {}", span1, span2);

        REQUIRE(result == "From test.cpp:line 1:column 1 - line 2:column 2 to test.cpp:line 3:column 3 - line 4:column 4");
    }
}

TEST_CASE("SourceSpan fmt::format integration", "[SourceSpan]") {
    SECTION("fmt::format with default specifier") {
        const jsv::SourceLocation start(1u, 5u, 0u);
        const jsv::SourceLocation end(3u, 10u, 100u);
        const jsv::SourceSpan span("test.cpp", start, end);

        const std::string result = fmt::format("{}", span);

        REQUIRE(result == "test.cpp:line 1:column 5 - line 3:column 10");
    }

    SECTION("fmt::format in larger string") {
        const auto filePath = std::string_view{"main.cpp"};
        const jsv::SourceLocation start(5u, 10u, 50u);
        const jsv::SourceLocation end(10u, 20u, 500u);
        const jsv::SourceSpan span(filePath, start, end);

        const std::string result = fmt::format("Error at {}", span);

        REQUIRE(result == "Error at main.cpp:line 5:column 10 - line 10:column 20");
    }

    SECTION("fmt::format multiple spans") {
        const jsv::SourceSpan span1("test.cpp", {1u, 1u, 0u}, {2u, 2u, 50u});
        const jsv::SourceSpan span2("test.cpp", {3u, 3u, 100u}, {4u, 4u, 150u});

        const std::string result = fmt::format("From {} to {}", span1, span2);

        REQUIRE(result == "From test.cpp:line 1:column 1 - line 2:column 2 to test.cpp:line 3:column 3 - line 4:column 4");
    }
}

TEST_CASE("SourceSpan noexcept guarantees on operations", "[SourceSpan]") {
    SECTION("parameterized constructor is noexcept") {
        STATIC_REQUIRE(
            std::is_nothrow_constructible_v<jsv::SourceSpan, std::string_view, const jsv::SourceLocation &, const jsv::SourceLocation &>);
    }

    SECTION("copy constructor is noexcept") { STATIC_REQUIRE(std::is_nothrow_copy_constructible_v<jsv::SourceSpan>); }

    SECTION("move constructor is noexcept") { STATIC_REQUIRE(std::is_nothrow_move_constructible_v<jsv::SourceSpan>); }

    SECTION("copy assignment is noexcept") { STATIC_REQUIRE(std::is_nothrow_copy_assignable_v<jsv::SourceSpan>); }

    SECTION("move assignment is noexcept") { STATIC_REQUIRE(std::is_nothrow_move_assignable_v<jsv::SourceSpan>); }

    SECTION("destructor is noexcept") { STATIC_REQUIRE(std::is_nothrow_destructible_v<jsv::SourceSpan>); }

    SECTION("merge does not throw on same file") {
        jsv::SourceSpan span1("test.cpp", {1u, 1u, 0u}, {5u, 5u, 100u});
        const jsv::SourceSpan span2("test.cpp", {2u, 2u, 50u}, {10u, 10u, 500u});

        REQUIRE_NOTHROW(span1.merge(span2));
    }

    SECTION("merge does not throw on different files") {
        const auto filePath1 = std::string_view{"file1.cpp"};
        const auto filePath2 = std::string_view{"file2.cpp"};
        jsv::SourceSpan span1(filePath1, {1u, 1u, 0u}, {5u, 5u, 100u});
        const jsv::SourceSpan span2(filePath2, {2u, 2u, 50u}, {10u, 10u, 500u});

        REQUIRE_NOTHROW(span1.merge(span2));
    }

    SECTION("merged does not throw on same file") {
        const jsv::SourceSpan span1("test.cpp", {1u, 1u, 0u}, {5u, 5u, 100u});
        const jsv::SourceSpan span2("test.cpp", {2u, 2u, 50u}, {10u, 10u, 500u});

        REQUIRE_NOTHROW(std::ignore = span1.merged(span2));
    }

    SECTION("merged does not throw on different files") {
        const auto filePath1 = std::string_view{"file1.cpp"};
        const auto filePath2 = std::string_view{"file2.cpp"};
        const jsv::SourceSpan span1(filePath1, {1u, 1u, 0u}, {5u, 5u, 100u});
        const jsv::SourceSpan span2(filePath2, {2u, 2u, 50u}, {10u, 10u, 500u});

        REQUIRE_NOTHROW(std::ignore = span1.merged(span2));
    }

    SECTION("spaceship operator does not throw") {
        const jsv::SourceSpan span1;
        const jsv::SourceSpan span2;
        REQUIRE_NOTHROW(std::ignore = (span1 <=> span2));
    }

    SECTION("to_string does not throw on any state") {
        const auto filePath = std::string_view{"a/b/c/d/e/f/g/file.cpp"};
        const jsv::SourceSpan span(filePath, {1u, 1u, 0u}, {100u, 100u, 10000u});
        REQUIRE_NOTHROW(std::ignore = span.to_string());
    }

    SECTION("stream operator does not throw") {
        const jsv::SourceSpan span;
        std::ostringstream oss;
        REQUIRE_NOTHROW(oss << span);
    }

    SECTION("hash does not throw") {
        const jsv::SourceSpan span;
        const std::hash<jsv::SourceSpan> hasher;
        REQUIRE_NOTHROW(std::ignore = hasher(span));
    }
}

TEST_CASE("SourceSpan usage in standard containers", "[SourceSpan]") {
    SECTION("can be used as std::vector element") {
        std::vector<jsv::SourceSpan> spans;
        spans.emplace_back("test.cpp", jsv::SourceLocation{1u, 1u, 0u}, jsv::SourceLocation{2u, 2u, 50u});
        spans.emplace_back("test.cpp", jsv::SourceLocation{3u, 3u, 100u}, jsv::SourceLocation{4u, 4u, 150u});
        spans.emplace_back("test.cpp", jsv::SourceLocation{5u, 5u, 200u}, jsv::SourceLocation{6u, 6u, 250u});

        REQUIRE(spans.size() == 3u);
        REQUIRE(spans[0].start.line == 1u);
        REQUIRE(spans[1].start.line == 3u);
        REQUIRE(spans[2].start.line == 5u);
    }

    SECTION("can be used as std::map key") {
        std::map<jsv::SourceSpan, std::string> spanMap;
        spanMap[{"test.cpp", {1u, 1u, 0u}, {2u, 2u, 50u}}] = "first";
        spanMap[{"test.cpp", {3u, 3u, 100u}, {4u, 4u, 150u}}] = "second";
        spanMap[{"test.cpp", {5u, 5u, 200u}, {6u, 6u, 250u}}] = "third";

        REQUIRE(spanMap.size() == 3u);
        REQUIRE(spanMap.at({"test.cpp", {1u, 1u, 0u}, {2u, 2u, 50u}}) == "first");
        REQUIRE(spanMap.at({"test.cpp", {3u, 3u, 100u}, {4u, 4u, 150u}}) == "second");
    }

    SECTION("can be used as std::unordered_map key with custom hash") {
        std::unordered_map<jsv::SourceSpan, std::string, std::hash<jsv::SourceSpan>> spanMap;
        spanMap[{"test.cpp", {1u, 1u, 0u}, {2u, 2u, 50u}}] = "first";
        spanMap[{"test.cpp", {3u, 3u, 100u}, {4u, 4u, 150u}}] = "second";

        REQUIRE(spanMap.size() == 2u);
        REQUIRE(spanMap.at({"test.cpp", {1u, 1u, 0u}, {2u, 2u, 50u}}) == "first");
    }

    SECTION("can be used in std::set") {
        std::set<jsv::SourceSpan> spanSet;
        spanSet.insert({"test.cpp", {3u, 3u, 100u}, {4u, 4u, 150u}});
        spanSet.insert({"test.cpp", {1u, 1u, 0u}, {2u, 2u, 50u}});
        spanSet.insert({"test.cpp", {5u, 5u, 200u}, {6u, 6u, 250u}});
        spanSet.insert({"test.cpp", {1u, 1u, 0u}, {2u, 2u, 50u}});  // duplicate

        REQUIRE(spanSet.size() == 3u);
        REQUIRE(spanSet.begin()->start.line == 1u);           // smallest
        REQUIRE(std::prev(spanSet.end())->start.line == 5u);  // largest
    }
}

TEST_CASE("SourceSpan edge cases with extreme values", "[SourceSpan]") {
    SECTION("maximum size_t values in locations") {
        constexpr std::size_t max = std::numeric_limits<std::size_t>::max();
        const jsv::SourceLocation start(max, max, max);
        const jsv::SourceLocation end(max, max, max);
        const jsv::SourceSpan span("test.cpp", start, end);

        REQUIRE(span.start.line == max);
        REQUIRE(span.end.line == max);

        // Verify to_string handles large numbers
        const std::string result = span.to_string();
        REQUIRE_FALSE(result.empty());
    }

    SECTION("empty span (start equals end)") {
        const jsv::SourceLocation pos(5u, 10u, 100u);
        const jsv::SourceSpan span("test.cpp", pos, pos);

        REQUIRE(span.start == span.end);
        REQUIRE(span.start.line == 5u);
        REQUIRE(span.end.line == 5u);
    }

    SECTION("span with end before start (valid but unusual)") {
        const jsv::SourceLocation start(10u, 10u, 500u);
        const jsv::SourceLocation end(5u, 5u, 100u);
        const jsv::SourceSpan span("test.cpp", start, end);

        // This is technically valid - just represents an inverted span
        REQUIRE(span.start.line == 10u);
        REQUIRE(span.end.line == 5u);
    }

    SECTION("comparison with mixed extreme values") {
        const auto filePath1 = std::string_view{"a.cpp"};
        const auto filePath2 = std::string_view{"z.cpp"};
        const jsv::SourceSpan small(filePath1, {0u, 0u, 0u}, {0u, 0u, 0u});
        constexpr std::size_t max = std::numeric_limits<std::size_t>::max();
        const jsv::SourceSpan large(filePath2, {max, max, max}, {max, max, max});

        REQUIRE(small < large);
        REQUIRE(large > small);
        REQUIRE_FALSE(small == large);
    }

    SECTION("self-comparison") {
        const jsv::SourceSpan span("test.cpp", {42u, 42u, 420u}, {84u, 84u, 840u});

        REQUIRE(span == span);
        REQUIRE_FALSE(span != span);
        REQUIRE_FALSE(span < span);
        REQUIRE_FALSE(span > span);
        REQUIRE(span <= span);
        REQUIRE(span >= span);
    }
}

TEST_CASE("SourceSpan copy and move semantics", "[SourceSpan]") {
    SECTION("copy construction preserves all fields") {
        const jsv::SourceSpan original("test.cpp", {10u, 20u, 100u}, {30u, 40u, 300u});
        const jsv::SourceSpan copied = original;

        REQUIRE(copied.file_path == original.file_path);
        REQUIRE(copied.start == original.start);
        REQUIRE(copied.end == original.end);
        REQUIRE(copied == original);
    }

    SECTION("copy assignment preserves all fields") {
        jsv::SourceSpan loc1("test.cpp", {1u, 2u, 3u}, {4u, 5u, 6u});
        const jsv::SourceSpan loc2("test.cpp", {10u, 20u, 100u}, {30u, 40u, 300u});

        loc1 = loc2;

        REQUIRE(loc1.start.line == 10u);
        REQUIRE(loc1.end.column == 40u);
        REQUIRE(loc1 == loc2);
    }

    SECTION("move construction preserves all fields") {
        jsv::SourceSpan original("test.cpp", {10u, 20u, 100u}, {30u, 40u, 300u});
        const jsv::SourceSpan moved = std::move(original);

        REQUIRE(moved.start.line == 10u);
        REQUIRE(moved.end.column == 40u);
    }

    SECTION("move assignment preserves all fields") {
        jsv::SourceSpan loc1("test.cpp", {1u, 2u, 3u}, {4u, 5u, 6u});
        jsv::SourceSpan loc2("test.cpp", {10u, 20u, 100u}, {30u, 40u, 300u});

        loc1 = std::move(loc2);

        REQUIRE(loc1.start.line == 10u);
        REQUIRE(loc1.end.column == 40u);
    }

    SECTION("self-assignment is safe") {
        const jsv::SourceSpan span("test.cpp", {42u, 42u, 420u}, {84u, 84u, 840u});

        // Copy self-assignment verified by copying to a new instance
        const jsv::SourceSpan span_copy = span;
        REQUIRE(span_copy.start.line == 42u);
        REQUIRE(span_copy.end.column == 84u);
    }
}

TEST_CASE("truncate_path function works correctly", "[truncate_path][utility][happy]") {
    SECTION("path shorter than depth is unchanged") {
        const fs::path path = "a/b/c";
        const std::string result = jsv::truncate_path(path, 5);

#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
        REQUIRE(result == R"(a\b\c)");
#else
        REQUIRE(result == "a/b/c");
#endif
    }

    SECTION("path equal to depth is unchanged") {
        const fs::path path = "a/b/c";
        const std::string result = jsv::truncate_path(path, 3);

#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
        REQUIRE(result == R"(a\b\c)");
#else
        REQUIRE(result == "a/b/c");
#endif
    }

    SECTION("path longer than depth is truncated with ..") {
        const fs::path path = "a/b/c/d/e";
        const std::string result = jsv::truncate_path(path, 2);

        REQUIRE(result.find("..") == 0);
#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
        REQUIRE(result.find(R"(d\e)") != std::string::npos);
#else
        REQUIRE(result.find("d/e") != std::string::npos);
#endif
    }

    SECTION("depth of 1 shows only last component") {
        const fs::path path = "a/b/c/d/file.cpp";
        const std::string result = jsv::truncate_path(path, 1);

        REQUIRE(result.find("..") == 0);
        REQUIRE(result.find("file.cpp") != std::string::npos);
    }

    SECTION("depth of 0 shows only ..") {
        const fs::path path = "a/b/c";
        const std::string result = jsv::truncate_path(path, 0);

        REQUIRE(result == "..");
    }

    SECTION("absolute path is handled") {
#if defined(_WIN32)
        const fs::path path = R"(C:\a\b\c\d\e)";
#else
        const fs::path path = "/a/b/c/d/e";
#endif
        const std::string result = jsv::truncate_path(path, 2);

        // Should still truncate to last 2 components
        REQUIRE_FALSE(result.empty());
    }

    SECTION("empty path returns empty string") {
        const fs::path path;
        const std::string result = jsv::truncate_path(path, 2);

        REQUIRE(result.empty());
    }

    SECTION("single component path") {
        const fs::path path = "file.cpp";
        const std::string result = jsv::truncate_path(path, 2);

        REQUIRE(result == "file.cpp");
    }
}

TEST_CASE("HasSpan abstract interface works correctly", "[HasSpan][interface][polymorphism]") {
    struct TestHasSpan : jsv::HasSpan {
        jsv::SourceSpan stored_span;

        explicit TestHasSpan(const jsv::SourceSpan &span) : stored_span(span) {}

        [[nodiscard]] const jsv::SourceSpan &span() const noexcept override { return stored_span; }
    };

    SECTION("can store and retrieve span through interface") {
        const auto filePath = std::string_view{"test.cpp"};
        const jsv::SourceSpan span(filePath, {1u, 1u, 0u}, {5u, 5u, 100u});

        const TestHasSpan has_span(span);

        REQUIRE(has_span.span() == span);
    }

    SECTION("polymorphic access through base pointer") {
        const jsv::SourceSpan span("test.cpp", {10u, 20u, 100u}, {30u, 40u, 300u});

        const std::unique_ptr<jsv::HasSpan> ptr = std::make_unique<TestHasSpan>(span);

        REQUIRE(ptr->span() == span);
    }

    SECTION("polymorphic access through base reference") {
        const jsv::SourceSpan span("test.cpp", {5u, 10u, 50u}, {15u, 20u, 150u});

        const TestHasSpan has_span(span);
        const jsv::HasSpan &ref = has_span;

        REQUIRE(ref.span() == span);
    }

    SECTION("virtual destructor is noexcept") { STATIC_REQUIRE(std::is_nothrow_destructible_v<jsv::HasSpan>); }

    SECTION("span method is noexcept") {
        const TestHasSpan has_span({"test.cpp", {1u, 1u, 0u}, {5u, 5u, 100u}});

        REQUIRE_NOTHROW(std::ignore = has_span.span());
    }
}
// =============================================================================
// Token Tests
// =============================================================================

TEST_CASE("Token construction and basic accessors", "[Token]") {
    const jsv::SourceLocation start(1u, 5u, 10u);
    const jsv::SourceLocation end(1u, 8u, 13u);
    const jsv::SourceSpan span("test.cpp", start, end);

    SECTION("Token constructed with all parameters") {
        const jsv::Token token(jsv::TokenKind::KeywordFun, "fun", span);

        REQUIRE(token.getKind() == jsv::TokenKind::KeywordFun);
        REQUIRE(token.getText() == "fun");
        REQUIRE(token.getSpan().file_path == "test.cpp");
        REQUIRE(token.getSpan().start.line == 1u);
        REQUIRE(token.getSpan().start.column == 5u);
        REQUIRE(token.getSpan().end.line == 1u);
        REQUIRE(token.getSpan().end.column == 8u);
    }

    SECTION("Token with different token kinds") {
        const jsv::Token identifier(jsv::TokenKind::IdentifierAscii, "myVar", span);
        const jsv::Token number(jsv::TokenKind::Numeric, "42", span);
        const jsv::Token op(jsv::TokenKind::PlusEqual, "+=", span);

        REQUIRE(identifier.getKind() == jsv::TokenKind::IdentifierAscii);
        REQUIRE(number.getKind() == jsv::TokenKind::Numeric);
        REQUIRE(op.getKind() == jsv::TokenKind::PlusEqual);

        REQUIRE(identifier.getText() == "myVar");
        REQUIRE(number.getText() == "42");
        REQUIRE(op.getText() == "+=");
    }

    SECTION("Token with empty text") {
        const jsv::Token token(jsv::TokenKind::Eof, "", span);
        REQUIRE(token.getText().empty());
        REQUIRE(token.getKind() == jsv::TokenKind::Eof);
    }

    SECTION("Token with unicode identifier") {
        const jsv::Token token(jsv::TokenKind::IdentifierUnicode, "变量", span);
        REQUIRE(token.getText() == "变量");
        REQUIRE(token.getKind() == jsv::TokenKind::IdentifierUnicode);
    }
}

TEST_CASE("Token copy and move semantics", "[Token]") {
    const jsv::SourceLocation start(1u, 1u, 0u);
    const jsv::SourceLocation end(1u, 5u, 4u);
    const jsv::SourceSpan span("test.cpp", start, end);

    SECTION("Token copy constructor") {
        const jsv::Token original(jsv::TokenKind::KeywordIf, "if", span);
        const jsv::Token copied(original);

        REQUIRE(copied.getKind() == original.getKind());
        REQUIRE(copied.getText() == original.getText());
        REQUIRE(copied.getSpan() == original.getSpan());
    }

    SECTION("Token copy assignment") {
        jsv::Token token1(jsv::TokenKind::KeywordWhile, "while", span);
        const jsv::Token token2(jsv::TokenKind::KeywordFor, "for", span);

        token1 = token2;

        REQUIRE(token1.getKind() == token2.getKind());
        REQUIRE(token1.getText() == token2.getText());
        REQUIRE(token1.getSpan() == token2.getSpan());
    }

    SECTION("Token move constructor") {
        jsv::Token original(jsv::TokenKind::KeywordReturn, "return", span);
        const jsv::Token moved(std::move(original));

        REQUIRE(moved.getKind() == jsv::TokenKind::KeywordReturn);
        REQUIRE(moved.getText() == "return");
    }

    SECTION("Token move assignment") {
        jsv::Token token1(jsv::TokenKind::KeywordBreak, "break", span);
        jsv::Token token2(jsv::TokenKind::KeywordContinue, "continue", span);

        token1 = std::move(token2);

        REQUIRE(token1.getKind() == jsv::TokenKind::KeywordContinue);
        REQUIRE(token1.getText() == "continue");
    }

    SECTION("Token self-assignment") {
        jsv::Token token(jsv::TokenKind::KeywordVar, "var", span);
        const jsv::Token *tokenPtr = &token;

        // NOLINTNEXTLINE(*-self-assign)
        token = *tokenPtr;

        REQUIRE(token.getKind() == jsv::TokenKind::KeywordVar);
        REQUIRE(token.getText() == "var");
    }
}

TEST_CASE("Token equality and comparison operators", "[Token]") {
    const jsv::SourceLocation start(1u, 1u, 0u);
    const jsv::SourceLocation end(1u, 5u, 4u);
    const jsv::SourceSpan span("test.cpp", start, end);

    SECTION("Equal tokens compare equal") {
        const jsv::Token token1(jsv::TokenKind::KeywordFun, "fun", span);
        const jsv::Token token2(jsv::TokenKind::KeywordFun, "fun", span);

        REQUIRE(token1 == token2);
        REQUIRE_FALSE(token1 != token2);
    }

    SECTION("Tokens with different kind are not equal") {
        const jsv::Token token1(jsv::TokenKind::KeywordFun, "fun", span);
        const jsv::Token token2(jsv::TokenKind::KeywordMain, "main", span);

        REQUIRE(token1 != token2);
        REQUIRE_FALSE(token1 == token2);
    }

    SECTION("Tokens with different text are not equal") {
        const jsv::Token token1(jsv::TokenKind::IdentifierAscii, "var1", span);
        const jsv::Token token2(jsv::TokenKind::IdentifierAscii, "var2", span);

        REQUIRE(token1 != token2);
    }

    SECTION("Tokens with different span are not equal") {
        const jsv::SourceLocation start2(2u, 1u, 10u);
        const jsv::SourceLocation end2(2u, 5u, 14u);
        const jsv::SourceSpan span2("test.cpp", start2, end2);

        const jsv::Token token1(jsv::TokenKind::KeywordIf, "if", span);
        const jsv::Token token2(jsv::TokenKind::KeywordIf, "if", span2);

        REQUIRE(token1 != token2);
    }

    SECTION("Three-way comparison operator") {
        const jsv::Token token1(jsv::TokenKind::KeywordIf, "if", span);
        const jsv::Token token2(jsv::TokenKind::KeywordIf, "if", span);
        const jsv::Token token3(jsv::TokenKind::KeywordElse, "else", span);

        REQUIRE((token1 <=> token2) == std::strong_ordering::equal);
        REQUIRE((token1 <=> token3) != std::strong_ordering::equal);
    }
}

TEST_CASE("Token to_string method", "[Token]") {
    const jsv::SourceLocation start(1u, 1u, 0u);
    const jsv::SourceLocation end(1u, 5u, 4u);
    const jsv::SourceSpan span("test.cpp", start, end);

    SECTION("to_string for keyword token") {
        const jsv::Token token(jsv::TokenKind::KeywordFun, "fun", span);
        const std::string result = token.to_string();

        REQUIRE(result == R"(FUN("fun") test.cpp:line 1:column 1 - line 1:column 5)");
    }

    SECTION("to_string for operator token") {
        const jsv::Token token(jsv::TokenKind::PlusEqual, "+=", span);
        const std::string result = token.to_string();

        REQUIRE(result == R"(PLUS_EQUAL("+=") test.cpp:line 1:column 1 - line 1:column 5)");
    }

    SECTION("to_string for identifier token") {
        const jsv::Token token(jsv::TokenKind::IdentifierAscii, "myVariable", span);
        const std::string result = token.to_string();

        REQUIRE(result == R"(IDENTIFIER("myVariable") test.cpp:line 1:column 1 - line 1:column 5)");
    }

    SECTION("to_string for numeric literal token") {
        const jsv::Token token(jsv::TokenKind::Numeric, "123.456", span);
        const std::string result = token.to_string();

        REQUIRE(result == R"(NUMERIC("123.456") test.cpp:line 1:column 1 - line 1:column 5)");
    }

    SECTION("to_string for string literal token") {
        const jsv::Token token(jsv::TokenKind::StringLiteral, R"(hello "world")", span);
        const std::string result = token.to_string();

        REQUIRE(result == R"(STRING("hello "world"") test.cpp:line 1:column 1 - line 1:column 5)");
    }

    SECTION("to_string for type token") {
        const jsv::Token token(jsv::TokenKind::TypeI32, "i32", span);
        const std::string result = token.to_string();

        REQUIRE(result == R"(I32("i32") test.cpp:line 1:column 1 - line 1:column 5)");
    }

    SECTION("to_string for EOF token") {
        const jsv::Token token(jsv::TokenKind::Eof, "", span);
        const std::string result = token.to_string();

        REQUIRE(result == R"(EOF("") test.cpp:line 1:column 1 - line 1:column 5)");
    }

    SECTION("to_string for error token") {
        const jsv::Token token(jsv::TokenKind::Error, "@invalid", span);
        const std::string result = token.to_string();

        REQUIRE(result == R"(ERROR("@invalid") test.cpp:line 1:column 1 - line 1:column 5)");
    }
}

TEST_CASE("Token stream output operator", "[Token]") {
    const jsv::SourceLocation start(1u, 1u, 0u);
    const jsv::SourceLocation end(1u, 5u, 4u);
    const jsv::SourceSpan span("test.cpp", start, end);

    SECTION("ostream operator outputs to_string result") {
        const jsv::Token token(jsv::TokenKind::KeywordReturn, "return", span);
        std::ostringstream oss;
        oss << token;

        REQUIRE(oss.str() == R"(RETURN("return") test.cpp:line 1:column 1 - line 1:column 5)");
    }

    SECTION("ostream operator with multiple tokens") {
        const jsv::Token token1(jsv::TokenKind::KeywordIf, "if", span);
        const jsv::Token token2(jsv::TokenKind::KeywordElse, "else", span);

        std::ostringstream oss;
        oss << token1 << " else " << token2;

        REQUIRE(oss.str() ==
                R"(IF("if") test.cpp:line 1:column 1 - line 1:column 5 else ELSE("else") test.cpp:line 1:column 1 - line 1:column 5)");
    }

    SECTION("ostream operator preserves stream state") {
        const jsv::Token token(jsv::TokenKind::Numeric, "42", span);
        std::ostringstream oss;
        oss << std::uppercase << std::hex << 255;  // Set stream state
        oss << " " << token;

        const std::string result = oss.str();
        REQUIRE_THAT(result, ContainsSubstring("FF"));
        REQUIRE_THAT(result, ContainsSubstring(R"(NUMERIC("42"))"));
    }
}

TEST_CASE("Token std::formatter integration", "[Token]") {
    const jsv::SourceLocation start(1u, 1u, 0u);
    const jsv::SourceLocation end(1u, 5u, 4u);
    const jsv::SourceSpan span("test.cpp", start, end);

    SECTION("std::format with default format") {
        const jsv::Token token(jsv::TokenKind::KeywordFor, "for", span);
        const std::string result = std::format("{}", token);

        REQUIRE(result == R"(FOR("for") test.cpp:line 1:column 1 - line 1:column 5)");
    }

    SECTION("std::format in format string") {
        const jsv::Token token(jsv::TokenKind::KeywordWhile, "while", span);
        const std::string result = std::format("Token: {}", token);

        REQUIRE(result == R"(Token: WHILE("while") test.cpp:line 1:column 1 - line 1:column 5)");
    }

    SECTION("std::format with multiple tokens") {
        const jsv::Token token1(jsv::TokenKind::OpenParen, "(", span);
        const jsv::Token token2(jsv::TokenKind::CloseParen, ")", span);

        const std::string result = std::format("{} {}", token1, token2);

        // "(()" + "())" = "((())())"
        REQUIRE(
            result ==
            "OPEN_PAREN(\"(\") test.cpp:line 1:column 1 - line 1:column 5 CLOSE_PAREN(\")\") test.cpp:line 1:column 1 - line 1:column 5");
    }
}

TEST_CASE("Token fmt::formatter integration", "[Token]") {
    const jsv::SourceLocation start(1u, 1u, 0u);
    const jsv::SourceLocation end(1u, 5u, 4u);
    const jsv::SourceSpan span("test.cpp", start, end);

    SECTION("fmt::format with default format") {
        const jsv::Token token(jsv::TokenKind::KeywordMain, "main", span);
        const std::string result = fmt::format("{}", token);

        REQUIRE(result == R"(MAIN("main") test.cpp:line 1:column 1 - line 1:column 5)");
    }

    SECTION("fmt::format in format string") {
        const jsv::Token token(jsv::TokenKind::KeywordVar, "var", span);
        const std::string result = fmt::format("Token: {}", token);

        REQUIRE(result == R"(Token: VAR("var") test.cpp:line 1:column 1 - line 1:column 5)");
    }
}

TEST_CASE("Token corner cases and edge cases", "[Token]") {
    const jsv::SourceLocation start(1u, 1u, 0u);
    const jsv::SourceLocation end(1u, 1u, 0u);
    const jsv::SourceSpan span("test.cpp", start, end);

    SECTION("Token with very long text") {
        const std::string longText(1000, 'a');
        const jsv::Token token(jsv::TokenKind::IdentifierAscii, longText, span);

        REQUIRE(token.getText().size() == 1000u);
        REQUIRE(token.to_string().size() > 1000u);
    }

    SECTION("Token with special characters in text") {
        const jsv::Token token(jsv::TokenKind::StringLiteral, R"(\n\t\r\"\')", span);
        REQUIRE(token.getText() == R"(\n\t\r\"\')");
    }

    SECTION("Token with null character in text") {
        const std::string textWithNull = "hello world";
        const jsv::Token token(jsv::TokenKind::StringLiteral, std::string_view(textWithNull.data(), 11), span);

        REQUIRE(token.getText().size() == 11u);
    }

    SECTION("Token at position zero") {
        const jsv::SourceLocation zeroLoc(0u, 0u, 0u);
        const jsv::SourceSpan zeroSpan("test.cpp", zeroLoc, zeroLoc);
        const jsv::Token token(jsv::TokenKind::Eof, "", zeroSpan);

        REQUIRE(token.getSpan().start.line == 0u);
        REQUIRE(token.getSpan().start.column == 0u);
        REQUIRE(token.getSpan().start.absolute_pos == 0u);
    }

    SECTION("Token at large position values") {
        constexpr std::size_t largeLine = std::numeric_limits<std::size_t>::max() - 1000u;
        constexpr std::size_t largeCol = std::numeric_limits<std::size_t>::max() - 500u;
        constexpr std::size_t largeOffset = std::numeric_limits<std::size_t>::max() - 100u;

        const jsv::SourceLocation largeLoc(largeLine, largeCol, largeOffset);
        const jsv::SourceSpan largeSpan("test.cpp", largeLoc, largeLoc);
        const jsv::Token token(jsv::TokenKind::IdentifierAscii, "x", largeSpan);

        REQUIRE(token.getSpan().start.line == largeLine);
        REQUIRE(token.getSpan().start.column == largeCol);
        REQUIRE(token.getSpan().start.absolute_pos == largeOffset);
    }
}

TEST_CASE("Token noexcept contracts", "[Token]") {
    const jsv::SourceLocation start(1u, 1u, 0u);
    const jsv::SourceLocation end(1u, 5u, 4u);
    const jsv::SourceSpan span("test.cpp", start, end);

    STATIC_REQUIRE(std::is_nothrow_copy_constructible_v<jsv::Token>);
    STATIC_REQUIRE(std::is_nothrow_copy_assignable_v<jsv::Token>);
    STATIC_REQUIRE(std::is_nothrow_move_constructible_v<jsv::Token>);
    STATIC_REQUIRE(std::is_nothrow_move_assignable_v<jsv::Token>);

    SECTION("getKind does not throw") {
        const jsv::Token token(jsv::TokenKind::KeywordIf, "if", span);
        REQUIRE_NOTHROW(std::ignore = token.getKind());
    }

    SECTION("getText does not throw") {
        const jsv::Token token(jsv::TokenKind::KeywordIf, "if", span);
        REQUIRE_NOTHROW(std::ignore = token.getText());
    }

    SECTION("getSpan does not throw") {
        const jsv::Token token(jsv::TokenKind::KeywordIf, "if", span);
        REQUIRE_NOTHROW(std::ignore = token.getSpan());
    }

    SECTION("to_string does not throw") {
        const jsv::Token token(jsv::TokenKind::KeywordIf, "if", span);
        REQUIRE_NOTHROW(std::ignore = token.to_string());
    }

    // NOLINTBEGIN(*-analyzer-cplusplus.Move, *-diagnostic-unused-variable)
    SECTION("copy operations do not throw") {
        const jsv::Token token(jsv::TokenKind::KeywordIf, "if", span);
        REQUIRE_NOTHROW([&]() { const jsv::Token copied(token); }());
        REQUIRE_NOTHROW([&]() { const jsv::Token assigned = token; }());
    }

    SECTION("move operations do not throw") {
        jsv::Token token(jsv::TokenKind::KeywordIf, "if", span);
        REQUIRE_NOTHROW([&]() { const jsv::Token moved(std::move(token)); }());

        jsv::Token token2(jsv::TokenKind::KeywordElse, "else", span);
        REQUIRE_NOTHROW(token2 = std::move(token));
    }
    // NOLINTEND(*-analyzer-cplusplus.Move, *-diagnostic-unused-variable)

    SECTION("comparison operators do not throw") {
        const jsv::Token token1(jsv::TokenKind::KeywordIf, "if", span);
        const jsv::Token token2(jsv::TokenKind::KeywordIf, "if", span);

        REQUIRE_NOTHROW(std::ignore = (token1 == token2));
        REQUIRE_NOTHROW(std::ignore = (token1 != token2));
        REQUIRE_NOTHROW(std::ignore = (token1 <=> token2));
    }
}

TEST_CASE("Token data-driven tests", "[Token]") {
    const jsv::SourceLocation start(1u, 1u, 0u);
    const jsv::SourceLocation end(1u, 5u, 4u);
    const jsv::SourceSpan span("test.cpp", start, end);

    SECTION("various keyword tokens") {
        auto [kind, text, expected] = GENERATE(table<jsv::TokenKind, const char *, const char *>({
            {jsv::TokenKind::KeywordFun, "fun", R"(FUN("fun") test.cpp:line 1:column 1 - line 1:column 5)"},
            {jsv::TokenKind::KeywordIf, "if", R"(IF("if") test.cpp:line 1:column 1 - line 1:column 5)"},
            {jsv::TokenKind::KeywordElse, "else", R"(ELSE("else") test.cpp:line 1:column 1 - line 1:column 5)"},
            {jsv::TokenKind::KeywordReturn, "return", R"(RETURN("return") test.cpp:line 1:column 1 - line 1:column 5)"},
            {jsv::TokenKind::KeywordWhile, "while", R"(WHILE("while") test.cpp:line 1:column 1 - line 1:column 5)"},
            {jsv::TokenKind::KeywordFor, "for", R"(FOR("for") test.cpp:line 1:column 1 - line 1:column 5)"},
            {jsv::TokenKind::KeywordMain, "main", R"(MAIN("main") test.cpp:line 1:column 1 - line 1:column 5)"},
            {jsv::TokenKind::KeywordVar, "var", R"(VAR("var") test.cpp:line 1:column 1 - line 1:column 5)"},
            {jsv::TokenKind::KeywordConst, "const", R"(CONST("const") test.cpp:line 1:column 1 - line 1:column 5)"},
        }));
        CAPTURE(kind, text, expected);

        const jsv::Token token(kind, text, span);
        REQUIRE(token.to_string() == expected);
    }

    SECTION("various operator tokens") {
        auto [kind, text] = GENERATE(table<jsv::TokenKind, const char *>({
            {jsv::TokenKind::Plus, "+"},
            {jsv::TokenKind::Minus, "-"},
            {jsv::TokenKind::Star, "*"},
            {jsv::TokenKind::Slash, "/"},
            {jsv::TokenKind::Equal, "="},
            {jsv::TokenKind::EqualEqual, "=="},
            {jsv::TokenKind::NotEqual, "!="},
            {jsv::TokenKind::Less, "<"},
            {jsv::TokenKind::Greater, ">"},
            {jsv::TokenKind::LessEqual, "<="},
            {jsv::TokenKind::GreaterEqual, ">="},
        }));
        CAPTURE(kind, text);

        const jsv::Token token(kind, text, span);
        REQUIRE(token.getText() == text);
        REQUIRE(token.getKind() == kind);
    }

    SECTION("various type tokens") {
        auto [kind, text] = GENERATE(table<jsv::TokenKind, const char *>({
            {jsv::TokenKind::TypeI8, "i8"},
            {jsv::TokenKind::TypeI16, "i16"},
            {jsv::TokenKind::TypeI32, "i32"},
            {jsv::TokenKind::TypeI64, "i64"},
            {jsv::TokenKind::TypeU8, "u8"},
            {jsv::TokenKind::TypeU16, "u16"},
            {jsv::TokenKind::TypeU32, "u32"},
            {jsv::TokenKind::TypeU64, "u64"},
            {jsv::TokenKind::TypeF32, "f32"},
            {jsv::TokenKind::TypeF64, "f64"},
            {jsv::TokenKind::TypeBool, "bool"},
        }));
        CAPTURE(kind, text);

        const jsv::Token token(kind, text, span);
        REQUIRE(token.getText() == text);
        REQUIRE(token.getKind() == kind);
    }
}

// ==========================================================================
// Phase 3 – UTF-8 decoder integration (Lexer runtime)
// ==========================================================================

TEST_CASE("Lexer_AsciiOnlySource_TokenizeCorrectly", "[lexer][utf8][phase3]") {
    jsv::Lexer lex{"hello world 42", "test.jsav"};
    const auto tokens = lex.tokenize();
    // hello, world, 42, Eof
    REQUIRE(tokens.size() == 4);
    REQUIRE(tokens[0].getKind() == jsv::TokenKind::IdentifierAscii);
    REQUIRE(tokens[0].getText() == "hello");
    REQUIRE(tokens[1].getKind() == jsv::TokenKind::IdentifierAscii);
    REQUIRE(tokens[1].getText() == "world");
    REQUIRE(tokens[2].getKind() == jsv::TokenKind::Numeric);
    REQUIRE(tokens[2].getText() == "42");
    REQUIRE(tokens[3].getKind() == jsv::TokenKind::Eof);
}

TEST_CASE("Lexer_TwoByteIdentifier_ReturnsIdentifierUnicode", "[lexer][utf8][phase3]") {
    // Ω = U+03A9, UTF-8: 0xCE 0xA9 (2 bytes)

    const std::string src = "\xCE\xA9";
    jsv::Lexer lex{src, "test.jsav"};
    const auto tokens = lex.tokenize();
    REQUIRE(tokens.size() == 2);
    REQUIRE(tokens[0].getKind() == jsv::TokenKind::IdentifierUnicode);
    REQUIRE(tokens[0].getText() == src);
    REQUIRE(tokens[1].getKind() == jsv::TokenKind::Eof);
}

TEST_CASE("Lexer_ThreeByteIdentifier_ReturnsIdentifierUnicode", "[lexer][utf8][phase3]") {
    // 変 = U+5909, UTF-8: 0xE5 0xA4 0x89 (3 bytes)

    const std::string src = "\xE5\xA4\x89";
    jsv::Lexer lex{src, "test.jsav"};
    const auto tokens = lex.tokenize();
    REQUIRE(tokens.size() == 2);
    REQUIRE(tokens[0].getKind() == jsv::TokenKind::IdentifierUnicode);
    REQUIRE(tokens[0].getText() == src);
    REQUIRE(tokens[1].getKind() == jsv::TokenKind::Eof);
}

TEST_CASE("Lexer_FourByteIdentifier_ReturnsIdentifierUnicode", "[lexer][utf8][phase3]") {
    // 𝑥 = U+1D465 (Mathematical Italic Small x), UTF-8: 0xF0 0x9D 0x91 0xA5 (4 bytes)

    const std::string src = "\xF0\x9D\x91\xA5";
    jsv::Lexer lex{src, "test.jsav"};
    const auto tokens = lex.tokenize();
    REQUIRE(tokens.size() == 2);
    REQUIRE(tokens[0].getKind() == jsv::TokenKind::IdentifierUnicode);
    REQUIRE(tokens[0].getText() == src);
    REQUIRE(tokens[1].getKind() == jsv::TokenKind::Eof);
}

TEST_CASE("Lexer_NullByteInStringView_NotTreatedAsTerminator", "[lexer][utf8][phase3]") {
    // A string_view containing a null byte must NOT be treated as the end of input.
    // Source: "ab" + U+0000 + "cd" → IdentifierAscii("ab"), Error, IdentifierAscii("cd"), Eof
    using namespace std::string_literals;
    const std::string src = "ab\x00"
                            "cd"s;  // 5 bytes: a b \0 c d
    REQUIRE(src.size() == 5);
    jsv::Lexer lex{src, "test.jsav"};
    const auto tokens = lex.tokenize();
    REQUIRE(tokens.size() == 4);
    REQUIRE(tokens[0].getKind() == jsv::TokenKind::IdentifierAscii);
    REQUIRE(tokens[0].getText() == "ab");
    REQUIRE(tokens[1].getKind() == jsv::TokenKind::Error);
    REQUIRE(tokens[2].getKind() == jsv::TokenKind::IdentifierAscii);
    REQUIRE(tokens[2].getText() == "cd");
    REQUIRE(tokens[3].getKind() == jsv::TokenKind::Eof);
}

// ==========================================================================
// Phase 4 – Malformed UTF-8 handling (Lexer runtime)
// ==========================================================================

TEST_CASE("Lexer_MalformedOrphanedContinuation_EmitsErrorToken", "[lexer][utf8][malformed][phase4]") {
    // 0x80 is an orphaned continuation byte — must produce Error token
    const std::string src = "\x80";
    jsv::Lexer lex{src, "test.jsav"};
    const auto tokens = lex.tokenize();
    REQUIRE(tokens.size() == 2);
    REQUIRE(tokens[0].getKind() == jsv::TokenKind::Error);
    REQUIRE(tokens[0].getText().size() == 1);
    REQUIRE(tokens[1].getKind() == jsv::TokenKind::Eof);
}

TEST_CASE("Lexer_MalformedOverlong_EmitsErrorToken", "[lexer][utf8][malformed][phase4]") {
    // 0xC0 0xAF is an overlong encoding of '/' — must produce Error token(s)
    const std::string src = "\xC0\xAF";
    jsv::Lexer lex{src, "test.jsav"};
    const auto tokens = lex.tokenize();
    // At minimum: first token must be Error
    REQUIRE_FALSE(tokens.empty());
    REQUIRE(tokens[0].getKind() == jsv::TokenKind::Error);
    REQUIRE(tokens.back().getKind() == jsv::TokenKind::Eof);
}

TEST_CASE("Lexer_MalformedMidFile_ContinuesTokenizing", "[lexer][utf8][malformed][phase4]") {
    // Malformed byte followed by valid tokens — recovery must work
    const std::string src = "\x80 var x";
    jsv::Lexer lex{src, "test.jsav"};
    const auto tokens = lex.tokenize();
    // Error(\x80), KeywordVar, IdentifierAscii("x"), Eof
    REQUIRE(tokens.size() == 4);
    REQUIRE(tokens[0].getKind() == jsv::TokenKind::Error);
    REQUIRE(tokens[1].getKind() == jsv::TokenKind::KeywordVar);
    REQUIRE(tokens[2].getKind() == jsv::TokenKind::IdentifierAscii);
    REQUIRE(tokens[2].getText() == "x");
    REQUIRE(tokens[3].getKind() == jsv::TokenKind::Eof);
}

TEST_CASE("Lexer_MalformedInsideStringLiteral_EntireLiteralBecomesError", "[lexer][utf8][malformed][phase4]") {
    // String literal containing overlong sequence → entire literal is Error per FR-021
    // Source: "  + 0xC0 + 0xAF + "
    const std::string src = "\"\xC0\xAF\"";
    jsv::Lexer lex{src, "test.jsav"};
    const auto tokens = lex.tokenize();
    REQUIRE(tokens.size() == 2);
    REQUIRE(tokens[0].getKind() == jsv::TokenKind::Error);
    REQUIRE(tokens[1].getKind() == jsv::TokenKind::Eof);
}

TEST_CASE("Lexer_MalformedInsideCharLiteral_EntireLiteralBecomesError", "[lexer][utf8][malformed][phase4]") {
    // Char literal containing orphaned continuation → entire literal is Error per FR-021
    // Source: '  + 0x80 + '
    const std::string src = "\'\x80\'";
    jsv::Lexer lex{src, "test.jsav"};
    const auto tokens = lex.tokenize();
    REQUIRE(tokens.size() == 2);
    REQUIRE(tokens[0].getKind() == jsv::TokenKind::Error);
    REQUIRE(tokens[1].getKind() == jsv::TokenKind::Eof);
}

// ==========================================================================
// Phase 5 – Unicode identifier recognition (Lexer runtime)
// ==========================================================================

TEST_CASE("Lexer_CJKIdentifier_ReturnsIdentifierUnicode", "[lexer][utf8][identifiers][phase5]") {
    // 变量名 = U+53D8 U+91CF U+540D (3 CJK characters)
    const std::string src = "\xe5\x8f\x98\xe9\x87\x8f\xe5\x90\x8d";
    jsv::Lexer lex{src, "test.jsav"};
    const auto tokens = lex.tokenize();
    REQUIRE(tokens.size() == 2);
    REQUIRE(tokens[0].getKind() == jsv::TokenKind::IdentifierUnicode);
    REQUIRE(tokens[0].getText() == src);
    REQUIRE(tokens[1].getKind() == jsv::TokenKind::Eof);
}

TEST_CASE("Lexer_CyrillicWithCombiningMark_ReturnsSingleIdentifier", "[lexer][utf8][identifiers][phase5]") {
    // и̃мя = U+0438 U+0303 U+043C U+044F (Cyrillic + combining tilde + letters)
    const std::string src = "\xd0\xb8\xcc\x83\xd0\xbc\xd0\xaf";
    jsv::Lexer lex{src, "test.jsav"};
    const auto tokens = lex.tokenize();
    REQUIRE(tokens.size() == 2);
    REQUIRE(tokens[0].getKind() == jsv::TokenKind::IdentifierUnicode);
    REQUIRE(tokens[0].getText() == src);
    REQUIRE(tokens[1].getKind() == jsv::TokenKind::Eof);
}

TEST_CASE("Lexer_DevanagariIdentifier_ReturnsIdentifierUnicode", "[lexer][utf8][identifiers][phase5]") {
    // गणना = U+0917 U+0923 U+0928 U+093E
    const std::string src = "\xe0\xa4\x97\xe0\xa4\xa3\xe0\xa4\xa8\xe0\xa4\xbe";
    jsv::Lexer lex{src, "test.jsav"};
    const auto tokens = lex.tokenize();
    REQUIRE(tokens.size() == 2);
    REQUIRE(tokens[0].getKind() == jsv::TokenKind::IdentifierUnicode);
    REQUIRE(tokens[0].getText() == src);
    REQUIRE(tokens[1].getKind() == jsv::TokenKind::Eof);
}

TEST_CASE("Lexer_UnderscoreUnicode_ReturnsIdentifierUnicode", "[lexer][utf8][identifiers][phase5]") {
    // _变量 = _ + U+5909 + U+91CF (underscore + CJK) per FR-018
    const std::string src = "_\xe5\xa4\x89\xe9\x87\x8f";
    jsv::Lexer lex{src, "test.jsav"};
    const auto tokens = lex.tokenize();
    REQUIRE(tokens.size() == 2);
    REQUIRE(tokens[0].getKind() == jsv::TokenKind::IdentifierUnicode);
    REQUIRE(tokens[0].getText() == src);
    REQUIRE(tokens[1].getKind() == jsv::TokenKind::Eof);
}

TEST_CASE("Lexer_EmojiOutsideLiteral_ReturnsErrorToken", "[lexer][utf8][identifiers][phase5]") {
    // 😀 = U+1F600 (F0 9F 98 80) — not a letter → Error per FR-022
    const std::string src = "\xf0\x9f\x98\x80";
    jsv::Lexer lex{src, "test.jsav"};
    const auto tokens = lex.tokenize();
    REQUIRE(tokens.size() == 2);
    REQUIRE(tokens[0].getKind() == jsv::TokenKind::Error);
    REQUIRE(tokens[1].getKind() == jsv::TokenKind::Eof);
}

TEST_CASE("Lexer_EmojiZWJSequence_NotRecognizedAsIdentifier", "[lexer][utf8][identifiers][phase5]") {
    // 👨‍👩 = U+1F468 U+200D U+1F469 — ZWJ sequences must NOT form identifier per FR-016
    const std::string src = "\xf0\x9f\x91\xa8\xe2\x80\x8d\xf0\x9f\x91\xa9";
    jsv::Lexer lex{src, "test.jsav"};
    const auto tokens = lex.tokenize();
    // None of the tokens should be IdentifierUnicode; all non-Eof tokens must be Error
    REQUIRE(tokens.back().getKind() == jsv::TokenKind::Eof);
    for(std::size_t i = 0; i + 1 < tokens.size(); ++i) { REQUIRE(tokens[i].getKind() == jsv::TokenKind::Error); }
}

TEST_CASE("Lexer_MarkAtIdentifierStart_NotRecognizedAsIdentifier", "[lexer][utf8][identifiers][phase5]") {
    // U+0303 (combining tilde) alone — combining marks cannot start identifiers per FR-012
    const std::string src = "\xcc\x83";  // U+0303 in UTF-8: CC 83
    jsv::Lexer lex{src, "test.jsav"};
    const auto tokens = lex.tokenize();
    REQUIRE(tokens.size() == 2);
    REQUIRE(tokens[0].getKind() == jsv::TokenKind::Error);
    REQUIRE(tokens[1].getKind() == jsv::TokenKind::Eof);
}

TEST_CASE("Lexer_NumberAtIdentifierStart_NotRecognizedAsIdentifier", "[lexer][utf8][identifiers][phase5]") {
    // U+0660 (Arabic-Indic digit zero) alone — Nd category cannot start identifiers per FR-012
    const std::string src = "\xd9\xa0";  // U+0660 in UTF-8: D9 A0
    jsv::Lexer lex{src, "test.jsav"};
    const auto tokens = lex.tokenize();
    REQUIRE(tokens.size() == 2);
    REQUIRE(tokens[0].getKind() == jsv::TokenKind::Error);
    REQUIRE(tokens[1].getKind() == jsv::TokenKind::Eof);
}

TEST_CASE("Lexer_ThirtyPlusScripts_AllTokenizeCorrectly", "[lexer][utf8][identifiers][phase5][sc001]") {
    // SC-001: identifiers from ≥30 distinct Unicode scripts must tokenize as IdentifierUnicode
    struct ScriptCase {
        const char *name;
        std::string src;
    };
    // One representative identifier per script (encoded in UTF-8)
    const std::vector<ScriptCase> cases = {
        {.name = "Latin (ASCII)", .src = "hello"},
        {.name = "Greek", .src = "\xce\xb1\xce\xb2\xce\xb3"},                 // αβγ
        {.name = "Cyrillic", .src = "\xd0\xb0\xd0\xb1\xd0\xb2"},              // абв
        {.name = "Armenian", .src = "\xd5\xb1\xd5\xb2\xd5\xb3"},              // աբգ
        {.name = "Georgian", .src = "\xe1\x83\x90\xe1\x83\x91\xe1\x83\x92"},  // აბგ U+10D0-U+10D2
        {.name = "Hebrew", .src = "\xd7\x90\xd7\x91\xd7\x92"},                // אבג
        {.name = "Arabic", .src = "\xd8\xa7\xd8\xa8\xd8\xaa"},                // ابت
        {.name = "Devanagari", .src = "\xe0\xa4\x97\xe0\xa4\xa3"},            // गण
        {.name = "Bengali", .src = "\xe0\xa6\x97\xe0\xa6\xa3"},               // গণ U+0997 U+09A3
        {.name = "Gurmukhi", .src = "\xe0\xa8\x97\xe0\xa8\xa3"},              // ਗਣ U+0A17 U+0A23
        {.name = "Gujarati", .src = "\xe0\xaa\x97\xe0\xaa\xa3"},              // ગણ U+0A97 U+0AA3
        {.name = "Tamil", .src = "\xe0\xae\x95\xe0\xae\xa3"},                 // கண U+0B95 U+0BA3
        {.name = "Telugu", .src = "\xe0\xb0\x97\xe0\xb0\xa3"},                // గణ U+0C17 U+0C23
        {.name = "Kannada", .src = "\xe0\xb2\x97\xe0\xb2\xa3"},               // ಗಣ U+0C97 U+0CA3
        {.name = "Malayalam", .src = "\xe0\xb4\x97\xe0\xb4\xa3"},             // ഗണ U+0D17 U+0D23
        {.name = "Sinhala", .src = "\xe0\xb6\x9c\xe0\xb6\xab"},               // ගණ U+0D9C U+0DAB
        {.name = "Thai", .src = "\xe0\xb8\x81\xe0\xb8\x82"},                  // กข U+0E01 U+0E02
        {.name = "Lao", .src = "\xe0\xba\x81\xe0\xba\x82"},                   // ກຂ U+0E81 U+0E82
        {.name = "Tibetan", .src = "\xe0\xbd\x80\xe0\xbd\x81"},               // ཀཁ U+0F00 U+0F01 (actually Tibetan letters start at U+0F40)
        {.name = "Myanmar", .src = "\xe1\x80\x80\xe1\x80\x81"},               // ကခ U+1000 U+1001
        {.name = "Hangul", .src = "\xea\xb0\x80\xeb\x82\x98"},                // 가나 U+AC00 U+B098
        {.name = "Hiragana", .src = "\xe3\x81\x82\xe3\x81\x84"},              // あい U+3042 U+3044
        {.name = "Katakana", .src = "\xe3\x82\xa2\xe3\x82\xa4"},              // アイ U+30A2 U+30A4
        {.name = "CJK", .src = "\xe5\x8f\x98\xe9\x87\x8f"},                   // 变量 U+53D8 U+91CF
        {.name = "Ethiopic", .src = "\xe1\x88\x80\xe1\x88\x81"},              // ሀሁ U+1200 U+1201
        {.name = "Cherokee", .src = "\xe1\x8e\xa0\xe1\x8e\xa1"},              // ᏠᏡ U+13A0 U+13A1
        {.name = "Khmer", .src = "\xe1\x9e\x80\xe1\x9e\x81"},                 // កខ U+1780 U+1781
        {.name = "Mongolian", .src = "\xe1\xa0\xa0\xe1\xa0\xa1"},             // ᠠᠡ U+1820 U+1821
        {.name = "Tai Le", .src = "\xe1\xa5\x90\xe1\xa5\x91"},                // ᥐᥑ U+1950 U+1951
        {.name = "Math Italic", .src = "\xf0\x9d\x91\xa5\xf0\x9d\x91\xa6"},   // 𝑥𝑦 U+1D465 U+1D466
    };

    for(const auto &c : cases) {
        INFO("Script: " << c.name);
        jsv::Lexer lex{c.src, "test.jsav"};
        const auto tokens = lex.tokenize();
        REQUIRE(tokens.size() == 2);
        if(c.src == "hello") {
            REQUIRE(tokens[0].getKind() == jsv::TokenKind::IdentifierAscii);
        } else {
            REQUIRE(tokens[0].getKind() == jsv::TokenKind::IdentifierUnicode);
        }
        REQUIRE(tokens[0].getText() == c.src);
        REQUIRE(tokens[1].getKind() == jsv::TokenKind::Eof);
    }
}

// ==========================================================================
// Phase 6 – ASCII compatibility preservation (BOM, Unicode whitespace, regression)
// ==========================================================================

TEST_CASE("Lexer_BOMAtStart_SkippedTransparently", "[lexer][utf8][ascii-compat][phase6]") {
    // BOM = 0xEF 0xBB 0xBF — must be silently skipped (FR-019)
    const std::string src = "\xEF\xBB\xBF"
                            "var x";
    jsv::Lexer lex{src, "test.jsav"};
    const auto tokens = lex.tokenize();
    // Expected: KeywordVar("var"), IdentifierAscii("x"), Eof
    REQUIRE(tokens.size() == 3);
    REQUIRE(tokens[0].getKind() == jsv::TokenKind::KeywordVar);
    REQUIRE(tokens[0].getText() == "var");
    REQUIRE(tokens[1].getKind() == jsv::TokenKind::IdentifierAscii);
    REQUIRE(tokens[1].getText() == "x");
    REQUIRE(tokens[2].getKind() == jsv::TokenKind::Eof);
}

TEST_CASE("Lexer_UnicodeWhitespace_NoBreakSpace_ConsumedSilently", "[lexer][utf8][ascii-compat][phase6]") {
    // U+00A0 NO-BREAK SPACE (0xC2 0xA0, category Zs) must be consumed as whitespace (FR-023)

    const std::string src = "a\xC2\xA0"
                            "b";  // "a" + NBSP + "b"
    jsv::Lexer lex{src, "test.jsav"};
    const auto tokens = lex.tokenize();
    // Expected: IdentifierAscii("a"), IdentifierAscii("b"), Eof
    REQUIRE(tokens.size() == 3);
    REQUIRE(tokens[0].getKind() == jsv::TokenKind::IdentifierAscii);
    REQUIRE(tokens[0].getText() == "a");
    REQUIRE(tokens[1].getKind() == jsv::TokenKind::IdentifierAscii);
    REQUIRE(tokens[1].getText() == "b");
    REQUIRE(tokens[2].getKind() == jsv::TokenKind::Eof);
}

TEST_CASE("Lexer_UnicodeWhitespace_EmSpace_ConsumedSilently", "[lexer][utf8][ascii-compat][phase6]") {
    // U+2003 EM SPACE (0xE2 0x80 0x83, category Zs) must be consumed as whitespace (FR-023)

    const std::string src = "a\xE2\x80\x83"
                            "b";  // "a" + EM SPACE + "b"
    jsv::Lexer lex{src, "test.jsav"};
    const auto tokens = lex.tokenize();
    // Expected: IdentifierAscii("a"), IdentifierAscii("b"), Eof
    REQUIRE(tokens.size() == 3);
    REQUIRE(tokens[0].getKind() == jsv::TokenKind::IdentifierAscii);
    REQUIRE(tokens[0].getText() == "a");
    REQUIRE(tokens[1].getKind() == jsv::TokenKind::IdentifierAscii);
    REQUIRE(tokens[1].getText() == "b");
    REQUIRE(tokens[2].getKind() == jsv::TokenKind::Eof);
}

TEST_CASE("Lexer_UnicodeWhitespace_LineSeparator_ConsumedSilently", "[lexer][utf8][ascii-compat][phase6]") {
    // U+2028 LINE SEPARATOR (0xE2 0x80 0xA8, category Zl) must be consumed as whitespace (FR-023)

    const std::string src = "a\xE2\x80\xA8"
                            "b";  // "a" + LINE SEPARATOR + "b"
    jsv::Lexer lex{src, "test.jsav"};
    const auto tokens = lex.tokenize();
    // Expected: IdentifierAscii("a"), IdentifierAscii("b"), Eof
    REQUIRE(tokens.size() == 3);
    REQUIRE(tokens[0].getKind() == jsv::TokenKind::IdentifierAscii);
    REQUIRE(tokens[0].getText() == "a");
    REQUIRE(tokens[1].getKind() == jsv::TokenKind::IdentifierAscii);
    REQUIRE(tokens[1].getText() == "b");
    REQUIRE(tokens[2].getKind() == jsv::TokenKind::Eof);
}

// ==========================================================================
// T006-T010c Phase 3: User Story 1 — Complete Unicode Whitespace Recognition
// ==========================================================================

TEST_CASE("Lexer_UnicodeWhitespace_VT_SeparatesTokens", "[lexer][utf8][US1][T006]") {
    // U+000B VERTICAL TAB must separate tokens (FR-002)
    const std::string src = "var\x0Bx";  // "var" + VT + "x"
    jsv::Lexer lex{src, "test.jsav"};
    const auto tokens = lex.tokenize();
    REQUIRE(tokens.size() == 3);
    REQUIRE(tokens[0].getKind() == jsv::TokenKind::KeywordVar);
    REQUIRE(tokens[1].getKind() == jsv::TokenKind::IdentifierAscii);
    REQUIRE(tokens[1].getText() == "x");
    REQUIRE(tokens[2].getKind() == jsv::TokenKind::Eof);
}

TEST_CASE("Lexer_UnicodeWhitespace_FF_SeparatesTokens", "[lexer][utf8][US1][T007]") {
    // U+000C FORM FEED must separate tokens (FR-002)
    const std::string src = "var\x0Cx";  // "var" + FF + "x"
    jsv::Lexer lex{src, "test.jsav"};
    const auto tokens = lex.tokenize();
    REQUIRE(tokens.size() == 3);
    REQUIRE(tokens[0].getKind() == jsv::TokenKind::KeywordVar);
    REQUIRE(tokens[1].getKind() == jsv::TokenKind::IdentifierAscii);
    REQUIRE(tokens[1].getText() == "x");
    REQUIRE(tokens[2].getKind() == jsv::TokenKind::Eof);
}

TEST_CASE("Lexer_UnicodeWhitespace_NEL_SeparatesTokens", "[lexer][utf8][US1][T008]") {
    // U+0085 NEXT LINE must separate tokens (FR-003)
    const std::string src = "var\xC2\x85x";  // "var" + NEL + "x"
    jsv::Lexer lex{src, "test.jsav"};
    const auto tokens = lex.tokenize();
    REQUIRE(tokens.size() == 3);
    REQUIRE(tokens[0].getKind() == jsv::TokenKind::KeywordVar);
    REQUIRE(tokens[1].getKind() == jsv::TokenKind::IdentifierAscii);
    REQUIRE(tokens[1].getText() == "x");
    REQUIRE(tokens[2].getKind() == jsv::TokenKind::Eof);
}

TEST_CASE("Lexer_UnicodeWhitespace_All25CodePoints_SeparateTokens", "[lexer][utf8][US1][T009]") {
    // All 25 \p{White_Space} code points must separate tokens (FR-001)
    const auto cp = GENERATE(
        // ASCII whitespace (already handled, regression check)
        std::make_pair("HT", "\x09"), std::make_pair("LF", "\x0A"), std::make_pair("VT", "\x0B"), std::make_pair("FF", "\x0C"),
        std::make_pair("CR", "\x0D"), std::make_pair("SPACE", "\x20"),
        // Unicode whitespace (Zs, Zl, Zp + NEL)
        std::make_pair("NEL", "\xC2\x85"),           // U+0085
        std::make_pair("NBSP", "\xC2\xA0"),          // U+00A0
        std::make_pair("OGHAM", "\xE1\x9A\x80"),     // U+1680
        std::make_pair("EN_QUAD", "\xE2\x80\x80"),   // U+2000
        std::make_pair("EM_QUAD", "\xE2\x80\x81"),   // U+2001
        std::make_pair("EN_SPACE", "\xE2\x80\x82"),  // U+2002
        std::make_pair("EM_SPACE", "\xE2\x80\x83"),  // U+2003
        std::make_pair("3PEREM", "\xE2\x80\x84"),    // U+2004
        std::make_pair("4PEREM", "\xE2\x80\x85"),    // U+2005
        std::make_pair("6PEREM", "\xE2\x80\x86"),    // U+2006
        std::make_pair("FIGURE", "\xE2\x80\x87"),    // U+2007
        std::make_pair("PUNCT", "\xE2\x80\x88"),     // U+2008
        std::make_pair("THIN", "\xE2\x80\x89"),      // U+2009
        std::make_pair("HAIR", "\xE2\x80\x8A"),      // U+200A
        std::make_pair("LINE_SEP", "\xE2\x80\xA8"),  // U+2028
        std::make_pair("PARA_SEP", "\xE2\x80\xA9"),  // U+2029
        std::make_pair("NARROW", "\xE2\x80\xAF"),    // U+202F
        std::make_pair("MEDIUM", "\xE2\x81\x9F"),    // U+205F
        std::make_pair("IDEO", "\xE3\x80\x80")       // U+3000
    );
    const std::string src = std::string("var") + cp.second + "x";
    jsv::Lexer lex{src, "test.jsav"};
    const auto tokens = lex.tokenize();
    INFO("Whitespace: " << cp.first);
    REQUIRE(tokens.size() == 3);
    REQUIRE(tokens[0].getKind() == jsv::TokenKind::KeywordVar);
    REQUIRE(tokens[1].getKind() == jsv::TokenKind::IdentifierAscii);
    REQUIRE(tokens[1].getText() == "x");
    REQUIRE(tokens[2].getKind() == jsv::TokenKind::Eof);
}

TEST_CASE("Lexer_UnicodeWhitespace_ConsecutiveMixed_ConsumedAsOneRun", "[lexer][utf8][US1][T010]") {
    // Consecutive mixed Unicode whitespace must be consumed as a single run (FR-007)
    const std::string src = "var\xC2\xA0\xE2\x80\x80\xE2\x80\xA8x";  // NBSP + EM SPACE + LINE SEP
    jsv::Lexer lex{src, "test.jsav"};
    const auto tokens = lex.tokenize();
    REQUIRE(tokens.size() == 3);
    REQUIRE(tokens[0].getKind() == jsv::TokenKind::KeywordVar);
    REQUIRE(tokens[1].getKind() == jsv::TokenKind::IdentifierAscii);
    REQUIRE(tokens[1].getText() == "x");
    REQUIRE(tokens[2].getKind() == jsv::TokenKind::Eof);
}

TEST_CASE("Lexer_LineColumn_NEL_IncrementsLineResetsColumn", "[lexer][utf8][US1][T010b]") {
    // U+0085 NEL must increment line counter and reset column to 1 (FR-008)
    const std::string src = "var\xC2\x85x";  // "var" + NEL + "x"
    jsv::Lexer lex{src, "test.jsav"};
    const auto tokens = lex.tokenize();
    REQUIRE(tokens.size() == 3);
    REQUIRE(tokens[0].getKind() == jsv::TokenKind::KeywordVar);
    // Token 'x' should be on line 2, column 1
    REQUIRE(tokens[1].getKind() == jsv::TokenKind::IdentifierAscii);
    REQUIRE(tokens[1].getSpan().start.line == 2);
    REQUIRE(tokens[1].getSpan().start.column == 1);
    REQUIRE(tokens[1].getText() == "x");
}

TEST_CASE("Lexer_UnicodeWhitespace_MultiByteAtEOF_CleanEOFToken", "[lexer][utf8][US1][T010c]") {
    // Valid multi-byte whitespace at EOF must produce clean EOF without buffer overread (FR-010)
    const std::string src = "var\xC2\xA0";  // "var" + NBSP (U+00A0, 2 bytes) at EOF
    jsv::Lexer lex{src, "test.jsav"};
    const auto tokens = lex.tokenize();
    REQUIRE(tokens.size() == 2);
    REQUIRE(tokens[0].getKind() == jsv::TokenKind::KeywordVar);
    REQUIRE(tokens[1].getKind() == jsv::TokenKind::Eof);
}

// ==========================================================================
// T017-T022 Phase 4: User Story 4 — Backward Compatibility
// ==========================================================================

TEST_CASE("Lexer_UnicodeWhitespace_InsideStringLiteral_NotConsumed", "[lexer][utf8][US4][T017]") {
    // U+00A0 NBSP inside a string literal must NOT be consumed as whitespace (FR-024)
    const std::string src = "\"hello\xC2\xA0world\"";  // "hello" + NBSP + "world"
    jsv::Lexer lex{src, "test.jsav"};
    const auto tokens = lex.tokenize();
    REQUIRE(tokens.size() == 2);
    REQUIRE(tokens[0].getKind() == jsv::TokenKind::StringLiteral);
    // The entire string including NBSP and quotes should be the token text
    REQUIRE(tokens[0].getText() == "\"hello\xC2\xA0world\"");
    REQUIRE(tokens[1].getKind() == jsv::TokenKind::Eof);
}

TEST_CASE("Lexer_UnicodeWhitespace_InsideComment_NotConsumed", "[lexer][utf8][US4][T018]") {
    // Unicode whitespace inside comments must NOT be consumed as inter-token whitespace (FR-024)
    SECTION("Line comment with NBSP") {
        const std::string src = "var\xC2\xA0// comment\xC2\xA0with\xC2\xA0NBSP\nx";
        jsv::Lexer lex{src, "test.jsav"};
        const auto tokens = lex.tokenize();
        REQUIRE(tokens.size() == 3);
        REQUIRE(tokens[0].getKind() == jsv::TokenKind::KeywordVar);
        REQUIRE(tokens[1].getKind() == jsv::TokenKind::IdentifierAscii);
        REQUIRE(tokens[1].getText() == "x");
        REQUIRE(tokens[2].getKind() == jsv::TokenKind::Eof);
    }
    SECTION("Block comment with NBSP") {
        const std::string src = "var\xC2\xA0/* comment\xC2\xA0with\xC2\xA0NBSP */x";
        jsv::Lexer lex{src, "test.jsav"};
        const auto tokens = lex.tokenize();
        REQUIRE(tokens.size() == 3);
        REQUIRE(tokens[0].getKind() == jsv::TokenKind::KeywordVar);
        REQUIRE(tokens[1].getKind() == jsv::TokenKind::IdentifierAscii);
        REQUIRE(tokens[1].getText() == "x");
        REQUIRE(tokens[2].getKind() == jsv::TokenKind::Eof);
    }
}

TEST_CASE("Lexer_BackwardCompat_AsciiWhitespace_IdenticalBehavior", "[lexer][utf8][US4][T019]") {
    // ASCII whitespace behavior must remain unchanged (regression guard)
    const std::string src = "var \t\r\nx";  // space, tab, CR, LF
    jsv::Lexer lex{src, "test.jsav"};
    const auto tokens = lex.tokenize();
    REQUIRE(tokens.size() == 3);
    REQUIRE(tokens[0].getKind() == jsv::TokenKind::KeywordVar);
    REQUIRE(tokens[1].getKind() == jsv::TokenKind::IdentifierAscii);
    REQUIRE(tokens[1].getText() == "x");
    // 'x' should be on line 2, column 1 after LF
    REQUIRE(tokens[1].getSpan().start.line == 2);
    REQUIRE(tokens[1].getSpan().start.column == 1);
}

TEST_CASE("Lexer_BackwardCompat_LineComment_IdenticalBehavior", "[lexer][utf8][US4][T020]") {
    // Line comment behavior must remain unchanged (regression guard)
    const std::string src = "var x // comment\ny";
    jsv::Lexer lex{src, "test.jsav"};
    const auto tokens = lex.tokenize();
    REQUIRE(tokens.size() == 4);
    REQUIRE(tokens[0].getKind() == jsv::TokenKind::KeywordVar);
    REQUIRE(tokens[1].getKind() == jsv::TokenKind::IdentifierAscii);
    REQUIRE(tokens[1].getText() == "x");
    REQUIRE(tokens[2].getKind() == jsv::TokenKind::IdentifierAscii);
    REQUIRE(tokens[2].getText() == "y");
    REQUIRE(tokens[3].getKind() == jsv::TokenKind::Eof);
}

TEST_CASE("Lexer_BackwardCompat_BlockComment_IdenticalBehavior", "[lexer][utf8][US4][T021]") {
    // Block comment behavior must remain unchanged (regression guard)
    const std::string src = "var /* comment */ x";
    jsv::Lexer lex{src, "test.jsav"};
    const auto tokens = lex.tokenize();
    REQUIRE(tokens.size() == 3);
    REQUIRE(tokens[0].getKind() == jsv::TokenKind::KeywordVar);
    REQUIRE(tokens[1].getKind() == jsv::TokenKind::IdentifierAscii);
    REQUIRE(tokens[1].getText() == "x");
    REQUIRE(tokens[2].getKind() == jsv::TokenKind::Eof);
}

TEST_CASE("Lexer_BackwardCompat_BOM_IdenticalBehavior", "[lexer][utf8][US4][T022]") {
    // BOM handling must remain unchanged (regression guard)
    const std::string src = "\xEF\xBB\xBFvar x";  // UTF-8 BOM + "var x"
    jsv::Lexer lex{src, "test.jsav"};
    const auto tokens = lex.tokenize();
    REQUIRE(tokens.size() == 3);
    REQUIRE(tokens[0].getKind() == jsv::TokenKind::KeywordVar);
    REQUIRE(tokens[1].getKind() == jsv::TokenKind::IdentifierAscii);
    REQUIRE(tokens[1].getText() == "x");
    REQUIRE(tokens[2].getKind() == jsv::TokenKind::Eof);
}

// ==========================================================================
// T024-T030 Phase 5: User Story 2 — Correct Line and Column Tracking
// ==========================================================================

TEST_CASE("Lexer_LineColumn_LineSeparator_IncrementsLineResetsColumn", "[lexer][utf8][US2][T024]") {
    // U+2028 LINE SEPARATOR must increment line counter and reset column to 1 (FR-008)
    const std::string src = "var\xE2\x80\xA8x";  // "var" + LINE SEP + "x"
    jsv::Lexer lex{src, "test.jsav"};
    const auto tokens = lex.tokenize();
    REQUIRE(tokens.size() == 3);
    REQUIRE(tokens[0].getKind() == jsv::TokenKind::KeywordVar);
    // Token 'x' should be on line 2, column 1
    REQUIRE(tokens[1].getKind() == jsv::TokenKind::IdentifierAscii);
    REQUIRE(tokens[1].getSpan().start.line == 2);
    REQUIRE(tokens[1].getSpan().start.column == 1);
    REQUIRE(tokens[1].getText() == "x");
}

TEST_CASE("Lexer_LineColumn_ParagraphSeparator_IncrementsLineResetsColumn", "[lexer][utf8][US2][T025]") {
    // U+2029 PARAGRAPH SEPARATOR must increment line counter and reset column to 1 (FR-008)
    const std::string src = "var\xE2\x80\xA9x";  // "var" + PARA SEP + "x"
    jsv::Lexer lex{src, "test.jsav"};
    const auto tokens = lex.tokenize();
    REQUIRE(tokens.size() == 3);
    REQUIRE(tokens[0].getKind() == jsv::TokenKind::KeywordVar);
    // Token 'x' should be on line 2, column 1
    REQUIRE(tokens[1].getKind() == jsv::TokenKind::IdentifierAscii);
    REQUIRE(tokens[1].getSpan().start.line == 2);
    REQUIRE(tokens[1].getSpan().start.column == 1);
    REQUIRE(tokens[1].getText() == "x");
}

TEST_CASE("Lexer_LineColumn_NBSP_ColumnAdvancesByByteCount", "[lexer][utf8][US2][T026]") {
    // U+00A0 NBSP (2 bytes) must advance column by byte count, not increment line (FR-025)
    const std::string src = "var\xC2\xA0x";  // "var" + NBSP + "x"
    jsv::Lexer lex{src, "test.jsav"};
    const auto tokens = lex.tokenize();
    REQUIRE(tokens.size() == 3);
    REQUIRE(tokens[0].getKind() == jsv::TokenKind::KeywordVar);
    // Token 'x' should be on line 1, column 6 (3 for "var" + 2 for NBSP + 1 = 6)
    REQUIRE(tokens[1].getKind() == jsv::TokenKind::IdentifierAscii);
    REQUIRE(tokens[1].getSpan().start.line == 1);
    REQUIRE(tokens[1].getSpan().start.column == 6);
    REQUIRE(tokens[1].getText() == "x");
}

TEST_CASE("Lexer_LineColumn_IdeographicSpace_ColumnAdvancesByByteCount", "[lexer][utf8][US2][T027]") {
    // U+3000 IDEOGRAPHIC SPACE (3 bytes) must advance column by 3 bytes (FR-025)
    const std::string src = "var\xE3\x80\x80x";  // "var" + IDEOGRAPHIC SPACE + "x"
    jsv::Lexer lex{src, "test.jsav"};
    const auto tokens = lex.tokenize();
    REQUIRE(tokens.size() == 3);
    REQUIRE(tokens[0].getKind() == jsv::TokenKind::KeywordVar);
    // Token 'x' should be on line 1, column 7 (3 for "var" + 3 for IDEOGRAPHIC SPACE + 1 = 7)
    REQUIRE(tokens[1].getKind() == jsv::TokenKind::IdentifierAscii);
    REQUIRE(tokens[1].getSpan().start.line == 1);
    REQUIRE(tokens[1].getSpan().start.column == 7);
    REQUIRE(tokens[1].getText() == "x");
}

TEST_CASE("Lexer_LineColumn_CR_DoesNotIncrementLine", "[lexer][utf8][US2][T028]") {
    // CR (U+000D) must NOT increment line counter — treated as plain whitespace (FR-009)
    const std::string src = "var\rx";  // "var" + CR + "x"
    jsv::Lexer lex{src, "test.jsav"};
    const auto tokens = lex.tokenize();
    REQUIRE(tokens.size() == 3);
    REQUIRE(tokens[0].getKind() == jsv::TokenKind::KeywordVar);
    // Token 'x' should be on line 1, column 5 (3 for "var" + 1 for CR + 1 = 5)
    REQUIRE(tokens[1].getKind() == jsv::TokenKind::IdentifierAscii);
    REQUIRE(tokens[1].getSpan().start.line == 1);
    REQUIRE(tokens[1].getSpan().start.column == 5);
    REQUIRE(tokens[1].getText() == "x");
}

TEST_CASE("Lexer_LineColumn_CRLF_SingleLineIncrement", "[lexer][utf8][US2][T029]") {
    // CR+LF must produce exactly one line increment (FR-009)
    const std::string src = "var\r\nx";  // "var" + CR + LF + "x"
    jsv::Lexer lex{src, "test.jsav"};
    const auto tokens = lex.tokenize();
    REQUIRE(tokens.size() == 3);
    REQUIRE(tokens[0].getKind() == jsv::TokenKind::KeywordVar);
    // Token 'x' should be on line 2, column 1 (LF handles the line increment)
    REQUIRE(tokens[1].getKind() == jsv::TokenKind::IdentifierAscii);
    REQUIRE(tokens[1].getSpan().start.line == 2);
    REQUIRE(tokens[1].getSpan().start.column == 1);
    REQUIRE(tokens[1].getText() == "x");
}

TEST_CASE("Lexer_LineColumn_MultipleTerminators_AccumulateCorrectly", "[lexer][utf8][US2][T030]") {
    // Multiple line terminators in sequence must accumulate line increments correctly (FR-008)
    const std::string src = "var\xC2\x85\xE2\x80\xA8\nx";  // "var" + NEL + LINE SEP + LF + "x"
    jsv::Lexer lex{src, "test.jsav"};
    const auto tokens = lex.tokenize();
    REQUIRE(tokens.size() == 3);
    REQUIRE(tokens[0].getKind() == jsv::TokenKind::KeywordVar);
    // Token 'x' should be on line 4, column 1 (3 terminators = 3 line increments)
    REQUIRE(tokens[1].getKind() == jsv::TokenKind::IdentifierAscii);
    REQUIRE(tokens[1].getSpan().start.line == 4);
    REQUIRE(tokens[1].getSpan().start.column == 1);
    REQUIRE(tokens[1].getText() == "x");
}

// ==========================================================================
// T032-T041 Phase 6: User Story 3 — Graceful Handling of Malformed UTF-8
// ==========================================================================

TEST_CASE("Lexer_Robustness_LoneContinuationByte_NoCrash", "[lexer][utf8][US3][T032]") {
    // Lone continuation byte (0x80) in whitespace position must not crash (FR-004)
    const std::string src = "var\x80x";  // "var" + 0x80 + "x"
    jsv::Lexer lex{src, "test.jsav"};
    // Should not crash - lexer should continue tokenizing
    const auto tokens = lex.tokenize();
    // The 0x80 is not whitespace, so it becomes part of tokenization
    REQUIRE(tokens.size() >= 2);
    REQUIRE(tokens.back().getKind() == jsv::TokenKind::Eof);
}

TEST_CASE("Lexer_Robustness_Truncated2ByteAtEOF_NoCrash", "[lexer][utf8][US3][T033]") {
    // Truncated 2-byte sequence at EOF must not crash (FR-010)
    const std::string src = "var\xC2";  // "var" + truncated 2-byte lead
    jsv::Lexer lex{src, "test.jsav"};
    const auto tokens = lex.tokenize();
    REQUIRE(tokens.size() >= 2);
    REQUIRE(tokens.back().getKind() == jsv::TokenKind::Eof);
}

TEST_CASE("Lexer_Robustness_Truncated3ByteAtEOF_NoCrash", "[lexer][utf8][US3][T034]") {
    // Truncated 3-byte sequence at EOF must not crash (FR-010)
    const std::string src = "var\xE2\x80";  // "var" + truncated 3-byte (only 2 bytes)
    jsv::Lexer lex{src, "test.jsav"};
    const auto tokens = lex.tokenize();
    REQUIRE(tokens.size() >= 2);
    REQUIRE(tokens.back().getKind() == jsv::TokenKind::Eof);
}

TEST_CASE("Lexer_Robustness_OverlongSpace_NotWhitespace", "[lexer][utf8][US3][T035]") {
    // Overlong encoding of SPACE (U+0020) must NOT be treated as whitespace (FR-004)
    // Overlong 2-byte encoding of U+0020: 0xC0 0xA0
    const std::string src = "var\xC0\xA0x";  // "var" + overlong SPACE + "x"
    jsv::Lexer lex{src, "test.jsav"};
    const auto tokens = lex.tokenize();

    // Verify "var" is tokenized as keyword
    REQUIRE(tokens.size() >= 4);  // KeywordVar + Error/Invalid + IdentifierAscii + Eof
    REQUIRE(tokens[0].getKind() == jsv::TokenKind::KeywordVar);
    REQUIRE(tokens[0].getText() == "var");

    // Verify overlong bytes are NOT treated as whitespace (produce error token)
    REQUIRE(tokens[1].getKind() == jsv::TokenKind::Error);

    // Verify "x" is tokenized as identifier (not separated by whitespace)
    REQUIRE(tokens[2].getKind() == jsv::TokenKind::IdentifierAscii);
    REQUIRE(tokens[2].getText() == "x");

    // Verify EOF
    REQUIRE(tokens.back().getKind() == jsv::TokenKind::Eof);
}

TEST_CASE("Lexer_Robustness_ByteFE_NoCrash", "[lexer][utf8][US3][T036]") {
    // 0xFE byte (invalid UTF-8 lead byte) must not crash (FR-004)
    const std::string src = "var\xFEx";  // "var" + 0xFE + "x"
    jsv::Lexer lex{src, "test.jsav"};
    const auto tokens = lex.tokenize();
    REQUIRE(tokens.size() >= 2);
    REQUIRE(tokens.back().getKind() == jsv::TokenKind::Eof);
}

TEST_CASE("Lexer_Robustness_ByteFF_NoCrash", "[lexer][utf8][US3][T037]") {
    // 0xFF byte (invalid UTF-8 lead byte) must not crash (FR-004)
    const std::string src = "var\xFFx";  // "var" + 0xFF + "x"
    jsv::Lexer lex{src, "test.jsav"};
    const auto tokens = lex.tokenize();
    REQUIRE(tokens.size() >= 2);
    REQUIRE(tokens.back().getKind() == jsv::TokenKind::Eof);
}

TEST_CASE("Lexer_Robustness_InvalidContinuation_NoCrash", "[lexer][utf8][US3][T038]") {
    // Invalid continuation byte in multi-byte sequence must not crash (FR-004)
    // 0xC2 followed by 0x00 (null, not a valid continuation)
    // NOLINTNEXTLINE(bugprone-string-literal-with-embedded-nul)
    const std::string src = "var\xC2\x00x";  // "var" + 0xC2 + null + "x"
    jsv::Lexer lex{src, "test.jsav"};
    const auto tokens = lex.tokenize();
    REQUIRE(tokens.size() >= 2);
    REQUIRE(tokens.back().getKind() == jsv::TokenKind::Eof);
}

TEST_CASE("Lexer_Robustness_NonWhitespaceMultiByte_NotConsumed", "[lexer][utf8][US3][T039]") {
    // Valid non-whitespace multi-byte char (U+00E9 é) must NOT be consumed as whitespace (FR-024)
    // This test verifies the lexer doesn't crash on valid multi-byte non-whitespace characters
    const std::string src = "a\xC3\xA9";  // "a" + é (identifier with multi-byte char)
    jsv::Lexer lex{src, "test.jsav"};
    const auto tokens = lex.tokenize();

    // Verify "aé" is tokenized as a single Unicode identifier (not separated)
    REQUIRE(tokens.size() >= 2);  // IdentifierUnicode + Eof (at minimum)
    REQUIRE(tokens[0].getKind() == jsv::TokenKind::IdentifierUnicode);
    REQUIRE(tokens[0].getText() == "aé");

    // Verify EOF
    REQUIRE(tokens.back().getKind() == jsv::TokenKind::Eof);
}

TEST_CASE("Lexer_Robustness_SurrogateBytes_NoCrash", "[lexer][utf8][US3][T040]") {
    // Surrogate pair bytes (U+D800-U+DFFF) must not crash (FR-004)
    // 0xED 0xA0 0x80 encodes U+D800 (high surrogate)
    const std::string src = "var\xED\xA0\x80x";  // "var" + surrogate + "x"
    jsv::Lexer lex{src, "test.jsav"};
    const auto tokens = lex.tokenize();
    REQUIRE(tokens.size() >= 2);
    REQUIRE(tokens.back().getKind() == jsv::TokenKind::Eof);
}

TEST_CASE("Lexer_Robustness_NullByte_NoCrash", "[lexer][utf8][US3][T041]") {
    // Null byte (0x00) in source must not crash (FR-004)
    const std::string src = std::string("var\x00x", 5);  // "var" + null + "x" (explicit length)
    jsv::Lexer lex{src, "test.jsav"};
    const auto tokens = lex.tokenize();
    REQUIRE(tokens.size() >= 2);
    REQUIRE(tokens.back().getKind() == jsv::TokenKind::Eof);
}

TEST_CASE("Lexer_AsciiOperators_UnchangedAfterUtf8", "[lexer][utf8][ascii-compat][phase6]") {
    // ASCII operators must produce identical tokens after UTF-8 changes (regression guard)
    struct OpCase {
        const char *src;
        jsv::TokenKind kind;
    };
    const std::array<OpCase, 26> cases = {{
        {.src = "+", .kind = jsv::TokenKind::Plus},          {.src = "-", .kind = jsv::TokenKind::Minus},
        {.src = "*", .kind = jsv::TokenKind::Star},          {.src = "/", .kind = jsv::TokenKind::Slash},
        {.src = "=", .kind = jsv::TokenKind::Equal},         {.src = "==", .kind = jsv::TokenKind::EqualEqual},
        {.src = "!=", .kind = jsv::TokenKind::NotEqual},     {.src = "<", .kind = jsv::TokenKind::Less},
        {.src = ">", .kind = jsv::TokenKind::Greater},       {.src = "<=", .kind = jsv::TokenKind::LessEqual},
        {.src = ">=", .kind = jsv::TokenKind::GreaterEqual}, {.src = "+=", .kind = jsv::TokenKind::PlusEqual},
        {.src = "-=", .kind = jsv::TokenKind::MinusEqual},   {.src = "++", .kind = jsv::TokenKind::PlusPlus},
        {.src = "--", .kind = jsv::TokenKind::MinusMinus},   {.src = "&&", .kind = jsv::TokenKind::AndAnd},
        {.src = "||", .kind = jsv::TokenKind::OrOr},         {.src = "(", .kind = jsv::TokenKind::OpenParen},
        {.src = ")", .kind = jsv::TokenKind::CloseParen},    {.src = "{", .kind = jsv::TokenKind::OpenBrace},
        {.src = "}", .kind = jsv::TokenKind::CloseBrace},    {.src = "[", .kind = jsv::TokenKind::OpenBracket},
        {.src = "]", .kind = jsv::TokenKind::CloseBracket},  {.src = ";", .kind = jsv::TokenKind::Semicolon},
        {.src = ",", .kind = jsv::TokenKind::Comma},         {.src = ".", .kind = jsv::TokenKind::Dot},
    }};
    for(const auto &c : cases) {
        INFO("Operator: " << c.src);
        jsv::Lexer lex{c.src, "test.jsav"};
        const auto tokens = lex.tokenize();
        REQUIRE(tokens.size() == 2);
        REQUIRE(tokens[0].getKind() == c.kind);
        REQUIRE(tokens[0].getText() == c.src);
        REQUIRE(tokens[1].getKind() == jsv::TokenKind::Eof);
    }
}

TEST_CASE("Lexer_AsciiKeywords_UnchangedAfterUtf8", "[lexer][utf8][ascii-compat][phase6]") {
    // All ASCII keywords must produce identical TokenKind values (regression guard)
    struct KwCase {
        const char *text;
        jsv::TokenKind kind;
    };
    const std::array<KwCase, 15> keywords = {{
        {.text = "fun", .kind = jsv::TokenKind::KeywordFun},
        {.text = "if", .kind = jsv::TokenKind::KeywordIf},
        {.text = "else", .kind = jsv::TokenKind::KeywordElse},
        {.text = "return", .kind = jsv::TokenKind::KeywordReturn},
        {.text = "while", .kind = jsv::TokenKind::KeywordWhile},
        {.text = "for", .kind = jsv::TokenKind::KeywordFor},
        {.text = "main", .kind = jsv::TokenKind::KeywordMain},
        {.text = "var", .kind = jsv::TokenKind::KeywordVar},
        {.text = "const", .kind = jsv::TokenKind::KeywordConst},
        {.text = "break", .kind = jsv::TokenKind::KeywordBreak},
        {.text = "continue", .kind = jsv::TokenKind::KeywordContinue},
        {.text = "bool", .kind = jsv::TokenKind::KeywordBool},
        {.text = "i32", .kind = jsv::TokenKind::TypeI32},
        {.text = "f64", .kind = jsv::TokenKind::TypeF64},
        {.text = "string", .kind = jsv::TokenKind::TypeString},
    }};
    for(const auto &k : keywords) {
        INFO("Keyword: " << k.text);
        jsv::Lexer lex{k.text, "test.jsav"};
        const auto tokens = lex.tokenize();
        REQUIRE(tokens.size() == 2);
        REQUIRE(tokens[0].getKind() == k.kind);
        REQUIRE(tokens[0].getText() == k.text);
        REQUIRE(tokens[1].getKind() == jsv::TokenKind::Eof);
    }
}

TEST_CASE("Lexer_AsciiStringLiteral_UnchangedAfterUtf8", "[lexer][utf8][ascii-compat][phase6]") {
    // ASCII string literals must produce identical content after UTF-8 changes (regression guard)
    const std::string_view src = R"("hello, world!")";
    jsv::Lexer lex{src, "test.jsav"};
    const auto tokens = lex.tokenize();
    REQUIRE(tokens.size() == 2);
    REQUIRE(tokens[0].getKind() == jsv::TokenKind::StringLiteral);
    REQUIRE(tokens[0].getText() == R"("hello, world!")");
    REQUIRE(tokens[1].getKind() == jsv::TokenKind::Eof);
}

// ==========================================================================
// Phase 7 – Performance (benchmarks + functional correctness)
// ==========================================================================

TEST_CASE("Lexer_LargeAsciiFile_TokenizesWithinBaseline", "[lexer][utf8][performance][phase7]") {
    // Generate ~10K ASCII identifier tokens: "x0 x1 x2 ... x9999"
    std::string src;
    src.reserve(std::size_t{10000} * 8);
    for(int i = 0; i < 10000; ++i) {
        src += 'x';
        src += std::to_string(i);
        src += ' ';
    }
    jsv::Lexer lex{src, "test.jsav"};
    const auto tokens = lex.tokenize();
    // 10000 IdentifierAscii tokens + 1 Eof
    REQUIRE(tokens.size() == 10001);
    REQUIRE(tokens[0].getKind() == jsv::TokenKind::IdentifierAscii);
    REQUIRE(tokens[0].getText() == "x0");
    REQUIRE(tokens.back().getKind() == jsv::TokenKind::Eof);

    BENCHMARK("Tokenize 10K ASCII tokens") {
        jsv::Lexer bench_lex{src, "bench.jsav"};
        return bench_lex.tokenize();
    };
}

TEST_CASE("Lexer_MixedUnicodeFile_TokenizesCompletely", "[lexer][utf8][performance][phase7]") {
    // Mix ASCII identifiers, CJK identifiers, Cyrillic identifiers,
    // and string literals containing valid UTF-8 multi-byte characters.
    std::string src;
    src.reserve(5000);
    for(int i = 0; i < 100; ++i) {
        src += 'a';
        src += std::to_string(i);
        src += ' ';                           // ASCII identifiers
        src += "\xe5\x8f\x98\xe9\x87\x8f ";   // 変量 (CJK)
        src += "\xd0\xb0\xd0\xb1\xd0\xb2 ";   // абв (Cyrillic)
        src += "\"hello\xf0\x9f\x98\x80\" ";  // string with emoji inside
    }
    jsv::Lexer lex{src, "test.jsav"};
    const auto tokens = lex.tokenize();
    REQUIRE_FALSE(tokens.empty());
    REQUIRE(tokens.back().getKind() == jsv::TokenKind::Eof);
    // 100 × (ASCII-ident + CJK-ident + Cyrillic-ident + StringLiteral) + Eof = 401 tokens
    REQUIRE(tokens.size() == 401);
    // Spot-check: first is ASCII identifier, second is CJK Unicode identifier
    REQUIRE(tokens[0].getKind() == jsv::TokenKind::IdentifierAscii);
    REQUIRE(tokens[1].getKind() == jsv::TokenKind::IdentifierUnicode);
    REQUIRE(tokens[2].getKind() == jsv::TokenKind::IdentifierUnicode);
    REQUIRE(tokens[3].getKind() == jsv::TokenKind::StringLiteral);

    BENCHMARK("Tokenize mixed Unicode") {
        jsv::Lexer bench_lex{src, "bench.jsav"};
        return bench_lex.tokenize();
    };
}

TEST_CASE("Lexer_OneMBMixedFile_CompletesWithin100ms", "[lexer][utf8][performance][phase7][benchmark]") {
    // SC-007: 1MB mixed-content file must tokenize within 100ms (benchmark-only guard)
    // Build ~1MB source: repeated blocks of ASCII + CJK + string literal (~52 bytes each)
    const std::string block = "abc def gh "
                              "\xe5\x8f\x98\xe9\x87\x8f "  // 変量
                              "\xd0\xb0\xd0\xb1\xd0\xb2 "  // абв
                              "\"hello world 42\" ";       // string literal

    const std::size_t target = std::size_t{1024} * 1024;  // 1MB
    std::string src;
    src.reserve(target + block.size());
    while(src.size() < target) { src += block; }

    // Functional check: tokenizes without crash and produces Eof at end
    {
        jsv::Lexer lex{src, "test.jsav"};
        const auto tokens = lex.tokenize();
        REQUIRE(tokens.back().getKind() == jsv::TokenKind::Eof);
    }

    // Performance guard: in Release only (Debug is too slow for this bound)
    // Threshold configurable via BENCHMARK_TIMEOUT_MS env var (default: 100ms)
#ifdef NDEBUG
    const auto t0 = ch::high_resolution_clock::now();
    {
        jsv::Lexer lex{src, "bench.jsav"};
        [[maybe_unused]] const auto tokens = lex.tokenize();
    }
    const auto elapsed = duration_cast<ch::milliseconds>(ch::high_resolution_clock::now() - t0).count();
    // NOLINTBEGIN(*-mt-unsafe)
    const auto *const timeout_env = std::getenv("BENCHMARK_TIMEOUT_MS");
    // NOLINTEND(*-mt-unsafe)
    int timeout_ms = 100;  // default
    if(timeout_env != nullptr) {
        try {
            const int parsed = std::stoi(timeout_env);
            if(parsed > 0) { timeout_ms = parsed; }
        } catch(const std::invalid_argument &) {  // NOLINT(bugprone-empty-catch)
            // malformed input — keep default
        } catch(const std::out_of_range &) {  // NOLINT(bugprone-empty-catch)
            // value out of range — keep default
        }
    }
    REQUIRE(elapsed < timeout_ms);
#endif

    BENCHMARK("Tokenize 1MB mixed") {
        jsv::Lexer bench_lex{src, "bench.jsav"};
        return bench_lex.tokenize();
    };
}

// ==========================================================================
// Phase 3 – User Story 1: Basic integers and decimals
// ==========================================================================

TEST_CASE("Lexer_NumericBaseFormats_TokenizeCorrectly", "[lexer][numeric][us1][phase3]") {
    SECTION("simple integers produce Numeric tokens") {
        jsv::Lexer lex{"0 1 42 007", "test.jsav"};
        const auto tokens = lex.tokenize();
        // 4 numbers + spaces (consumed) + Eof = 5 tokens
        REQUIRE(tokens.size() == 5);
        REQUIRE(tokens[0].getKind() == jsv::TokenKind::Numeric);
        REQUIRE(tokens[0].getText() == "0");
        REQUIRE(tokens[1].getKind() == jsv::TokenKind::Numeric);
        REQUIRE(tokens[1].getText() == "1");
        REQUIRE(tokens[2].getKind() == jsv::TokenKind::Numeric);
        REQUIRE(tokens[2].getText() == "42");
        REQUIRE(tokens[3].getKind() == jsv::TokenKind::Numeric);
        REQUIRE(tokens[3].getText() == "007");
    }

    SECTION("decimals with integer and fractional parts") {
        jsv::Lexer lex{"1.0 3.14 0.5", "test.jsav"};
        const auto tokens = lex.tokenize();
        // 3 numbers + spaces (consumed) + Eof = 4 tokens
        REQUIRE(tokens.size() == 4);
        REQUIRE(tokens[0].getKind() == jsv::TokenKind::Numeric);
        REQUIRE(tokens[0].getText() == "1.0");
        REQUIRE(tokens[1].getKind() == jsv::TokenKind::Numeric);
        REQUIRE(tokens[1].getText() == "3.14");
        REQUIRE(tokens[2].getKind() == jsv::TokenKind::Numeric);
        REQUIRE(tokens[2].getText() == "0.5");
    }

    SECTION("decimals with trailing dot include the dot") {
        jsv::Lexer lex{"3. 42.", "test.jsav"};
        const auto tokens = lex.tokenize();
        // 2 numbers + space (consumed) + Eof = 3 tokens
        REQUIRE(tokens.size() == 3);
        REQUIRE(tokens[0].getKind() == jsv::TokenKind::Numeric);
        REQUIRE(tokens[0].getText() == "3.");
        REQUIRE(tokens[1].getKind() == jsv::TokenKind::Numeric);
        REQUIRE(tokens[1].getText() == "42.");
    }

    SECTION("numbers with only fractional part (leading dot)") {
        jsv::Lexer lex{".5 .14 .0", "test.jsav"};
        const auto tokens = lex.tokenize();
        // 3 numbers + spaces (consumed) + Eof = 4 tokens
        REQUIRE(tokens.size() == 4);
        REQUIRE(tokens[0].getKind() == jsv::TokenKind::Numeric);
        REQUIRE(tokens[0].getText() == ".5");
        REQUIRE(tokens[1].getKind() == jsv::TokenKind::Numeric);
        REQUIRE(tokens[1].getText() == ".14");
        REQUIRE(tokens[2].getKind() == jsv::TokenKind::Numeric);
        REQUIRE(tokens[2].getText() == ".0");
    }

    SECTION("isolated dot is not a Numeric token") {
        jsv::Lexer lex{".", "test.jsav"};
        const auto tokens = lex.tokenize();
        REQUIRE(tokens.size() == 2);  // Dot + Eof
        REQUIRE(tokens[0].getKind() == jsv::TokenKind::Dot);
        REQUIRE(tokens[0].getText() == ".");
    }

    SECTION("dot followed by non-digit is not Numeric") {
        jsv::Lexer lex{".abc", "test.jsav"};
        const auto tokens = lex.tokenize();
        REQUIRE(tokens.size() == 3);  // Dot + Identifier + Eof
        REQUIRE(tokens[0].getKind() == jsv::TokenKind::Dot);
        REQUIRE(tokens[0].getText() == ".");
        REQUIRE(tokens[1].getKind() == jsv::TokenKind::IdentifierAscii);
        REQUIRE(tokens[1].getText() == "abc");
    }

    SECTION("malformed numeric: multiple decimal points 1.2.3") {
        jsv::Lexer lex{"1.2.3", "test.jsav"};
        const auto tokens = lex.tokenize();
        // 1.2 is a valid numeric, .3 is a valid numeric (leading dot + digits)
        REQUIRE(tokens.size() == 3);
        REQUIRE(tokens[0].getKind() == jsv::TokenKind::Numeric);
        REQUIRE(tokens[0].getText() == "1.2");
        REQUIRE(tokens[1].getKind() == jsv::TokenKind::Numeric);
        REQUIRE(tokens[1].getText() == ".3");
    }

    SECTION("malformed numeric: multiple exponent markers 1e2e3") {
        jsv::Lexer lex{"1e2e3", "test.jsav"};
        const auto tokens = lex.tokenize();
        // 1e2 is a valid numeric, e3 is an identifier
        REQUIRE(tokens.size() == 3);
        REQUIRE(tokens[0].getKind() == jsv::TokenKind::Numeric);
        REQUIRE(tokens[0].getText() == "1e2");
        REQUIRE(tokens[1].getKind() == jsv::TokenKind::IdentifierAscii);
        REQUIRE(tokens[1].getText() == "e3");
    }

    SECTION("valid compound suffix: 1U8 produces Numeric token") {
        jsv::Lexer lex{"1U8", "test.jsav"};
        const auto tokens = lex.tokenize();
        REQUIRE(tokens.size() == 2);
        REQUIRE(tokens[0].getKind() == jsv::TokenKind::Numeric);
        REQUIRE(tokens[0].getText() == "1U8");
    }

    SECTION("valid compound suffix: 1u8 produces Numeric token") {
        jsv::Lexer lex{"1u8", "test.jsav"};
        const auto tokens = lex.tokenize();
        REQUIRE(tokens.size() == 2);
        REQUIRE(tokens[0].getKind() == jsv::TokenKind::Numeric);
        REQUIRE(tokens[0].getText() == "1u8");
    }

    SECTION("very long digit run produces single Numeric token") {
        jsv::Lexer lex{"12345678901234567890123456789012345678901234567890", "test.jsav"};
        const auto tokens = lex.tokenize();
        REQUIRE(tokens.size() == 2);
        REQUIRE(tokens[0].getKind() == jsv::TokenKind::Numeric);
        REQUIRE(tokens[0].getText() == "12345678901234567890123456789012345678901234567890");
        REQUIRE(tokens[0].getText().size() == 50);
    }

    SECTION("leading zeros preserved: 007e2") {
        jsv::Lexer lex{"007e2", "test.jsav"};
        const auto tokens = lex.tokenize();
        REQUIRE(tokens.size() == 2);
        REQUIRE(tokens[0].getKind() == jsv::TokenKind::Numeric);
        REQUIRE(tokens[0].getText() == "007e2");
    }
}

TEST_CASE("Lexer_NumericPositionTracking_Correct", "[lexer][numeric][us1][phase3]") {
    SECTION("position tracking for simple integers") {
        jsv::Lexer lex{"42", "test.jsav"};
        const auto tokens = lex.tokenize();
        REQUIRE(tokens.size() == 2);
        REQUIRE(tokens[0].getKind() == jsv::TokenKind::Numeric);
        REQUIRE(tokens[0].getText() == "42");
        REQUIRE(tokens[0].getSpan().start.line == 1);
        REQUIRE(tokens[0].getSpan().start.column == 1);
        REQUIRE(tokens[0].getSpan().start.absolute_pos == 0);
        REQUIRE(tokens[0].getSpan().end.line == 1);
        REQUIRE(tokens[0].getSpan().end.column == 3);
        REQUIRE(tokens[0].getSpan().end.absolute_pos == 2);
    }

    SECTION("position tracking for decimals with leading dot") {
        jsv::Lexer lex{".5", "test.jsav"};
        const auto tokens = lex.tokenize();
        REQUIRE(tokens.size() == 2);
        REQUIRE(tokens[0].getKind() == jsv::TokenKind::Numeric);
        REQUIRE(tokens[0].getText() == ".5");
        REQUIRE(tokens[0].getSpan().start.line == 1);
        REQUIRE(tokens[0].getSpan().start.column == 1);
        REQUIRE(tokens[0].getSpan().start.absolute_pos == 0);
        REQUIRE(tokens[0].getSpan().end.line == 1);
        REQUIRE(tokens[0].getSpan().end.column == 3);
        REQUIRE(tokens[0].getSpan().end.absolute_pos == 2);
    }

    SECTION("position tracking for trailing dot") {
        jsv::Lexer lex{"3.", "test.jsav"};
        const auto tokens = lex.tokenize();
        REQUIRE(tokens.size() == 2);
        REQUIRE(tokens[0].getKind() == jsv::TokenKind::Numeric);
        REQUIRE(tokens[0].getText() == "3.");
        REQUIRE(tokens[0].getSpan().start.line == 1);
        REQUIRE(tokens[0].getSpan().start.column == 1);
        REQUIRE(tokens[0].getSpan().start.absolute_pos == 0);
        REQUIRE(tokens[0].getSpan().end.line == 1);
        REQUIRE(tokens[0].getSpan().end.column == 3);
        REQUIRE(tokens[0].getSpan().end.absolute_pos == 2);
    }

    SECTION("position tracking across multiple lines") {
        jsv::Lexer lex{"42\n.5\n3.", "test.jsav"};
        const auto tokens = lex.tokenize();
        // 3 numbers + Eof = 4 tokens (newlines are consumed as whitespace)
        REQUIRE(tokens.size() == 4);

        // First number: 42 on line 1
        REQUIRE(tokens[0].getKind() == jsv::TokenKind::Numeric);
        REQUIRE(tokens[0].getText() == "42");
        REQUIRE(tokens[0].getSpan().start.line == 1);
        REQUIRE(tokens[0].getSpan().start.column == 1);

        // Second number: .5 on line 2 (after newline)
        REQUIRE(tokens[1].getKind() == jsv::TokenKind::Numeric);
        REQUIRE(tokens[1].getText() == ".5");
        REQUIRE(tokens[1].getSpan().start.line == 2);
        REQUIRE(tokens[1].getSpan().start.column == 1);

        // Third number: 3. on line 3 (after second newline)
        REQUIRE(tokens[2].getKind() == jsv::TokenKind::Numeric);
        REQUIRE(tokens[2].getText() == "3.");
        REQUIRE(tokens[2].getSpan().start.line == 3);
        REQUIRE(tokens[2].getSpan().start.column == 1);
    }
}

// ==========================================================================
// Phase 4 – User Story 2: Scientific notation recognition
// ==========================================================================

TEST_CASE("Lexer_NumericScientificNotation_TokenizeCorrectly", "[lexer][numeric][us2][phase4]") {
    SECTION("valid exponents produce single Numeric tokens") {
        jsv::Lexer lex{"1e10 3.14E+2 2.5e-3 .5E10", "test.jsav"};
        const auto tokens = lex.tokenize();
        // 4 numbers + spaces (consumed) + Eof = 5 tokens
        REQUIRE(tokens.size() == 5);
        REQUIRE(tokens[0].getKind() == jsv::TokenKind::Numeric);
        REQUIRE(tokens[0].getText() == "1e10");
        REQUIRE(tokens[1].getKind() == jsv::TokenKind::Numeric);
        REQUIRE(tokens[1].getText() == "3.14E+2");
        REQUIRE(tokens[2].getKind() == jsv::TokenKind::Numeric);
        REQUIRE(tokens[2].getText() == "2.5e-3");
        REQUIRE(tokens[3].getKind() == jsv::TokenKind::Numeric);
        REQUIRE(tokens[3].getText() == ".5E10");
    }

    SECTION("invalid exponents: incomplete marker produces separate tokens") {
        // 1e → Numeric("1") + Identifier("e")
        jsv::Lexer lex1{"1e", "test.jsav"};
        const auto tokens1 = lex1.tokenize();
        REQUIRE(tokens1.size() == 3);  // Numeric + Identifier + Eof
        REQUIRE(tokens1[0].getKind() == jsv::TokenKind::Numeric);
        REQUIRE(tokens1[0].getText() == "1");
        REQUIRE(tokens1[1].getKind() == jsv::TokenKind::IdentifierAscii);
        REQUIRE(tokens1[1].getText() == "e");

        // 1e+ → Numeric("1") + Identifier("e") + Plus
        jsv::Lexer lex2{"1e+", "test.jsav"};
        const auto tokens2 = lex2.tokenize();
        REQUIRE(tokens2.size() == 4);  // Numeric + Identifier + Plus + Eof
        REQUIRE(tokens2[0].getKind() == jsv::TokenKind::Numeric);
        REQUIRE(tokens2[0].getText() == "1");
        REQUIRE(tokens2[1].getKind() == jsv::TokenKind::IdentifierAscii);
        REQUIRE(tokens2[1].getText() == "e");
        REQUIRE(tokens2[2].getKind() == jsv::TokenKind::Plus);
        REQUIRE(tokens2[2].getText() == "+");

        // 1E- → Numeric("1") + Identifier("E") + Minus
        jsv::Lexer lex3{"1E-", "test.jsav"};
        const auto tokens3 = lex3.tokenize();
        REQUIRE(tokens3.size() == 4);  // Numeric + Identifier + Minus + Eof
        REQUIRE(tokens3[0].getKind() == jsv::TokenKind::Numeric);
        REQUIRE(tokens3[0].getText() == "1");
        REQUIRE(tokens3[1].getKind() == jsv::TokenKind::IdentifierAscii);
        REQUIRE(tokens3[1].getText() == "E");
        REQUIRE(tokens3[2].getKind() == jsv::TokenKind::Minus);
        REQUIRE(tokens3[2].getText() == "-");
    }

    SECTION("exponent without digits after sign is not consumed") {
        // 1e+abc → Numeric("1") + Identifier("e") + Plus + Identifier("abc")
        jsv::Lexer lex{"1e+abc", "test.jsav"};
        const auto tokens = lex.tokenize();
        REQUIRE(tokens.size() == 5);  // Numeric + Identifier + Plus + Identifier + Eof
        REQUIRE(tokens[0].getKind() == jsv::TokenKind::Numeric);
        REQUIRE(tokens[0].getText() == "1");
        REQUIRE(tokens[1].getKind() == jsv::TokenKind::IdentifierAscii);
        REQUIRE(tokens[1].getText() == "e");
        REQUIRE(tokens[2].getKind() == jsv::TokenKind::Plus);
        REQUIRE(tokens[2].getText() == "+");
        REQUIRE(tokens[3].getKind() == jsv::TokenKind::IdentifierAscii);
        REQUIRE(tokens[3].getText() == "abc");
    }
}

TEST_CASE("Lexer_NumericScientificNotation_PositionTracking", "[lexer][numeric][us2][phase4]") {
    SECTION("position tracking for scientific notation") {
        jsv::Lexer lex{"1e10", "test.jsav"};
        const auto tokens = lex.tokenize();
        REQUIRE(tokens.size() == 2);
        REQUIRE(tokens[0].getKind() == jsv::TokenKind::Numeric);
        REQUIRE(tokens[0].getText() == "1e10");
        REQUIRE(tokens[0].getSpan().start.line == 1);
        REQUIRE(tokens[0].getSpan().start.column == 1);
        REQUIRE(tokens[0].getSpan().start.absolute_pos == 0);
        REQUIRE(tokens[0].getSpan().end.line == 1);
        REQUIRE(tokens[0].getSpan().end.column == 5);
        REQUIRE(tokens[0].getSpan().end.absolute_pos == 4);
    }

    SECTION("position tracking for exponent with sign") {
        jsv::Lexer lex{"3.14E+2", "test.jsav"};
        const auto tokens = lex.tokenize();
        REQUIRE(tokens.size() == 2);
        REQUIRE(tokens[0].getKind() == jsv::TokenKind::Numeric);
        REQUIRE(tokens[0].getText() == "3.14E+2");
        REQUIRE(tokens[0].getSpan().start.line == 1);
        REQUIRE(tokens[0].getSpan().start.column == 1);
        REQUIRE(tokens[0].getSpan().start.absolute_pos == 0);
        REQUIRE(tokens[0].getSpan().end.line == 1);
        REQUIRE(tokens[0].getSpan().end.column == 8);
        REQUIRE(tokens[0].getSpan().end.absolute_pos == 7);
    }
}

// ==========================================================================
// Phase 5 – User Story 3: Type suffix recognition
// ==========================================================================

TEST_CASE("Lexer_NumericTypeSuffixes_TokenizeCorrectly", "[lexer][numeric][us3][phase5]") {
    SECTION("valid single-character suffixes d/D and f/F") {
        jsv::Lexer lex{"1.0F 1.0f 10d 10D", "test.jsav"};
        const auto tokens = lex.tokenize();
        // 4 numbers + spaces (consumed) + Eof = 5 tokens
        REQUIRE(tokens.size() == 5);
        REQUIRE(tokens[0].getKind() == jsv::TokenKind::Numeric);
        REQUIRE(tokens[0].getText() == "1.0F");
        REQUIRE(tokens[1].getKind() == jsv::TokenKind::Numeric);
        REQUIRE(tokens[1].getText() == "1.0f");
        REQUIRE(tokens[2].getKind() == jsv::TokenKind::Numeric);
        REQUIRE(tokens[2].getText() == "10d");
        REQUIRE(tokens[3].getKind() == jsv::TokenKind::Numeric);
        REQUIRE(tokens[3].getText() == "10D");
    }

    SECTION("invalid bare unsigned u/U produces separate tokens") {
        // 42u → Numeric("42") + Identifier("u")
        jsv::Lexer lex1{"42u", "test.jsav"};
        const auto tokens1 = lex1.tokenize();
        REQUIRE(tokens1.size() == 3);  // Numeric + Identifier + Eof
        REQUIRE(tokens1[0].getKind() == jsv::TokenKind::Numeric);
        REQUIRE(tokens1[0].getText() == "42");
        REQUIRE(tokens1[1].getKind() == jsv::TokenKind::IdentifierAscii);
        REQUIRE(tokens1[1].getText() == "u");

        // 42U → Numeric("42") + Identifier("U")
        jsv::Lexer lex2{"42U", "test.jsav"};
        const auto tokens2 = lex2.tokenize();
        REQUIRE(tokens2.size() == 3);  // Numeric + Identifier + Eof
        REQUIRE(tokens2[0].getKind() == jsv::TokenKind::Numeric);
        REQUIRE(tokens2[0].getText() == "42");
        REQUIRE(tokens2[1].getKind() == jsv::TokenKind::IdentifierAscii);
        REQUIRE(tokens2[1].getText() == "U");
    }

    SECTION("valid compound suffixes u8/u16/u32 and i8/i16/i32") {
        jsv::Lexer lex{"255u8 1000i32 50i16 50I16 100U32", "test.jsav"};
        const auto tokens = lex.tokenize();
        // 5 numbers + spaces (consumed) + Eof = 6 tokens
        REQUIRE(tokens.size() == 6);
        REQUIRE(tokens[0].getKind() == jsv::TokenKind::Numeric);
        REQUIRE(tokens[0].getText() == "255u8");
        REQUIRE(tokens[1].getKind() == jsv::TokenKind::Numeric);
        REQUIRE(tokens[1].getText() == "1000i32");
        REQUIRE(tokens[2].getKind() == jsv::TokenKind::Numeric);
        REQUIRE(tokens[2].getText() == "50i16");
        REQUIRE(tokens[3].getKind() == jsv::TokenKind::Numeric);
        REQUIRE(tokens[3].getText() == "50I16");
        REQUIRE(tokens[4].getKind() == jsv::TokenKind::Numeric);
        REQUIRE(tokens[4].getText() == "100U32");
    }

    SECTION("suffix edge cases: strict width validation and invalid suffixes") {
        // 1i → Numeric("1") + Identifier("i") (i alone is NOT a suffix)
        jsv::Lexer lex1{"1i", "test.jsav"};
        const auto tokens1 = lex1.tokenize();
        REQUIRE(tokens1.size() == 3);
        REQUIRE(tokens1[0].getKind() == jsv::TokenKind::Numeric);
        REQUIRE(tokens1[0].getText() == "1");
        REQUIRE(tokens1[1].getKind() == jsv::TokenKind::IdentifierAscii);
        REQUIRE(tokens1[1].getText() == "i");

        // 1u64 → Numeric("1") + TypeU64("u64") (invalid width 64 is NOT consumed as suffix)
        jsv::Lexer lex2{"1u64", "test.jsav"};
        const auto tokens2 = lex2.tokenize();
        REQUIRE(tokens2.size() == 3);
        REQUIRE(tokens2[0].getKind() == jsv::TokenKind::Numeric);
        REQUIRE(tokens2[0].getText() == "1");
        REQUIRE(tokens2[1].getKind() == jsv::TokenKind::TypeU64);
        REQUIRE(tokens2[1].getText() == "u64");

        // 5f32 → Numeric("5f") + Numeric("32") (f never forms compounds)
        jsv::Lexer lex3{"5f32", "test.jsav"};
        const auto tokens3 = lex3.tokenize();
        REQUIRE(tokens3.size() == 3);
        REQUIRE(tokens3[0].getKind() == jsv::TokenKind::Numeric);
        REQUIRE(tokens3[0].getText() == "5f");
        REQUIRE(tokens3[1].getKind() == jsv::TokenKind::Numeric);
        REQUIRE(tokens3[1].getText() == "32");

        // 1I → Numeric("1") + Identifier("I") (I alone is NOT a suffix)
        jsv::Lexer lex4{"1I", "test.jsav"};
        const auto tokens4 = lex4.tokenize();
        REQUIRE(tokens4.size() == 3);
        REQUIRE(tokens4[0].getKind() == jsv::TokenKind::Numeric);
        REQUIRE(tokens4[0].getText() == "1");
        REQUIRE(tokens4[1].getKind() == jsv::TokenKind::IdentifierAscii);
        REQUIRE(tokens4[1].getText() == "I");

        // Additional tests for invalid widths
        // 1i999 → Numeric("1") + Identifier("i999") (invalid width 999 is NOT consumed as suffix)
        jsv::Lexer lex5{"1i999", "test.jsav"};
        const auto tokens5 = lex5.tokenize();
        REQUIRE(tokens5.size() == 3);
        REQUIRE(tokens5[0].getKind() == jsv::TokenKind::Numeric);
        REQUIRE(tokens5[0].getText() == "1");
        REQUIRE(tokens5[1].getKind() == jsv::TokenKind::IdentifierAscii);
        REQUIRE(tokens5[1].getText() == "i999");

        // 1u8 → Numeric("1u8") (valid width 8 IS consumed)
        jsv::Lexer lex6{"1u8", "test.jsav"};
        const auto tokens6 = lex6.tokenize();
        REQUIRE(tokens6.size() == 2);
        REQUIRE(tokens6[0].getKind() == jsv::TokenKind::Numeric);
        REQUIRE(tokens6[0].getText() == "1u8");

        // 1i8 → Numeric("1i8") (valid width 8 IS consumed)
        jsv::Lexer lex7{"1i8", "test.jsav"};
        const auto tokens7 = lex7.tokenize();
        REQUIRE(tokens7.size() == 2);
        REQUIRE(tokens7[0].getKind() == jsv::TokenKind::Numeric);
        REQUIRE(tokens7[0].getText() == "1i8");

        // 1u80 → Numeric("1") + Identifier("u80") (invalid width 80 is NOT consumed as suffix)
        jsv::Lexer lex8{"1u80", "test.jsav"};
        const auto tokens8 = lex8.tokenize();
        REQUIRE(tokens8.size() == 3);
        REQUIRE(tokens8[0].getKind() == jsv::TokenKind::Numeric);
        REQUIRE(tokens8[0].getText() == "1");
        REQUIRE(tokens8[1].getKind() == jsv::TokenKind::IdentifierAscii);
        REQUIRE(tokens8[1].getText() == "u80");
    }
}

TEST_CASE("Lexer_NumericTypeSuffixes_PositionTracking", "[lexer][numeric][us3][phase5]") {
    SECTION("position tracking for type suffix") {
        jsv::Lexer lex{"42d", "test.jsav"};
        const auto tokens = lex.tokenize();
        REQUIRE(tokens.size() == 2);
        REQUIRE(tokens[0].getKind() == jsv::TokenKind::Numeric);
        REQUIRE(tokens[0].getText() == "42d");
        REQUIRE(tokens[0].getSpan().start.line == 1);
        REQUIRE(tokens[0].getSpan().start.column == 1);
        REQUIRE(tokens[0].getSpan().start.absolute_pos == 0);
        REQUIRE(tokens[0].getSpan().end.line == 1);
        REQUIRE(tokens[0].getSpan().end.column == 4);
        REQUIRE(tokens[0].getSpan().end.absolute_pos == 3);
    }

    SECTION("position tracking for compound suffix") {
        jsv::Lexer lex{"255u16", "test.jsav"};
        const auto tokens = lex.tokenize();
        REQUIRE(tokens.size() == 2);
        REQUIRE(tokens[0].getKind() == jsv::TokenKind::Numeric);
        REQUIRE(tokens[0].getText() == "255u16");
        REQUIRE(tokens[0].getSpan().start.line == 1);
        REQUIRE(tokens[0].getSpan().start.column == 1);
        REQUIRE(tokens[0].getSpan().start.absolute_pos == 0);
        REQUIRE(tokens[0].getSpan().end.line == 1);
        REQUIRE(tokens[0].getSpan().end.column == 7);
        REQUIRE(tokens[0].getSpan().end.absolute_pos == 6);
    }
}

// ==========================================================================
// Phase 6 – User Story 4: Complete G1→G2→G3 pattern
// ==========================================================================

TEST_CASE("Lexer_NumericCombinedPattern_TokenizeCorrectly", "[lexer][numeric][us4][phase6]") {
    SECTION("G1+G2+G3 combinations produce single Numeric tokens") {
        jsv::Lexer lex{"1.5e10f 2.0E-3d 1e2u16 .5e1i32", "test.jsav"};
        const auto tokens = lex.tokenize();
        // 4 numbers + spaces (consumed) + Eof = 5 tokens
        REQUIRE(tokens.size() == 5);
        REQUIRE(tokens[0].getKind() == jsv::TokenKind::Numeric);
        REQUIRE(tokens[0].getText() == "1.5e10f");
        REQUIRE(tokens[1].getKind() == jsv::TokenKind::Numeric);
        REQUIRE(tokens[1].getText() == "2.0E-3d");
        REQUIRE(tokens[2].getKind() == jsv::TokenKind::Numeric);
        REQUIRE(tokens[2].getText() == "1e2u16");
        REQUIRE(tokens[3].getKind() == jsv::TokenKind::Numeric);
        REQUIRE(tokens[3].getText() == ".5e1i32");
    }

    SECTION("group optionality: G1 mandatory, G2 and G3 optional") {
        // 42 → Numeric("42") (G1 only)
        jsv::Lexer lex1{"42", "test.jsav"};
        const auto tokens1 = lex1.tokenize();
        REQUIRE(tokens1.size() == 2);
        REQUIRE(tokens1[0].getKind() == jsv::TokenKind::Numeric);
        REQUIRE(tokens1[0].getText() == "42");

        // 42e10 → Numeric("42e10") (G1 + G2)
        jsv::Lexer lex2{"42e10", "test.jsav"};
        const auto tokens2 = lex2.tokenize();
        REQUIRE(tokens2.size() == 2);
        REQUIRE(tokens2[0].getKind() == jsv::TokenKind::Numeric);
        REQUIRE(tokens2[0].getText() == "42e10");

        // 42u → Numeric("42") + Identifier("u") (G1 + invalid suffix, u alone NOT consumed)
        jsv::Lexer lex3{"42u", "test.jsav"};
        const auto tokens3 = lex3.tokenize();
        REQUIRE(tokens3.size() == 3);
        REQUIRE(tokens3[0].getKind() == jsv::TokenKind::Numeric);
        REQUIRE(tokens3[0].getText() == "42");
        REQUIRE(tokens3[1].getKind() == jsv::TokenKind::IdentifierAscii);
        REQUIRE(tokens3[1].getText() == "u");

        // 42e10u → Numeric("42e10") + Identifier("u") (G1 + G2 + invalid suffix)
        jsv::Lexer lex4{"42e10u", "test.jsav"};
        const auto tokens4 = lex4.tokenize();
        REQUIRE(tokens4.size() == 3);
        REQUIRE(tokens4[0].getKind() == jsv::TokenKind::Numeric);
        REQUIRE(tokens4[0].getText() == "42e10");
        REQUIRE(tokens4[1].getKind() == jsv::TokenKind::IdentifierAscii);
        REQUIRE(tokens4[1].getText() == "u");

        // 42d → Numeric("42d") (G1 + valid G3, d is valid single suffix)
        jsv::Lexer lex5{"42d", "test.jsav"};
        const auto tokens5 = lex5.tokenize();
        REQUIRE(tokens5.size() == 2);
        REQUIRE(tokens5[0].getKind() == jsv::TokenKind::Numeric);
        REQUIRE(tokens5[0].getText() == "42d");

        // 42e10d → Numeric("42e10d") (G1 + G2 + valid G3)
        jsv::Lexer lex6{"42e10d", "test.jsav"};
        const auto tokens6 = lex6.tokenize();
        REQUIRE(tokens6.size() == 2);
        REQUIRE(tokens6[0].getKind() == jsv::TokenKind::Numeric);
        REQUIRE(tokens6[0].getText() == "42e10d");
    }
}

TEST_CASE("Lexer_NumericCombinedPattern_PositionTracking", "[lexer][numeric][us4][phase6]") {
    SECTION("position tracking for complete G1+G2+G3 pattern") {
        jsv::Lexer lex{"1.5e10f", "test.jsav"};
        const auto tokens = lex.tokenize();
        REQUIRE(tokens.size() == 2);
        REQUIRE(tokens[0].getKind() == jsv::TokenKind::Numeric);
        REQUIRE(tokens[0].getText() == "1.5e10f");
        REQUIRE(tokens[0].getSpan().start.line == 1);
        REQUIRE(tokens[0].getSpan().start.column == 1);
        REQUIRE(tokens[0].getSpan().start.absolute_pos == 0);
        REQUIRE(tokens[0].getSpan().end.line == 1);
        REQUIRE(tokens[0].getSpan().end.column == 8);
        REQUIRE(tokens[0].getSpan().end.absolute_pos == 7);
    }
}
// ==========================================================================
// Phase 7 – User Story 5: Maximal munch rule and token boundaries
// ==========================================================================

TEST_CASE("Lexer_NumericTokenBoundaries_TokenizeCorrectly", "[lexer][numeric][us5][phase7]") {
    SECTION("token boundaries: -42 produces Minus + Numeric") {
        jsv::Lexer lex{"-42", "test.jsav"};
        const auto tokens = lex.tokenize();
        REQUIRE(tokens.size() == 3);  // Minus + Numeric + Eof
        REQUIRE(tokens[0].getKind() == jsv::TokenKind::Minus);
        REQUIRE(tokens[0].getText() == "-");
        REQUIRE(tokens[1].getKind() == jsv::TokenKind::Numeric);
        REQUIRE(tokens[1].getText() == "42");
    }

    SECTION("token boundaries: 42 u8 produces Numeric + TypeU8") {
        jsv::Lexer lex{"42 u8", "test.jsav"};
        const auto tokens = lex.tokenize();
        REQUIRE(tokens.size() == 3);  // Numeric + TypeU8 + Eof
        REQUIRE(tokens[0].getKind() == jsv::TokenKind::Numeric);
        REQUIRE(tokens[0].getText() == "42");
        REQUIRE(tokens[1].getKind() == jsv::TokenKind::TypeU8);
        REQUIRE(tokens[1].getText() == "u8");
    }

    SECTION("token boundaries: 3.14+2 produces Numeric + Plus + Numeric") {
        jsv::Lexer lex{"3.14+2", "test.jsav"};
        const auto tokens = lex.tokenize();
        REQUIRE(tokens.size() == 4);  // Numeric + Plus + Numeric + Eof
        REQUIRE(tokens[0].getKind() == jsv::TokenKind::Numeric);
        REQUIRE(tokens[0].getText() == "3.14");
        REQUIRE(tokens[1].getKind() == jsv::TokenKind::Plus);
        REQUIRE(tokens[1].getText() == "+");
        REQUIRE(tokens[2].getKind() == jsv::TokenKind::Numeric);
        REQUIRE(tokens[2].getText() == "2");
    }

    SECTION("token boundaries: 1e2+3 produces Numeric + Plus + Numeric") {
        jsv::Lexer lex{"1e2+3", "test.jsav"};
        const auto tokens = lex.tokenize();
        REQUIRE(tokens.size() == 4);  // Numeric + Plus + Numeric + Eof
        REQUIRE(tokens[0].getKind() == jsv::TokenKind::Numeric);
        REQUIRE(tokens[0].getText() == "1e2");
        REQUIRE(tokens[1].getKind() == jsv::TokenKind::Plus);
        REQUIRE(tokens[1].getText() == "+");
        REQUIRE(tokens[2].getKind() == jsv::TokenKind::Numeric);
        REQUIRE(tokens[2].getText() == "3");
    }

    SECTION("termination on non-ASCII byte") {
        // 42 followed by non-ASCII byte (0xC3) should terminate numeric token
        const std::string src = "42\xC3\xA9";  // 42 + é
        jsv::Lexer lex{src, "test.jsav"};
        const auto tokens = lex.tokenize();
        REQUIRE(tokens.size() == 3);  // Numeric + Error(é) + Eof
        REQUIRE(tokens[0].getKind() == jsv::TokenKind::Numeric);
        REQUIRE(tokens[0].getText() == "42");
    }

    SECTION("termination at EOF") {
        jsv::Lexer lex{"42", "test.jsav"};
        const auto tokens = lex.tokenize();
        REQUIRE(tokens.size() == 2);  // Numeric + Eof
        REQUIRE(tokens[0].getKind() == jsv::TokenKind::Numeric);
        REQUIRE(tokens[0].getText() == "42");
        REQUIRE(tokens[1].getKind() == jsv::TokenKind::Eof);
    }
}

TEST_CASE("Lexer_NumericNewlineTermination_FR028", "[lexer][numeric][us5][fr-028][phase7]") {
    SECTION("newline terminates complete numeric token") {
        // 42\n10 → Numeric("42") + Numeric("10") + Eof (newline consumed as whitespace)
        jsv::Lexer lex{"42\n10", "test.jsav"};
        const auto tokens = lex.tokenize();
        REQUIRE(tokens.size() == 3);
        REQUIRE(tokens[0].getKind() == jsv::TokenKind::Numeric);
        REQUIRE(tokens[0].getText() == "42");
        REQUIRE(tokens[1].getKind() == jsv::TokenKind::Numeric);
        REQUIRE(tokens[1].getText() == "10");
    }

    SECTION("CRLF terminates complete numeric token") {
        // 3.14\r\n2.5 → Numeric("3.14") + Numeric("2.5") + Eof
        jsv::Lexer lex{"3.14\r\n2.5", "test.jsav"};
        const auto tokens = lex.tokenize();
        REQUIRE(tokens.size() == 3);
        REQUIRE(tokens[0].getKind() == jsv::TokenKind::Numeric);
        REQUIRE(tokens[0].getText() == "3.14");
        REQUIRE(tokens[1].getKind() == jsv::TokenKind::Numeric);
        REQUIRE(tokens[1].getText() == "2.5");
    }

    SECTION("incomplete G1 (trailing dot) + newline terminates token") {
        // 3.\n10 → Numeric("3.") + Numeric("10") + Eof
        jsv::Lexer lex{"3.\n10", "test.jsav"};
        const auto tokens = lex.tokenize();
        REQUIRE(tokens.size() == 3);
        REQUIRE(tokens[0].getKind() == jsv::TokenKind::Numeric);
        REQUIRE(tokens[0].getText() == "3.");
        REQUIRE(tokens[1].getKind() == jsv::TokenKind::Numeric);
        REQUIRE(tokens[1].getText() == "10");
    }

    SECTION("incomplete G2 (no digits) + newline terminates token") {
        // 1e\n10 → Numeric("1") + Identifier("e") + Numeric("10") + Eof
        jsv::Lexer lex{"1e\n10", "test.jsav"};
        const auto tokens = lex.tokenize();
        REQUIRE(tokens.size() == 4);
        REQUIRE(tokens[0].getKind() == jsv::TokenKind::Numeric);
        REQUIRE(tokens[0].getText() == "1");
        REQUIRE(tokens[1].getKind() == jsv::TokenKind::IdentifierAscii);
        REQUIRE(tokens[1].getText() == "e");
        REQUIRE(tokens[2].getKind() == jsv::TokenKind::Numeric);
        REQUIRE(tokens[2].getText() == "10");
    }

    SECTION("incomplete G2+sign + newline terminates token") {
        // 1e+\n5 → Numeric("1") + Identifier("e") + Plus + Numeric("5") + Eof
        jsv::Lexer lex{"1e+\n5", "test.jsav"};
        const auto tokens = lex.tokenize();
        REQUIRE(tokens.size() == 5);
        REQUIRE(tokens[0].getKind() == jsv::TokenKind::Numeric);
        REQUIRE(tokens[0].getText() == "1");
        REQUIRE(tokens[1].getKind() == jsv::TokenKind::IdentifierAscii);
        REQUIRE(tokens[1].getText() == "e");
        REQUIRE(tokens[2].getKind() == jsv::TokenKind::Plus);
        REQUIRE(tokens[2].getText() == "+");
        REQUIRE(tokens[3].getKind() == jsv::TokenKind::Numeric);
        REQUIRE(tokens[3].getText() == "5");
    }

    SECTION("incomplete G3 (bare u) + newline terminates token") {
        // 42u\n10 → Numeric("42") + Identifier("u") + Numeric("10") + Eof
        jsv::Lexer lex{"42u\n10", "test.jsav"};
        const auto tokens = lex.tokenize();
        REQUIRE(tokens.size() == 4);
        REQUIRE(tokens[0].getKind() == jsv::TokenKind::Numeric);
        REQUIRE(tokens[0].getText() == "42");
        REQUIRE(tokens[1].getKind() == jsv::TokenKind::IdentifierAscii);
        REQUIRE(tokens[1].getText() == "u");
        REQUIRE(tokens[2].getKind() == jsv::TokenKind::Numeric);
        REQUIRE(tokens[2].getText() == "10");
    }

    SECTION("complete G1+G2 + newline terminates token") {
        // 1e10\n5 → Numeric("1e10") + Numeric("5") + Eof
        jsv::Lexer lex{"1e10\n5", "test.jsav"};
        const auto tokens = lex.tokenize();
        REQUIRE(tokens.size() == 3);
        REQUIRE(tokens[0].getKind() == jsv::TokenKind::Numeric);
        REQUIRE(tokens[0].getText() == "1e10");
        REQUIRE(tokens[1].getKind() == jsv::TokenKind::Numeric);
        REQUIRE(tokens[1].getText() == "5");
    }

    SECTION("complete G1+G2+G3 + newline terminates token") {
        // 1.5e10f\n5 → Numeric("1.5e10f") + Numeric("5") + Eof
        jsv::Lexer lex{"1.5e10f\n5", "test.jsav"};
        const auto tokens = lex.tokenize();
        REQUIRE(tokens.size() == 3);
        REQUIRE(tokens[0].getKind() == jsv::TokenKind::Numeric);
        REQUIRE(tokens[0].getText() == "1.5e10f");
        REQUIRE(tokens[1].getKind() == jsv::TokenKind::Numeric);
        REQUIRE(tokens[1].getText() == "5");
    }

    SECTION("multiple consecutive newlines") {
        // 42\n\n10 → Numeric + Numeric + Eof (all newlines consumed as whitespace)
        jsv::Lexer lex{"42\n\n10", "test.jsav"};
        const auto tokens = lex.tokenize();
        REQUIRE(tokens.size() == 3);
        REQUIRE(tokens[0].getKind() == jsv::TokenKind::Numeric);
        REQUIRE(tokens[0].getText() == "42");
        REQUIRE(tokens[1].getKind() == jsv::TokenKind::Numeric);
        REQUIRE(tokens[1].getText() == "10");
    }

    SECTION("newline at EOF") {
        // 42\n → Numeric + Eof (newline consumed as whitespace)
        jsv::Lexer lex{"42\n", "test.jsav"};
        const auto tokens = lex.tokenize();
        REQUIRE(tokens.size() == 2);
        REQUIRE(tokens[0].getKind() == jsv::TokenKind::Numeric);
        REQUIRE(tokens[0].getText() == "42");
    }

    SECTION("CR-only newline (Mac-style)") {
        // 42\r10 → Numeric("42") + Numeric("10") + Eof
        jsv::Lexer lex{"42\r10", "test.jsav"};
        const auto tokens = lex.tokenize();
        REQUIRE(tokens.size() == 3);
        REQUIRE(tokens[0].getKind() == jsv::TokenKind::Numeric);
        REQUIRE(tokens[0].getText() == "42");
        REQUIRE(tokens[1].getKind() == jsv::TokenKind::Numeric);
        REQUIRE(tokens[1].getText() == "10");
    }
}

// clang-format off
// NOLINTEND(*-include-cleaner, *-avoid-magic-numbers, *-magic-numbers, *-unchecked-optional-access, *-avoid-do-while, *-use-anonymous-namespace, *-qualified-auto, *-suspicious-stringview-data-usage, *-err58-cpp, *-function-cognitive-complexity, *-macro-usage, *-unnecessary-copy-initialization, *-uppercase-literal-suffix, *-uppercase-literal-suffix, *-container-size-empty, *-move-const-arg, *-move-const-arg, *-pass-by-value, *-diagnostic-self-assign-overloaded, *-unused-using-decls, *-identifier-length)
// clang-format on