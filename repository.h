#pragma once
#include <vector>
#include <QSqlDatabase>
#include "domain.h"

class Repository {
private:
	QSqlDatabase& database;

	std::vector<Agent> agents;
	std::vector<Parcel> parcels;

	void loadAgents();
	void loadParcels();

public:
	explicit Repository(QSqlDatabase& database);

	std::vector<Agent> getAgents() const;
	std::vector<Parcel> getParcels() const;

	void addParcel(const Parcel& parcel);
	void deliverParcel(const std::string& recipient, const std::string& street, const std::string& number);

};
