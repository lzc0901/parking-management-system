#pragma once
#include <iosfwd>
#include <string>
class ParkingService;
class Storage;

// Run the role selector until exit or end-of-input.
void runMenu(std::istream& in, std::ostream& out, ParkingService& service, Storage& storage);
bool runReadChoice(const std::string& choice, std::istream& in, std::ostream& out, const ParkingService& service);
