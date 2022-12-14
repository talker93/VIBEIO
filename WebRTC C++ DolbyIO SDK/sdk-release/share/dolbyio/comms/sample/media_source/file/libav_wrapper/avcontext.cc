/***************************************************************************
 * This program is licensed by the accompanying "license" file. This file is
 * distributed "AS IS" AND WITHOUT WARRANTY OF ANY KIND WHATSOEVER, either
 * express or implied. See the License for the specific language governing
 * permissions and limitations under the License.
 *
 *                Copyright (C) 2022 by Dolby Laboratories.
 ***************************************************************************/

#include "comms/sample/media_source/file/libav_wrapper/avcontext.h"

#include <iostream>

namespace dolbyio::comms::sample {

libav_context::libav_context(const std::string& name) noexcept(false) : packet_(av_packet_alloc()), file_name_(name) {
  int av_return = 0;
  if ((av_return = avformat_open_input(&format_, file_name_.c_str(), nullptr, nullptr)) < 0) {
    destroy_context();
    throw std::runtime_error("Failure to open file! code: " + std::to_string(av_return));
  }
  if ((av_return = avformat_find_stream_info(format_, nullptr)) < 0) {
    destroy_context();
    throw std::runtime_error("Failure to find stream info! code: " + std::to_string(av_return));
  }
}

libav_context::~libav_context() {
  video_.reset();
  audio_.reset();
  destroy_context();
}

void libav_context::destroy_context() {
  if (packet_)
    av_packet_free(&packet_);
  if (format_)
    avformat_close_input(&format_);
}

bool libav_context::create_decoder(AVMediaType type) {
  AVStream* stream = nullptr;
  int index = -1;

  if ((index = av_find_best_stream(format_, type, -1, -1, nullptr, 0)) < 0) {
    std::cerr << "Failed to find stream in this file! code: " << std::to_string(index);
    return false;
  }
  stream = format_->streams[index];
  if (type == AVMEDIA_TYPE_AUDIO) {
    audio_.index = index;
    audio_.stream_ = stream;
    audio_.decoder_ = std::make_unique<decoder>(stream, true);
  } else if (type == AVMEDIA_TYPE_VIDEO) {
    video_.index = index;
    video_.stream_ = stream;
    video_.decoder_ = std::make_unique<decoder>(stream, true);
  }
  return true;
}

int libav_context::read_single_packet() {
  int av_return = 0;
  if (packet_->data)
    return av_return;

  av_return = av_read_frame(format_, packet_);
  return av_return;
}

void libav_context::packet_finished() {
  av_packet_unref(packet_);
}

int libav_context::packet_to_decoder(bool capture_video, bool capture_audio) {
  int av_return = -1;
  if (is_video() && capture_video) {
    av_return = video_->send(packet_);
  } else if (is_audio() && capture_audio) {
    av_return = audio_->send(packet_);
  }
  return av_return;
}

template <typename T>
int libav_context::frame_from_decoder(dolbyio::comms::sample::frame* frame) {
  int av_return = -1;
  if constexpr (std::is_same_v<video, T>) {
    av_return = video_->receive(frame);
  } else if constexpr (std::is_same_v<audio, T>) {
    av_return = audio_->receive(frame);
  }

  return av_return;
}

bool libav_context::seek_set_time() {
  if (video_ || audio_) {
    int index = video_ ? video_.index : audio_.index;
    if (av_seek_frame(format_, index, next_seek_time_, 0) >= 0)
      return true;
  }
  return false;
}

bool libav_context::set_next_seek_time(int64_t time) {
  if (time < 0) {
    std::cerr << "Trying to seek to negative timestamp in file!\n";
  } else if (video_ || audio_) {
    if (!format_) {
      std::cerr << "Can't seek when their is no open AVFormat!\n";
      return false;
    }
    int64_t request_timestamp = 0;
    AVRational stream_time_base = video_ ? video_.stream_->time_base : audio_.stream_->time_base;
    request_timestamp = ((double)time) / av_q2d(stream_time_base);
    if (request_timestamp > format_->duration) {
      std::cerr << "Trying to seek past end of this file! request:" << request_timestamp
                << " file duration:" << format_->duration << std::endl;
      return false;
    }
    next_seek_time_ = request_timestamp;
    return true;
  }
  std::cerr << "Can't seek as there is no Video or Audio streams for this file!\n";
  return false;
}

std::chrono::milliseconds libav_context::frame_interval() {
  std::chrono::milliseconds interval{33};
  if (video_) {
    interval = std::chrono::milliseconds(
        (int)(1000 * (float)video_.stream_->avg_frame_rate.den / (float)video_.stream_->avg_frame_rate.num));
  } else
    std::cerr << "Warning, there is no video stream for this file, returning "
                 "default 33ms frame interval\n";
  return interval;
}

template int libav_context::frame_from_decoder<video>(frame* frame);
template int libav_context::frame_from_decoder<audio>(frame* frame);

};  // namespace dolbyio::comms::sample
