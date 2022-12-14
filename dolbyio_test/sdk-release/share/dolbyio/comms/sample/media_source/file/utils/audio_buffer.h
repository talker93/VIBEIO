#pragma once

/***************************************************************************
 * This program is licensed by the accompanying "license" file. This file is
 * distributed "AS IS" AND WITHOUT WARRANTY OF ANY KIND WHATSOEVER, either
 * express or implied. See the License for the specific language governing
 * permissions and limitations under the License.
 *
 *                Copyright (C) 2022 by Dolby Laboratories.
 ***************************************************************************/

#include <memory>

namespace dolbyio::comms::sample {

class audio_buffer {
 public:
  explicit audio_buffer(int samples, int sample_rate, int channels);

  void push(int16_t value);
  bool full();
  void reset();

  const int16_t* data() const;
  int sample_rate() const;
  int channels() const;
  int samples() const;

 private:
  int size_;
  std::unique_ptr<int16_t[]> data_;
  int sample_rate_;
  int channels_;
  int samples_;
  int index_;
};

};  // namespace dolbyio::comms::sample
