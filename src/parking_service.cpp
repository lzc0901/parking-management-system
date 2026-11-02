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

#include "src/parking_service.h"

#include <cstddef>
#include <ctime>
#include <map>
#include <string>
#include <vector>

namespace {
VehicleResult Validated(const VehicleRecord& input, std::time_t reference,
                        VehicleRecord& output) {
  VehicleRecord candidate;
  if (!NormalizeField(input.plate, true, candidate.plate) ||
      !NormalizeField(input.category, false, candidate.category) ||
      !NormalizeField(input.color, false, candidate.color))
    return VehicleResult::kInvalidField;
  if (!ParseTimestamp(input.entry_time_text, candidate.entry_time_text,
                      candidate.entry_timestamp))
    return VehicleResult::kInvalidTime;
  std::time_t minute;
  if (!NormalizeToMinute(reference, minute)) return VehicleResult::kInvalidTime;
  if (candidate.entry_timestamp > minute) return VehicleResult::kFutureTime;
  output = candidate;
  return VehicleResult::kOk;
}
}  // namespace

VehicleResult ParkingService::AddVehicle(Role role, const VehicleRecord& record,
                                         std::time_t reference) {
  if (role != Role::kAdmin) return VehicleResult::kForbidden;
  VehicleRecord candidate;
  const auto result = Validated(record, reference, candidate);
  if (result != VehicleResult::kOk) return result;
  if (records_.count(candidate.plate)) return VehicleResult::kDuplicatePlate;
  records_.emplace(candidate.plate, candidate);
  return VehicleResult::kOk;
}

VehicleResult ParkingService::ModifyVehicle(Role role, const std::string& plate,
                                            const VehicleRecord& record,
                                            std::time_t reference) {
  if (role != Role::kAdmin) return VehicleResult::kForbidden;
  std::string key;
  if (!NormalizeField(plate, true, key)) return VehicleResult::kInvalidField;
  auto current = records_.find(key);
  if (current == records_.end()) return VehicleResult::kNotFound;
  VehicleRecord candidate;
  const auto result = Validated(record, reference, candidate);
  if (result != VehicleResult::kOk) return result;
  if (candidate.plate != key && records_.count(candidate.plate))
    return VehicleResult::kDuplicatePlate;
  if (candidate.plate == key) {
    current->second = candidate;
  } else {
    records_.emplace(candidate.plate, candidate);
    records_.erase(current);
  }
  return VehicleResult::kOk;
}

VehicleResult ParkingService::DeleteVehicle(Role role,
                                            const std::string& plate) {
  if (role != Role::kAdmin) return VehicleResult::kForbidden;
  std::string key;
  if (!NormalizeField(plate, true, key)) return VehicleResult::kInvalidField;
  return records_.erase(key) ? VehicleResult::kOk : VehicleResult::kNotFound;
}

const VehicleRecord* ParkingService::FindByPlate(
    const std::string& plate) const {
  std::string key;
  if (!NormalizeField(plate, true, key)) return nullptr;
  const auto it = records_.find(key);
  return it == records_.end() ? nullptr : &it->second;
}

std::vector<VehicleRecord> ParkingService::ListAll() const {
  std::vector<VehicleRecord> result;
  for (const auto& entry : records_) result.push_back(entry.second);
  return result;
}

VehicleResult ParkingService::FindWithFee(const std::string& plate,
                                          std::time_t reference,
                                          VehicleFeeView& output) const {
  std::string key;
  if (!NormalizeField(plate, true, key)) return VehicleResult::kInvalidField;
  const auto it = records_.find(key);
  if (it == records_.end()) return VehicleResult::kNotFound;
  VehicleFeeView view{it->second, {}};
  if (!CalculateParkingFee(it->second.entry_timestamp, reference, view.fee))
    return VehicleResult::kInvalidTime;
  output = view;
  return VehicleResult::kOk;
}

VehicleResult ParkingService::ListWithFees(
    std::time_t reference, std::vector<VehicleFeeView>& output) const {
  std::time_t normalized;
  if (!NormalizeToMinute(reference, normalized))
    return VehicleResult::kInvalidTime;
  std::vector<VehicleFeeView> views;
  for (const auto& entry : records_) {
    VehicleFeeView view{entry.second, {}};
    if (!CalculateParkingFee(entry.second.entry_timestamp, normalized,
                             view.fee))
      return VehicleResult::kInvalidTime;
    views.push_back(view);
  }
  output = views;
  return VehicleResult::kOk;
}
