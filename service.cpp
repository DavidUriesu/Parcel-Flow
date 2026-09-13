#include "service.h"
#include <algorithm>
#include <stdexcept>

namespace {
	bool isBlank(const std::string& text) {
		return text.find_first_not_of(" \t\r\n") == std::string::npos;
	}
}

void Subject::addObserver(Observer* observer) {
	observers.push_back(observer);
}

void Subject::removeObserver(Observer* observer) {
	observers.erase(std::remove(observers.begin(), observers.end(), observer), observers.end());
}

void Subject::notify() {
	for (Observer* observer : observers) {
		observer->update();
	}
}

Service::Service(Repository& repository) : repository{ repository } {
}

bool Service::parcelInAgentArea(const Parcel& parcel, const Agent& agent) const {
	int dx = parcel.getX() - agent.getCenterX();
	int dy = parcel.getY() - agent.getCenterY();

	return dx * dx + dy * dy <= agent.getRadius() * agent.getRadius();
}

bool Service::agentHasStreet(const Agent& agent, const std::string& street) const {
	for (const std::string& agentStreet : agent.getStreets()) {
		if (agentStreet == street) {
			return true;
		}
	}

	return false;
}

int Service::findAgentId(const Parcel& parcel) const {
	std::vector<Agent> agents = repository.getAgents();
	int selectedAgentId = -1;
	int shortestDistance = 0;

	for (const Agent& agent : agents) {
		if (agentHasStreet(agent, parcel.getStreet())) {
			int dx = parcel.getX() - agent.getCenterX();
			int dy = parcel.getY() - agent.getCenterY();
			int distance = dx * dx + dy * dy;

			if (selectedAgentId == -1 || distance < shortestDistance) {
				selectedAgentId = agent.getId();
				shortestDistance = distance;
			}
		}
	}

	if (selectedAgentId != -1) {
		return selectedAgentId;
	}

	for (const Agent& agent : agents) {
		if (parcelInAgentArea(parcel, agent)) {
			int dx = parcel.getX() - agent.getCenterX();
			int dy = parcel.getY() - agent.getCenterY();
			int distance = dx * dx + dy * dy;

			if (selectedAgentId == -1 || distance < shortestDistance) {
				selectedAgentId = agent.getId();
				shortestDistance = distance;
			}
		}
	}

	return selectedAgentId;
}

std::vector<Agent> Service::getAgents() const {
	return repository.getAgents();
}

std::vector<Parcel> Service::getParcels() const {
	return repository.getParcels();
}

std::vector<Parcel> Service::getUndeliveredParcels() const {
	std::vector<Parcel> result;

	for (const Parcel& parcel : repository.getParcels()) {
		if (parcel.isDelivered() == false) {
			result.push_back(parcel);
		}
	}

	return result;
}

std::vector<Parcel> Service::getParcelsForAgent(const Agent& agent, const std::string& selectedStreet) const {
	std::vector<Parcel> result;

	for (const Parcel& parcel : repository.getParcels()) {
		if (parcel.isDelivered() == false && parcel.getAssignedAgentId() == agent.getId()) {
			if (selectedStreet == "All streets" || parcel.getStreet() == selectedStreet) {
				result.push_back(parcel);
			}
		}
	}

	return result;
}

std::vector<std::string> Service::getAllStreets() const {
	std::vector<std::string> streets;

	for (const Agent& agent : repository.getAgents()) {
		for (const std::string& street : agent.getStreets()) {
			if (std::find(streets.begin(), streets.end(), street) == streets.end()) {
				streets.push_back(street);
			}
		}
	}

	for (const Parcel& parcel : repository.getParcels()) {
		if (std::find(streets.begin(), streets.end(), parcel.getStreet()) == streets.end()) {
			streets.push_back(parcel.getStreet());
		}
	}

	std::sort(streets.begin(), streets.end());
	return streets;
}

std::string Service::getAssignedAgentName(const Parcel& parcel) const {
	for (const Agent& agent : repository.getAgents()) {
		if (agent.getId() == parcel.getAssignedAgentId()) {
			return agent.getName();
		}
	}

	return "Unassigned";
}

void Service::addParcel(const std::string& recipient, const std::string& street, const std::string& number, int x, int y) {
	if (isBlank(recipient)) {
		throw std::invalid_argument{ "Recipient cannot be empty." };
	}

	if (isBlank(street)) {
		throw std::invalid_argument{ "Street cannot be empty." };
	}

	if (isBlank(number)) {
		throw std::invalid_argument{ "Address number cannot be empty." };
	}

	if (x < 0 || y < 0) {
		throw std::invalid_argument{ "Coordinates cannot be negative." };
	}

	Parcel parcel{ recipient, street, number, x, y, false };
	parcel.setAssignedAgentId(findAgentId(parcel));
	repository.addParcel(parcel);
	notify();
}

void Service::deliverParcel(const std::string& trackingNumber) {
	repository.deliverParcel(trackingNumber);
	notify();
}
