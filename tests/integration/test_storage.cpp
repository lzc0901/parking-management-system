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
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

#include "src/parking_service.h"
#include "src/storage.h"
#include "src/vehicle.h"
namespace fs = std::filesystem;
int main() {
  int failures = 0;
  auto check = [&](bool b, const char* name) {
    if (!b) {
      std::cerr << "FAIL: " << name << '\n';
      ++failures;
    }
  };
  auto dir = fs::temp_directory_path() /
             ("parking-test-" + std::to_string(std::time(nullptr)));
  fs::create_directories(dir);
  auto path = dir / "data.txt";
  ParkingService service;
  Storage storage(path);
  check(storage.Load(service).ok && service.Count() == 0, "first run");
  std::string canonical;
  std::time_t now{};
  ParseTimestamp("2026-11-02 09:30", canonical, now);
  VehicleRecord a{"MA-12-34", u8"轎車", u8"白色", "2026-11-01 08:30", {}};
  VehicleRecord b{"MB-56-78", "SUV", "Black", "2026-11-01 09:30", {}};
  check(service.AddVehicle(Role::kAdmin, a, now) == VehicleResult::kOk &&
            service.AddVehicle(Role::kAdmin, b, now) == VehicleResult::kOk &&
            storage.Save(service).ok,
        "save additions");
  a.color = u8"黑色";
  check(
      service.ModifyVehicle(Role::kAdmin, a.plate, a, now) ==
              VehicleResult::kOk &&
          service.DeleteVehicle(Role::kAdmin, b.plate) == VehicleResult::kOk &&
          storage.Save(service).ok,
      "save edits and deletion");
  ParkingService reloaded;
  Storage restart(path);
  check(
      restart.Load(reloaded).ok && reloaded.Count() == 1 &&
          reloaded.FindByPlate(a.plate) &&
          reloaded.FindByPlate(a.plate)->color == a.color &&
          reloaded.FindByPlate(a.plate)->category == a.category &&
          reloaded.FindByPlate(a.plate)->entry_time_text == a.entry_time_text &&
          !reloaded.FindByPlate(b.plate),
      "restart round trip");
  fs::rename(path, path.string() + ".bak");
  ParkingService recovered;
  Storage recovery(path);
  check(
      recovery.Load(recovered).ok && recovered.Count() == 1 && fs::exists(path),
      "interrupted save recovery");
  {
    std::ofstream out(path, std::ios::binary | std::ios::trunc);
    out << "# PARKING_SYSTEM_V1\nMA\tSUV\tWhite\t2026-11-01 08:30\nbad line\n";
  }
  Storage broken(path);
  auto result = broken.Load(recovered);
  check(!result.ok && result.message.find("Line 3") != std::string::npos &&
            recovered.Count() == 1 && !broken.Save(recovered).ok,
        "malformed file blocks save without partial load");
  {
    std::ofstream out(path, std::ios::binary | std::ios::trunc);
    out << "MA\tSUV\tWhite\t2026-11-01 08:30\nMA\tSUV\tBlack\t2026-11-01 "
           "08:30\n";
  }
  Storage duplicate(path);
  result = duplicate.Load(recovered);
  check(!result.ok && result.message.find("duplicate") != std::string::npos &&
            !duplicate.Save(recovered).ok,
        "duplicate blocks save");
  {
    std::ofstream out(path, std::ios::binary | std::ios::trunc);
    out << "MA-12-34\tSUV\tWhite\t2026-11-01 08:30\n"
           "ma1234\tSUV\tBlack\t2026-11-01 08:30\n";
  }
  Storage collision(path);
  ParkingService preserved;
  VehicleFeeView ambiguous;
  const auto collision_load = collision.Load(preserved);
  check(collision_load.ok &&
            collision_load.message.find("ambiguous") != std::string::npos &&
            preserved.Count() == 2 &&
            preserved.FindWithFee("MA 12 34", now, ambiguous) ==
                VehicleResult::kAmbiguousPlate &&
            collision.Save(preserved).ok,
        "historical normalized collision retained and reported");
  ParkingService again;
  Storage collision_restart(path);
  check(collision_restart.Load(again).ok && again.Count() == 2 &&
            again.FindWithFee("ma1234", now, ambiguous) ==
                VehicleResult::kAmbiguousPlate,
        "collision survives save and reload");
  fs::remove(path);
  {
    std::ofstream out(path.string() + ".bak");
    out << "bad\n";
  }
  Storage bad_backup(path);
  check(!bad_backup.Load(recovered).ok && fs::exists(path.string() + ".bak") &&
            !bad_backup.Save(recovered).ok,
        "invalid backup preserved");
  auto blocker = dir / "not-directory";
  {
    std::ofstream out(blocker);
    out << "x";
  }
  Storage unwritable(blocker / "data.txt");
  ParkingService empty;
  unwritable.Load(empty);
  check(!unwritable.Save(empty).ok, "unusable path blocks save");
  fs::remove_all(dir);
  if (failures) return 1;
  std::cout << "All persistence and recovery tests passed.\n";
}
