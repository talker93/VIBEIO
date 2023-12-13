/***************************************************************************
 * This program is licensed by the accompanying "license" file. This file is
 * distributed "AS IS" AND WITHOUT WARRANTY OF ANY KIND WHATSOEVER, either
 * express or implied. See the License for the specific language governing
 * permissions and limitations under the License.
 *
 *                Copyright (C) 2022 by Dolby Laboratories.
 ***************************************************************************/

#include "custom_recorder.h"

using decoder_config = dolbyio::comms::video_sink_encoded::decoder_config;

custom_recorder_impl::custom_recorder_impl(dolbyio::comms::sdk& sdk, video_format video, audio_format audio)
    : sdk_(sdk), af_(audio), vf_(video) {
  switch (vf_) {
    case custom_recorder_impl::video_format::YUV: {
      wait(sdk_.video().remote().set_video_sink(this));
      break;
    }
    case custom_recorder_impl::video_format::ENCODED:
    case custom_recorder_impl::video_format::ENCODED_OPTIMIZED: {
      wait(sdk_.media_io().set_encoded_video_sink(this));
      break;
    }
    case custom_recorder_impl::video_format::NONE: {
      wait(sdk_.video().remote().set_video_sink(nullptr));
      wait(sdk_.media_io().set_encoded_video_sink(nullptr));
      break;
    }
  }

  if (af_ != custom_recorder_impl::audio_format::NONE) {
    wait(sdk_.media_io().set_audio_sink(this));
  } else {
    wait(sdk_.media_io().set_audio_sink(nullptr));
  }
}

void custom_recorder_impl::handle_audio(const std::string&, const std::string&, const int16_t*, size_t, int, size_t) {
  // handle audio frame
}

void custom_recorder_impl::handle_frame(const std::string&,
                                        const std::string&,
                                        std::unique_ptr<dolbyio::comms::video_frame>) {
  // handle raw video frame
}

dolbyio::comms::video_sink_encoded::decoder_config custom_recorder_impl::configure_encoded_sink(const std::string&,
                                                                                                const std::string&) {
  // set the codec for encoded video frames
  if (vf_ == video_format::ENCODED_OPTIMIZED)
      return decoder_config::optimized_decoding;
  else
    return decoder_config::full_decoding;
}

void custom_recorder_impl::handle_frame_encoded(const std::string&,
                                                std::unique_ptr<dolbyio::comms::encoded_video_frame>) {
  // handle encoded video frame
}

dolbyio::comms::video_sink_encoded::decoder_config custom_recorder_impl::decoder_configuration() const {
  return vf_ == video_format::ENCODED_OPTIMIZED ? decoder_config::optimized_decoding : decoder_config::full_decoding;
}

dolbyio::comms::audio_sink* custom_recorder_impl::audio() {
  if (af_ != audio_format::NONE)
    return this;

  return nullptr;
}

dolbyio::comms::video_sink* custom_recorder_impl::video_raw() {
  if (vf_ == video_format::YUV)
    return this;

  return nullptr;
}

dolbyio::comms::video_sink_encoded* custom_recorder_impl::video_enc() {
  if (vf_ == video_format::ENCODED)
    return this;

  return nullptr;
}
