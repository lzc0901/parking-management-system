#include "parking_service.h"

namespace {
VehicleResult validated(const VehicleRecord& input, std::time_t reference, VehicleRecord& output) {
    VehicleRecord candidate;
    if (!normalizeField(input.plate, true, candidate.plate) ||
        !normalizeField(input.category, false, candidate.category) ||
        !normalizeField(input.color, false, candidate.color)) return VehicleResult::InvalidField;
    if (!parseTimestamp(input.entryTimeText, candidate.entryTimeText, candidate.entryTimestamp)) return VehicleResult::InvalidTime;
    std::time_t minute;
    if (!normalizeToMinute(reference, minute)) return VehicleResult::InvalidTime;
    if (candidate.entryTimestamp > minute) return VehicleResult::FutureTime;
    output = candidate;
    return VehicleResult::Ok;
}
}

VehicleResult ParkingService::addVehicle(Role role, const VehicleRecord& record, std::time_t reference) {
    if (role != Role::Admin) return VehicleResult::Forbidden;
    VehicleRecord candidate;
    const auto result = validated(record, reference, candidate);
    if (result != VehicleResult::Ok) return result;
    if (records_.count(candidate.plate)) return VehicleResult::DuplicatePlate;
    records_.emplace(candidate.plate, candidate);
    return VehicleResult::Ok;
}

VehicleResult ParkingService::modifyVehicle(Role role, const std::string& plate, const VehicleRecord& record, std::time_t reference) {
    if (role != Role::Admin) return VehicleResult::Forbidden;
    std::string key;
    if (!normalizeField(plate, true, key)) return VehicleResult::InvalidField;
    auto current = records_.find(key);
    if (current == records_.end()) return VehicleResult::NotFound;
    VehicleRecord candidate;
    const auto result = validated(record, reference, candidate);
    if (result != VehicleResult::Ok) return result;
    if (candidate.plate != key && records_.count(candidate.plate)) return VehicleResult::DuplicatePlate;
    if (candidate.plate == key) current->second = candidate;
    else { records_.emplace(candidate.plate, candidate); records_.erase(current); }
    return VehicleResult::Ok;
}

VehicleResult ParkingService::deleteVehicle(Role role, const std::string& plate) {
    if (role != Role::Admin) return VehicleResult::Forbidden;
    std::string key;
    if (!normalizeField(plate, true, key)) return VehicleResult::InvalidField;
    return records_.erase(key) ? VehicleResult::Ok : VehicleResult::NotFound;
}

const VehicleRecord* ParkingService::findByPlate(const std::string& plate) const {
    std::string key;
    if (!normalizeField(plate, true, key)) return nullptr;
    const auto it = records_.find(key);
    return it == records_.end() ? nullptr : &it->second;
}

std::vector<VehicleRecord> ParkingService::listAll() const {
    std::vector<VehicleRecord> result;
    for (const auto& entry : records_) result.push_back(entry.second);
    return result;
}

VehicleResult ParkingService::findWithFee(const std::string& plate, std::time_t reference, VehicleFeeView& output) const {
    std::string key;
    if (!normalizeField(plate, true, key)) return VehicleResult::InvalidField;
    const auto it = records_.find(key);
    if (it == records_.end()) return VehicleResult::NotFound;
    VehicleFeeView view{it->second, {}};
    if (!calculateParkingFee(it->second.entryTimestamp, reference, view.fee)) return VehicleResult::InvalidTime;
    output = view;
    return VehicleResult::Ok;
}

VehicleResult ParkingService::listWithFees(std::time_t reference, std::vector<VehicleFeeView>& output) const {
    std::time_t normalized;
    if (!normalizeToMinute(reference, normalized)) return VehicleResult::InvalidTime;
    std::vector<VehicleFeeView> views;
    for (const auto& entry : records_) {
        VehicleFeeView view{entry.second, {}};
        if (!calculateParkingFee(entry.second.entryTimestamp, normalized, view.fee)) return VehicleResult::InvalidTime;
        views.push_back(view);
    }
    output = views;
    return VehicleResult::Ok;
}
