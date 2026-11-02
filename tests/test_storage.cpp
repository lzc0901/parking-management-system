#include "storage.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
namespace fs = std::filesystem;
int main() {
    int failures = 0;
    auto check = [&](bool b, const char* name) { if (!b) { std::cerr << "FAIL: " << name << '\n'; ++failures; } };
    auto dir = fs::temp_directory_path() / ("parking-test-" + std::to_string(std::time(nullptr)));
    fs::create_directories(dir);
    auto path = dir / "data.txt";
    ParkingService service; Storage storage(path);
    check(storage.load(service).ok && service.count() == 0, "first run");
    std::string canonical; std::time_t now{}; parseTimestamp("2026-11-02 09:30", canonical, now);
    VehicleRecord a{"MA-12-34", u8"轎車", u8"白色", "2026-11-01 08:30", {}};
    VehicleRecord b{"MB-56-78", "SUV", "Black", "2026-11-01 09:30", {}};
    check(service.addVehicle(Role::Admin, a, now) == VehicleResult::Ok && service.addVehicle(Role::Admin, b, now) == VehicleResult::Ok && storage.save(service).ok, "save additions");
    a.color = u8"黑色";
    check(service.modifyVehicle(Role::Admin, a.plate, a, now) == VehicleResult::Ok && service.deleteVehicle(Role::Admin, b.plate) == VehicleResult::Ok && storage.save(service).ok, "save edits and deletion");
    ParkingService reloaded; Storage restart(path);
    check(restart.load(reloaded).ok && reloaded.count() == 1 && reloaded.findByPlate(a.plate) && reloaded.findByPlate(a.plate)->color == a.color && reloaded.findByPlate(a.plate)->category == a.category && reloaded.findByPlate(a.plate)->entryTimeText == a.entryTimeText && !reloaded.findByPlate(b.plate), "restart round trip");
    fs::rename(path, path.string() + ".bak");
    ParkingService recovered; Storage recovery(path);
    check(recovery.load(recovered).ok && recovered.count() == 1 && fs::exists(path), "interrupted save recovery");
    { std::ofstream out(path, std::ios::binary | std::ios::trunc); out << "# PARKING_SYSTEM_V1\nMA\tSUV\tWhite\t2026-11-01 08:30\nbad line\n"; }
    Storage broken(path); auto result = broken.load(recovered);
    check(!result.ok && result.message.find("Line 3") != std::string::npos && recovered.count() == 1 && !broken.save(recovered).ok, "malformed file blocks save without partial load");
    { std::ofstream out(path, std::ios::binary | std::ios::trunc); out << "MA\tSUV\tWhite\t2026-11-01 08:30\nMA\tSUV\tBlack\t2026-11-01 08:30\n"; }
    Storage duplicate(path); result = duplicate.load(recovered);
    check(!result.ok && result.message.find("duplicate") != std::string::npos && !duplicate.save(recovered).ok, "duplicate blocks save");
    fs::remove(path); { std::ofstream out(path.string() + ".bak"); out << "bad\n"; }
    Storage badBackup(path); check(!badBackup.load(recovered).ok && fs::exists(path.string() + ".bak") && !badBackup.save(recovered).ok, "invalid backup preserved");
    auto blocker = dir / "not-directory"; { std::ofstream out(blocker); out << "x"; }
    Storage unwritable(blocker / "data.txt"); ParkingService empty;
    unwritable.load(empty); check(!unwritable.save(empty).ok, "unusable path blocks save");
    fs::remove_all(dir);
    if (failures) return 1;
    std::cout << "All persistence and recovery tests passed.\n";
}

