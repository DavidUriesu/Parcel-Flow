#pragma once
#include <string>
#include <vector>

class Agent {
private:
	int id;
	std::string name;
	std::vector<std::string> streets;
	int centerX;
	int centerY;
	int radius;

public:
	Agent();
	Agent(const std::string& name, const std::vector<std::string>& streets, int centerX,
		int centerY, int radius, int id = -1);

	int getId() const;
	std::string getName() const;
	std::vector<std::string> getStreets() const;
	int getCenterX() const;
	int getCenterY() const;
	int getRadius() const;
};

class Parcel {
private:
	std::string recipient;
	std::string street;
	std::string number;
	int x;
	int y;
	bool delivered;
	int assignedAgentId;
	std::string trackingNumber;

public:
	Parcel();
	Parcel(const std::string& recipient, const std::string& street, const std::string& number,
		int x, int y, bool delivered, int assignedAgentId = -1,
		const std::string& trackingNumber = "");

	std::string getRecipient() const;
	std::string getStreet() const;
	std::string getNumber() const;
	int getX() const;
	int getY() const;
	bool isDelivered() const;
	int getAssignedAgentId() const;
	std::string getTrackingNumber() const;

	void setDelivered(bool value);
	void setAssignedAgentId(int value);
	void setTrackingNumber(const std::string& value);
};
