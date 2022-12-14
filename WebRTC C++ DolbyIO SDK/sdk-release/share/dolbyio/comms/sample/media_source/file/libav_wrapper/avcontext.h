#pragma once

/***************************************************************************
 * This program is licensed by the accompanying "license" file. This file is
 * distributed "AS IS" AND WITHOUT WARRANTY OF ANY KIND WHATSOEVER, either
 * express or implied. See the License for the specific language governing
 * permissions and limitations under the License.
 *
 *                Copyright (C) 2022 by Dolby Laboratories.
 ***************************************************************************/

#include "comms/sample/media_source/file/libav_wrapper/decoder.h"

#include <chrono>
#include <memory>
#include <string>

namespace dolbyio::comms::sample {

struct video {
  AVMediaType type = AVMEDIA_TYPE_VIDEO;
};

struct audio {
  AVMediaType type = AVMEDIA_TYPE_AUDIO;
  bool dummy = false;
};

struct media {
  void reset() {
    decoder_.reset();
    stream_ = nullptr;
    index = -1;
  }

  explicit operator bool() const { return decoder_ && stream_; }

  decoder* operator->() { return decoder_.get(); }

  std::unique_ptr<decoder> decoder_{};
  AVStream* stream_ = nullptr;
  int index = -1;
};

class libav_context {
 public:
  libav_context(const std::string& name) noexcept(false);
  ~libav_context();

  bool create_decoder(AVMediaType type);

  int read_single_packet();
  void packet_finished();
  int packet_to_decoder(bool capture_video, bool capture_audio);

  template <typename T>
  int frame_from_decoder(frame* frame);

  bool is_audio() const { return packet_->stream_index == audio_.index; }
  bool is_video() const { return packet_->stream_index == video_.index; }

  bool seek_set_time();
  bool set_next_seek_time(int64_t time);
  std::chrono::milliseconds frame_interval();
  int sample_rate() { return audio_->sample_rate(); }
  int channels() { return audio_->channels(); }

  AVPacket* packet() { return packet_; }

 private:
  void destroy_context();

  AVFormatContext* format_ = nullptr;
  AVPacket* packet_;
  std::string file_name_;
  media video_{};
  media audio_{};
  int64_t next_seek_time_;
};

};  // namespace dolbyio::comms::sample
