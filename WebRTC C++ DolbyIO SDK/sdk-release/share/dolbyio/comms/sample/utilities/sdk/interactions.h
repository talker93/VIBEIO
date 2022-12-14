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

#include <optional>
#include <string>

namespace dolbyio::comms::sample {

class sdk_interactor_common {
 public:
  virtual void update_conference_status(const conference_status& status) = 0;
  virtual void add_invitation(const conference_invitation_received& invitation) = 0;
};

// Handles interactions with the dolbyio::comms::service::conference service and the
// dolbyio::comms::service::session service. This includes everything from open/closing
// of sessions, to joining/leaving conferences, setting spatial configurations and everything
// else handled by the aforementioned services.
// This class will use the interactor to setup command line switches it will respond
// to as well as interactive commands.
class sdk_interactions : public interactor, public sdk_interactor_common {
 public:
  struct initial_params {
    std::string access_token{};
    dolbyio::comms::log_level sdk_log_level{log_level::INFO};
    dolbyio::comms::log_level me_log_level{log_level::OFF};
    std::string log_dir{};
    std::string user_name{};
    std::string external_id{};
    bool display_video{true};

    struct conf {
      std::optional<std::string> alias;
      std::optional<std::string> cat;
      std::optional<std::string> id;

      std::optional<bool> nonlistener_join{};
      bool default_nonlistener_join{true};

      struct audio_video {
        bool audio{true};
        bool video{true};
      };
      std::optional<audio_video> send_audio_video{};
      audio_video default_send_audio_video{};

      bool send_only{false};
      bool simulcast{false};
      spatial_audio_style spatial{spatial_audio_style::disabled};
      std::optional<int> max_vfs;
      bool log_active_speaker;

      bool join_as_user() const { return nonlistener_join.value_or(default_nonlistener_join); }
      bool join_with_audio() const { return send_audio_video.value_or(default_send_audio_video).audio; }
      bool join_with_video() const { return send_audio_video.value_or(default_send_audio_video).video; }
    };

    conf conf;
    dolbyio::comms::video_frame_handler* video_frame_handler = nullptr;
  };

  const initial_params& get_params() const { return params_; }
  initial_params& get_params() { return params_; }
  void set_sdk(dolbyio::comms::sdk* sdk) override { sdk_ = sdk; }

  dolbyio::comms::services::session::user_info session_options() const;
  dolbyio::comms::services::conference::conference_options conference_options() const;
  dolbyio::comms::services::conference::join_options join_options() const;
  dolbyio::comms::services::conference::listen_options listen_options() const;

  std::future<void> set_local_spatial_position();
  dolbyio::comms::spatial_audio_batch_update set_spatial_environment();

  void register_command_line_handlers(commands_handler& handler) override;
  void register_interactive_commands(commands_handler& handler) override;

  void add_invitation(const conference_invitation_received& invitation) override;
  void update_conference_status(const conference_status& status) override;

  // Conference Info helpers
  dolbyio::comms::conference_info conference_info() { return conf_info_; }
  void set_conference_info(const dolbyio::comms::conference_info& info);
  void update_conference_id(const std::string& id);

 private:
  void create_and_join();
  void join();
  void accept_invitation();
  void decline_invitation();
  void stop_video();
  void start_video();
  void stop_audio();
  void start_audio();
  void start_remote_audio();
  void stop_remote_audio();
  void list_participants();
  void mute_input(bool muted);
  void mute_remote(bool muted);
  void mute_output(bool muted);
  void send_message();
  void subscribe();
  void unsubscribe();
  void get_audio_level();
  void get_audio_levels();
  void set_audio_capture_mode();
  void get_audio_capture_mode();

  std::optional<conference_invitation_received> consume_invitation();
  void list_invitations();

  std::vector<std::string> split(const std::string& s, char delim = ' ');
  dolbyio::comms::spatial_audio_batch_update get_update_spatial_settings();
  bool get_spatial_position(dolbyio::comms::spatial_audio_batch_update& batch_update, const std::string& msg);
  bool get_spatial_direction(dolbyio::comms::spatial_audio_batch_update& batch_update, const std::string& msg);
  bool get_spatial_environment(dolbyio::comms::spatial_audio_batch_update& batch_update, const std::string& msg);
  void set_spatial_configuration(dolbyio::comms::spatial_audio_batch_update&& batch_update,
                                 std::shared_ptr<std::promise<void>> waiter = nullptr);

  dolbyio::comms::sdk* sdk_{nullptr};
  initial_params params_{};
  dolbyio::comms::conference_info conf_info_{};
  std::unordered_map<std::string, conference_invitation_received> conference_invitations_;
  std::mutex sdk_inter_lock_{};
};

};  // namespace dolbyio::comms::sample
