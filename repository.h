#pragma once
#include <vector>
#include <string>
#include "domain.h"

class Repository {
private:
	std::string agentsFile;
	std::string parcelsFile;

	std::vector<Agent> agents;
	std::vector<Parcel> parcels;

	void loadAgents();
	void loadParcels();

public:
	Repository(const std::string& agentsFile, const std::string& parcelsFile);

	std::vector<Agent> getAgents() const;
	std::vector<Parcel> getParcels() const;

	void addParcel(const Parcel& parcel);
	void deliverParcel(const std::string& recipient, const std::string& street, const std::string& number);

	void saveParcels() const;
};