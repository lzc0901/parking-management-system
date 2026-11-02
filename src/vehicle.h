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

#ifndef SRC_VEHICLE_H_
#define SRC_VEHICLE_H_

#include <ctime>
#include <string>

// The entry timestamp is derived from the canonical local-time text.
struct VehicleRecord {
  std::string plate;
  std::string category;
  std::string color;
  std::string entry_time_text;
  std::time_t entry_timestamp{};
};

// Reject ASCII controls before trimming outer ASCII spaces.
bool NormalizeField(const std::string& input, bool plate, std::string& output);
bool ParseTimestamp(const std::string& input, std::string& canonical,
                    std::time_t& timestamp);
bool NormalizeToMinute(std::time_t input, std::time_t& output);

#endif  // SRC_VEHICLE_H_
