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
#include <istream>
#include <ostream>
#include <string>

#include "src/menus.h"
#include "src/parking_service.h"
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
    case VehicleResult::kNotFound:
      return "Vehicle not found.";
    case VehicleResult::kOk:
      return "Updated.";
  }
  return "Operation failed.";
}
bool RunAdminUserSession(std::istream& in, std::ostream& out,
                         ParkingService& service, Storage& storage) {
  std::string choice;
  while (true) {
    out << "Administrator session: 1 = list, 2 = find, 3 = count, 4 = add, 5 = "
           "modify, 6 = delete, 0 = logout\n> ";
    if (!std::getline(in, choice)) return false;
    if (choice == "0") return true;
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
