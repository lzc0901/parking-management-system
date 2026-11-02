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
  check(ParseTimestamp(" 2024-02-29 09:30 ", canonical, now) &&
            canonical == "2024-02-29 09:30",
        "leap date and trimming");
  for (const auto* invalid :
       {"2026-02-29 09:30", "2026-04-31 09:30", "2026-11-02 24:00",
        "2026-11-02 09:60", "2026/11/02 09:30", "2026-11-02 09:30:00"}) {
    const auto before = now;
    const auto text = canonical;
    check(!ParseTimestamp(invalid, canonical, now) && now == before &&
              canonical == text,
          "invalid timestamp preserves output");
  }
  std::time_t minute{};
  check(NormalizeToMinute(now + 59, minute) && minute == now,
        "normalize seconds to minute");
  if (failures) return 1;
  std::cout << "Time tests passed.\n";
}
