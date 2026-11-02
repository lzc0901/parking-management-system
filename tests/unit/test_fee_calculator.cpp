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
#include <string>

#include "src/fee_calculator.h"
#include "src/vehicle.h"

int main() {
  int failures = 0;
  auto check = [&](bool condition, const char* name) {
    if (!condition) {
      std::cerr << "FAIL: " << name << '\n';
      ++failures;
    }
  };

  std::string canonical;
  std::time_t now{};
  check(ParseTimestamp("2026-11-02 09:30", canonical, now),
        "parse reference time");
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
  if (failures) return 1;
  std::cout << "Fee calculator tests passed.\n";
}
