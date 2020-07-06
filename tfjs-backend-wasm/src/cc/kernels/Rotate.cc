/* Copyright 2020 Google LLC. All Rights Reserved.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 * ===========================================================================*/

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#include <math.h>
#include <cmath>
#include <cstddef>
#include <vector>

#include "src/cc/backend.h"
#include "src/cc/util.h"

namespace tfjs {
namespace wasm {

extern "C" {

#ifdef __EMSCRIPTEN__
EMSCRIPTEN_KEEPALIVE
#endif

void Rotate(const size_t image_id, const size_t batch,
            const size_t image_height, const size_t image_width,
            const size_t num_channels, const float radians,
            const float center_x, const float center_y, size_t* fill_ptr,
            const size_t fill_length, const size_t out_id) {
  auto fill = std::vector<size_t>(fill_ptr, fill_ptr + fill_length);
  auto& image_info = backend::get_tensor_info(image_id);
  auto& out_info = backend::get_tensor_info_out(out_id);

  const float* image_buf = image_info.f32();
  float* out_buf = out_info.f32_write();

  const float sin_factor = sin(-radians);
  const float cos_factor = cos(-radians);

  for (size_t batch_idx = 0; batch_idx < batch; ++batch_idx) {
    for (size_t row = 0; row < image_height; ++row) {
      for (size_t col = 0; col < image_width; ++col) {
        for (size_t channel = 0; channel < num_channels; ++channel) {
          const size_t x = col;
          const size_t y = row;

          size_t coord_x = round(center_x + (x - center_x) * cos_factor -
                                 (y - center_y) * sin_factor);
          size_t coord_y = round(center_y + (x - center_x) * sin_factor -
                                 (y - center_y) * cos_factor);

          float output_value = 255.;
          if (channel != 3) {
            output_value = fill[channel];
          }

          if (coord_x >= 0 && coord_x < image_width && coord_y >= 0 &&
              coord_y < image_height) {
            const size_t image_idx =
                batch_idx * image_width * image_height * num_channels +
                coord_y * (image_width * num_channels) +
                coord_x * num_channels + channel;
            output_value = image_buf[image_idx];
          }

          *out_buf = output_value;
          out_buf++;
        }
      }
    }
  }
}

}  // extern "C"
}  // namespace wasm
}  // namespace tfjs
