#pragma once

/***************************************************************************
 * This program is licensed by the accompanying "license" file. This file is
 * distributed "AS IS" AND WITHOUT WARRANTY OF ANY KIND WHATSOEVER, either
 * express or implied. See the License for the specific language governing
 * permissions and limitations under the License.
 *
 *                Copyright (C) 2022 by Dolby Laboratories.
 ***************************************************************************/

#include <dolbyio/comms/multimedia_streaming/injector.h>
#include <dolbyio/comms/multimedia_streaming/recorder.h>
#include <dolbyio/comms/sample/media_source/file/source_capture.h>
#include <dolbyio/comms/sdk.h>

#include "comms/sample/utilities/commands_handler.h"
#include "comms/sample/utilities/interactor.h"
#include "comms/sample/utilities/sdk/interactions.h"

#include <string>
#include <vector>

namespace dolbyio::comms::sample {

// Handles interactions with the dolbyio::comms::plugins and dolbyio::comms:sample::media_source_file
// sample library which captures media from file.
// This class will use the interactor to setup command line switches it will respond
// to as well as interactive commands.
class media_io_interactions : public interactor {
 public:
  media_io_interactions(sdk_interactions::initial_params& sdk_params) : sdk_params_(sdk_params) {}
  ~media_io_interactions() override;
  struct initial_params {
    std::vector<std::string> files;
    std::string output_dir{"tmp"};
    dolbyio::comms::plugin::recorder::video_recording_config vid_config{
        dolbyio::comms::plugin::recorder::video_recording_config::ENCODED_OPTIMIZED};
    dolbyio::comms::plugin::recorder::audio_recording_config aud_config{
        dolbyio::comms::plugin::recorder::audio_recording_config::PCM};
  };

  void set_initial_capture(bool audio, bool video);

  // interactor interface
  void set_sdk(dolbyio::comms::sdk* sdk) override;
  void initialize_injection();
  void register_command_line_handlers(commands_handler& handler) override;
  void register_interactive_commands(commands_handler& handler) override;

  bool media_io_enabled() const { return media_io_; }

  const initial_params& get_params() const { return params_; }

 private:
  void new_file(bool add);
  void seek_to_in_file();

  sdk_interactions::initial_params& sdk_params_;
  std::unique_ptr<plugin::injector_paced> injector_{};
  std::unique_ptr<file_source> source_{};
  std::mutex sdk_lock_{};
  dolbyio::comms::sdk* sdk_;
  initial_params params_;

  bool media_io_{false};
  std::string cmdline_config_touched_{};
};

};  // namespace dolbyio::comms::sample
