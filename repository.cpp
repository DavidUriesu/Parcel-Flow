#include "repository.h"
#include <fstream>
#include <sstream>

Repository::Repository(const std::string& agentsFile, const std::string& parcelsFile)
	: agentsFile{ agentsFile }, parcelsFile{ parcelsFile } {
	loadAgents();
	loadParcels();
}

void Repository::loadAgents() {
	std::ifstream file{ agentsFile };

	std::string name;
	std::string streetsText;
	std::string centerXText;
	std::string centerYText;
	std::string radiusText;

	while (std::getline(file, name, '|') &&
		std::getline(file, streetsText, '|') &&
		std::getline(file, centerXText, '|') &&
		std::getline(file, centerYText, '|') &&
		std::getline(file, radiusText)) {

		std::vector<std::string> streets;
		std::stringstream stream{ streetsText };
		std::string street;

		while (std::getline(stream, street, ',')) {
			streets.push_back(street);
		}

		agents.push_back(Agent{ name, streets, std::stoi(centerXText), std::stoi(centerYText), std::stoi(radiusText) });
	}

	file.close();
}

void Repository::loadParcels() {
	std::ifstream file{ parcelsFile };

	std::string recipient;
	std::string street;
	std::string number;
	std::string xText;
	std::string yText;
	std::string deliveredText;

	while (std::getline(file, recipient, '|') &&
		std::getline(file, street, '|') &&
		std::getline(file, number, '|') &&
		std::getline(file, xText, '|') &&
		std::getline(file, yText, '|') &&
		std::getline(file, deliveredText)) {

		bool delivered = deliveredText == "1";
		parcels.push_back(Parcel{ recipient, street, number, std::stoi(xText), std::stoi(yText), delivered });
	}

	file.close();
}

std::vector<Agent> Repository::getAgents() const {
	return agents;
}

std::vector<Parcel> Repository::getParcels() const {
	return parcels;
}

void Repository::addParcel(const Parcel& parcel) {
	parcels.push_back(parcel);
}

void Repository::deliverParcel(const std::string& recipient, const std::string& street, const std::string& number) {
	for (Parcel& parcel : parcels) {
		if (parcel.getRecipient() == recipient &&
			parcel.getStreet() == street &&
			parcel.getNumber() == number &&
			parcel.isDelivered() == false) {

			parcel.setDelivered(true);
			return;
		}
	}
}

void Repository::saveParcels() const {
	std::ofstream file{ parcelsFile };

	for (const Parcel& parcel : parcels) {
		file << parcel.getRecipient() << "|"
			<< parcel.getStreet() << "|"
			<< parcel.getNumber() << "|"
			<< parcel.getX() << "|"
			<< parcel.getY() << "|"
			<< parcel.isDelivered() << "\n";
	}

	file.close();
}