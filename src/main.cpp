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

#include <iostream>

#include "src/menus.h"
#include "src/parking_service.h"
#include "src/storage.h"

int main() {
  ParkingService service;
  Storage storage;
  auto loaded = storage.Load(service);
  std::cout << loaded.message << '\n';
  if (!loaded.ok) return 1;
  RunMenu(std::cin, std::cout, service, storage);
  auto saved = storage.Save(service);
  std::cout << saved.message << '\n';
  return saved.ok ? 0 : 1;
}
