/***************************************************************************
 * This program is licensed by the accompanying "license" file. This file is
 * distributed "AS IS" AND WITHOUT WARRANTY OF ANY KIND WHATSOEVER, either
 * express or implied. See the License for the specific language governing
 * permissions and limitations under the License.
 *
 *                Copyright (C) 2022 by Dolby Laboratories.
 ***************************************************************************/

#include <dolbyio/comms/media_engine/media_engine.h>

#include <memory>
#include <mutex>

class custom_injector_impl : public dolbyio::comms::media_source_interface,
                             public dolbyio::comms::video_source,
                             public dolbyio::comms::audio_source {
 public:
  custom_injector_impl();
  ~custom_injector_impl();

  bool inject_audio_frame(std::unique_ptr<dolbyio::comms::audio_frame>&& frame);
  bool inject_video_frame(std::unique_ptr<dolbyio::comms::video_frame>&& frame);

  // media_source_interface
  dolbyio::comms::audio_source* audio() override { return this; }

  // audio_source interface
  void register_audio_frame_rtc_source(dolbyio::comms::rtc_audio_source* source) override;
  void deregister_audio_frame_rtc_source() override;

  // video_source interface
  void set_sink(dolbyio::comms::video_sink* sink, const dolbyio::comms::video_source::config& config) override;

 private:
  // These are essentially audio/video sinks from the POV of the injector
  // providing the connection in the respective media pipelines to Webrtc
  dolbyio::comms::rtc_audio_source* rtc_audio_ = nullptr;
  dolbyio::comms::video_sink* video_sink_ = nullptr;

  std::mutex audio_lock_;
  std::mutex video_lock_;
};
