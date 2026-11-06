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

#include <iostream>
#include <string>

#include "src/vehicle.h"

int main() {
  int failures = 0;
  auto check = [&](bool condition, const char* name) {
    if (!condition) {
      std::cerr << "FAIL: " << name << '\n';
      ++failures;
    }
  };

  VehicleRecord record{"MA-12-34", "Sedan", "White", "2026-11-02 09:30", {}};
  check(record.plate == "MA-12-34", "vehicle record");

  std::string value = "unchanged";
  check(NormalizeField(" Model Y ", false, value) && value == "Model Y",
        "trim field spaces");
  check(NormalizeField(" MA-12-34 ", true, value) && value == "MA-12-34",
        "normalize plate");
  for (const auto& invalid : {std::string("   "), std::string("\tWhite"),
                              std::string("Wh\nite"), std::string(1, '\x7f')}) {
    value = "unchanged";
    check(!NormalizeField(invalid, false, value) && value == "unchanged",
          "invalid fields preserve output");
  }
  check(NormalizeField("ma 12", true, value) && value == "ma 12",
        "plate spelling retained");
  check(PlateComparisonKey("ma-12 34", value) && value == "MA1234",
        "plate comparison key");
  check(PlateComparisonKey("O-01", value) && value == "O01",
        "letter O is not replaced by zero");
  if (failures) return 1;
  std::cout << "Vehicle tests passed.\n";
}
