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

#ifndef SRC_PARKING_SERVICE_H_
#define SRC_PARKING_SERVICE_H_

#include <cstddef>
#include <ctime>
#include <map>
#include <string>
#include <vector>

#include "src/fee_calculator.h"
#include "src/vehicle.h"

enum class Role { kOrdinaryUser, kAdmin };
enum class VehicleResult {
  kOk,
  kForbidden,
  kInvalidField,
  kInvalidTime,
  kFutureTime,
  kDuplicatePlate,
  kAmbiguousPlate,
  kNotFound
};
// Pairs a vehicle snapshot with its fee at a common reference time.
struct VehicleFeeView {
  VehicleRecord vehicle;
  ParkingFee fee;
};

// In-memory operations; persistence and console presentation are separate.
class ParkingService {
 public:
  // Validates and adds a record for an administrator.
  VehicleResult AddVehicle(Role role, const VehicleRecord& record,
                           std::time_t reference);
  // Storage restoration preserves separately spelled historical records.
  VehicleResult RestoreVehicle(Role role, const VehicleRecord& record,
                               std::time_t reference);
  bool HasPlateCollisions() const;
  // Replaces a record identified by plate after validation.
  VehicleResult ModifyVehicle(Role role, const std::string& plate,
                              const VehicleRecord& record,
                              std::time_t reference);
  // Removes the matching record for an administrator.
  VehicleResult DeleteVehicle(Role role, const std::string& plate);
  // Returns a pointer to the record, or nullptr if no valid plate matches.
  // The pointer is invalidated when the record is modified or removed.
  const VehicleRecord* FindByPlate(const std::string& plate) const;
  // Returns a snapshot sorted by plate.
  std::vector<VehicleRecord> ListAll() const;
  // A failed query leaves its output unchanged; all rows share one reference.
  VehicleResult FindWithFee(const std::string& plate, std::time_t reference,
                            VehicleFeeView& output) const;
  VehicleResult ListWithFees(std::time_t reference,
                             std::vector<VehicleFeeView>& output) const;
  std::size_t Count() const { return records_.size(); }

 private:
  VehicleResult ResolvePlate(const std::string& plate, std::string& key) const;
  std::map<std::string, VehicleRecord> records_;
};

#endif  // SRC_PARKING_SERVICE_H_
