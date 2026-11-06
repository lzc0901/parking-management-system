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

#ifndef SRC_IMAGE_IMAGE_PREPROCESS_H_
#define SRC_IMAGE_IMAGE_PREPROCESS_H_

#include <string>
#include <vector>

// The output is a temporary in-memory JPEG. No image copy is written to disk.
bool PrepareImage(const std::string& utf8_path,
                  std::vector<unsigned char>& jpeg, std::string& error);

#endif  // SRC_IMAGE_IMAGE_PREPROCESS_H_
