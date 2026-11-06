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

#include <windows.h>

#include <cstdio>
#include <iostream>
#include <sstream>
#include <string>

#include "src/admin_user.h"
#include "src/ordinary_user.h"
#include "src/parking_service.h"
#include "src/plate_recognition.h"
#include "src/storage.h"

class FakeRecognizer : public PlateRecognizer {
 public:
  RecognitionResult Recognize(const std::string&) override {
    return {RecognitionStatus::kOk, {"MA-12-34"}, ""};
  }
};

int main() {
  int failures = 0;
  auto check = [&](bool ok, const char* name) {
    if (!ok) {
      std::cerr << "FAIL: " << name << '\n';
      ++failures;
    }
  };
  char directory[MAX_PATH];
  char filename[MAX_PATH];
  if (!GetTempPathA(MAX_PATH, directory) ||
      !GetTempFileNameA(directory, "pms", 0, filename))
    return 1;
  std::remove(filename);
  Storage storage(filename);
  ParkingService service;
  check(storage.Load(service).ok, "initialize storage");
  FakeRecognizer recognizer;
  std::ostringstream out;
  std::istringstream cancelled(
      "7\n1\ny\nimage.jpg\n\ny\nSedan\nWhite\n\nn\n0\n");
  check(RunAdminUserSession(cancelled, out, service, storage, &recognizer) &&
            service.Count() == 0,
        "registration cancellation leaves state unchanged");
  std::istringstream add("7\n1\ny\nimage.jpg\n\ny\nSedan\nWhite\n\ny\n0\n");
  check(RunAdminUserSession(add, out, service, storage, &recognizer) &&
            service.Count() == 1,
        "confirmed registration");
  ParkingService reloaded;
  Storage restart(filename);
  check(restart.Load(reloaded).ok && reloaded.FindByPlate("MA-12-34"),
        "registration survives restart");
  std::istringstream duplicate(
      "7\n1\ny\nimage.jpg\n\ny\nSedan\nWhite\n\ny\n0\n");
  check(RunAdminUserSession(duplicate, out, service, storage, &recognizer) &&
            service.Count() == 1 &&
            out.str().find("Plate already exists") != std::string::npos,
        "duplicate registration rejected");
  std::istringstream query("4\n1\ny\nimage.jpg\n\ny\n0\n");
  check(RunOrdinaryUserSession(query, out, service, &recognizer) &&
            service.Count() == 1 &&
            out.str().find("Duration:") != std::string::npos,
        "ordinary image lookup is read only");
  std::istringstream eof("7\n1\ny\nimage.jpg\n\ny\nSedan\nWhite\n\n");
  check(!RunAdminUserSession(eof, out, service, storage, &recognizer) &&
            service.Count() == 1,
        "EOF before submit leaves state unchanged");
  ParkingService unsaved;
  Storage blocked("unused.txt");
  std::ostringstream failure_output;
  std::istringstream failure("7\n1\ny\nimage.jpg\n\ny\nSedan\nWhite\n\ny\n0\n");
  check(RunAdminUserSession(failure, failure_output, unsaved, blocked,
                            &recognizer) &&
            unsaved.Count() == 1 &&
            failure_output.str().find("in memory, but save failed") !=
                std::string::npos,
        "save failure distinguishes in-memory change");
  std::remove(filename);
  if (failures) return 1;
  std::cout << "Image session tests passed.\n";
}
