#include "repository.h"
#include <fstream>
#include <sstream>
#include <stdexcept>

namespace {
	int parseInteger(const std::string& text, const std::string& fileName, int lineNumber) {
		size_t parsedCharacters = 0;
		int value;

		try {
			value = std::stoi(text, &parsedCharacters);
		}
		catch (const std::exception&) {
			throw std::runtime_error{
				"Invalid number in " + fileName + " at line " + std::to_string(lineNumber) + "."
			};
		}

		if (parsedCharacters != text.size()) {
			throw std::runtime_error{
				"Invalid number in " + fileName + " at line " + std::to_string(lineNumber) + "."
			};
		}

		return value;
	}
}

Repository::Repository(const std::string& agentsFile, const std::string& parcelsFile)
	: agentsFile{ agentsFile }, parcelsFile{ parcelsFile } {
	loadAgents();
	loadParcels();
}

void Repository::loadAgents() {
	std::ifstream file{ agentsFile };
	if (!file.is_open()) {
		throw std::runtime_error{ "Could not open agents file: " + agentsFile };
	}

	std::string line;
	int lineNumber = 0;

	while (std::getline(file, line)) {
		lineNumber++;
		if (line.empty()) {
			continue;
		}

		std::stringstream row{ line };
	std::string name;
	std::string streetsText;
	std::string centerXText;
	std::string centerYText;
	std::string radiusText;

		if (!std::getline(row, name, '|') ||
			!std::getline(row, streetsText, '|') ||
			!std::getline(row, centerXText, '|') ||
			!std::getline(row, centerYText, '|') ||
			!std::getline(row, radiusText) ||
			name.empty() || streetsText.empty()) {
			throw std::runtime_error{
				"Malformed agent data in " + agentsFile + " at line " + std::to_string(lineNumber) + "."
			};
		}

		std::vector<std::string> streets;
		std::stringstream stream{ streetsText };
		std::string street;

		while (std::getline(stream, street, ',')) {
			if (street.empty()) {
				throw std::runtime_error{
					"Malformed agent data in " + agentsFile + " at line " + std::to_string(lineNumber) + "."
				};
			}
			streets.push_back(street);
		}

		int centerX = parseInteger(centerXText, agentsFile, lineNumber);
		int centerY = parseInteger(centerYText, agentsFile, lineNumber);
		int radius = parseInteger(radiusText, agentsFile, lineNumber);

		if (radius < 0) {
			throw std::runtime_error{
				"Agent radius cannot be negative in " + agentsFile + " at line " + std::to_string(lineNumber) + "."
			};
		}

		agents.push_back(Agent{ name, streets, centerX, centerY, radius });
	}
}

void Repository::loadParcels() {
	std::ifstream file{ parcelsFile };
	if (!file.is_open()) {
		throw std::runtime_error{ "Could not open parcels file: " + parcelsFile };
	}

	std::string line;
	int lineNumber = 0;

	while (std::getline(file, line)) {
		lineNumber++;
		if (line.empty()) {
			continue;
		}

		std::stringstream row{ line };
	std::string recipient;
	std::string street;
	std::string number;
	std::string xText;
	std::string yText;
	std::string deliveredText;

		if (!std::getline(row, recipient, '|') ||
			!std::getline(row, street, '|') ||
			!std::getline(row, number, '|') ||
			!std::getline(row, xText, '|') ||
			!std::getline(row, yText, '|') ||
			!std::getline(row, deliveredText) ||
			recipient.empty() || street.empty() || number.empty()) {
			throw std::runtime_error{
				"Malformed parcel data in " + parcelsFile + " at line " + std::to_string(lineNumber) + "."
			};
		}

		int x = parseInteger(xText, parcelsFile, lineNumber);
		int y = parseInteger(yText, parcelsFile, lineNumber);

		if (x < 0 || y < 0) {
			throw std::runtime_error{
				"Parcel coordinates cannot be negative in " + parcelsFile + " at line " + std::to_string(lineNumber) + "."
			};
		}

		if (deliveredText != "0" && deliveredText != "1") {
			throw std::runtime_error{
				"Invalid delivery status in " + parcelsFile + " at line " + std::to_string(lineNumber) + "."
			};
		}

		bool delivered = deliveredText == "1";
		parcels.push_back(Parcel{ recipient, street, number, x, y, delivered });
	}
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
	if (!file.is_open()) {
		throw std::runtime_error{ "Could not open parcels file for saving: " + parcelsFile };
	}

	for (const Parcel& parcel : parcels) {
		file << parcel.getRecipient() << "|"
			<< parcel.getStreet() << "|"
			<< parcel.getNumber() << "|"
			<< parcel.getX() << "|"
			<< parcel.getY() << "|"
			<< parcel.isDelivered() << "\n";
	}

	file.close();
	if (!file) {
		throw std::runtime_error{ "Could not save parcels to file: " + parcelsFile };
	}
}
