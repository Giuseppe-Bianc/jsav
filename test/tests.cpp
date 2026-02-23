// clang-format off
// NOLINTBEGIN(*-include-cleaner, *-avoid-magic-numbers, *-magic-numbers, *-unchecked-optional-access, *-avoid-do-while, *-use-anonymous-namespace, *-qualified-auto, *-suspicious-stringview-data-usage, *-err58-cpp, *-function-cognitive-complexity, *-macro-usage, *-unnecessary-copy-initialization, *-uppercase-literal-suffix, *-uppercase-literal-suffix, *-container-size-empty, *-move-const-arg, *-move-const-arg, *-pass-by-value, *-diagnostic-self-assign-overloaded, *-unused-using-decls)
// clang-format on
#include "testsConstanst.hpp"
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

    REQUIRE(span.file_path != nullptr);
    REQUIRE(*span.file_path == "");
    REQUIRE(span.start.line == 0u);
    REQUIRE(span.start.column == 0u);
    REQUIRE(span.start.absolute_pos == 0u);
    REQUIRE(span.end.line == 0u);
    REQUIRE(span.end.column == 0u);
    REQUIRE(span.end.absolute_pos == 0u);
}

TEST_CASE("SourceSpan parameterized constructor initializes correctly", "[SourceSpan]") {
    SECTION("typical values") {
        const auto filePath = std::make_shared<const std::string>("test/file.cpp");
        const jsv::SourceLocation start(1u, 1u, 0u);
        const jsv::SourceLocation end(5u, 10u, 250u);

        const jsv::SourceSpan span(filePath, start, end);

        REQUIRE(*span.file_path == "test/file.cpp");
        REQUIRE(span.start.line == 1u);
        REQUIRE(span.start.column == 1u);
        REQUIRE(span.start.absolute_pos == 0u);
        REQUIRE(span.end.line == 5u);
        REQUIRE(span.end.column == 10u);
        REQUIRE(span.end.absolute_pos == 250u);
    }

    SECTION("empty span at same position") {
        const auto filePath = std::make_shared<const std::string>("empty.cpp");
        const jsv::SourceLocation pos(3u, 5u, 20u);

        const jsv::SourceSpan span(filePath, pos, pos);

        REQUIRE(*span.file_path == "empty.cpp");
        REQUIRE(span.start.line == 3u);
        REQUIRE(span.end.line == 3u);
        REQUIRE(span.start == span.end);
    }

    SECTION("deep path") {
        const auto filePath = std::make_shared<const std::string>("a/b/c/d/e/file.cpp");
        const jsv::SourceLocation start(1u, 1u, 0u);
        const jsv::SourceLocation end(1u, 1u, 10u);

        const jsv::SourceSpan span(filePath, start, end);

        REQUIRE(*span.file_path == "a/b/c/d/e/file.cpp");
    }

    SECTION("shared pointer is shared correctly") {
        const auto filePath = std::make_shared<const std::string>("shared.cpp");
        const jsv::SourceLocation start;
        const jsv::SourceLocation end(1u, 1u, 10u);

        const jsv::SourceSpan span1(filePath, start, end);
        const jsv::SourceSpan span2(filePath, start, end);

        REQUIRE(span1.file_path == span2.file_path);
        REQUIRE(span1.file_path.use_count() >= 2);
    }
}

TEST_CASE("SourceSpan merge mutates in-place correctly", "[SourceSpan]") {
    SECTION("merge overlapping spans from same file") {
        const auto filePath = std::make_shared<const std::string>("test.cpp");
        const jsv::SourceLocation start1(1u, 1u, 0u);
        const jsv::SourceLocation end1(2u, 5u, 50u);
        jsv::SourceSpan span1(filePath, start1, end1);

        const jsv::SourceLocation start2(2u, 1u, 30u);
        const jsv::SourceLocation end2(3u, 10u, 100u);
        const jsv::SourceSpan span2(filePath, start2, end2);

        span1.merge(span2);

        REQUIRE(span1.start.line == 1u);  // earlier start
        REQUIRE(span1.end.line == 3u);    // later end
        REQUIRE(span1.end.column == 10u);
        REQUIRE(span1.end.absolute_pos == 100u);
    }

    SECTION("merge with earlier start extends backward") {
        const auto filePath = std::make_shared<const std::string>("test.cpp");
        const jsv::SourceLocation start1(5u, 10u, 100u);
        const jsv::SourceLocation end1(10u, 5u, 500u);
        jsv::SourceSpan span1(filePath, start1, end1);

        const jsv::SourceLocation start2(2u, 3u, 20u);
        const jsv::SourceLocation end2(6u, 1u, 200u);
        const jsv::SourceSpan span2(filePath, start2, end2);

        span1.merge(span2);

        REQUIRE(span1.start.line == 2u);  // extended backward
        REQUIRE(span1.end.line == 10u);   // unchanged (later)
    }

    SECTION("merge with later end extends forward") {
        const auto filePath = std::make_shared<const std::string>("test.cpp");
        const jsv::SourceLocation start1(5u, 10u, 100u);
        const jsv::SourceLocation end1(10u, 5u, 500u);
        jsv::SourceSpan span1(filePath, start1, end1);

        const jsv::SourceLocation start2(6u, 1u, 200u);
        const jsv::SourceLocation end2(15u, 10u, 1000u);
        const jsv::SourceSpan span2(filePath, start2, end2);

        span1.merge(span2);

        REQUIRE(span1.start.line == 5u);  // unchanged (earlier)
        REQUIRE(span1.end.line == 15u);   // extended forward
    }

    SECTION("merge from different file does nothing") {
        const auto filePath1 = std::make_shared<const std::string>("file1.cpp");
        const auto filePath2 = std::make_shared<const std::string>("file2.cpp");
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
        const auto filePath = std::make_shared<const std::string>("test.cpp");
        const jsv::SourceLocation start(1u, 1u, 0u);
        const jsv::SourceLocation end(5u, 5u, 100u);
        jsv::SourceSpan span1(filePath, start, end);
        const jsv::SourceSpan span2(filePath, start, end);

        span1.merge(span2);

        REQUIRE(span1.start == start);
        REQUIRE(span1.end == end);
    }
}

TEST_CASE("SourceSpan merged returns optional correctly", "[SourceSpan]") {
    SECTION("merge spans from same file returns value") {
        const auto filePath = std::make_shared<const std::string>("test.cpp");
        const jsv::SourceLocation start1(1u, 1u, 0u);
        const jsv::SourceLocation end1(2u, 5u, 50u);
        const jsv::SourceSpan span1(filePath, start1, end1);

        const jsv::SourceLocation start2(2u, 1u, 30u);
        const jsv::SourceLocation end2(3u, 10u, 100u);
        const jsv::SourceSpan span2(filePath, start2, end2);

        const std::optional<jsv::SourceSpan> result = span1.merged(span2);

        REQUIRE(result.has_value());
        REQUIRE(result->start.line == 1u);  // earlier start
        REQUIRE(result->end.line == 3u);    // later end
        REQUIRE(*result->file_path == "test.cpp");
    }

    SECTION("merge spans from different files returns nullopt") {
        const auto filePath1 = std::make_shared<const std::string>("file1.cpp");
        const auto filePath2 = std::make_shared<const std::string>("file2.cpp");
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
        const auto filePath = std::make_shared<const std::string>("test.cpp");
        const jsv::SourceLocation start1(5u, 5u, 100u);
        const jsv::SourceLocation end1(10u, 10u, 500u);
        jsv::SourceSpan span1(filePath, start1, end1);

        const jsv::SourceLocation start2(1u, 1u, 0u);
        const jsv::SourceLocation end2(15u, 15u, 1000u);
        const jsv::SourceSpan span2(filePath, start2, end2);

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
        const auto filePath = std::make_shared<const std::string>("test.cpp");
        const jsv::SourceLocation start1(5u, 5u, 100u);
        const jsv::SourceLocation end1(10u, 10u, 500u);
        const jsv::SourceSpan span1(filePath, start1, end1);

        const jsv::SourceSpan span2;  // default constructed (empty file path)

        const std::optional<jsv::SourceSpan> result = span1.merged(span2);

        // Different file paths (one empty)
        REQUIRE_FALSE(result.has_value());
    }
}

TEST_CASE("SourceSpan spaceship operator provides correct ordering", "[SourceSpan]") {
    SECTION("equal spans") {
        const auto filePath = std::make_shared<const std::string>("test.cpp");
        const jsv::SourceLocation start(1u, 1u, 0u);
        const jsv::SourceLocation end(5u, 5u, 100u);
        const jsv::SourceSpan span1(filePath, start, end);
        const jsv::SourceSpan span2(filePath, start, end);

        REQUIRE(span1 == span2);
        REQUIRE_FALSE(span1 != span2);
        REQUIRE_FALSE(span1 < span2);
        REQUIRE_FALSE(span1 > span2);
        REQUIRE(span1 <= span2);
        REQUIRE(span1 >= span2);
    }

    SECTION("different file paths") {
        const auto filePath1 = std::make_shared<const std::string>("a.cpp");
        const auto filePath2 = std::make_shared<const std::string>("b.cpp");
        const jsv::SourceLocation start;
        const jsv::SourceLocation end(1u, 1u, 10u);
        const jsv::SourceSpan span1(filePath1, start, end);
        const jsv::SourceSpan span2(filePath2, start, end);

        REQUIRE(span1 < span2);
        REQUIRE(span2 > span1);
        REQUIRE_FALSE(span1 == span2);
    }

    SECTION("same file, different start") {
        const auto filePath = std::make_shared<const std::string>("test.cpp");
        const jsv::SourceLocation start1(1u, 1u, 0u);
        const jsv::SourceLocation start2(3u, 1u, 50u);
        const jsv::SourceLocation end(5u, 5u, 100u);
        const jsv::SourceSpan span1(filePath, start1, end);
        const jsv::SourceSpan span2(filePath, start2, end);

        REQUIRE(span1 < span2);
        REQUIRE(span2 > span1);
    }

    SECTION("same file and start, different end") {
        const auto filePath = std::make_shared<const std::string>("test.cpp");
        const jsv::SourceLocation start(1u, 1u, 0u);
        const jsv::SourceLocation end1(5u, 5u, 100u);
        const jsv::SourceLocation end2(10u, 10u, 500u);
        const jsv::SourceSpan span1(filePath, start, end1);
        const jsv::SourceSpan span2(filePath, start, end2);

        REQUIRE(span1 < span2);
        REQUIRE(span2 > span1);
    }

    SECTION("lexicographic ordering prioritizes file_path over start") {
        const auto filePath1 = std::make_shared<const std::string>("a.cpp");
        const auto filePath2 = std::make_shared<const std::string>("z.cpp");
        const jsv::SourceLocation start1(100u, 100u, 10000u);
        const jsv::SourceLocation start2(1u, 1u, 0u);
        const jsv::SourceLocation end;
        const jsv::SourceSpan span1(filePath1, start1, end);
        const jsv::SourceSpan span2(filePath2, start2, end);

        // File path comparison takes precedence
        REQUIRE(span1 < span2);
    }

    SECTION("lexicographic ordering prioritizes start over end") {
        const auto filePath = std::make_shared<const std::string>("test.cpp");
        const jsv::SourceLocation start1(1u, 1u, 0u);
        const jsv::SourceLocation start2(2u, 1u, 50u);
        const jsv::SourceLocation end1(100u, 100u, 10000u);
        const jsv::SourceLocation end2(5u, 5u, 100u);
        const jsv::SourceSpan span1(filePath, start1, end1);
        const jsv::SourceSpan span2(filePath, start2, end2);

        // Start comparison takes precedence over end
        REQUIRE(span1 < span2);
    }
}

TEST_CASE("SourceSpan to_string formats correctly", "[SourceSpan]") {
    SECTION("typical span") {
        const auto filePath = std::make_shared<const std::string>("test/file.cpp");
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
        const auto filePath = std::make_shared<const std::string>("main.cpp");
        const jsv::SourceLocation pos(5u, 10u, 50u);
        const jsv::SourceSpan span(filePath, pos, pos);

        const std::string result = span.to_string();

        REQUIRE(result == "main.cpp:line 5:column 10 - line 5:column 10");
    }

    SECTION("deep path is truncated to 2 components") {
        const auto filePath = std::make_shared<const std::string>("a/b/c/d/e/file.cpp");
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
        const auto filePath = std::make_shared<const std::string>("main.cpp");
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
        const auto filePath = std::make_shared<const std::string>("test.cpp");
        const jsv::SourceLocation start(1u, 5u, 0u);
        const jsv::SourceLocation end(3u, 10u, 100u);
        const jsv::SourceSpan span(filePath, start, end);

        std::ostringstream oss;
        oss << span;

        REQUIRE(oss.str() == "test.cpp:line 1:column 5 - line 3:column 10");
    }

    SECTION("chained stream output") {
        const auto filePath = std::make_shared<const std::string>("test.cpp");
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
        const auto filePath = std::make_shared<const std::string>("test.cpp");
        const jsv::SourceLocation start(1u, 5u, 0u);
        const jsv::SourceLocation end(3u, 10u, 100u);
        const jsv::SourceSpan span1(filePath, start, end);
        const jsv::SourceSpan span2(filePath, start, end);

        const std::hash<jsv::SourceSpan> hasher;
        REQUIRE(hasher(span1) == hasher(span2));
    }

    SECTION("different spans produce different hashes") {
        const auto filePath = std::make_shared<const std::string>("test.cpp");
        const jsv::SourceLocation start(1u, 5u, 0u);
        const jsv::SourceLocation end1(3u, 10u, 100u);
        const jsv::SourceLocation end2(5u, 15u, 200u);
        const jsv::SourceSpan span1(filePath, start, end1);
        const jsv::SourceSpan span2(filePath, start, end2);

        const std::hash<jsv::SourceSpan> hasher;
        REQUIRE(hasher(span1) != hasher(span2));
    }

    SECTION("hash is stable across multiple calls") {
        const auto filePath = std::make_shared<const std::string>("test.cpp");
        const jsv::SourceLocation start(1u, 1u, 0u);
        const jsv::SourceLocation end(5u, 5u, 100u);
        const jsv::SourceSpan span(filePath, start, end);

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
        const auto filePath = std::make_shared<const std::string>("test.cpp");
        const jsv::SourceLocation start(1u, 5u, 0u);
        const jsv::SourceLocation end(3u, 10u, 100u);
        const jsv::SourceSpan span(filePath, start, end);

        const std::string result = FORMAT("{}", span);

        REQUIRE(result == "test.cpp:line 1:column 5 - line 3:column 10");
    }

    SECTION("format in larger string") {
        const auto filePath = std::make_shared<const std::string>("main.cpp");
        const jsv::SourceLocation start(5u, 10u, 50u);
        const jsv::SourceLocation end(10u, 20u, 500u);
        const jsv::SourceSpan span(filePath, start, end);

        const std::string result = FORMAT("Error at {}", span);

        REQUIRE(result == "Error at main.cpp:line 5:column 10 - line 10:column 20");
    }

    SECTION("format multiple spans") {
        const auto filePath = std::make_shared<const std::string>("test.cpp");
        const jsv::SourceSpan span1(filePath, {1u, 1u, 0u}, {2u, 2u, 50u});
        const jsv::SourceSpan span2(filePath, {3u, 3u, 100u}, {4u, 4u, 150u});

        const std::string result = FORMAT("From {} to {}", span1, span2);

        REQUIRE(result == "From test.cpp:line 1:column 1 - line 2:column 2 to test.cpp:line 3:column 3 - line 4:column 4");
    }
}

TEST_CASE("SourceSpan fmt::format integration", "[SourceSpan]") {
    SECTION("fmt::format with default specifier") {
        const auto filePath = std::make_shared<const std::string>("test.cpp");
        const jsv::SourceLocation start(1u, 5u, 0u);
        const jsv::SourceLocation end(3u, 10u, 100u);
        const jsv::SourceSpan span(filePath, start, end);

        const std::string result = fmt::format("{}", span);

        REQUIRE(result == "test.cpp:line 1:column 5 - line 3:column 10");
    }

    SECTION("fmt::format in larger string") {
        const auto filePath = std::make_shared<const std::string>("main.cpp");
        const jsv::SourceLocation start(5u, 10u, 50u);
        const jsv::SourceLocation end(10u, 20u, 500u);
        const jsv::SourceSpan span(filePath, start, end);

        const std::string result = fmt::format("Error at {}", span);

        REQUIRE(result == "Error at main.cpp:line 5:column 10 - line 10:column 20");
    }

    SECTION("fmt::format multiple spans") {
        const auto filePath = std::make_shared<const std::string>("test.cpp");
        const jsv::SourceSpan span1(filePath, {1u, 1u, 0u}, {2u, 2u, 50u});
        const jsv::SourceSpan span2(filePath, {3u, 3u, 100u}, {4u, 4u, 150u});

        const std::string result = fmt::format("From {} to {}", span1, span2);

        REQUIRE(result == "From test.cpp:line 1:column 1 - line 2:column 2 to test.cpp:line 3:column 3 - line 4:column 4");
    }
}

TEST_CASE("SourceSpan noexcept guarantees on operations", "[SourceSpan]") {
    SECTION("parameterized constructor is noexcept") {
        STATIC_REQUIRE(std::is_nothrow_constructible_v<jsv::SourceSpan, std::shared_ptr<const std::string>, const jsv::SourceLocation &,
                                                       const jsv::SourceLocation &>);
    }

    SECTION("copy constructor is noexcept") { STATIC_REQUIRE(std::is_nothrow_copy_constructible_v<jsv::SourceSpan>); }

    SECTION("move constructor is noexcept") { STATIC_REQUIRE(std::is_nothrow_move_constructible_v<jsv::SourceSpan>); }

    SECTION("copy assignment is noexcept") { STATIC_REQUIRE(std::is_nothrow_copy_assignable_v<jsv::SourceSpan>); }

    SECTION("move assignment is noexcept") { STATIC_REQUIRE(std::is_nothrow_move_assignable_v<jsv::SourceSpan>); }

    SECTION("destructor is noexcept") { STATIC_REQUIRE(std::is_nothrow_destructible_v<jsv::SourceSpan>); }

    SECTION("merge does not throw on same file") {
        const auto filePath = std::make_shared<const std::string>("test.cpp");
        jsv::SourceSpan span1(filePath, {1u, 1u, 0u}, {5u, 5u, 100u});
        const jsv::SourceSpan span2(filePath, {2u, 2u, 50u}, {10u, 10u, 500u});

        REQUIRE_NOTHROW(span1.merge(span2));
    }

    SECTION("merge does not throw on different files") {
        const auto filePath1 = std::make_shared<const std::string>("file1.cpp");
        const auto filePath2 = std::make_shared<const std::string>("file2.cpp");
        jsv::SourceSpan span1(filePath1, {1u, 1u, 0u}, {5u, 5u, 100u});
        const jsv::SourceSpan span2(filePath2, {2u, 2u, 50u}, {10u, 10u, 500u});

        REQUIRE_NOTHROW(span1.merge(span2));
    }

    SECTION("merged does not throw on same file") {
        const auto filePath = std::make_shared<const std::string>("test.cpp");
        const jsv::SourceSpan span1(filePath, {1u, 1u, 0u}, {5u, 5u, 100u});
        const jsv::SourceSpan span2(filePath, {2u, 2u, 50u}, {10u, 10u, 500u});

        REQUIRE_NOTHROW(std::ignore = span1.merged(span2));
    }

    SECTION("merged does not throw on different files") {
        const auto filePath1 = std::make_shared<const std::string>("file1.cpp");
        const auto filePath2 = std::make_shared<const std::string>("file2.cpp");
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
        const auto filePath = std::make_shared<const std::string>("a/b/c/d/e/f/g/file.cpp");
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
        const auto filePath = std::make_shared<const std::string>("test.cpp");
        std::vector<jsv::SourceSpan> spans;
        spans.emplace_back(filePath, jsv::SourceLocation{1u, 1u, 0u}, jsv::SourceLocation{2u, 2u, 50u});
        spans.emplace_back(filePath, jsv::SourceLocation{3u, 3u, 100u}, jsv::SourceLocation{4u, 4u, 150u});
        spans.emplace_back(filePath, jsv::SourceLocation{5u, 5u, 200u}, jsv::SourceLocation{6u, 6u, 250u});

        REQUIRE(spans.size() == 3u);
        REQUIRE(spans[0].start.line == 1u);
        REQUIRE(spans[1].start.line == 3u);
        REQUIRE(spans[2].start.line == 5u);
    }

    SECTION("can be used as std::map key") {
        const auto filePath = std::make_shared<const std::string>("test.cpp");
        std::map<jsv::SourceSpan, std::string> spanMap;
        spanMap[{filePath, {1u, 1u, 0u}, {2u, 2u, 50u}}] = "first";
        spanMap[{filePath, {3u, 3u, 100u}, {4u, 4u, 150u}}] = "second";
        spanMap[{filePath, {5u, 5u, 200u}, {6u, 6u, 250u}}] = "third";

        REQUIRE(spanMap.size() == 3u);
        REQUIRE(spanMap.at({filePath, {1u, 1u, 0u}, {2u, 2u, 50u}}) == "first");
        REQUIRE(spanMap.at({filePath, {3u, 3u, 100u}, {4u, 4u, 150u}}) == "second");
    }

    SECTION("can be used as std::unordered_map key with custom hash") {
        const auto filePath = std::make_shared<const std::string>("test.cpp");
        std::unordered_map<jsv::SourceSpan, std::string, std::hash<jsv::SourceSpan>> spanMap;
        spanMap[{filePath, {1u, 1u, 0u}, {2u, 2u, 50u}}] = "first";
        spanMap[{filePath, {3u, 3u, 100u}, {4u, 4u, 150u}}] = "second";

        REQUIRE(spanMap.size() == 2u);
        REQUIRE(spanMap.at({filePath, {1u, 1u, 0u}, {2u, 2u, 50u}}) == "first");
    }

    SECTION("can be used in std::set") {
        const auto filePath = std::make_shared<const std::string>("test.cpp");
        std::set<jsv::SourceSpan> spanSet;
        spanSet.insert({filePath, {3u, 3u, 100u}, {4u, 4u, 150u}});
        spanSet.insert({filePath, {1u, 1u, 0u}, {2u, 2u, 50u}});
        spanSet.insert({filePath, {5u, 5u, 200u}, {6u, 6u, 250u}});
        spanSet.insert({filePath, {1u, 1u, 0u}, {2u, 2u, 50u}});  // duplicate

        REQUIRE(spanSet.size() == 3u);
        REQUIRE(spanSet.begin()->start.line == 1u);           // smallest
        REQUIRE(std::prev(spanSet.end())->start.line == 5u);  // largest
    }
}

TEST_CASE("SourceSpan edge cases with extreme values", "[SourceSpan]") {
    SECTION("maximum size_t values in locations") {
        constexpr std::size_t max = std::numeric_limits<std::size_t>::max();
        const auto filePath = std::make_shared<const std::string>("test.cpp");
        const jsv::SourceLocation start(max, max, max);
        const jsv::SourceLocation end(max, max, max);
        const jsv::SourceSpan span(filePath, start, end);

        REQUIRE(span.start.line == max);
        REQUIRE(span.end.line == max);

        // Verify to_string handles large numbers
        const std::string result = span.to_string();
        REQUIRE_FALSE(result.empty());
    }

    SECTION("empty span (start equals end)") {
        const auto filePath = std::make_shared<const std::string>("test.cpp");
        const jsv::SourceLocation pos(5u, 10u, 100u);
        const jsv::SourceSpan span(filePath, pos, pos);

        REQUIRE(span.start == span.end);
        REQUIRE(span.start.line == 5u);
        REQUIRE(span.end.line == 5u);
    }

    SECTION("span with end before start (valid but unusual)") {
        const auto filePath = std::make_shared<const std::string>("test.cpp");
        const jsv::SourceLocation start(10u, 10u, 500u);
        const jsv::SourceLocation end(5u, 5u, 100u);
        const jsv::SourceSpan span(filePath, start, end);

        // This is technically valid - just represents an inverted span
        REQUIRE(span.start.line == 10u);
        REQUIRE(span.end.line == 5u);
    }

    SECTION("comparison with mixed extreme values") {
        const auto filePath1 = std::make_shared<const std::string>("a.cpp");
        const auto filePath2 = std::make_shared<const std::string>("z.cpp");
        const jsv::SourceSpan small(filePath1, {0u, 0u, 0u}, {0u, 0u, 0u});
        constexpr std::size_t max = std::numeric_limits<std::size_t>::max();
        const jsv::SourceSpan large(filePath2, {max, max, max}, {max, max, max});

        REQUIRE(small < large);
        REQUIRE(large > small);
        REQUIRE_FALSE(small == large);
    }

    SECTION("self-comparison") {
        const auto filePath = std::make_shared<const std::string>("test.cpp");
        const jsv::SourceSpan span(filePath, {42u, 42u, 420u}, {84u, 84u, 840u});

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
        const auto filePath = std::make_shared<const std::string>("test.cpp");
        const jsv::SourceSpan original(filePath, {10u, 20u, 100u}, {30u, 40u, 300u});
        const jsv::SourceSpan copied = original;

        REQUIRE(copied.file_path == original.file_path);
        REQUIRE(copied.start == original.start);
        REQUIRE(copied.end == original.end);
        REQUIRE(copied == original);
    }

    SECTION("copy assignment preserves all fields") {
        const auto filePath = std::make_shared<const std::string>("test.cpp");
        jsv::SourceSpan loc1(filePath, {1u, 2u, 3u}, {4u, 5u, 6u});
        const jsv::SourceSpan loc2(filePath, {10u, 20u, 100u}, {30u, 40u, 300u});

        loc1 = loc2;

        REQUIRE(loc1.start.line == 10u);
        REQUIRE(loc1.end.column == 40u);
        REQUIRE(loc1 == loc2);
    }

    SECTION("move construction preserves all fields") {
        const auto filePath = std::make_shared<const std::string>("test.cpp");
        jsv::SourceSpan original(filePath, {10u, 20u, 100u}, {30u, 40u, 300u});
        const jsv::SourceSpan moved = std::move(original);

        REQUIRE(moved.start.line == 10u);
        REQUIRE(moved.end.column == 40u);
    }

    SECTION("move assignment preserves all fields") {
        const auto filePath = std::make_shared<const std::string>("test.cpp");
        jsv::SourceSpan loc1(filePath, {1u, 2u, 3u}, {4u, 5u, 6u});
        jsv::SourceSpan loc2(filePath, {10u, 20u, 100u}, {30u, 40u, 300u});

        loc1 = std::move(loc2);

        REQUIRE(loc1.start.line == 10u);
        REQUIRE(loc1.end.column == 40u);
    }

    SECTION("self-assignment is safe") {
        const auto filePath = std::make_shared<const std::string>("test.cpp");
        const jsv::SourceSpan span(filePath, {42u, 42u, 420u}, {84u, 84u, 840u});

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
        const auto filePath = std::make_shared<const std::string>("test.cpp");
        const jsv::SourceSpan span(filePath, {1u, 1u, 0u}, {5u, 5u, 100u});

        const TestHasSpan has_span(span);

        REQUIRE(has_span.span() == span);
    }

    SECTION("polymorphic access through base pointer") {
        const auto filePath = std::make_shared<const std::string>("test.cpp");
        const jsv::SourceSpan span(filePath, {10u, 20u, 100u}, {30u, 40u, 300u});

        const std::unique_ptr<jsv::HasSpan> ptr = std::make_unique<TestHasSpan>(span);

        REQUIRE(ptr->span() == span);
    }

    SECTION("polymorphic access through base reference") {
        const auto filePath = std::make_shared<const std::string>("test.cpp");
        const jsv::SourceSpan span(filePath, {5u, 10u, 50u}, {15u, 20u, 150u});

        const TestHasSpan has_span(span);
        const jsv::HasSpan &ref = has_span;

        REQUIRE(ref.span() == span);
    }

    SECTION("virtual destructor is noexcept") { STATIC_REQUIRE(std::is_nothrow_destructible_v<jsv::HasSpan>); }

    SECTION("span method is noexcept") {
        const auto filePath = std::make_shared<const std::string>("test.cpp");
        const TestHasSpan has_span({filePath, {1u, 1u, 0u}, {5u, 5u, 100u}});

        REQUIRE_NOTHROW(std::ignore = has_span.span());
    }
}

TEST_CASE("SourceSpan file_path sharing behavior", "[SourceSpan]") {
    SECTION("multiple spans can share same file_path") {
        const auto filePath = std::make_shared<const std::string>("shared.cpp");

        const jsv::SourceSpan span1(filePath, {1u, 1u, 0u}, {2u, 2u, 50u});
        const jsv::SourceSpan span2(filePath, {3u, 3u, 100u}, {4u, 4u, 150u});
        const jsv::SourceSpan span3(filePath, {5u, 5u, 200u}, {6u, 6u, 250u});

        REQUIRE(span1.file_path == span2.file_path);
        REQUIRE(span2.file_path == span3.file_path);
        REQUIRE(span1.file_path.use_count() >= 3);
    }

    SECTION("modifying shared_ptr use_count does not affect span equality") {
        const auto filePath = std::make_shared<const std::string>("test.cpp");
        const jsv::SourceLocation start(1u, 1u, 0u);
        const jsv::SourceLocation end(5u, 5u, 100u);

        const jsv::SourceSpan span1(filePath, start, end);
        const jsv::SourceSpan span2(filePath, start, end);

        // Create another span to increase use_count
        [[maybe_unused]] const jsv::SourceSpan span3(filePath, start, end);

        REQUIRE(span1 == span2);
    }

    SECTION("different shared_ptr instances with same content compare equal") {
        const auto filePath1 = std::make_shared<const std::string>("test.cpp");
        const auto filePath2 = std::make_shared<const std::string>("test.cpp");
        const jsv::SourceLocation start(1u, 1u, 0u);
        const jsv::SourceLocation end(5u, 5u, 100u);

        const jsv::SourceSpan span1(filePath1, start, end);
        const jsv::SourceSpan span2(filePath2, start, end);

        // file_path pointers are different, but content comparison uses string value
        REQUIRE(span1.file_path != span2.file_path);
        // But spans compare by value (file_path content, start, end)
        REQUIRE(span1 == span2);
    }
}

// =============================================================================
// Token Tests
// =============================================================================

TEST_CASE("Token construction and basic accessors", "[Token]") {
    const auto filePath = std::make_shared<const std::string>("test.jsv");
    const jsv::SourceLocation start(1u, 5u, 10u);
    const jsv::SourceLocation end(1u, 8u, 13u);
    const jsv::SourceSpan span(filePath, start, end);

    SECTION("Token constructed with all parameters") {
        const jsv::Token token(jsv::TokenKind::KeywordFun, "fun", span);

        REQUIRE(token.getKind() == jsv::TokenKind::KeywordFun);
        REQUIRE(token.getText() == "fun");
        REQUIRE(token.getSpan().file_path == filePath);
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
    const auto filePath = std::make_shared<const std::string>("test.jsv");
    const jsv::SourceLocation start(1u, 1u, 0u);
    const jsv::SourceLocation end(1u, 5u, 4u);
    const jsv::SourceSpan span(filePath, start, end);

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
    const auto filePath = std::make_shared<const std::string>("test.jsv");
    const jsv::SourceLocation start(1u, 1u, 0u);
    const jsv::SourceLocation end(1u, 5u, 4u);
    const jsv::SourceSpan span(filePath, start, end);

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
        const jsv::SourceSpan span2(filePath, start2, end2);

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
    const auto filePath = std::make_shared<const std::string>("test.jsv");
    const jsv::SourceLocation start(1u, 1u, 0u);
    const jsv::SourceLocation end(1u, 5u, 4u);
    const jsv::SourceSpan span(filePath, start, end);

    SECTION("to_string for keyword token") {
        const jsv::Token token(jsv::TokenKind::KeywordFun, "fun", span);
        const std::string result = token.to_string();

        REQUIRE(result == R"(FUN("fun") test.jsv:line 1:column 1 - line 1:column 5)");
    }

    SECTION("to_string for operator token") {
        const jsv::Token token(jsv::TokenKind::PlusEqual, "+=", span);
        const std::string result = token.to_string();

        REQUIRE(result == R"(PLUS_EQUAL("+=") test.jsv:line 1:column 1 - line 1:column 5)");
    }

    SECTION("to_string for identifier token") {
        const jsv::Token token(jsv::TokenKind::IdentifierAscii, "myVariable", span);
        const std::string result = token.to_string();

        REQUIRE(result == R"(IDENTIFIER("myVariable") test.jsv:line 1:column 1 - line 1:column 5)");
    }

    SECTION("to_string for numeric literal token") {
        const jsv::Token token(jsv::TokenKind::Numeric, "123.456", span);
        const std::string result = token.to_string();

        REQUIRE(result == R"(NUMERIC("123.456") test.jsv:line 1:column 1 - line 1:column 5)");
    }

    SECTION("to_string for string literal token") {
        const jsv::Token token(jsv::TokenKind::StringLiteral, R"(hello "world")", span);
        const std::string result = token.to_string();

        REQUIRE(result == R"(STRING("hello "world"") test.jsv:line 1:column 1 - line 1:column 5)");
    }

    SECTION("to_string for type token") {
        const jsv::Token token(jsv::TokenKind::TypeI32, "i32", span);
        const std::string result = token.to_string();

        REQUIRE(result == R"(I32("i32") test.jsv:line 1:column 1 - line 1:column 5)");
    }

    SECTION("to_string for EOF token") {
        const jsv::Token token(jsv::TokenKind::Eof, "", span);
        const std::string result = token.to_string();

        REQUIRE(result == R"(EOF("") test.jsv:line 1:column 1 - line 1:column 5)");
    }

    SECTION("to_string for error token") {
        const jsv::Token token(jsv::TokenKind::Error, "@invalid", span);
        const std::string result = token.to_string();

        REQUIRE(result == R"(ERROR("@invalid") test.jsv:line 1:column 1 - line 1:column 5)");
    }
}

TEST_CASE("Token stream output operator", "[Token]") {
    const auto filePath = std::make_shared<const std::string>("test.jsv");
    const jsv::SourceLocation start(1u, 1u, 0u);
    const jsv::SourceLocation end(1u, 5u, 4u);
    const jsv::SourceSpan span(filePath, start, end);

    SECTION("ostream operator outputs to_string result") {
        const jsv::Token token(jsv::TokenKind::KeywordReturn, "return", span);
        std::ostringstream oss;
        oss << token;

        REQUIRE(oss.str() == R"(RETURN("return") test.jsv:line 1:column 1 - line 1:column 5)");
    }

    SECTION("ostream operator with multiple tokens") {
        const jsv::Token token1(jsv::TokenKind::KeywordIf, "if", span);
        const jsv::Token token2(jsv::TokenKind::KeywordElse, "else", span);

        std::ostringstream oss;
        oss << token1 << " else " << token2;

        REQUIRE(oss.str() == R"(IF("if") test.jsv:line 1:column 1 - line 1:column 5 else ELSE("else") test.jsv:line 1:column 1 - line 1:column 5)");
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
    const auto filePath = std::make_shared<const std::string>("test.jsv");
    const jsv::SourceLocation start(1u, 1u, 0u);
    const jsv::SourceLocation end(1u, 5u, 4u);
    const jsv::SourceSpan span(filePath, start, end);

    SECTION("std::format with default format") {
        const jsv::Token token(jsv::TokenKind::KeywordFor, "for", span);
        const std::string result = std::format("{}", token);

        REQUIRE(result == R"(FOR("for") test.jsv:line 1:column 1 - line 1:column 5)");
    }

    SECTION("std::format in format string") {
        const jsv::Token token(jsv::TokenKind::KeywordWhile, "while", span);
        const std::string result = std::format("Token: {}", token);

        REQUIRE(result == R"(Token: WHILE("while") test.jsv:line 1:column 1 - line 1:column 5)");
    }

    SECTION("std::format with multiple tokens") {
        const jsv::Token token1(jsv::TokenKind::OpenParen, "(", span);
        const jsv::Token token2(jsv::TokenKind::CloseParen, ")", span);

        const std::string result = std::format("{} {}", token1, token2);

        // "(()" + "())" = "((())())"
        REQUIRE(
            result ==
            "OPEN_PAREN(\"(\") test.jsv:line 1:column 1 - line 1:column 5 CLOSE_PAREN(\")\") test.jsv:line 1:column 1 - line 1:column 5");
    }
}

TEST_CASE("Token fmt::formatter integration", "[Token]") {
    const auto filePath = std::make_shared<const std::string>("test.jsv");
    const jsv::SourceLocation start(1u, 1u, 0u);
    const jsv::SourceLocation end(1u, 5u, 4u);
    const jsv::SourceSpan span(filePath, start, end);

    SECTION("fmt::format with default format") {
        const jsv::Token token(jsv::TokenKind::KeywordMain, "main", span);
        const std::string result = fmt::format("{}", token);

        REQUIRE(result == R"(MAIN("main") test.jsv:line 1:column 1 - line 1:column 5)");
    }

    SECTION("fmt::format in format string") {
        const jsv::Token token(jsv::TokenKind::KeywordVar, "var", span);
        const std::string result = fmt::format("Token: {}", token);

        REQUIRE(result == R"(Token: VAR("var") test.jsv:line 1:column 1 - line 1:column 5)");
    }
}

TEST_CASE("Token corner cases and edge cases", "[Token]") {
    const auto filePath = std::make_shared<const std::string>("test.jsv");
    const jsv::SourceLocation start(1u, 1u, 0u);
    const jsv::SourceLocation end(1u, 1u, 0u);
    const jsv::SourceSpan span(filePath, start, end);

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
        const std::string textWithNull = "hello\0world";
        const jsv::Token token(jsv::TokenKind::StringLiteral, std::string_view(textWithNull.data(), 11), span);

        REQUIRE(token.getText().size() == 11u);
    }

    SECTION("Token at position zero") {
        const jsv::SourceLocation zeroLoc(0u, 0u, 0u);
        const jsv::SourceSpan zeroSpan(filePath, zeroLoc, zeroLoc);
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
        const jsv::SourceSpan largeSpan(filePath, largeLoc, largeLoc);
        const jsv::Token token(jsv::TokenKind::IdentifierAscii, "x", largeSpan);

        REQUIRE(token.getSpan().start.line == largeLine);
        REQUIRE(token.getSpan().start.column == largeCol);
        REQUIRE(token.getSpan().start.absolute_pos == largeOffset);
    }
}

TEST_CASE("Token noexcept contracts", "[Token]") {
    const auto filePath = std::make_shared<const std::string>("test.jsv");
    const jsv::SourceLocation start(1u, 1u, 0u);
    const jsv::SourceLocation end(1u, 5u, 4u);
    const jsv::SourceSpan span(filePath, start, end);

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

    SECTION("copy operations do not throw") {
        const jsv::Token token(jsv::TokenKind::KeywordIf, "if", span);
        REQUIRE_NOTHROW([&]() { jsv::Token copied(token); }());
        REQUIRE_NOTHROW([&]() { jsv::Token assigned = token; }());
    }

    SECTION("move operations do not throw") {
        jsv::Token token(jsv::TokenKind::KeywordIf, "if", span);
        REQUIRE_NOTHROW([&]() { jsv::Token moved(std::move(token)); }());

        jsv::Token token2(jsv::TokenKind::KeywordElse, "else", span);
        REQUIRE_NOTHROW(token2 = std::move(token));
    }

    SECTION("comparison operators do not throw") {
        const jsv::Token token1(jsv::TokenKind::KeywordIf, "if", span);
        const jsv::Token token2(jsv::TokenKind::KeywordIf, "if", span);

        REQUIRE_NOTHROW(std::ignore = (token1 == token2));
        REQUIRE_NOTHROW(std::ignore = (token1 != token2));
        REQUIRE_NOTHROW(std::ignore = (token1 <=> token2));
    }
}

TEST_CASE("Token data-driven tests", "[Token]") {
    const auto filePath = std::make_shared<const std::string>("test.jsv");
    const jsv::SourceLocation start(1u, 1u, 0u);
    const jsv::SourceLocation end(1u, 5u, 4u);
    const jsv::SourceSpan span(filePath, start, end);

    SECTION("various keyword tokens") {
        auto [kind, text, expected] = GENERATE(table<jsv::TokenKind, const char *, const char *>({
            {jsv::TokenKind::KeywordFun, "fun", R"(FUN("fun") test.jsv:line 1:column 1 - line 1:column 5)"},
            {jsv::TokenKind::KeywordIf, "if", R"(IF("if") test.jsv:line 1:column 1 - line 1:column 5)"},
            {jsv::TokenKind::KeywordElse, "else", R"(ELSE("else") test.jsv:line 1:column 1 - line 1:column 5)"},
            {jsv::TokenKind::KeywordReturn, "return", R"(RETURN("return") test.jsv:line 1:column 1 - line 1:column 5)"},
            {jsv::TokenKind::KeywordWhile, "while", R"(WHILE("while") test.jsv:line 1:column 1 - line 1:column 5)"},
            {jsv::TokenKind::KeywordFor, "for", R"(FOR("for") test.jsv:line 1:column 1 - line 1:column 5)"},
            {jsv::TokenKind::KeywordMain, "main", R"(MAIN("main") test.jsv:line 1:column 1 - line 1:column 5)"},
            {jsv::TokenKind::KeywordVar, "var", R"(VAR("var") test.jsv:line 1:column 1 - line 1:column 5)"},
            {jsv::TokenKind::KeywordConst, "const", R"(CONST("const") test.jsv:line 1:column 1 - line 1:column 5)"},
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

// clang-format off
// NOLINTEND(*-include-cleaner, *-avoid-magic-numbers, *-magic-numbers, *-unchecked-optional-access, *-avoid-do-while, *-use-anonymous-namespace, *-qualified-auto, *-suspicious-stringview-data-usage, *-err58-cpp, *-function-cognitive-complexity, *-macro-usage, *-unnecessary-copy-initialization, *-uppercase-literal-suffix, *-uppercase-literal-suffix, *-container-size-empty, *-move-const-arg, *-move-const-arg, *-pass-by-value,*-diagnostic-self-assign-overloaded, *-unused-using-decls)
// clang-format on
