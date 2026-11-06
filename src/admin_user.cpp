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

#include "src/admin_user.h"

#include <ctime>
#include <iomanip>
#include <istream>
#include <ostream>
#include <sstream>
#include <string>

#include "src/menus.h"
#include "src/parking_service.h"
#include "src/plate_recognition.h"
#include "src/storage.h"
#include "src/vehicle.h"

static bool ReadRecord(std::istream& in, std::ostream& out,
                       VehicleRecord& record) {
  out << "Plate: ";
  if (!std::getline(in, record.plate)) return false;
  out << "Category: ";
  if (!std::getline(in, record.category)) return false;
  out << "Color: ";
  if (!std::getline(in, record.color)) return false;
  out << "Entry time (YYYY-MM-DD HH:MM): ";
  return static_cast<bool>(std::getline(in, record.entry_time_text));
}
static const char* Message(VehicleResult result) {
  switch (result) {
    case VehicleResult::kForbidden:
      return "Permission denied.";
    case VehicleResult::kInvalidField:
      return "Invalid required field.";
    case VehicleResult::kInvalidTime:
      return "Invalid entry time.";
    case VehicleResult::kFutureTime:
      return "Entry time is in the future.";
    case VehicleResult::kDuplicatePlate:
      return "Plate already exists.";
    case VehicleResult::kAmbiguousPlate:
      return "Ambiguous plate: multiple historical records match.";
    case VehicleResult::kNotFound:
      return "Vehicle not found.";
    case VehicleResult::kOk:
      return "Updated.";
  }
  return "Operation failed.";
}
bool RunAdminUserSession(std::istream& in, std::ostream& out,
                         ParkingService& service, Storage& storage,
                         PlateRecognizer* recognizer) {
  std::string choice;
  while (true) {
    out << "Administrator session: 1 = list, 2 = find, 3 = count, 4 = add, 5 = "
           "modify, 6 = delete";
    if (recognizer) out << ", 7 = image registration";
    out << ", 0 = logout\n> ";
    if (!std::getline(in, choice)) return false;
    if (choice == "0") return true;
    if (choice == "7" && recognizer) {
      VehicleRecord record;
      const auto selected =
          ReadConfirmedPlate(in, out, *recognizer, record.plate);
      if (selected == PlateInputResult::kEndOfInput) return false;
      if (selected == PlateInputResult::kCancelled) continue;
      out << "Category: ";
      if (!std::getline(in, record.category)) return false;
      out << "Color: ";
      if (!std::getline(in, record.color)) return false;
      out << "Entry time (YYYY-MM-DD HH:MM; empty = current time): ";
      if (!std::getline(in, record.entry_time_text)) return false;
      out << "Confirm registration of " << record.plate << " | "
          << record.category << " | " << record.color << " | "
          << (record.entry_time_text.empty() ? "current time"
                                             : record.entry_time_text)
          << "? (y/n): ";
      if (!std::getline(in, choice)) return false;
      if (choice != "y" && choice != "Y") continue;
      const auto now = std::time(nullptr);
      if (record.entry_time_text.empty()) {
        const auto* local = std::localtime(&now);
        if (!local) {
          out << "Unable to read local time.\n";
          continue;
        }
        std::ostringstream formatted;
        formatted << std::put_time(local, "%Y-%m-%d %H:%M");
        record.entry_time_text = formatted.str();
      }
      const auto result = service.AddVehicle(Role::kAdmin, record, now);
      if (result != VehicleResult::kOk) {
        out << Message(result) << '\n';
        continue;
      }
      const auto saved = storage.Save(service);
      if (saved.ok)
        out << "Updated and saved. " << saved.message << '\n';
      else
        out << "Updated in memory, but save failed: " << saved.message << '\n';
      continue;
    }
    if (choice != "4" && choice != "5" && choice != "6") {
      if (!RunReadChoice(choice, in, out, service)) return false;
      continue;
    }
    std::string plate;
    VehicleRecord record;
    VehicleResult result;
    if (choice != "4") {
      out << "Existing plate: ";
      if (!std::getline(in, plate)) return false;
    }
    if (choice != "6" && !ReadRecord(in, out, record)) return false;
    if (choice == "4")
      result = service.AddVehicle(Role::kAdmin, record, std::time(nullptr));
    else if (choice == "5")
      result = service.ModifyVehicle(Role::kAdmin, plate, record,
                                     std::time(nullptr));
    else
      result = service.DeleteVehicle(Role::kAdmin, plate);
    if (result != VehicleResult::kOk) {
      out << Message(result) << '\n';
      continue;
    }
    const auto saved = storage.Save(service);
    if (saved.ok)
      out << "Updated and saved. " << saved.message << '\n';
    else
      out << "Updated in memory, but save failed: " << saved.message << '\n';
  }
}
