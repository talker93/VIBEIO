#pragma once

#ifdef __APPLE__
#include "ui.h"

#include <dolbyio/comms/media_engine/media_engine.h>
#include <dolbyio/comms/media_engine/video_frame_macos.h>
#include <dolbyio/comms/sdk.h>

#include <unordered_map>

namespace dolbyio::comms::sample {

class macosui : public ui_interface,
                private dolbyio::comms::video_sink {
 public:
  macosui(int argc, char** argv);
  ~macosui() override;

  // The main run call, on macOS this will offload all SDK interfactions to a helper thread and then
  // proceed to run the NSMainLoop. This NSMainLoop is required by macOS system libraries for
  // video rendering, camera start/stop, as well as receiving device notifications.
  void run(dolbyio::comms::sdk* sdk) override;

  // Runs the macOS specific UI configurations. This is mainly subscribing to video tracks and participant
  // events which are necessary for rendering.
  void ui_loop_on_helper_thread() override;

  // Run this macOS NsMainLoop
  void application_loop_on_main_thread() override;

  // Override of video_sink interface to receive incoming frames and be able to render them.
  void handle_frame(const std::string& stream_id,
                    const std::string&,
                    std::unique_ptr<dolbyio::comms::video_frame> frame) override;

  struct track_data {
    struct impl;
    virtual ~track_data() = default;
    virtual impl* get_impl() = 0;
  };

 private:
  void cleanup_before_exit();
  void update_participant(const dolbyio::comms::participant_info& participant);

  // Only accessed from the UI loop thread:
  std::vector<dolbyio::comms::event_handler_id> ev_handlers_{};

  // Accessed from the SDK thread and the video decoder's thread:
  std::mutex video_tracks_lock_{};
  std::unordered_map<std::string /* stream ID */, std::unique_ptr<track_data>> video_tracks_{};

  // Only accessed from the SDK thread:
  std::unordered_map<std::string /* participant ID */, dolbyio::comms::participant_info> participants_{};
};

using ui_impl = macosui;

} // namespace dolbyio::comms::sample
#endif  // Apple

