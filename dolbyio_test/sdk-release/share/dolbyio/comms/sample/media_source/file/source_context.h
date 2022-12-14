#pragma once

/***************************************************************************
 * This program is licensed by the accompanying "license" file. This file is
 * distributed "AS IS" AND WITHOUT WARRANTY OF ANY KIND WHATSOEVER, either
 * express or implied. See the License for the specific language governing
 * permissions and limitations under the License.
 *
 *                Copyright (C) 2022 by Dolby Laboratories.
 ***************************************************************************/

#include <string>

namespace dolbyio::comms::sample {

struct audio_settings {
  void settings(int sample_rate, int channels) {
    sample_rate_ = sample_rate;
    channels_ = channels;
  }
  bool compare(int sample_rate, int channels) { return sample_rate_ == sample_rate && channels_ == channels; }
  int sample_rate_ = 0;
  int channels_ = 0;
};

struct file_state {
  enum state_change { PLAYING = 0, NEW, SEEK, STOP, PAUSE, RESUME };

  state_change state() { return state_; }
  const std::string& name() { return name_; }
  void playing() { state_ = PLAYING; }
  void new_file(const std::string& file) {
    name_ = file;
    state_ = NEW;
  }
  void seek() { state_ = SEEK; }
  void stop() { state_ = STOP; }
  void pause() { state_ = PAUSE; }
  void resume() { state_ = RESUME; }

  audio_settings audio;

 private:
  state_change state_ = STOP;
  std::string name_;
};

};  // namespace dolbyio::comms::sample
