#include "domain.h"

Agent::Agent() : id{ -1 }, name{ "" }, streets{}, centerX{ 0 }, centerY{ 0 }, radius{ 0 } {

}

Agent::Agent(const std::string& name, const std::vector<std::string>& streets, int centerX,
	int centerY, int radius, int id)
	: id{ id }, name{ name }, streets{ streets }, centerX{ centerX }, centerY{ centerY }, radius{ radius } {

}

int Agent::getId() const {
	return id;
}

std::string Agent::getName() const {
	return name;
}

std::vector<std::string> Agent::getStreets() const {
	return streets;
}

int Agent::getCenterX() const {
	return centerX;
}

int Agent::getCenterY() const {
	return centerY;
}

int Agent::getRadius() const {
	return radius;
}

Parcel::Parcel() : recipient{ "" }, street{ "" }, number{ "" }, x{ 0 }, y{ 0 },
	delivered{ false }, assignedAgentId{ -1 }, trackingNumber{ "" } {

}

Parcel::Parcel(const std::string& recipient, const std::string& street, const std::string& number,
	int x, int y, bool delivered, int assignedAgentId, const std::string& trackingNumber)
	: recipient{ recipient }, street{ street }, number{ number }, x{ x }, y{ y },
	delivered{ delivered }, assignedAgentId{ assignedAgentId }, trackingNumber{ trackingNumber } {

}

std::string Parcel::getRecipient() const {
	return recipient;
}

std::string Parcel::getStreet() const {
	return street;
}

std::string Parcel::getNumber() const {
	return number;
}

int Parcel::getX() const {
	return x;
}

int Parcel::getY() const {
	return y;
}

bool Parcel::isDelivered() const {
	return delivered;
}

int Parcel::getAssignedAgentId() const {
	return assignedAgentId;
}

std::string Parcel::getTrackingNumber() const {
	return trackingNumber;
}

void Parcel::setDelivered(bool value) {
	delivered = value;
}

void Parcel::setAssignedAgentId(int value) {
	assignedAgentId = value;
}

void Parcel::setTrackingNumber(const std::string& value) {
	trackingNumber = value;
}
