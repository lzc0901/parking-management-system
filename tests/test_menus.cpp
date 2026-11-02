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

#include <filesystem>
#include <iostream>
#include <sstream>
#include <string>

#include "src/menus.h"
#include "src/parking_service.h"
#include "src/storage.h"

int main() {
  auto path =
      std::filesystem::temp_directory_path() / "parking_menu_test_e05.txt";
  std::filesystem::remove(path);
  std::filesystem::remove(path.string() + ".bak");
  ParkingService service;
  Storage storage(path);
  if (!storage.Load(service).ok) return 1;
  std::istringstream input(
      "2\n4\nAA1\nSUV\nWhite\n2020-01-01 "
      "00:00\n5\nAA1\nBB2\nSedan\nRed\n2020-01-01 "
      "00:00\n0\n1\n1\n2\nBB2\n4\n0\n2\n6\nBB2\n0\n0\n");
  std::ostringstream output;
  RunMenu(input, output, service, storage);
  auto text = output.str();
  if (text.find("Updated and saved") == std::string::npos ||
      text.find("Plate: BB2") == std::string::npos ||
      text.find("Fee:") == std::string::npos ||
      text.find("Invalid choice.") == std::string::npos || service.Count() != 0)
    return 2;
  ParkingService reloaded;
  Storage reload(path);
  if (!reload.Load(reloaded).ok || reloaded.Count() != 0) return 3;
  Storage blocked(path);
  ParkingService unsaved;
  std::istringstream fail_input(
      "2\n4\nCC3\nSUV\nBlue\n2020-01-01 00:00\n0\n0\n");
  std::ostringstream fail_output;
  RunMenu(fail_input, fail_output, unsaved, blocked);
  if (fail_output.str().find("Updated in memory, but save failed") ==
          std::string::npos ||
      unsaved.Count() != 1)
    return 4;
  std::filesystem::remove(path);
  std::filesystem::remove(path.string() + ".bak");
  std::cout << "Menu workflow and save failure tests passed.\n";
}
