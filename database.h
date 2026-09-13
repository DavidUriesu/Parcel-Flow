#pragma once

#include <QSqlDatabase>
#include <QString>

class Database {
private:
    QString connectionName;
    QSqlDatabase connection;

    void executeSqlFile(const QString& filePath);
    void loadInitialData(const QString& seedPath);
    void closeConnection();

public:
    Database(const QString& databasePath, const QString& schemaPath,
        const QString& connectionName = QStringLiteral("ParcelFlowConnection"),
        const QString& seedPath = {});
    ~Database();

    Database(const Database&) = delete;
    Database& operator=(const Database&) = delete;

    QSqlDatabase& getConnection();
};
