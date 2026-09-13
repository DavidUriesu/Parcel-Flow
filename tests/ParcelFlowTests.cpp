#include <functional>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

#include <QCoreApplication>
#include <QSqlError>
#include <QSqlQuery>
#include "database.h"
#include "repository.h"
#include "service.h"

namespace {
    class TestDatabase {
    private:
        Database database;

    public:
        explicit TestDatabase(const QString& connectionName, const QString& seedPath = {})
            : database{ ":memory:", "database/schema.sql", connectionName, seedPath } {
        }

        QSqlDatabase& connection() {
            return database.getConnection();
        }
    };

    void require(bool condition, const std::string& message) {
        if (!condition) {
            throw std::runtime_error{ message };
        }
    }

    void executeSql(QSqlDatabase& database, const QString& sql) {
        QSqlQuery query{ database };
        if (!query.exec(sql)) {
            throw std::runtime_error{ query.lastError().text().toStdString() };
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

    void testAddValidParcel() {
        TestDatabase testDatabase{ "AddParcelTest" };
        Repository repository{ testDatabase.connection() };
        Service service{ repository };

        service.addParcel("David", "Main Street", "12", 4, 7);

        std::vector<Parcel> parcels = service.getParcels();
        require(parcels.size() == 1, "The parcel was not added.");
        require(parcels[0].getRecipient() == "David", "The recipient was stored incorrectly.");
        require(!parcels[0].isDelivered(), "A new parcel must be undelivered.");
        require(parcels[0].getTrackingNumber().find("PF-") == 0,
            "The parcel tracking number has the wrong format.");

        QSqlQuery query{ testDatabase.connection() };
        require(query.exec("SELECT COUNT(*) FROM parcels"), "Could not inspect stored parcels.");
        require(query.next() && query.value(0).toInt() == 1,
            "The parcel was not written to SQLite.");
    }

    void testRejectInvalidParcelInput() {
        TestDatabase testDatabase{ "InvalidParcelTest" };
        Repository repository{ testDatabase.connection() };
        Service service{ repository };

        requireInvalidArgument([&service]() { service.addParcel("", "Main Street", "12", 1, 1); }, "empty recipient");
        requireInvalidArgument([&service]() { service.addParcel("David", "   ", "12", 1, 1); }, "blank street");
        requireInvalidArgument([&service]() { service.addParcel("David", "Main Street", "", 1, 1); }, "empty address number");
        requireInvalidArgument([&service]() { service.addParcel("David", "Main Street", "12", -1, 1); }, "negative coordinate");
        require(service.getParcels().empty(), "Invalid parcels entered the repository.");
    }

    void testAgentParcelFiltering() {
        TestDatabase testDatabase{ "FilteringTest" };
        QSqlDatabase& database = testDatabase.connection();

        executeSql(database,
            "INSERT INTO agents (name, center_x, center_y, radius) "
            "VALUES ('Alice', 10, 10, 5)");
        executeSql(database,
            "INSERT INTO streets (name) VALUES ('Main Street')");
        executeSql(database,
            "INSERT INTO agent_streets (agent_id, street_id) VALUES (1, 1)");

        Repository repository{ database };
        Service service{ repository };
        service.addParcel("Street Match", "Main Street", "1", 100, 100);
        service.addParcel("Area Match", "Other Street", "2", 13, 14);
        service.addParcel("No Match", "Other Street", "3", 16, 10);
        service.addParcel("Delivered", "Main Street", "4", 10, 10);
        service.deliverParcel(service.getParcels().back().getTrackingNumber());

        Agent agent = service.getAgents()[0];
        std::vector<Parcel> allParcels = service.getParcelsForAgent(agent, "All streets");
        std::vector<Parcel> streetParcels = service.getParcelsForAgent(agent, "Main Street");

        require(allParcels.size() == 2, "Agent filtering returned the wrong parcels.");
        require(streetParcels.size() == 1, "Street filtering returned the wrong parcels.");
        require(streetParcels[0].getRecipient() == "Street Match",
            "Street filtering selected the wrong parcel.");

        QSqlQuery assignmentQuery{ database };
        require(assignmentQuery.exec(
            "SELECT COUNT(assigned_agent_id), COUNT(*) FROM parcels"),
            "Could not inspect parcel assignments.");
        require(assignmentQuery.next() && assignmentQuery.value(0).toInt() == 3 &&
            assignmentQuery.value(1).toInt() == 4,
            "Assigned and unassigned parcels were stored incorrectly.");
    }

    void testClosestAgentSelection() {
        TestDatabase testDatabase{ "ClosestAgentTest" };
        QSqlDatabase& database = testDatabase.connection();

        executeSql(database,
            "INSERT INTO agents (name, center_x, center_y, radius) VALUES "
            "('Alice', 0, 0, 5), ('Carol', 20, 20, 5)");
        executeSql(database,
            "INSERT INTO streets (name) VALUES ('Main Street')");
        executeSql(database,
            "INSERT INTO agent_streets (agent_id, street_id) VALUES (1, 1), (2, 1)");

        Repository repository{ database };
        Service service{ repository };
        service.addParcel("David", "Main Street", "12", 19, 20);

        require(service.getParcels()[0].getAssignedAgentId() == 2,
            "The closest street agent was not selected.");
    }

    void testDeliverParcel() {
        TestDatabase testDatabase{ "DeliveryTest" };
        Repository repository{ testDatabase.connection() };
        Service service{ repository };
        service.addParcel("David", "Main Street", "12", 10, 10);
        service.addParcel("David", "Main Street", "12", 10, 10);

        std::vector<Parcel> parcels = service.getParcels();
        service.deliverParcel(parcels[1].getTrackingNumber());

        require(!service.getParcels()[0].isDelivered(),
            "The wrong parcel was marked as delivered.");
        require(service.getParcels()[1].isDelivered(),
            "The selected parcel was not marked as delivered.");

        QSqlQuery query{ testDatabase.connection() };
        query.prepare("SELECT status FROM parcels WHERE tracking_number = ?");
        query.addBindValue(QString::fromStdString(parcels[1].getTrackingNumber()));
        require(query.exec(), "Could not inspect the parcel status.");
        require(query.next() && query.value(0).toString() == "Delivered",
            "The delivery status was not written to SQLite.");
    }

    void testDatabasePersistence() {
        TestDatabase testDatabase{ "PersistenceTest" };

        {
            Repository repository{ testDatabase.connection() };
            Service service{ repository };
            service.addParcel("David", "Main Street", "12", 4, 7);
        }

        Repository reloadedRepository{ testDatabase.connection() };
        require(reloadedRepository.getParcels().size() == 1,
            "The stored parcel was not reloaded.");
        require(reloadedRepository.getParcels()[0].getRecipient() == "David",
            "The stored data changed.");
    }

    void testDatabaseInitialization() {
        TestDatabase testDatabase{ "InitializationTest" };
        QSqlDatabase& database = testDatabase.connection();

        QSqlQuery tablesQuery{ database };
        require(tablesQuery.exec(
            "SELECT COUNT(*) FROM sqlite_master "
            "WHERE type = 'table' AND name IN "
            "('customers', 'streets', 'agents', 'addresses', "
            "'agent_streets', 'parcels', 'parcel_events')"
        ), "Could not inspect the database tables.");
        require(tablesQuery.next(), "The table query returned no result.");
        require(tablesQuery.value(0).toInt() == 7,
            "The schema did not create all seven tables.");

        QSqlQuery foreignKeysQuery{ database };
        require(foreignKeysQuery.exec("PRAGMA foreign_keys"),
            "Could not inspect foreign-key settings.");
        require(foreignKeysQuery.next(), "The foreign-key query returned no result.");
        require(foreignKeysQuery.value(0).toInt() == 1,
            "Foreign-key enforcement is not enabled.");

        QSqlQuery customerColumnsQuery{ database };
        require(customerColumnsQuery.exec("PRAGMA table_info(customers)"),
            "Could not inspect customer columns.");
        while (customerColumnsQuery.next()) {
            QString column = customerColumnsQuery.value(1).toString();
            require(column != "phone" && column != "email",
                "The customers table contains an unused column.");
        }

        QSqlQuery addressColumnsQuery{ database };
        require(addressColumnsQuery.exec("PRAGMA table_info(addresses)"),
            "Could not inspect address columns.");
        while (addressColumnsQuery.next()) {
            require(addressColumnsQuery.value(1).toString() != "postal_code",
                "The addresses table contains an unused postal-code column.");
        }

        QSqlQuery streetColumnsQuery{ database };
        require(streetColumnsQuery.exec("PRAGMA table_info(streets)"),
            "Could not inspect street columns.");
        while (streetColumnsQuery.next()) {
            require(streetColumnsQuery.value(1).toString() != "city",
                "The streets table contains an unused city column.");
        }

        QSqlQuery eventColumnsQuery{ database };
        require(eventColumnsQuery.exec("PRAGMA table_info(parcel_events)"),
            "Could not inspect parcel-event columns.");
        while (eventColumnsQuery.next()) {
            require(eventColumnsQuery.value(1).toString() != "notes",
                "The parcel-events table contains an unused notes column.");
        }
    }

    void testInitialData() {
        TestDatabase testDatabase{ "InitialDataTest", "database/seed.sql" };
        Repository repository{ testDatabase.connection() };

        require(repository.getAgents().size() == 3,
            "The initial agents were not added.");
        require(repository.getParcels().size() == 5,
            "The initial parcels were not added.");
        for (const Parcel& parcel : repository.getParcels()) {
            require(parcel.getAssignedAgentId() != -1,
                "An initial parcel has no assigned agent.");
        }
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
        { "Select closest eligible agent", testClosestAgentSelection },
        { "Deliver parcel", testDeliverParcel },
        { "Reload data from SQLite", testDatabasePersistence },
        { "Initialize SQLite database", testDatabaseInitialization },
        { "Load initial demonstration data", testInitialData }
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

    std::cout << "\n" << tests.size() - failedTests << "/" << tests.size()
        << " tests passed.\n";
    return failedTests == 0 ? 0 : 1;
}
