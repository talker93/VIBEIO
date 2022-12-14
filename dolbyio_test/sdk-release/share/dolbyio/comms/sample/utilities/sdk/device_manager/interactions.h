#pragma once

/***************************************************************************
 * This program is licensed by the accompanying "license" file. This file is
 * distributed "AS IS" AND WITHOUT WARRANTY OF ANY KIND WHATSOEVER, either
 * express or implied. See the License for the specific language governing
 * permissions and limitations under the License.
 *
 *                Copyright (C) 2022 by Dolby Laboratories.
 ***************************************************************************/

#include "comms/sample/utilities/interactor.h"

#include <dolbyio/comms/sdk.h>

namespace dolbyio::comms::sample {

// Handles interactions with the dolbyio::comms::service::device_manager service.
// This includes the handling of audio/video devices and receiving events about
// their changes.
// This class will use the interactor to setup command line switches it will respond
// to as well as interactive commands.
class device_interactions : public interactor {
 public:
  void set_sdk(dolbyio::comms::sdk* sdk) override;

  void register_command_line_handlers(commands_handler& handler) override;
  void register_interactive_commands(commands_handler& handler) override;

 private:
  void enable();
  void disable();
  void get_audio_devices();
  void set_input_audio_device();
  void set_output_audio_device();

  void get_video_devices();
  void set_video_device();

  dolbyio::comms::sdk* sdk_ = nullptr;
  std::vector<dolbyio::comms::dvc_device> devices_;
  std::vector<dolbyio::comms::camera_device> video_devices_;
  std::optional<dolbyio::comms::camera_device> curr_video_device_;
  std::optional<dolbyio::comms::dvc_device> curr_input_device_;
  std::optional<dolbyio::comms::dvc_device> curr_output_device_;
  std::vector<event_handler_id> handlers_;
};

};  // namespace dolbyio::comms::sample
