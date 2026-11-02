#include "storage.h"
#include <fstream>
#include <sstream>
#include <utility>

namespace fs = std::filesystem;
namespace {
StorageResult parseFile(const fs::path& path, ParkingService& result) {
    std::ifstream in(path, std::ios::binary);
    if (!in) return {false, "Cannot open " + path.string() + "; check permissions."};
    ParkingService candidate;
    std::string line;
    std::size_t number = 0;
    while (std::getline(in, line)) {
        ++number;
        if (!line.empty() && line.back() == '\r') line.pop_back();
        const auto first = line.find_first_not_of(" \t");
        if (first == std::string::npos || line[first] == '#') continue;
        std::istringstream fields(line);
        VehicleRecord record;
        std::string extra;
        if (!std::getline(fields, record.plate, '\t') || !std::getline(fields, record.category, '\t') ||
            !std::getline(fields, record.color, '\t') || !std::getline(fields, record.entryTimeText, '\t') ||
            std::getline(fields, extra, '\t'))
            return {false, "Line " + std::to_string(number) + ": expected four tab-separated fields."};
        std::string canonical;
        std::time_t timestamp{};
        if (!parseTimestamp(record.entryTimeText, canonical, timestamp))
            return {false, "Line " + std::to_string(number) + ": invalid timestamp."};
        const auto status = candidate.addVehicle(Role::Admin, record, timestamp);
        if (status != VehicleResult::Ok)
            return {false, "Line " + std::to_string(number) + (status == VehicleResult::DuplicatePlate ? ": duplicate plate." : ": invalid field or timestamp.")};
    }
    if (in.bad()) return {false, "Error reading " + path.string() + "; preserve the file and check the disk."};
    result = std::move(candidate);
    return {true, "Loaded " + path.string() + "."};
}
}
Storage::Storage(fs::path path) : path_(std::move(path)) {}
StorageResult Storage::load(ParkingService& service) {
    blocked_ = true;
    const fs::path backup = path_.string() + ".bak";
    std::error_code ec;
    bool targetExists = fs::exists(path_, ec);
    if (ec) return {false, "Cannot inspect data path: " + ec.message()};
    bool backupExists = fs::exists(backup, ec);
    if (ec) return {false, "Cannot inspect backup: " + ec.message()};
    if (!targetExists && !backupExists) { service = ParkingService{}; blocked_ = false; return {true, "First run: no parking data file."}; }
    ParkingService candidate;
    auto result = parseFile(targetExists ? path_ : backup, candidate);
    if (!result.ok) return result;
    if (!targetExists) {
        fs::rename(backup, path_, ec);
        if (ec) return {false, "Backup recovery failed; preserve " + backup.string() + ": " + ec.message()};
        result.message = "Recovered parking data from " + backup.string() + ".";
    }
    service = std::move(candidate);
    blocked_ = false;
    return result;
}
StorageResult Storage::save(const ParkingService& service) {
    if (blocked_) return {false, "Save blocked: resolve the load or recovery error and reload first."};
    const fs::path temp = path_.string() + ".tmp", backup = path_.string() + ".bak";
    std::error_code ec;
    if (path_.has_parent_path()) fs::create_directories(path_.parent_path(), ec);
    if (ec) return {false, "Cannot create data directory: " + ec.message()};
    // Validate the complete snapshot before touching the existing file.
    ParkingService checked;
    for (const auto& record : service.listAll()) {
        std::string canonical; std::time_t timestamp{};
        if (!parseTimestamp(record.entryTimeText, canonical, timestamp) ||
            checked.addVehicle(Role::Admin, record, timestamp) != VehicleResult::Ok)
            return {false, "Invalid record; data file was not changed."};
    }
    std::ofstream out(temp, std::ios::binary | std::ios::trunc);
    if (!out) return {false, "Cannot write temporary file " + temp.string() + "; check permissions."};
    out << "# PARKING_SYSTEM_V1\n";
    for (const auto& r : checked.listAll()) out << r.plate << '\t' << r.category << '\t' << r.color << '\t' << r.entryTimeText << '\n';
    out.flush();
    bool written = static_cast<bool>(out);
    out.close(); written = written && !out.fail();
    if (!written) { fs::remove(temp, ec); return {false, "Writing temporary data failed; original retained."}; }
    bool exists = fs::exists(path_, ec);
    if (ec) return {false, "Cannot inspect target: " + ec.message()};
    if (exists) {
        // Confirm the current file is valid before removing any older backup.
        ParkingService current;
        auto valid = parseFile(path_, current);
        if (!valid.ok) { blocked_ = true; fs::remove(temp, ec); return {false, "Existing file is invalid; save blocked. " + valid.message}; }
        fs::remove(backup, ec);
        if (ec) return {false, "Cannot remove old backup; original retained: " + ec.message()};
        fs::rename(path_, backup, ec);
        if (ec) return {false, "Cannot back up original file: " + ec.message()};
    } else {
        bool backupExists = fs::exists(backup, ec);
        if (ec || backupExists) { blocked_ = true; return {false, "Backup exists or cannot be inspected; reload to recover before saving."}; }
    }
    fs::rename(temp, path_, ec);
    if (ec) {
        const auto reason = ec.message();
        if (exists) {
            std::error_code restore;
            fs::rename(backup, path_, restore);
            if (restore) { blocked_ = true; return {false, "Replacement and rollback failed; preserve backup " + backup.string() + ": " + restore.message()}; }
        }
        return {false, "Replacement failed; previous data retained: " + reason};
    }
    if (exists) fs::remove(backup, ec);
    return {true, ec ? "Saved; old backup could not be removed: " + ec.message() : "Parking data saved."};
}
