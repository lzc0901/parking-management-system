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

#include "src/vehicle.h"

#include <cstddef>
#include <ctime>
#include <string>

bool NormalizeField(const std::string& input, bool plate, std::string& output) {
  for (unsigned char c : input)
    if (c < 0x20 || c == 0x7f) return false;
  const auto first = input.find_first_not_of(' ');
  if (first == std::string::npos) return false;
  const auto value =
      input.substr(first, input.find_last_not_of(' ') - first + 1);
  if (plate && value.find(' ') != std::string::npos) return false;
  output = value;
  return true;
}

bool NormalizeToMinute(std::time_t input, std::time_t& output) {
  const std::tm* local = std::localtime(&input);
  if (!local) return false;
  std::tm parts = *local;
  parts.tm_sec = 0;
  const auto result = std::mktime(&parts);
  if (result == static_cast<std::time_t>(-1)) return false;
  output = result;
  return true;
}

bool ParseTimestamp(const std::string& input, std::string& canonical,
                    std::time_t& timestamp) {
  std::string value;
  if (!NormalizeField(input, false, value) || value.size() != 16 ||
      value[4] != '-' || value[7] != '-' || value[10] != ' ' ||
      value[13] != ':')
    return false;
  for (std::size_t i = 0; i < value.size(); ++i) {
    if (i == 4 || i == 7 || i == 10 || i == 13) continue;
    if (value[i] < '0' || value[i] > '9') return false;
  }
  auto number = [&](std::size_t pos, std::size_t count) {
    return std::stoi(value.substr(pos, count));
  };
  const int year = number(0, 4), month = number(5, 2), day = number(8, 2);
  const int hour = number(11, 2), minute = number(14, 2);
  if (year < 1 || month < 1 || month > 12 || day < 1 || hour > 23 ||
      minute > 59)
    return false;
  const int days[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
  const bool leap = year % 4 == 0 && (year % 100 != 0 || year % 400 == 0);
  if (day > days[month - 1] + (month == 2 && leap ? 1 : 0)) return false;
  std::tm parts{};
  parts.tm_year = year - 1900;
  parts.tm_mon = month - 1;
  parts.tm_mday = day;
  parts.tm_hour = hour;
  parts.tm_min = minute;
  parts.tm_isdst = -1;
  const auto result = std::mktime(&parts);
  if (result == static_cast<std::time_t>(-1) || parts.tm_year != year - 1900 ||
      parts.tm_mon != month - 1 || parts.tm_mday != day ||
      parts.tm_hour != hour || parts.tm_min != minute || parts.tm_sec != 0)
    return false;
  canonical = value;
  timestamp = result;
  return true;
}
