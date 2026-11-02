#include "menus.h"
#include "admin_user.h"
#include "ordinary_user.h"
#include "parking_service.h"
#include <ctime>
#include <istream>
#include <ostream>

static void show(const VehicleFeeView& view, std::ostream& out) {
    const auto& v = view.vehicle; const auto& f = view.fee;
    out << "Plate: " << v.plate << " | Category: " << v.category << " | Color: " << v.color
        << " | Entry: " << v.entryTimeText << " | Duration: " << f.days << " days "
        << f.hours << " hours " << f.minutes << " minutes | Fee: " << f.mop << " MOP\n";
}

bool runReadChoice(const std::string& choice, std::istream& in, std::ostream& out, const ParkingService& service) {
    if (choice == "3") { out << "Current count: " << service.count() << '\n'; return true; }
    if (choice == "1") {
        std::vector<VehicleFeeView> rows;
        if (service.listWithFees(std::time(nullptr), rows) != VehicleResult::Ok) out << "Unable to calculate parking durations.\n";
        else { if (rows.empty()) out << "No vehicles.\n"; for (const auto& row : rows) show(row, out); }
        out << "Current count: " << service.count() << '\n'; return true;
    }
    if (choice == "2") {
        std::string plate; out << "Plate: "; if (!std::getline(in, plate)) return false;
        VehicleFeeView row;
        const auto result = service.findWithFee(plate, std::time(nullptr), row);
        if (result == VehicleResult::NotFound) out << "Vehicle not found.\n";
        else if (result != VehicleResult::Ok) out << "Unable to retrieve vehicle or calculate duration.\n";
        else show(row, out);
        out << "Current count: " << service.count() << '\n'; return true;
    }
    out << "Invalid choice.\n"; return true;
}

void runMenu(std::istream& in, std::ostream& out, ParkingService& service, Storage& storage) {
    std::string choice;
    out << "Fee: 20 MOP per started hour, capped at 400 MOP per successive 24-hour block.\n";
    while (true) {
        out << "Parking system: 1 = ordinary, 2 = administrator, 0 = exit\n> ";
        if (!std::getline(in, choice) || choice == "0") return;
        if (choice == "1") { if (!runOrdinaryUserSession(in, out, service)) return; }
        else if (choice == "2") { if (!runAdminUserSession(in, out, service, storage)) return; }
        else out << "Invalid choice.\n";
    }
}
