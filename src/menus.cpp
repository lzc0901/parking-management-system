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

#include "src/menus.h"

#include <ctime>
#include <istream>
#include <ostream>
#include <string>
#include <vector>

#include "src/admin_user.h"
#include "src/ordinary_user.h"
#include "src/parking_service.h"
#include "src/vehicle.h"

static void Show(const VehicleFeeView& view, std::ostream& out) {
  const auto& v = view.vehicle;
  const auto& f = view.fee;
  out << "Plate: " << v.plate << " | Category: " << v.category
      << " | Color: " << v.color << " | Entry: " << v.entry_time_text
      << " | Duration: " << f.days << " days " << f.hours << " hours "
      << f.minutes << " minutes | Fee: " << f.mop << " MOP\n";
}

bool RunReadChoice(const std::string& choice, std::istream& in,
                   std::ostream& out, const ParkingService& service) {
  if (choice == "3") {
    out << "Current count: " << service.Count() << '\n';
    return true;
  }
  if (choice == "1") {
    std::vector<VehicleFeeView> rows;
    if (service.ListWithFees(std::time(nullptr), rows) != VehicleResult::kOk) {
      out << "Unable to calculate parking durations.\n";
    } else {
      if (rows.empty()) out << "No vehicles.\n";
      for (const auto& row : rows) Show(row, out);
    }
    out << "Current count: " << service.Count() << '\n';
    return true;
  }
  if (choice == "2") {
    std::string plate;
    out << "Plate: ";
    if (!std::getline(in, plate)) return false;
    VehicleFeeView row;
    const auto result = service.FindWithFee(plate, std::time(nullptr), row);
    if (result == VehicleResult::kNotFound)
      out << "Vehicle not found.\n";
    else if (result != VehicleResult::kOk)
      out << "Unable to retrieve vehicle or calculate duration.\n";
    else
      Show(row, out);
    out << "Current count: " << service.Count() << '\n';
    return true;
  }
  out << "Invalid choice.\n";
  return true;
}

void RunMenu(std::istream& in, std::ostream& out, ParkingService& service,
             Storage& storage) {
  std::string choice;
  out << "Fee: 20 MOP per started hour, capped at 400 MOP per successive "
         "24-hour block.\n";
  while (true) {
    out << "Parking system: 1 = ordinary, 2 = administrator, 0 = exit\n> ";
    if (!std::getline(in, choice) || choice == "0") return;
    if (choice == "1") {
      if (!RunOrdinaryUserSession(in, out, service)) return;
    } else if (choice == "2") {
      if (!RunAdminUserSession(in, out, service, storage)) return;
    } else {
      out << "Invalid choice.\n";
    }
  }
}
