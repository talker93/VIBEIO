#pragma once

/***************************************************************************
 * This program is licensed by the accompanying "license" file. This file is
 * distributed "AS IS" AND WITHOUT WARRANTY OF ANY KIND WHATSOEVER, either
 * express or implied. See the License for the specific language governing
 * permissions and limitations under the License.
 *
 *                Copyright (C) 2022 by Dolby Laboratories.
 ***************************************************************************/

#include <dolbyio/comms/media_engine/media_engine.h>

#include "comms/sample/media_source/file/libav_wrapper/frame.h"
#include "comms/sample/media_source/file/utils/audio_buffer.h"
#include "comms/sample/media_source/file/utils/frame_pool.h"

#include <memory>

namespace dolbyio::comms::sample {

class video_frame_impl : public dolbyio::comms::video_frame, private dolbyio::comms::video_frame_i420 {
 public:
  explicit video_frame_impl(std::unique_ptr<frame_from_pool<frame>> frame);
  ~video_frame_impl();

  video_frame_impl(const video_frame_impl&) = delete;
  video_frame_impl(video_frame_impl&&) = default;
  video_frame_impl& operator=(const video_frame_impl&) = delete;
  video_frame_impl& operator=(video_frame_impl&&) = default;

  video_frame_i420* get_i420_frame() override { return this; }

#if defined(__APPLE__)
  video_frame_macos* get_native_frame() override { return nullptr; }
#endif

  int width() const override;
  int height() const override;

  const uint8_t* get_y() const override;
  const uint8_t* get_u() const override;
  const uint8_t* get_v() const override;
  int stride_y() const override;
  int stride_u() const override;
  int stride_v() const override;
  int64_t timestamp_us() const override { return 0; }

 private:
  std::unique_ptr<frame_from_pool<frame>> frame_;
};

class audio_frame_impl : public dolbyio::comms::audio_frame {
 public:
  explicit audio_frame_impl(std::unique_ptr<frame_from_pool<audio_buffer>> audio_buf);
  ~audio_frame_impl();

  audio_frame_impl(const audio_frame_impl&) = delete;
  audio_frame_impl(audio_frame_impl&&) = default;
  audio_frame_impl& operator=(const audio_frame_impl&) = delete;
  audio_frame_impl& operator=(audio_frame_impl&&) = default;

  const int16_t* data() const override { return frame_->val()->data(); }
  int sample_rate() const override { return frame_->val()->sample_rate(); }
  int channels() const override { return frame_->val()->channels(); }
  int samples() const override { return frame_->val()->samples(); }

 private:
  std::unique_ptr<frame_from_pool<audio_buffer>> frame_;
};

}  // namespace dolbyio::comms::sample
