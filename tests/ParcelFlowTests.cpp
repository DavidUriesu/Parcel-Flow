#include <chrono>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

#include <QCoreApplication>
#include <QSqlQuery>
#include "database.h"
#include "repository.h"
#include "service.h"

namespace {
    class TestFiles {
    private:
        std::filesystem::path directory;

        static void writeFile(const std::filesystem::path& path, const std::string& content) {
            std::ofstream file{ path };
            if (!file) {
                throw std::runtime_error{ "Could not create test file." };
            }
            file << content;
        }

    public:
        std::filesystem::path agentsFile;
        std::filesystem::path parcelsFile;

        TestFiles(const std::string& agentsContent, const std::string& parcelsContent) {
            long long uniqueNumber = std::chrono::steady_clock::now().time_since_epoch().count();
            directory = std::filesystem::temp_directory_path() /
                ("ParcelFlowTests_" + std::to_string(uniqueNumber));
            agentsFile = directory / "agents.txt";
            parcelsFile = directory / "parcels.txt";

            std::filesystem::create_directories(directory);
            writeFile(agentsFile, agentsContent);
            writeFile(parcelsFile, parcelsContent);
        }

        ~TestFiles() {
            std::error_code error;
            std::filesystem::remove_all(directory, error);
        }
    };

    void require(bool condition, const std::string& message) {
        if (!condition) {
            throw std::runtime_error{ message };
        }
    }

    void requireInvalidArgument(const std::function<void()>& action, const std::string& caseName) {
        try {
            action();
        }
        catch (const std::invalid_argument&) {
            return;
        }

        throw std::runtime_error{ "Expected std::invalid_argument for " + caseName + "." };
    }

    void requireRuntimeError(const std::function<void()>& action) {
        try {
            action();
        }
        catch (const std::runtime_error&) {
            return;
        }

        throw std::runtime_error{ "Expected std::runtime_error." };
    }

    void testAddValidParcel() {
        TestFiles files{ "Alice|Main Street|10|10|5\n", "" };
        Repository repository{ files.agentsFile.string(), files.parcelsFile.string() };
        Service service{ repository };

        service.addParcel("David", "Main Street", "12", 4, 7);

        std::vector<Parcel> parcels = service.getParcels();
        require(parcels.size() == 1, "The parcel was not added.");
        require(parcels[0].getRecipient() == "David", "The recipient was stored incorrectly.");
        require(parcels[0].isDelivered() == false, "A new parcel must be undelivered.");
    }

    void testRejectInvalidParcelInput() {
        TestFiles files{ "Alice|Main Street|10|10|5\n", "" };
        Repository repository{ files.agentsFile.string(), files.parcelsFile.string() };
        Service service{ repository };

        requireInvalidArgument([&service]() { service.addParcel("", "Main Street", "12", 1, 1); }, "empty recipient");
        requireInvalidArgument([&service]() { service.addParcel("David", "   ", "12", 1, 1); }, "blank street");
        requireInvalidArgument([&service]() { service.addParcel("David", "Main Street", "", 1, 1); }, "empty address number");
        requireInvalidArgument([&service]() { service.addParcel("David", "Main Street", "12", -1, 1); }, "negative coordinate");
        requireInvalidArgument([&service]() { service.addParcel("David|Test", "Main Street", "12", 1, 1); }, "pipe character");

        require(service.getParcels().empty(), "Invalid parcels entered the repository.");
    }

    void testAgentParcelFiltering() {
        TestFiles files{
            "Alice|Main Street|10|10|5\n",
            "Street Match|Main Street|1|100|100|0\n"
            "Area Match|Other Street|2|13|14|0\n"
            "No Match|Other Street|3|16|10|0\n"
            "Delivered|Main Street|4|10|10|1\n"
        };
        Repository repository{ files.agentsFile.string(), files.parcelsFile.string() };
        Service service{ repository };

        Agent agent = service.getAgents()[0];
        std::vector<Parcel> allParcels = service.getParcelsForAgent(agent, "All streets");
        std::vector<Parcel> streetParcels = service.getParcelsForAgent(agent, "Main Street");

        require(allParcels.size() == 2, "Agent filtering returned the wrong parcels.");
        require(streetParcels.size() == 1, "Street filtering returned the wrong parcels.");
        require(streetParcels[0].getRecipient() == "Street Match", "Street filtering selected the wrong parcel.");
    }

    void testDeliverParcel() {
        TestFiles files{
            "Alice|Main Street|10|10|5\n",
            "David|Main Street|12|10|10|0\n"
        };
        Repository repository{ files.agentsFile.string(), files.parcelsFile.string() };
        Service service{ repository };

        service.deliverParcel("David", "Main Street", "12");

        require(service.getParcels()[0].isDelivered(), "The parcel was not marked as delivered.");
    }

    void testSaveAndReloadParcels() {
        TestFiles files{ "Alice|Main Street|10|10|5\n", "" };

        {
            Repository repository{ files.agentsFile.string(), files.parcelsFile.string() };
            Service service{ repository };
            service.addParcel("David", "Main Street", "12", 4, 7);
            service.saveParcels();
        }

        Repository reloadedRepository{ files.agentsFile.string(), files.parcelsFile.string() };
        require(reloadedRepository.getParcels().size() == 1, "The saved parcel was not reloaded.");
        require(reloadedRepository.getParcels()[0].getRecipient() == "David", "The saved data changed.");
    }

    void testFileErrors() {
        std::filesystem::path missingDirectory = std::filesystem::temp_directory_path() /
            "ParcelFlowMissingFiles";

        requireRuntimeError([&missingDirectory]() {
            Repository repository{
                (missingDirectory / "agents.txt").string(),
                (missingDirectory / "parcels.txt").string()
            };
        });

        TestFiles malformedFiles{
            "Alice|Main Street|10|10|5\n",
            "David|Main Street|12|not-a-number|10|0\n"
        };

        requireRuntimeError([&malformedFiles]() {
            Repository repository{
                malformedFiles.agentsFile.string(),
                malformedFiles.parcelsFile.string()
            };
        });
    }

    void testDatabaseInitialization() {
        Database database{ ":memory:", "database/schema.sql", "ParcelFlowTestConnection" };

        QSqlQuery tablesQuery{ database.getConnection() };
        require(tablesQuery.exec(
            "SELECT COUNT(*) FROM sqlite_master "
            "WHERE type = 'table' AND name IN "
            "('customers', 'streets', 'agents', 'addresses', "
            "'agent_streets', 'parcels', 'parcel_events')"
        ), "Could not inspect the database tables.");
        require(tablesQuery.next(), "The table query returned no result.");
        require(tablesQuery.value(0).toInt() == 7, "The schema did not create all seven tables.");

        QSqlQuery foreignKeysQuery{ database.getConnection() };
        require(foreignKeysQuery.exec("PRAGMA foreign_keys"), "Could not inspect foreign-key settings.");
        require(foreignKeysQuery.next(), "The foreign-key query returned no result.");
        require(foreignKeysQuery.value(0).toInt() == 1, "Foreign-key enforcement is not enabled.");
    }
}

int main(int argc, char* argv[]) {
    QCoreApplication application{ argc, argv };

    struct TestCase {
        std::string name;
        std::function<void()> run;
    };

    std::vector<TestCase> tests{
        { "Add valid parcel", testAddValidParcel },
        { "Reject invalid parcel input", testRejectInvalidParcelInput },
        { "Filter parcels for agent", testAgentParcelFiltering },
        { "Deliver parcel", testDeliverParcel },
        { "Save and reload parcels", testSaveAndReloadParcels },
        { "Report file errors", testFileErrors },
        { "Initialize SQLite database", testDatabaseInitialization }
    };

    int failedTests = 0;

    for (const TestCase& test : tests) {
        try {
            test.run();
            std::cout << "[PASS] " << test.name << "\n";
        }
        catch (const std::exception& error) {
            failedTests++;
            std::cout << "[FAIL] " << test.name << ": " << error.what() << "\n";
        }
    }

    std::cout << "\n" << tests.size() - failedTests << "/" << tests.size() << " tests passed.\n";
    return failedTests == 0 ? 0 : 1;
}
