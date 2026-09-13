#include "repository.h"

#include <QSqlError>
#include <QSqlQuery>
#include <QUuid>
#include <stdexcept>

namespace {
    std::runtime_error queryError(const QString& action, const QSqlQuery& query) {
        return std::runtime_error{ (action + ": " + query.lastError().text()).toStdString() };
    }

    QString text(const std::string& value) {
        return QString::fromStdString(value);
    }
}

Repository::Repository(QSqlDatabase& database) : database{ database } {
    loadAgents();
    loadParcels();
}

void Repository::loadAgents() {
    agents.clear();

    QSqlQuery agentQuery{ database };
    if (!agentQuery.exec(
        "SELECT id, name, center_x, center_y, radius "
        "FROM agents ORDER BY name")) {
        throw queryError("Could not load agents", agentQuery);
    }

    while (agentQuery.next()) {
        int agentId = agentQuery.value(0).toInt();
        std::vector<std::string> streets;

        QSqlQuery streetQuery{ database };
        streetQuery.prepare(
            "SELECT s.name FROM streets s "
            "JOIN agent_streets ast ON ast.street_id = s.id "
            "WHERE ast.agent_id = ? ORDER BY s.name");
        streetQuery.addBindValue(agentId);

        if (!streetQuery.exec()) {
            throw queryError("Could not load an agent's streets", streetQuery);
        }

        while (streetQuery.next()) {
            streets.push_back(streetQuery.value(0).toString().toStdString());
        }

        agents.push_back(Agent{
            agentQuery.value(1).toString().toStdString(),
            streets,
            agentQuery.value(2).toInt(),
            agentQuery.value(3).toInt(),
            agentQuery.value(4).toInt()
        });
    }
}

void Repository::loadParcels() {
    parcels.clear();

    QSqlQuery query{ database };
    if (!query.exec(
        "SELECT c.name, s.name, a.number, a.x, a.y, p.status "
        "FROM parcels p "
        "JOIN addresses a ON a.id = p.address_id "
        "JOIN customers c ON c.id = a.customer_id "
        "JOIN streets s ON s.id = a.street_id "
        "ORDER BY p.id")) {
        throw queryError("Could not load parcels", query);
    }

    while (query.next()) {
        parcels.push_back(Parcel{
            query.value(0).toString().toStdString(),
            query.value(1).toString().toStdString(),
            query.value(2).toString().toStdString(),
            query.value(3).toInt(),
            query.value(4).toInt(),
            query.value(5).toString() == "Delivered"
        });
    }
}

std::vector<Agent> Repository::getAgents() const {
    return agents;
}

std::vector<Parcel> Repository::getParcels() const {
    return parcels;
}

void Repository::addParcel(const Parcel& parcel) {
    if (!database.transaction()) {
        throw std::runtime_error{ "Could not start the parcel transaction." };
    }

    try {
        QSqlQuery customerQuery{ database };
        customerQuery.prepare("SELECT id FROM customers WHERE name = ? ORDER BY id LIMIT 1");
        customerQuery.addBindValue(text(parcel.getRecipient()));
        if (!customerQuery.exec()) {
            throw queryError("Could not find the customer", customerQuery);
        }

        int customerId;
        if (customerQuery.next()) {
            customerId = customerQuery.value(0).toInt();
        }
        else {
            customerQuery.prepare("INSERT INTO customers (name) VALUES (?)");
            customerQuery.addBindValue(text(parcel.getRecipient()));
            if (!customerQuery.exec()) {
                throw queryError("Could not add the customer", customerQuery);
            }
            customerId = customerQuery.lastInsertId().toInt();
        }

        QSqlQuery streetQuery{ database };
        streetQuery.prepare("SELECT id FROM streets WHERE name = ? AND city = ?");
        streetQuery.addBindValue(text(parcel.getStreet()));
        streetQuery.addBindValue("Cluj-Napoca");
        if (!streetQuery.exec()) {
            throw queryError("Could not find the street", streetQuery);
        }

        int streetId;
        if (streetQuery.next()) {
            streetId = streetQuery.value(0).toInt();
        }
        else {
            streetQuery.prepare("INSERT INTO streets (name, city) VALUES (?, ?)");
            streetQuery.addBindValue(text(parcel.getStreet()));
            streetQuery.addBindValue("Cluj-Napoca");
            if (!streetQuery.exec()) {
                throw queryError("Could not add the street", streetQuery);
            }
            streetId = streetQuery.lastInsertId().toInt();
        }

        QSqlQuery addressQuery{ database };
        addressQuery.prepare(
            "INSERT INTO addresses (customer_id, street_id, number, x, y) "
            "VALUES (?, ?, ?, ?, ?)");
        addressQuery.addBindValue(customerId);
        addressQuery.addBindValue(streetId);
        addressQuery.addBindValue(text(parcel.getNumber()));
        addressQuery.addBindValue(parcel.getX());
        addressQuery.addBindValue(parcel.getY());
        if (!addressQuery.exec()) {
            throw queryError("Could not add the address", addressQuery);
        }

        QSqlQuery parcelQuery{ database };
        parcelQuery.prepare(
            "INSERT INTO parcels (tracking_number, address_id, status) "
            "VALUES (?, ?, 'Created')");
        parcelQuery.addBindValue(QUuid::createUuid().toString(QUuid::WithoutBraces));
        parcelQuery.addBindValue(addressQuery.lastInsertId());
        if (!parcelQuery.exec()) {
            throw queryError("Could not add the parcel", parcelQuery);
        }

        QSqlQuery eventQuery{ database };
        eventQuery.prepare(
            "INSERT INTO parcel_events (parcel_id, status) VALUES (?, 'Created')");
        eventQuery.addBindValue(parcelQuery.lastInsertId());
        if (!eventQuery.exec()) {
            throw queryError("Could not add the parcel history", eventQuery);
        }

        if (!database.commit()) {
            throw std::runtime_error{ "Could not commit the parcel transaction." };
        }
    }
    catch (...) {
        database.rollback();
        throw;
    }

    parcels.push_back(parcel);
}

void Repository::deliverParcel(const std::string& recipient, const std::string& street,
    const std::string& number) {
    QSqlQuery findQuery{ database };
    findQuery.prepare(
        "SELECT p.id FROM parcels p "
        "JOIN addresses a ON a.id = p.address_id "
        "JOIN customers c ON c.id = a.customer_id "
        "JOIN streets s ON s.id = a.street_id "
        "WHERE c.name = ? AND s.name = ? AND a.number = ? "
        "AND p.status <> 'Delivered' ORDER BY p.id LIMIT 1");
    findQuery.addBindValue(text(recipient));
    findQuery.addBindValue(text(street));
    findQuery.addBindValue(text(number));

    if (!findQuery.exec()) {
        throw queryError("Could not find the parcel", findQuery);
    }
    if (!findQuery.next()) {
        return;
    }

    int parcelId = findQuery.value(0).toInt();
    if (!database.transaction()) {
        throw std::runtime_error{ "Could not start the delivery transaction." };
    }

    try {
        QSqlQuery updateQuery{ database };
        updateQuery.prepare(
            "UPDATE parcels SET status = 'Delivered', delivered_at = CURRENT_TIMESTAMP "
            "WHERE id = ?");
        updateQuery.addBindValue(parcelId);
        if (!updateQuery.exec()) {
            throw queryError("Could not deliver the parcel", updateQuery);
        }

        QSqlQuery eventQuery{ database };
        eventQuery.prepare(
            "INSERT INTO parcel_events (parcel_id, status) VALUES (?, 'Delivered')");
        eventQuery.addBindValue(parcelId);
        if (!eventQuery.exec()) {
            throw queryError("Could not add the delivery history", eventQuery);
        }

        if (!database.commit()) {
            throw std::runtime_error{ "Could not commit the delivery transaction." };
        }
    }
    catch (...) {
        database.rollback();
        throw;
    }

    for (Parcel& parcel : parcels) {
        if (parcel.getRecipient() == recipient &&
            parcel.getStreet() == street &&
            parcel.getNumber() == number &&
            !parcel.isDelivered()) {
            parcel.setDelivered(true);
            return;
        }
    }
}
