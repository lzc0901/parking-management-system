// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "src/storage.h"

#include <cstddef>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <system_error>
#include <utility>

#include "src/parking_service.h"
#include "src/vehicle.h"

namespace fs = std::filesystem;
namespace {
StorageResult ParseFile(const fs::path& path, ParkingService& result) {
  std::ifstream in(path, std::ios::binary);
  if (!in)
    return {false, "Cannot open " + path.string() + "; check permissions."};
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
    if (!std::getline(fields, record.plate, '\t') ||
        !std::getline(fields, record.category, '\t') ||
        !std::getline(fields, record.color, '\t') ||
        !std::getline(fields, record.entry_time_text, '\t') ||
        std::getline(fields, extra, '\t'))
      return {false, "Line " + std::to_string(number) +
                         ": expected four tab-separated fields."};
    std::string canonical;
    std::time_t timestamp{};
    if (!ParseTimestamp(record.entry_time_text, canonical, timestamp))
      return {false, "Line " + std::to_string(number) + ": invalid timestamp."};
    const auto status =
        candidate.RestoreVehicle(Role::kAdmin, record, timestamp);
    if (status != VehicleResult::kOk)
      return {false, "Line " + std::to_string(number) +
                         (status == VehicleResult::kDuplicatePlate
                              ? ": duplicate plate."
                              : ": invalid field or timestamp.")};
  }
  if (in.bad())
    return {false, "Error reading " + path.string() +
                       "; preserve the file and check the disk."};
  const bool collisions = candidate.HasPlateCollisions();
  result = std::move(candidate);
  return {true, "Loaded " + path.string() +
                    (collisions ? ". Warning: ambiguous historical plate "
                                  "matches; records were preserved."
                                : ".")};
}
}  // namespace
Storage::Storage(fs::path path) : path_(std::move(path)) {}
StorageResult Storage::Load(ParkingService& service) {
  blocked_ = true;
  const fs::path backup = path_.string() + ".bak";
  std::error_code ec;
  bool target_exists = fs::exists(path_, ec);
  if (ec) return {false, "Cannot inspect data path: " + ec.message()};
  bool backup_exists = fs::exists(backup, ec);
  if (ec) return {false, "Cannot inspect backup: " + ec.message()};
  if (!target_exists && !backup_exists) {
    service = ParkingService{};
    blocked_ = false;
    return {true, "First run: no parking data file."};
  }
  ParkingService candidate;
  auto result = ParseFile(target_exists ? path_ : backup, candidate);
  if (!result.ok) return result;
  if (!target_exists) {
    fs::rename(backup, path_, ec);
    if (ec)
      return {false, "Backup recovery failed; preserve " + backup.string() +
                         ": " + ec.message()};
    result.message = "Recovered parking data from " + backup.string() + ". " +
                     result.message;
  }
  service = std::move(candidate);
  blocked_ = false;
  return result;
}
StorageResult Storage::Save(const ParkingService& service) {
  if (blocked_)
    return {
        false,
        "Save blocked: resolve the load or recovery error and reload first."};
  const fs::path temp = path_.string() + ".tmp",
                 backup = path_.string() + ".bak";
  std::error_code ec;
  if (path_.has_parent_path()) fs::create_directories(path_.parent_path(), ec);
  if (ec) return {false, "Cannot create data directory: " + ec.message()};
  // Validate the complete snapshot before touching the existing file.
  ParkingService checked;
  for (const auto& record : service.ListAll()) {
    std::string canonical;
    std::time_t timestamp{};
    if (!ParseTimestamp(record.entry_time_text, canonical, timestamp) ||
        checked.RestoreVehicle(Role::kAdmin, record, timestamp) !=
            VehicleResult::kOk)
      return {false, "Invalid record; data file was not changed."};
  }
  std::ofstream out(temp, std::ios::binary | std::ios::trunc);
  if (!out)
    return {false, "Cannot write temporary file " + temp.string() +
                       "; check permissions."};
  out << "# PARKING_SYSTEM_V1\n";
  for (const auto& r : checked.ListAll())
    out << r.plate << '\t' << r.category << '\t' << r.color << '\t'
        << r.entry_time_text << '\n';
  out.flush();
  bool written = static_cast<bool>(out);
  out.close();
  written = written && !out.fail();
  if (!written) {
    fs::remove(temp, ec);
    return {false, "Writing temporary data failed; original retained."};
  }
  bool exists = fs::exists(path_, ec);
  if (ec) return {false, "Cannot inspect target: " + ec.message()};
  if (exists) {
    // Confirm the current file is valid before removing any older backup.
    ParkingService current;
    auto valid = ParseFile(path_, current);
    if (!valid.ok) {
      blocked_ = true;
      fs::remove(temp, ec);
      return {false,
              "Existing file is invalid; save blocked. " + valid.message};
    }
    fs::remove(backup, ec);
    if (ec)
      return {false,
              "Cannot remove old backup; original retained: " + ec.message()};
    fs::rename(path_, backup, ec);
    if (ec) return {false, "Cannot back up original file: " + ec.message()};
  } else {
    bool backup_exists = fs::exists(backup, ec);
    if (ec || backup_exists) {
      blocked_ = true;
      return {false,
              "Backup exists or cannot be inspected; reload to recover before "
              "saving."};
    }
  }
  fs::rename(temp, path_, ec);
  if (ec) {
    const auto reason = ec.message();
    if (exists) {
      std::error_code restore;
      fs::rename(backup, path_, restore);
      if (restore) {
        blocked_ = true;
        return {false, "Replacement and rollback failed; preserve backup " +
                           backup.string() + ": " + restore.message()};
      }
    }
    return {false, "Replacement failed; previous data retained: " + reason};
  }
  if (exists) fs::remove(backup, ec);
  return {true, ec ? "Saved; old backup could not be removed: " + ec.message()
                   : "Parking data saved."};
}
