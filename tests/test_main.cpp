#include "menus.h"
#include "storage.h"
#include "vehicle.h"
#include "parking_service.h"
#include <iostream>
#include <sstream>
#include <string>

int main() {
    int failures = 0;
    auto check = [&](bool condition, const char* name) {
        if (!condition) { std::cerr << "FAIL: " << name << '\n'; ++failures; }
    };

    std::istringstream input("invalid\n1\ninvalid\n0\n2\n0\n0\n");
    std::ostringstream output;
    ParkingService menuService; Storage menuStorage;
    runMenu(input, output, menuService, menuStorage);
    const auto text = output.str();
    check(text.find("Invalid choice.") != std::string::npos, "invalid input");
    check(text.find("Ordinary user session") != std::string::npos, "ordinary session");
    check(text.find("Administrator session") != std::string::npos, "administrator session after logout");

    std::istringstream eof("1\n");
    std::ostringstream eofOutput;
    runMenu(eof, eofOutput, menuService, menuStorage);
    check(eofOutput.str().find("Ordinary user session") != std::string::npos, "EOF in session");
    VehicleRecord record{"MA-12-34", "Sedan", "White", "2026-11-02 09:30", {}};
    check(record.plate == "MA-12-34", "vehicle record");

    std::string canonical;
    std::time_t now{};
    check(parseTimestamp("2026-11-02 09:30", canonical, now), "parse reference time");
    ParkingService service;
    const auto admin = Role::Admin;
    const auto ordinary = Role::OrdinaryUser;
    const auto ok = VehicleResult::Ok;
    struct FeeCase { int minutes; int mop; };
    const FeeCase feeCases[] = {{0,0}, {1,20}, {60,20}, {61,40}, {1199,400},
        {1200,400}, {1439,400}, {1440,400}, {1441,420}, {2640,800}, {2880,800}, {2881,820}};
    for (const auto& test : feeCases) {
        ParkingFee fee;
        check(calculateParkingFee(now, now + test.minutes * 60 + 59, fee) &&
              fee.totalMinutes == test.minutes && fee.mop == test.mop &&
              fee.days == test.minutes / 1440 && fee.hours == (test.minutes % 1440) / 60 &&
              fee.minutes == test.minutes % 60, "fee boundary and duration");
    }
    ParkingFee unchanged; unchanged.mop = 123;
    check(!calculateParkingFee(now, now - 60, unchanged) && unchanged.mop == 123, "negative fee rejected without output mutation");
    check(service.count() == 0 && service.listAll().empty() && !service.findByPlate("unknown"), "empty lot");
    VehicleRecord a{" MA-12-34 ", " Model Y ", " Pearl White ", " 2024-02-29 09:30 ", 42};
    VehicleRecord b{"MB-56-78", "SUV", "Black", "2026-11-02 09:30", 0};
    check(service.addVehicle(ordinary, a, now) == VehicleResult::Forbidden && service.count() == 0, "ordinary cannot add");
    check(service.addVehicle(admin, b, now + 59) == ok && service.addVehicle(admin, a, now) == ok, "add and same minute");
    VehicleFeeView view;
    std::vector<VehicleFeeView> views;
    check(service.listWithFees(now + 61 * 60 + 59, views) == ok && views.size() == 2 &&
          views[1].vehicle.plate == "MB-56-78" && views[1].fee.totalMinutes == 61 && views[1].fee.mop == 40,
          "list fee query sorted and minute normalized");
    check(service.findWithFee(" MB-56-78 ", now + 61 * 60, view) == ok && view.fee.mop == 40,
          "single fee query");
    check(service.findWithFee("missing", now, view) == VehicleResult::NotFound && view.fee.mop == 40 &&
          service.listWithFees(now - 60, views) == VehicleResult::InvalidTime && views.size() == 2 &&
          service.count() == 2, "failed fee queries preserve output and vehicle state");
    auto list = service.listAll();
    check(service.count() == 2 && list.size() == 2 && list[0].plate == "MA-12-34" && list[1].plate == "MB-56-78", "count and sorted listing");
    const auto found = service.findByPlate(" MA-12-34 ");
    check(found && found->category == "Model Y" && found->color == "Pearl White" && found->entryTimeText == "2024-02-29 09:30" && found->entryTimestamp != 42, "normalized fields and derived time");
    check(service.addVehicle(admin, a, now) == VehicleResult::DuplicatePlate, "duplicate rejected");
    auto invalid = a;
    invalid.plate = "MA 12";
    check(service.addVehicle(admin, invalid, now) == VehicleResult::InvalidField, "plate space rejected");
    invalid = a; invalid.category = "   ";
    check(service.addVehicle(admin, invalid, now) == VehicleResult::InvalidField, "empty field rejected");
    invalid = a; invalid.color = "\tWhite";
    check(service.addVehicle(admin, invalid, now) == VehicleResult::InvalidField, "outer control rejected");
    invalid = a; invalid.entryTimeText = "2026-02-29 09:30";
    check(service.addVehicle(admin, invalid, now) == VehicleResult::InvalidTime, "invalid leap date");
    invalid.entryTimeText = "2026-04-31 09:30";
    check(service.addVehicle(admin, invalid, now) == VehicleResult::InvalidTime, "invalid calendar date");
    invalid.entryTimeText = "2026-11-02 09:31";
    check(service.addVehicle(admin, invalid, now + 59) == VehicleResult::FutureTime, "future minute rejected");
    check(service.modifyVehicle(admin, "MA-12-34", b, now) == VehicleResult::DuplicatePlate, "rename collision");
    check(service.modifyVehicle(admin, "MA-12-34", invalid, now) == VehicleResult::FutureTime, "invalid edit rejected");
    check(service.modifyVehicle(ordinary, "MA-12-34", a, now) == VehicleResult::Forbidden, "ordinary cannot edit");
    check(service.modifyVehicle(admin, "missing", a, now) == VehicleResult::NotFound, "missing edit");
    check(service.count() == 2 && service.findByPlate("MA-12-34") && service.findByPlate("MB-56-78"), "rejections preserve state");
    a.plate = "MC-00-01"; a.color = "Red";
    check(service.modifyVehicle(admin, "MA-12-34", a, now) == ok && !service.findByPlate("MA-12-34") && service.findByPlate("MC-00-01")->color == "Red" && service.count() == 2, "successful rename and edit");
    check(service.deleteVehicle(ordinary, "MC-00-01") == VehicleResult::Forbidden && service.deleteVehicle(admin, "missing") == VehicleResult::NotFound && service.count() == 2, "delete rejection");
    check(service.deleteVehicle(admin, " MC-00-01 ") == ok && service.count() == 1 && service.deleteVehicle(admin, "MB-56-78") == ok && service.count() == 0 && service.listAll().empty(), "delete and count");
    if (failures) return 1;
    std::cout << "All foundation, vehicle service and fee tests passed.\n";
}

