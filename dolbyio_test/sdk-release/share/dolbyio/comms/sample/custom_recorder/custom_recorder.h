/***************************************************************************
 * This program is licensed by the accompanying "license" file. This file is
 * distributed "AS IS" AND WITHOUT WARRANTY OF ANY KIND WHATSOEVER, either
 * express or implied. See the License for the specific language governing
 * permissions and limitations under the License.
 *
 *                Copyright (C) 2022 by Dolby Laboratories.
 ***************************************************************************/

#include <dolbyio/comms/media_engine/media_engine.h>
#include <dolbyio/comms/sdk.h>

#include <memory>
#include <string>

class custom_recorder_impl : private dolbyio::comms::audio_sink,
                             private dolbyio::comms::video_sink,
                             private dolbyio::comms::video_sink_encoded,
                             public dolbyio::comms::media_sink_interface {
 public:
  enum class audio_format { NONE, PCM, AAC };

  enum class video_format { NONE, ENCODED, ENCODED_OPTIMIZED, YUV };

  custom_recorder_impl(dolbyio::comms::sdk& sdk, video_format video, audio_format audio);
  ~custom_recorder_impl() = default;

  dolbyio::comms::video_sink* video_raw();

  // audio_recorder interface
  void handle_audio(const std::string&, const std::string&, const int16_t*, size_t, int, size_t) override;

  // video_recorder_yuv interface
  void handle_frame(const std::string&, const std::string&, std::unique_ptr<dolbyio::comms::video_frame>) override;

  // video_recorder_encoded interface
  dolbyio::comms::video_sink_encoded::decoder_config configure_encoded_sink(const std::string&,
                                                                            const std::string&) override;

  void handle_frame_encoded(const std::string&, std::unique_ptr<dolbyio::comms::encoded_video_frame>) override;

  decoder_config decoder_configuration() const override;

  // media_sink_interface
  dolbyio::comms::audio_sink* audio() override;
  dolbyio::comms::video_sink_encoded* video_enc() override;

 private:
  dolbyio::comms::sdk& sdk_;

  audio_format af_{audio_format::NONE};
  video_format vf_{video_format::NONE};
};
