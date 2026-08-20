#pragma once
#include <string>
#include <vector>

class Agent {
private:
	std::string name;
	std::vector<std::string> streets;
	int centerX;
	int centerY;
	int radius;

public:
	Agent();
	Agent(const std::string& name, const std::vector<std::string>& streets, int centerX, int centerY, int radius);


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

public:
	Parcel();
	Parcel(const std::string& recipient, const std::string& street, const std::string& number, int x, int y, bool delivered);

	std::string getRecipient() const;
	std::string getStreet() const;
	std::string getNumber() const;
	int getX() const;
	int getY() const;
	bool isDelivered() const;

	void setDelivered(bool value);
};
