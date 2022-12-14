#include "ui.h"

#include <cassert>
#include <iostream>

namespace dolbyio::comms::sample {

ui_interface::ui_interface(int argc, char** argv) {
  cmd_handler.add_interactive_command("q", "exit", [this]() { quit_ = true; });

  cmd_handler.add_interactor(sdk_inter);
  cmd_handler.add_interactor(inject_inter);
  cmd_handler.add_interactor(dev_inter);

  cmd_handler.parse_command_line(argc, argv);
}

ui_interface::~ui_interface() {
  assert(!ui_thread_);
}

void ui_interface::run(dolbyio::comms::sdk* sdk) {
  // Set sdk on the command handler
  sdk_ = sdk;
  cmd_handler.set_sdk(sdk);

  if (inject_inter->media_io_enabled()) {
    // Create the default recorder plugin and set the recording formats
    recorder = dolbyio::comms::plugin::recorder::create(inject_inter->get_params().output_dir, *sdk,
                                                        inject_inter->get_params().aud_config,
                                                        inject_inter->get_params().vid_config);
  }

  event_inter =
      std::make_unique<dolbyio::comms::sample::event_logger>(*sdk, *sdk_inter, sdk_params().conf.log_active_speaker);

  ui_thread_ = std::make_unique<std::thread>([this]() { ui_loop_on_helper_thread(); });

  // Run the application message loop (for non-MacOS this is no-op and will just return)
  application_loop_on_main_thread();

  ui_thread_->join();
  ui_thread_ = {};
  cmd_handler.set_sdk(nullptr);
}

bool ui_interface::create_join_conference() {
  bool ready_join = false, joined = false;
  auto conf_params = sdk_inter->get_params().conf;

  if (conf_params.id) {
    sdk_inter->update_conference_id(conf_params.id.value());
    ready_join = true;
  } else if (conf_params.alias) {
    if (conf_params.alias->compare("DEMO") == 0) {
      sdk_inter->set_conference_info(wait(sdk_->conference().demo(sdk_inter->join_options().connection.spatial_audio)));
      joined = true;
    } else {
      sdk_inter->set_conference_info(wait(sdk_->conference().create(sdk_inter->conference_options())));
      ready_join = true;
    }
  }

  if (ready_join) {
    if (conf_params.join_as_user()) {
      sdk_inter->set_conference_info(
          wait(sdk_->conference().join(sdk_inter->conference_info(), sdk_inter->join_options())));
    } else {
      sdk_inter->set_conference_info(
          wait(sdk_->conference().listen(sdk_inter->conference_info(), sdk_inter->listen_options())));
    }
    joined = true;
  }
  return joined;
}

void ui_interface::ui_loop_on_helper_thread() {
  try {
    if (inject_inter->media_io_enabled()) {
      // Initialize injection if join as user
      if (sdk_params().conf.join_as_user())
        inject_inter->initialize_injection();
    }
    // Open up the session, logging in the participant specified in participant_info struct
    auto info = wait(sdk_->session().open(sdk_inter->session_options()));

    std::string user_id_message =
        "You (user: " + sdk_inter->session_options().name + " id: " + info.participant_id.value_or("N/A") + ")";
    std::string conf_info_string = "";

    // Create and/or join the conference specified by command line parameters.
    if (create_join_conference()) {
      // Turn of server audio processing if we are injecitng audio from file
      if (inject_inter->media_io_enabled() && sdk_params().conf.join_as_user()) {
        // Turn off audio processing
        sdk_->audio()
            .local()
            .set_capture_mode(dolbyio::comms::audio_capture_mode::unprocessed())
            .then([this]() {
              inject_inter->set_initial_capture(sdk_params().conf.join_with_audio(),
                                                sdk_params().conf.join_with_video());
            })
            .on_error(
                [](auto&&) { std::cerr << "Failed to set local audio to unprocessed, not starting injection!\n"; });
      }
      conf_info_string = "have joined the conference " + wait(sdk_->conference().get_current_conference()).id +
                         " as a" + (sdk_params().conf.join_as_user() ? "n Active-User" : " Listener");
    }
    std::cerr << user_id_message << conf_info_string << std::endl;

    try {
      // Set initial spatial position to 0,0,0
      if (sdk_params().conf.join_as_user() &&
          sdk_params().conf.spatial != dolbyio::comms::spatial_audio_style::disabled)
        sdk_inter->set_local_spatial_position().get();
    } catch (const std::exception& ex) {
      std::cerr << "Failed to update initial spatial position: " << ex.what() << std::endl;
    }

    std::cerr << "Input command. Each command is a single letter, when processed you "
                 "may be prompted for more input. You may enter multiple commands as "
                 "a string, they will be processed one by one."
              << std::endl;
    while (!quit_) {
      cmd_handler.print_interactive_options();

      std::string command;
      std::cin >> command;
      cmd_handler.handle_interactive_command(command);
    }
  } catch (const std::exception& ex) {
    std::cerr << "Failure: " << ex.what() << std::endl;
  }

  try {
    if (sdk_) {
      // TODO: Note I noticed a bug if the conference fails after joining to go
      // to joined, the current_conference_ is found in sdk_impl and will throw
      if (sdk_inter->conference_info().status == dolbyio::comms::conference_status::joined)
        wait(sdk_->conference().leave());
      wait(sdk_->session().close());
    }
  } catch (const std::exception& e) {
    std::cerr << "Error! " << e.what() << std::endl;
  }
}

}  // namespace dolbyio::comms::sample
