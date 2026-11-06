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

#ifndef SRC_MENUS_H_
#define SRC_MENUS_H_

#include <iosfwd>
#include <string>
class ParkingService;
class PlateRecognizer;
class Storage;

// Run the role selector until exit or end-of-input.
void RunMenu(std::istream& in, std::ostream& out, ParkingService& service,
             Storage& storage, PlateRecognizer* recognizer = nullptr);
// Handles a read-only menu choice; returns false on end-of-input.
bool RunReadChoice(const std::string& choice, std::istream& in,
                   std::ostream& out, const ParkingService& service);

#endif  // SRC_MENUS_H_
