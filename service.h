#pragma once
#include <vector>
#include <string>
#include "repository.h"

class Observer {
public:
	virtual void update() = 0;
	virtual ~Observer() = default;
};

class Subject {
private:
	std::vector<Observer*> observers;

public:
	void addObserver(Observer* observer);
	void removeObserver(Observer* observer);
	void notify();
};

class Service : public Subject {
private:
	Repository& repository;

	bool parcelInAgentArea(const Parcel& parcel, const Agent& agent) const;
	bool agentHasStreet(const Agent& agent, const std::string& street) const;
	int findAgentId(const Parcel& parcel) const;

public:
	Service(Repository& repository);

	std::vector<Agent> getAgents() const;
	std::vector<Parcel> getParcels() const;
	std::vector<Parcel> getUndeliveredParcels() const;
	std::vector<Parcel> getParcelsForAgent(const Agent& agent, const std::string& selectedStreet) const;
	std::vector<std::string> getAllStreets() const;
	std::string getAssignedAgentName(const Parcel& parcel) const;

	void addParcel(const std::string& recipient, const std::string& street, const std::string& number, int x, int y);
	void deliverParcel(const std::string& trackingNumber);

};
