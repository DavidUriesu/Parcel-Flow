#include "database.h"

#include <QFile>
#include <QSqlError>
#include <QSqlQuery>
#include <stdexcept>

namespace {
    std::runtime_error databaseError(const QString& message) {
        return std::runtime_error{ message.toStdString() };
    }
}

Database::Database(const QString& databasePath, const QString& schemaPath,
    const QString& connectionName, const QString& seedPath)
    : connectionName{ connectionName } {

    if (!QSqlDatabase::isDriverAvailable("QSQLITE")) {
        throw databaseError("The QSQLITE database driver is not available.");
    }

    if (QSqlDatabase::contains(connectionName)) {
        throw databaseError("A database connection with this name already exists.");
    }

    connection = QSqlDatabase::addDatabase("QSQLITE", connectionName);
    connection.setDatabaseName(databasePath);

    if (!connection.open()) {
        QString error = connection.lastError().text();
        closeConnection();
        throw databaseError("Could not open the database: " + error);
    }

    try {
        QSqlQuery foreignKeysQuery{ connection };
        if (!foreignKeysQuery.exec("PRAGMA foreign_keys = ON")) {
            throw databaseError("Could not enable foreign keys: " + foreignKeysQuery.lastError().text());
        }

        executeSqlFile(schemaPath);
        loadInitialData(seedPath);
    }
    catch (...) {
        closeConnection();
        throw;
    }
}

void Database::loadInitialData(const QString& seedPath) {
    if (seedPath.isEmpty()) {
        return;
    }

    QSqlQuery countQuery{ connection };
    if (!countQuery.exec(
        "SELECT (SELECT COUNT(*) FROM agents) + "
        "(SELECT COUNT(*) FROM parcels)")) {
        throw databaseError("Could not inspect the database: " + countQuery.lastError().text());
    }

    if (!countQuery.next()) {
        throw databaseError("The database count query returned no result.");
    }

    if (countQuery.value(0).toInt() == 0) {
        executeSqlFile(seedPath);
    }
}

Database::~Database() {
    closeConnection();
}

void Database::executeSqlFile(const QString& filePath) {
    QFile sqlFile{ filePath };
    if (!sqlFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        throw databaseError("Could not open the SQL file: " + filePath);
    }

    QString sql = QString::fromUtf8(sqlFile.readAll());
    QStringList statements = sql.split(';', Qt::SkipEmptyParts);

    for (const QString& statement : statements) {
        QString trimmedStatement = statement.trimmed();
        if (trimmedStatement.isEmpty()) {
            continue;
        }

        QSqlQuery query{ connection };
        if (!query.exec(trimmedStatement)) {
            connection.rollback();
            throw databaseError("Could not initialize the database: " + query.lastError().text());
        }
    }
}

void Database::closeConnection() {
    if (!connection.isValid()) {
        return;
    }

    connection.close();
    connection = QSqlDatabase{};
    QSqlDatabase::removeDatabase(connectionName);
}

QSqlDatabase& Database::getConnection() {
    return connection;
}
