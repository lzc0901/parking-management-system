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

#include "src/ordinary_user.h"

#include <istream>
#include <ostream>
#include <sstream>
#include <string>

#include "src/menus.h"
#include "src/plate_recognition.h"

bool RunOrdinaryUserSession(std::istream& in, std::ostream& out,
                            const ParkingService& service,
                            PlateRecognizer* recognizer) {
  std::string choice;
  while (true) {
    out << "Ordinary user session: 1 = list, 2 = find, 3 = count";
    if (recognizer) out << ", 4 = image lookup";
    out << ", 0 = logout\n> ";
    if (!std::getline(in, choice)) return false;
    if (choice == "0") return true;
    if (choice == "4" && recognizer) {
      std::string plate;
      const auto result = ReadConfirmedPlate(in, out, *recognizer, plate);
      if (result == PlateInputResult::kEndOfInput) return false;
      if (result == PlateInputResult::kConfirmed) {
        std::istringstream plate_input(plate + "\n");
        RunReadChoice("2", plate_input, out, service);
      }
      continue;
    }
    if (!RunReadChoice(choice, in, out, service)) return false;
  }
}
