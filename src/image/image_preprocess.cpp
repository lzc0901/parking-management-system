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

#include "src/image/image_preprocess.h"

#include <windows.h>

#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "opencv2/imgcodecs.hpp"
#include "opencv2/imgproc.hpp"

namespace {
bool DimensionsAllowed(const std::vector<unsigned char>& data, bool png) {
  std::uint64_t width = 0, height = 0;
  if (png) {
    if (data.size() < 24 || data[12] != 'I' || data[13] != 'H' ||
        data[14] != 'D' || data[15] != 'R')
      return false;
    for (int i = 16; i < 20; ++i) width = (width << 8) | data[i];
    for (int i = 20; i < 24; ++i) height = (height << 8) | data[i];
  } else {
    std::size_t i = 2;
    while (i + 3 < data.size()) {
      if (data[i++] != 0xff) return false;
      while (i < data.size() && data[i] == 0xff) ++i;
      if (i >= data.size()) return false;
      const unsigned char marker = data[i++];
      if (marker == 0xd9 || marker == 0xda) return false;
      if (marker == 0x01 || (marker >= 0xd0 && marker <= 0xd7)) continue;
      if (i + 2 > data.size()) return false;
      const std::size_t length = (data[i] << 8) | data[i + 1];
      if (length < 2 || length > data.size() - i) return false;
      const bool sof = marker >= 0xc0 && marker <= 0xcf && marker != 0xc4 &&
                       marker != 0xc8 && marker != 0xcc;
      if (sof) {
        if (length < 7) return false;
        height = (data[i + 3] << 8) | data[i + 4];
        width = (data[i + 5] << 8) | data[i + 6];
        break;
      }
      i += length;
    }
  }
  return width > 0 && height > 0 && width * height <= 16000000;
}
}  // namespace

bool PrepareImage(const std::string& utf8_path,
                  std::vector<unsigned char>& jpeg, std::string& error) {
  constexpr std::size_t kMaxFileBytes = 10 * 1024 * 1024;
  constexpr int kMaxSide = 2048;
  try {
    if (utf8_path.find('\0') != std::string::npos) {
      error = "Invalid image path.";
      return false;
    }
    const int count = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS,
                                          utf8_path.c_str(), -1, nullptr, 0);
    if (!count) {
      error = "Invalid image path encoding.";
      return false;
    }
    std::vector<wchar_t> wide(count);
    if (!MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, utf8_path.c_str(),
                             -1, wide.data(), count)) {
      error = "Invalid image path encoding.";
      return false;
    }
    std::unique_ptr<FILE, decltype(&std::fclose)> in(
        _wfopen(wide.data(), L"rb"), &std::fclose);
    if (!in) {
      error = "Cannot open image.";
      return false;
    }
    if (std::fseek(in.get(), 0, SEEK_END) != 0) {
      error = "Cannot inspect image size.";
      return false;
    }
    const std::int64_t size = std::ftell(in.get());
    if (size <= 0 || size > static_cast<std::int64_t>(kMaxFileBytes)) {
      error = "Image is empty or exceeds 10 MiB.";
      return false;
    }
    std::vector<unsigned char> bytes(static_cast<std::size_t>(size));
    if (std::fseek(in.get(), 0, SEEK_SET) != 0 ||
        std::fread(bytes.data(), 1, bytes.size(), in.get()) != bytes.size()) {
      error = "Cannot read image.";
      return false;
    }
    const bool jpg = bytes.size() >= 3 && bytes[0] == 0xff &&
                     bytes[1] == 0xd8 && bytes[2] == 0xff;
    const unsigned char png_magic[] = {0x89, 'P',  'N',  'G',
                                       0x0d, 0x0a, 0x1a, 0x0a};
    const bool png =
        bytes.size() >= sizeof(png_magic) &&
        std::equal(png_magic, png_magic + sizeof(png_magic), bytes.begin());
    if (!jpg && !png) {
      error = "Only JPEG and PNG images are supported.";
      return false;
    }
    if (!DimensionsAllowed(bytes, png)) {
      error = "Invalid dimensions or image exceeds 16 million pixels.";
      return false;
    }
    auto image = cv::imdecode(bytes, cv::IMREAD_COLOR);
    if (image.empty() || image.total() > 16000000) {
      error = "Image is invalid or exceeds 16 million pixels.";
      return false;
    }
    if (std::max(image.cols, image.rows) > kMaxSide) {
      const double scale =
          static_cast<double>(kMaxSide) / std::max(image.cols, image.rows);
      cv::resize(image, image, cv::Size(), scale, scale, cv::INTER_AREA);
    }
    std::vector<unsigned char> encoded;
    if (!cv::imencode(".jpg", image, encoded, {cv::IMWRITE_JPEG_QUALITY, 90})) {
      error = "Image encoding failed.";
      return false;
    }
    jpeg = std::move(encoded);
    error.clear();
    return true;
  } catch (const std::exception&) {
    error = "Image decode failed or image exceeds the pixel limit.";
    return false;
  }
}
