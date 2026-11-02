#pragma once
#include <iosfwd>
class ParkingService;

// Return false on end-of-input so the application can exit cleanly.
bool runOrdinaryUserSession(std::istream& in, std::ostream& out, const ParkingService& service);
