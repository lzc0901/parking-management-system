#pragma once
#include <iosfwd>
class ParkingService;
class Storage;

// Return false on end-of-input so the application can exit cleanly.
bool runAdminUserSession(std::istream& in, std::ostream& out, ParkingService& service, Storage& storage);
