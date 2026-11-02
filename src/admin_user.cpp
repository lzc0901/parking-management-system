#include "admin_user.h"
#include "menus.h"
#include "parking_service.h"
#include "storage.h"
#include <ctime>
#include <istream>
#include <ostream>
#include <string>

static bool readRecord(std::istream& in, std::ostream& out, VehicleRecord& record) {
    out << "Plate: "; if (!std::getline(in, record.plate)) return false;
    out << "Category: "; if (!std::getline(in, record.category)) return false;
    out << "Color: "; if (!std::getline(in, record.color)) return false;
    out << "Entry time (YYYY-MM-DD HH:MM): "; return static_cast<bool>(std::getline(in, record.entryTimeText));
}
static const char* message(VehicleResult result) {
    switch (result) {
        case VehicleResult::Forbidden: return "Permission denied.";
        case VehicleResult::InvalidField: return "Invalid required field.";
        case VehicleResult::InvalidTime: return "Invalid entry time.";
        case VehicleResult::FutureTime: return "Entry time is in the future.";
        case VehicleResult::DuplicatePlate: return "Plate already exists.";
        case VehicleResult::NotFound: return "Vehicle not found.";
        case VehicleResult::Ok: return "Updated.";
    }
    return "Operation failed.";
}
bool runAdminUserSession(std::istream& in, std::ostream& out, ParkingService& service, Storage& storage) {
    std::string choice;
    while (true) {
        out << "Administrator session: 1 = list, 2 = find, 3 = count, 4 = add, 5 = modify, 6 = delete, 0 = logout\n> ";
        if (!std::getline(in, choice)) return false;
        if (choice == "0") return true;
        if (choice != "4" && choice != "5" && choice != "6") { if (!runReadChoice(choice, in, out, service)) return false; continue; }
        std::string plate; VehicleRecord record; VehicleResult result;
        if (choice != "4") { out << "Existing plate: "; if (!std::getline(in, plate)) return false; }
        if (choice != "6" && !readRecord(in, out, record)) return false;
        if (choice == "4") result = service.addVehicle(Role::Admin, record, std::time(nullptr));
        else if (choice == "5") result = service.modifyVehicle(Role::Admin, plate, record, std::time(nullptr));
        else result = service.deleteVehicle(Role::Admin, plate);
        if (result != VehicleResult::Ok) { out << message(result) << '\n'; continue; }
        const auto saved = storage.save(service);
        if (saved.ok) out << "Updated and saved. " << saved.message << '\n';
        else out << "Updated in memory, but save failed: " << saved.message << '\n';
    }
}
