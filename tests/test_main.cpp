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

#include <ctime>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "src/fee_calculator.h"
#include "src/menus.h"
#include "src/parking_service.h"
#include "src/storage.h"
#include "src/vehicle.h"

int main() {
  int failures = 0;
  auto check = [&](bool condition, const char* name) {
    if (!condition) {
      std::cerr << "FAIL: " << name << '\n';
      ++failures;
    }
  };

  std::istringstream input("invalid\n1\ninvalid\n0\n2\n0\n0\n");
  std::ostringstream output;
  ParkingService menu_service;
  Storage menu_storage;
  RunMenu(input, output, menu_service, menu_storage);
  const auto text = output.str();
  check(text.find("Invalid choice.") != std::string::npos, "invalid input");
  check(text.find("Ordinary user session") != std::string::npos,
        "ordinary session");
  check(text.find("Administrator session") != std::string::npos,
        "administrator session after logout");

  std::istringstream eof("1\n");
  std::ostringstream eof_output;
  RunMenu(eof, eof_output, menu_service, menu_storage);
  check(eof_output.str().find("Ordinary user session") != std::string::npos,
        "EOF in session");
  VehicleRecord record{"MA-12-34", "Sedan", "White", "2026-11-02 09:30", {}};
  check(record.plate == "MA-12-34", "vehicle record");

  std::string canonical;
  std::time_t now{};
  check(ParseTimestamp("2026-11-02 09:30", canonical, now),
        "parse reference time");
  ParkingService service;
  const auto admin = Role::kAdmin;
  const auto ordinary = Role::kOrdinaryUser;
  const auto ok = VehicleResult::kOk;
  struct FeeCase {
    int minutes;
    int mop;
  };
  const FeeCase fee_cases[] = {{0, 0},      {1, 20},     {60, 20},
                               {61, 40},    {1199, 400}, {1200, 400},
                               {1439, 400}, {1440, 400}, {1441, 420},
                               {2640, 800}, {2880, 800}, {2881, 820}};
  for (const auto& test : fee_cases) {
    ParkingFee fee;
    check(CalculateParkingFee(now, now + test.minutes * 60 + 59, fee) &&
              fee.total_minutes == test.minutes && fee.mop == test.mop &&
              fee.days == test.minutes / 1440 &&
              fee.hours == (test.minutes % 1440) / 60 &&
              fee.minutes == test.minutes % 60,
          "fee boundary and duration");
  }
  ParkingFee unchanged;
  unchanged.mop = 123;
  check(!CalculateParkingFee(now, now - 60, unchanged) && unchanged.mop == 123,
        "negative fee rejected without output mutation");
  check(service.Count() == 0 && service.ListAll().empty() &&
            !service.FindByPlate("unknown"),
        "empty lot");
  VehicleRecord a{" MA-12-34 ", " Model Y ", " Pearl White ",
                  " 2024-02-29 09:30 ", 42};
  VehicleRecord b{"MB-56-78", "SUV", "Black", "2026-11-02 09:30", 0};
  check(service.AddVehicle(ordinary, a, now) == VehicleResult::kForbidden &&
            service.Count() == 0,
        "ordinary cannot add");
  check(service.AddVehicle(admin, b, now + 59) == ok &&
            service.AddVehicle(admin, a, now) == ok,
        "add and same minute");
  VehicleFeeView view;
  std::vector<VehicleFeeView> views;
  check(service.ListWithFees(now + 61 * 60 + 59, views) == ok &&
            views.size() == 2 && views[1].vehicle.plate == "MB-56-78" &&
            views[1].fee.total_minutes == 61 && views[1].fee.mop == 40,
        "list fee query sorted and minute normalized");
  check(service.FindWithFee(" MB-56-78 ", now + 61 * 60, view) == ok &&
            view.fee.mop == 40,
        "single fee query");
  check(service.FindWithFee("missing", now, view) == VehicleResult::kNotFound &&
            view.fee.mop == 40 &&
            service.ListWithFees(now - 60, views) ==
                VehicleResult::kInvalidTime &&
            views.size() == 2 && service.Count() == 2,
        "failed fee queries preserve output and vehicle state");
  auto list = service.ListAll();
  check(service.Count() == 2 && list.size() == 2 &&
            list[0].plate == "MA-12-34" && list[1].plate == "MB-56-78",
        "count and sorted listing");
  const auto found = service.FindByPlate(" MA-12-34 ");
  check(found && found->category == "Model Y" &&
            found->color == "Pearl White" &&
            found->entry_time_text == "2024-02-29 09:30" &&
            found->entry_timestamp != 42,
        "normalized fields and derived time");
  check(service.AddVehicle(admin, a, now) == VehicleResult::kDuplicatePlate,
        "duplicate rejected");
  auto invalid = a;
  invalid.plate = "MA 12";
  check(service.AddVehicle(admin, invalid, now) == VehicleResult::kInvalidField,
        "plate space rejected");
  invalid = a;
  invalid.category = "   ";
  check(service.AddVehicle(admin, invalid, now) == VehicleResult::kInvalidField,
        "empty field rejected");
  invalid = a;
  invalid.color = "\tWhite";
  check(service.AddVehicle(admin, invalid, now) == VehicleResult::kInvalidField,
        "outer control rejected");
  invalid = a;
  invalid.entry_time_text = "2026-02-29 09:30";
  check(service.AddVehicle(admin, invalid, now) == VehicleResult::kInvalidTime,
        "invalid leap date");
  invalid.entry_time_text = "2026-04-31 09:30";
  check(service.AddVehicle(admin, invalid, now) == VehicleResult::kInvalidTime,
        "invalid calendar date");
  invalid.entry_time_text = "2026-11-02 09:31";
  check(service.AddVehicle(admin, invalid, now + 59) ==
            VehicleResult::kFutureTime,
        "future minute rejected");
  check(service.ModifyVehicle(admin, "MA-12-34", b, now) ==
            VehicleResult::kDuplicatePlate,
        "rename collision");
  check(service.ModifyVehicle(admin, "MA-12-34", invalid, now) ==
            VehicleResult::kFutureTime,
        "invalid edit rejected");
  check(service.ModifyVehicle(ordinary, "MA-12-34", a, now) ==
            VehicleResult::kForbidden,
        "ordinary cannot edit");
  check(service.ModifyVehicle(admin, "missing", a, now) ==
            VehicleResult::kNotFound,
        "missing edit");
  check(service.Count() == 2 && service.FindByPlate("MA-12-34") &&
            service.FindByPlate("MB-56-78"),
        "rejections preserve state");
  a.plate = "MC-00-01";
  a.color = "Red";
  check(service.ModifyVehicle(admin, "MA-12-34", a, now) == ok &&
            !service.FindByPlate("MA-12-34") &&
            service.FindByPlate("MC-00-01")->color == "Red" &&
            service.Count() == 2,
        "successful rename and edit");
  check(
      service.DeleteVehicle(ordinary, "MC-00-01") ==
              VehicleResult::kForbidden &&
          service.DeleteVehicle(admin, "missing") == VehicleResult::kNotFound &&
          service.Count() == 2,
      "delete rejection");
  check(service.DeleteVehicle(admin, " MC-00-01 ") == ok &&
            service.Count() == 1 &&
            service.DeleteVehicle(admin, "MB-56-78") == ok &&
            service.Count() == 0 && service.ListAll().empty(),
        "delete and count");
  if (failures) return 1;
  std::cout << "All foundation, vehicle service and fee tests passed.\n";
}
