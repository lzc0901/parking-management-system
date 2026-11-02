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

#ifndef SRC_STORAGE_H_
#define SRC_STORAGE_H_

#include <filesystem>
#include <string>

#include "src/parking_service.h"

// Reports the result and message of a storage operation.
struct StorageResult {
  bool ok;
  std::string message;
};

// Load before saving. A failed load or failed rollback blocks subsequent saves.
class Storage {
 public:
  explicit Storage(std::filesystem::path path = "parking_data.txt");
  // Loads records or recovers a backup; failure blocks saving.
  StorageResult Load(ParkingService& service);
  // Saves validated records using a recoverable temporary-file protocol.
  StorageResult Save(const ParkingService& service);

 private:
  std::filesystem::path path_;
  bool blocked_ = true;
};

#endif  // SRC_STORAGE_H_
