#pragma once

#include <QSqlDatabase>
#include <QString>

class Database {
private:
    QString connectionName;
    QSqlDatabase connection;

    void executeSchema(const QString& schemaPath);
    void closeConnection();

public:
    Database(const QString& databasePath, const QString& schemaPath,
        const QString& connectionName = QStringLiteral("ParcelFlowConnection"));
    ~Database();

    Database(const Database&) = delete;
    Database& operator=(const Database&) = delete;

    QSqlDatabase& getConnection();
};
