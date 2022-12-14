/***************************************************************************
 * This program is licensed by the accompanying "license" file. This file is
 * distributed "AS IS" AND WITHOUT WARRANTY OF ANY KIND WHATSOEVER, either
 * express or implied. See the License for the specific language governing
 * permissions and limitations under the License.
 *
 *                Copyright (C) 2022 by Dolby Laboratories.
 ***************************************************************************/

#include "custom_injector.h"

custom_injector_impl::custom_injector_impl() = default;
custom_injector_impl::~custom_injector_impl() = default;

bool custom_injector_impl::inject_audio_frame(std::unique_ptr<dolbyio::comms::audio_frame>&& frame) {
  std::lock_guard<std::mutex> lock(audio_lock_);
  if (frame && rtc_audio_) {
    rtc_audio_->on_data(frame->data(), 16, frame->sample_rate(), frame->channels(), frame->samples());
    return true;
  }
  return false;
}

bool custom_injector_impl::inject_video_frame(std::unique_ptr<dolbyio::comms::video_frame>&& frame) {
  std::lock_guard<std::mutex> lock(video_lock_);
  if (frame && video_sink_) {
    video_sink_->handle_frame("", "", std::move(frame));
    return true;
  }
  return false;
}

// audio_source interface
void custom_injector_impl::register_audio_frame_rtc_source(dolbyio::comms::rtc_audio_source* source) {
  std::lock_guard<std::mutex> lock(audio_lock_);
  rtc_audio_ = source;
}

void custom_injector_impl::deregister_audio_frame_rtc_source() {
  std::lock_guard<std::mutex> lock(audio_lock_);
  rtc_audio_ = nullptr;
}

// video_source interface
void custom_injector_impl::set_sink(dolbyio::comms::video_sink* sink, const config&) {
  std::lock_guard<std::mutex> lock(video_lock_);
  video_sink_ = sink;
}
