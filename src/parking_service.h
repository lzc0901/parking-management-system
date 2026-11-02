#pragma once
#include "vehicle.h"
#include "fee_calculator.h"
#include <map>
#include <vector>

enum class Role { OrdinaryUser, Admin };
enum class VehicleResult { Ok, Forbidden, InvalidField, InvalidTime, FutureTime, DuplicatePlate, NotFound };
struct VehicleFeeView { VehicleRecord vehicle; ParkingFee fee; };

// In-memory operations; persistence and console presentation are separate.
class ParkingService {
public:
    VehicleResult addVehicle(Role role, const VehicleRecord& record, std::time_t reference);
    VehicleResult modifyVehicle(Role role, const std::string& plate, const VehicleRecord& record, std::time_t reference);
    VehicleResult deleteVehicle(Role role, const std::string& plate);
    const VehicleRecord* findByPlate(const std::string& plate) const;
    std::vector<VehicleRecord> listAll() const;
    // A failed query leaves its output unchanged; all rows share one reference.
    VehicleResult findWithFee(const std::string& plate, std::time_t reference, VehicleFeeView& output) const;
    VehicleResult listWithFees(std::time_t reference, std::vector<VehicleFeeView>& output) const;
    std::size_t count() const { return records_.size(); }
private:
    std::map<std::string, VehicleRecord> records_;
};
