/***************************************************************************
 * This program is licensed by the accompanying "license" file. This file is
 * distributed "AS IS" AND WITHOUT WARRANTY OF ANY KIND WHATSOEVER, either
 * express or implied. See the License for the specific language governing
 * permissions and limitations under the License.
 *
 *                Copyright (C) 2022 by Dolby Laboratories.
 ***************************************************************************/

#include "comms/sample/utilities/sdk/interactions.h"
#include "comms/sample/utilities/commands_handler.h"

#include <iostream>
#include <sstream>

namespace dolbyio::comms::sample {

namespace {

void throw_bad_args_error(const char* option, const std::string& value) {
  std::cerr << "Invalid value for " << option << " argument: " << value << std::endl;
  throw std::runtime_error("Bad cmdline arguments");
}

auto to_int(const std::string& value, const char* option) {
  int ret;
  try {
    ret = std::stoi(value);
  } catch (const std::exception& ex) {
    throw_bad_args_error(option, value + " " + ex.what());
  }
  if (std::to_string(ret) != value)
    throw_bad_args_error(option, value);
  return ret;
}

}  // namespace

void sdk_interactions::register_command_line_handlers(commands_handler& handler) {
  handler.add_command_line_switch(
      {"-u", "--user_name"}, "<name>\n\tUser name to use in conferences.",
      [this](const std::string& arg) { params_.user_name = arg; }, commands_handler::mandatory::yes);

  handler.add_command_line_switch({"-e"}, "<id>\n\tUser external ID.",
                                  [this](const std::string& arg) { params_.external_id = arg; });

  handler.add_command_line_switch(
      {"-k"}, "<token>\n\tAccess token required to connect to the DolbyIo backend.",
      [this](const std::string& arg) { params_.access_token = arg; }, commands_handler::mandatory::yes);

  handler.add_command_line_switch({"-l"},
                                  "[0..5]\n\tC++ SDK logging level (0=off, 1=error, 2=warning, 3=info, "
                                  "4=debug, 5=verbose; default: 3).",
                                  [this](const std::string& arg) {
                                    const auto ll = to_int(arg, "-l");
                                    if (ll < static_cast<int>(dolbyio::comms::log_level::OFF) ||
                                        ll > static_cast<int>(dolbyio::comms::log_level::VERBOSE))
                                      throw_bad_args_error("-l", arg);
                                    params_.sdk_log_level = static_cast<dolbyio::comms::log_level>(ll);
                                  });

  handler.add_command_line_switch(
      {"-ml"},
      "[0..5]\n\tMedia Engine logging level (0=off, 1=error, 2=warning, 3=info, 4=debug, 5=verbose; default: 0)",
      [this](const std::string& arg) {
        const auto ll = to_int(arg, "-ml");
        if (ll < static_cast<int>(dolbyio::comms::log_level::OFF) ||
            ll > static_cast<int>(dolbyio::comms::log_level::VERBOSE))
          throw_bad_args_error("-ml", arg);
        params_.me_log_level = static_cast<dolbyio::comms::log_level>(ll);
      });

  handler.add_command_line_switch({"-ld", "--log_dir"}, "<dir>\n\tLog to file in directory.",
                                  [this](const std::string& arg) { params_.log_dir = arg; });

  handler.add_command_line_switch({"-i"}, "<id>\n\tJoin conference with ID (no conference creation attempt).",
                                  [this](const std::string& arg) { params_.conf.id = arg; });

  handler.add_command_line_switch({"-c"},
                                  "<alias>\n\tJoin conference with alias (create if no such "
                                  "conference).\n\tUse -c DEMO to create and join a demo conference.",
                                  [this](const std::string& arg) { params_.conf.alias = arg; });

  handler.add_command_line_switch({"-t"}, "<token>\n\tCAT token.",
                                  [this](const std::string& arg) { params_.conf.cat = arg; });

  handler.add_command_line_switch({"-p"},
                                  "[user|listener]\n\tParticipant type (user=active, listener=inactive; "
                                  "default: listener)",
                                  [this](const std::string& arg) {
                                    if (arg == "user")
                                      params_.conf.nonlistener_join = true;
                                    else if (arg == "listener")
                                      params_.conf.nonlistener_join = false;
                                    else
                                      throw_bad_args_error("-p", arg);
                                  });

  handler.add_command_line_switch({"-m"},
                                  "[AV|A|V]\n\tInitial send media enabled "
                                  "(AV=audio+video, A=audio, V=video).",
                                  [this](const std::string& arg) {
                                    initial_params::conf::audio_video av{};
                                    av.audio = arg.find('A') != std::string::npos;
                                    av.video = arg.find('V') != std::string::npos;
                                    params_.conf.send_audio_video = av;
                                  });

  handler.add_command_line_switch({"-s", "--send_only"}, "\n\tJoin as send-only user.",
                                  [this]() { params_.conf.send_only = true; });

  handler.add_command_line_switch({"-V", "--max_vfs"}, "[0..25]\n\tMax video forwarding strategy (default: 25).",
                                  [this](const std::string& arg) { params_.conf.max_vfs = to_int(arg, "--max_vfs"); });

  handler.add_command_line_switch({"-spatial"},
                                  "[shared|individual|disabled]\n\tEnable spatial audio (default: "
                                  "disabled).",
                                  [this](const std::string& arg) {
                                    if (arg == "shared")
                                      params_.conf.spatial = spatial_audio_style::shared;
                                    else if (arg == "individual")
                                      params_.conf.spatial = spatial_audio_style::individual;
                                    else if (arg == "disabled")
                                      params_.conf.spatial = spatial_audio_style::disabled;
                                    else
                                      throw_bad_args_error("-spatial", arg);
                                  });

  handler.add_command_line_switch({"-log_speaker", "--log_speaker"}, "\n\tEnable Active Speaker logs.",
                                  [this]() { params_.conf.log_active_speaker = true; });

  handler.add_command_line_switch({"-simulcast"}, "\n\tJoin with simulcast enabled.",
                                  [this]() { params_.conf.simulcast = true; });
#ifdef __APPLE__
  handler.add_command_line_switch({"--no-display-video", "-no-display-video"}, "\n\tDisable displaying video windows.",
                                  [this]() { params_.display_video = false; });
#endif  // Apple
}

void sdk_interactions::register_interactive_commands(commands_handler& handler) {
  handler.add_interactive_command("v", "stop video", [this]() { stop_video(); });
  handler.add_interactive_command("V", "start video", [this]() { start_video(); });
  handler.add_interactive_command("P", "list participants", [this]() { list_participants(); });
  handler.add_interactive_command("k", "mute output audio", [this]() { mute_output(true); });
  handler.add_interactive_command("K", "unmute output audio", [this]() { mute_output(false); });
  handler.add_interactive_command("m", "send message", [this]() { send_message(); });
  handler.add_interactive_command("S", "set spatial audio configuration",
                                  [this]() { set_spatial_configuration(get_update_spatial_settings()); });
  handler.add_interactive_command("subscribe", "subscribe to all conference events", [this]() { subscribe(); });
  handler.add_interactive_command("unsubscribe", "unsubscribe from all conference events", [this]() { unsubscribe(); });
  handler.add_interactive_command("join", "join a conference by id", [this]() { join(); });
  handler.add_interactive_command("create", "create a conference by alias", [this]() { create_and_join(); });
  handler.add_interactive_command("accept", "accept invitation for a conference", [this]() { accept_invitation(); });
  handler.add_interactive_command("decline", "decline invitation for a conference", [this]() { decline_invitation(); });
  handler.add_interactive_command("mute", "Mute microphone", [this]() { mute_input(true); });
  handler.add_interactive_command("unmute", "Unmute microphone", [this]() { mute_input(false); });
  handler.add_interactive_command("stop-audio", "Stop audio for yourself", [this]() { stop_audio(); });
  handler.add_interactive_command("start-audio", "Start audio for yourself", [this]() { start_audio(); });
  handler.add_interactive_command("mute-remote", "Mute remote participant (only available for non-DVC)",
                                  [this]() { mute_remote(true); });
  handler.add_interactive_command("unmute-remote", "Unmute remote participant (only available for non-DVC)",
                                  [this]() { mute_remote(false); });
  handler.add_interactive_command("stop-audio-remote", "Stop audio for remote participant (only avaliable for DVC)",
                                  [this]() { stop_remote_audio(); });
  handler.add_interactive_command("start-audio-remote", "Start audio for remote participant (only available for DVC)",
                                  [this]() { start_remote_audio(); });
  handler.add_interactive_command("audio-level", "Get audio levels for a single participant",
                                  [this]() { get_audio_level(); });
  handler.add_interactive_command("audio-levels", "Get audio levels for all participants in conferences",
                                  [this]() { get_audio_levels(); });
  handler.add_interactive_command("set-audio-capture-mode", "Sets audio capture mode",
                                  [this]() { set_audio_capture_mode(); });
  handler.add_interactive_command("get-audio-capture-mode", "Gets audio capture mode",
                                  [this]() { get_audio_capture_mode(); });
}

dolbyio::comms::services::session::user_info sdk_interactions::session_options() const {
  dolbyio::comms::services::session::user_info participant{};
  participant.externalId = params_.external_id;
  participant.name = params_.user_name;
  return participant;
}

dolbyio::comms::services::conference::conference_options sdk_interactions::conference_options() const {
  dolbyio::comms::services::conference::conference_options create{};
  create.alias = params_.conf.alias;
  create.params.spatial_audio_style = params_.conf.spatial;
  return create;
}

dolbyio::comms::services::conference::join_options sdk_interactions::join_options() const {
  dolbyio::comms::services::conference::join_options join{};
  join.constraints.audio = params_.conf.join_with_audio();
  join.constraints.video = params_.conf.join_with_video();
  join.constraints.send_only = params_.conf.send_only;
  join.connection.conference_access_token = params_.conf.cat;
  join.connection.max_video_forwarding = params_.conf.max_vfs;
  join.connection.spatial_audio = (params_.conf.spatial != spatial_audio_style::disabled);
  join.connection.simulcast = params_.conf.simulcast;
  return join;
}

dolbyio::comms::services::conference::listen_options sdk_interactions::listen_options() const {
  dolbyio::comms::services::conference::listen_options listen{};
  listen.connection.conference_access_token = params_.conf.cat;
  listen.connection.max_video_forwarding = params_.conf.max_vfs;
  listen.connection.spatial_audio = (params_.conf.spatial != spatial_audio_style::disabled);
  listen.connection.simulcast = params_.conf.simulcast;
  return listen;
}

void sdk_interactions::add_invitation(const dolbyio::comms::conference_invitation_received& invitation) {
  std::lock_guard<std::mutex> lock(sdk_inter_lock_);
  conference_invitations_.emplace(invitation.conference_id, invitation);
}

// TODO: Not sure if I need these locks for conference info actually?
void sdk_interactions::update_conference_status(const dolbyio::comms::conference_status& status) {
  std::lock_guard<std::mutex> lock(sdk_inter_lock_);
  conf_info_.status = status;
}

void sdk_interactions::update_conference_id(const std::string& id) {
  std::lock_guard<std::mutex> lock(sdk_inter_lock_);
  conf_info_.id = id;
}

void sdk_interactions::set_conference_info(const dolbyio::comms::conference_info& info) {
  std::lock_guard<std::mutex> lock(sdk_inter_lock_);
  conf_info_ = info;
}

void sdk_interactions::create_and_join() {
  std::string alias;
  std::cerr << "Please enter Alias of Conference to Create:\n";
  std::cin >> alias;

  // TODO: Maybe add CAT input via command line option here?

  params_.conf.alias = alias;
  auto create = conference_options();
  conf_info_ = wait(sdk_->conference().create(create));

  if (params_.conf.join_as_user()) {
    auto join = join_options();
    if (params_.conf.alias == "DEMO")
      conf_info_ = wait(sdk_->conference().demo(params_.conf.spatial != dolbyio::comms::spatial_audio_style::disabled));
    else
      conf_info_ = wait(sdk_->conference().join(conf_info_, join));
  } else {
    auto listen = listen_options();
    conf_info_ = wait(sdk_->conference().listen(conf_info_, listen));
  }
}

void sdk_interactions::join() {
  std::string id;
  std::cerr << "Please enter ID of Conference to Join:\n";
  std::cin >> id;

  conf_info_.id = id;
  conf_info_.alias = {};

  if (params_.conf.join_as_user()) {
    auto join = join_options();
    conf_info_ = wait(sdk_->conference().join(conf_info_, join));
  } else {
    auto listen = listen_options();
    conf_info_ = wait(sdk_->conference().listen(conf_info_, listen));
  }
}

void sdk_interactions::accept_invitation() {
  auto invitation = consume_invitation();
  if (invitation) {
    {
      std::lock_guard<std::mutex> lock(sdk_inter_lock_);
      conf_info_.id = invitation->conference_id;
    }
    if (params_.conf.join_as_user()) {
      wait(sdk_->conference().join(conf_info_, join_options()));
    } else {
      auto listen = listen_options();
      wait(sdk_->conference().listen(conf_info_, listen));
    }
  }
}

void sdk_interactions::decline_invitation() {
  auto invitation = consume_invitation();
  if (invitation) {
    wait(sdk_->conference().decline_invitation(invitation->conference_id));
  }
}

void sdk_interactions::stop_video() {
  wait(sdk_->video().local().stop());
}

void sdk_interactions::start_video() {
  wait(sdk_->video().local().start({}, params_.video_frame_handler));
}

void sdk_interactions::stop_audio() {
  wait(sdk_->audio().local().stop());
}

void sdk_interactions::start_audio() {
  wait(sdk_->audio().local().start());
}

void sdk_interactions::stop_remote_audio() {
  std::string participant;
  std::cerr << "Enter the ParticipantID for whom to Stop Audio:\n";
  std::cin >> participant;
  wait(sdk_->audio().remote().stop(participant));
}

void sdk_interactions::start_remote_audio() {
  std::string participant;
  std::cerr << "Enter the ParticipantID for whom to Start Audio:\n";
  std::cin >> participant;
  wait(sdk_->audio().remote().start(participant));
}

void sdk_interactions::list_participants() {
  auto conf = wait(sdk_->conference().get_current_conference());
  auto info = wait(sdk_->session().session_info());
  for (const auto& p : conf.participants)
    std::cout << ((info.participant_id == p.second.user_id) ? "(YOU) -> " : "         ")
              << p.second.info.name.value_or("(no name)") << ", id=" << p.second.user_id << std::endl;
}

void sdk_interactions::mute_input(bool muted) {
  wait(sdk_->conference().mute(muted));
}

void sdk_interactions::mute_remote(bool muted) {
  std::string participant;
  std::cerr << "Enter the ParticipantID for whom to " << (muted ? "Mute" : "Unmute") << ":\n";
  std::cin >> participant;
  wait(sdk_->conference().remote_mute(muted, participant));
}

void sdk_interactions::mute_output(bool muted) {
  wait(sdk_->conference().mute_output(muted));
}

void sdk_interactions::send_message() {
  std::cout << "Put message content and press enter:" << std::endl;
  std::string msg;
  std::cin.ignore();
  std::getline(std::cin, msg);
  wait(sdk_->conference().send(msg));
}

namespace {
std::vector<notification_subscription> make_subscriptions_list() {
  std::cout << "Put conference alias and press enter:" << std::endl;
  std::string alias;
  std::cin.ignore();
  std::getline(std::cin, alias);
  std::vector<notification_subscription> subscriptions;
  subscriptions.emplace_back(alias, notification_subscription_type::active_participants);
  subscriptions.emplace_back(alias, notification_subscription_type::conference_created);
  subscriptions.emplace_back(alias, notification_subscription_type::conference_ended);
  subscriptions.emplace_back(alias, notification_subscription_type::participant_joined);
  subscriptions.emplace_back(alias, notification_subscription_type::participant_left);
  return subscriptions;
}
}  // namespace

void sdk_interactions::subscribe() {
  auto subs = make_subscriptions_list();
  std::cout << "Subscribing to events in conference " << subs[0].conference_alias << std::endl;
  wait(sdk_->session().subscribe(subs));
}

void sdk_interactions::unsubscribe() {
  auto subs = make_subscriptions_list();
  std::cout << "Unsubscribing from events in conference " << subs[0].conference_alias << std::endl;
  wait(sdk_->session().unsubscribe(subs));
}

std::optional<conference_invitation_received> sdk_interactions::consume_invitation() {
  if (conference_invitations_.empty()) {
    std::cerr << "There are no Conference invitations!\n";
    return {};
  }
  std::cerr << "Here are the possible conference invtations to chose from:\n";
  for (const auto& invitation : conference_invitations_) {
    std::cerr << "Inviter ExternalID: " << invitation.second.sender_info.external_id.value()
              << " Conference ID: " << invitation.first << std::endl;
  }
  std::string conf_id;
  std::cerr << "Please enter the Conference ID for Invitation:\n";
  std::cin >> conf_id;

  std::lock_guard<std::mutex> lock(sdk_inter_lock_);
  auto iter = conference_invitations_.find(conf_id);
  if (iter != conference_invitations_.end()) {
    auto invitation = iter->second;
    std::cerr << "conusme invite: " << invitation.conference_id << invitation.conference_alias << std::endl;
    conference_invitations_.erase(iter);
    return invitation;
  }
  return {};
}

void sdk_interactions::list_invitations() {
  std::lock_guard<std::mutex> lock(sdk_inter_lock_);
  for (const auto& invitation : conference_invitations_) {
    std::cerr << "Invitation: " << invitation.second.conference_alias << " " << invitation.second.conference_id
              << " from " << invitation.second.sender_info.name.value_or("N/A") << " "
              << invitation.second.sender_info.external_id.value() << std::endl;
  }
}

void sdk_interactions::get_audio_level() {
  std::string id;
  std::cerr << "Please enter the Participant ID whose Audio Level you want:\n";
  std::cin >> id;
  auto level = wait(sdk_->conference().get_audio_level(id));

  std::cerr << "Audio Level for: " << id << " is: " << std::to_string(level) << std::endl;
}

void sdk_interactions::get_audio_levels() {
  std::cerr << "Audio Levels for All Participants:\n";
  auto levels = wait(sdk_->conference().get_all_audio_levels());

  for (const auto& iter : levels) {
    std::cerr << "Audio Level for: " << iter.participant_id << " is: " << std::to_string(iter.level) << std::endl;
  }
}

void sdk_interactions::set_audio_capture_mode() {
  std::string processing;
  std::cerr << "Please enter the audio capture setting (as number):\n"
            << "   1 - standard (high noise reduction)\n"
            << "   2 - standard (low noise reduction)\n"
            << "   3 - unprocessed" << std::endl;
  std::cin >> processing;

  using namespace dolbyio::comms;
  if (processing == "1") {
    wait(sdk_->audio().local().set_capture_mode(audio_capture_mode::standard{noise_reduction::high}));
  } else if (processing == "2") {
    wait(sdk_->audio().local().set_capture_mode(audio_capture_mode::standard{noise_reduction::low}));
  } else if (processing == "3") {
    wait(sdk_->audio().local().set_capture_mode(audio_capture_mode::unprocessed{}));
  } else {
    throw std::runtime_error("Invalid processing mode");
  }
}

void sdk_interactions::get_audio_capture_mode() {
  auto processing = wait(sdk_->audio().local().get_capture_mode());

  struct mode_printer {
    static void print(const dolbyio::comms::audio_capture_mode::unprocessed&) {
      std::cerr << "Audio processing mode: unprocessed" << std::endl;
    }
    static void print(const dolbyio::comms::audio_capture_mode::standard& mode) {
      std::string nr{};
      switch (mode.reduce_noise) {
        case dolbyio::comms::noise_reduction::high:
          nr = "high";
          break;
        case dolbyio::comms::noise_reduction::low:
          nr = "low";
          break;
      }

      std::cerr << "Audio processing mode: standard (" << nr << " noise reduction)" << std::endl;
    }
  };

  std::visit([](const auto& mode) { mode_printer::print(mode); }, processing);
}

std::vector<std::string> sdk_interactions::split(const std::string& s, char delim) {
  std::vector<std::string> result;
  std::stringstream ss(s);
  std::string item;

  while (getline(ss, item, delim)) {
    result.push_back(item);
  }

  return result;
}

bool sdk_interactions::get_spatial_position(dolbyio::comms::spatial_audio_batch_update& batch_update,
                                            const std::string& msg) {
  auto vals = split(msg);
  if (vals.size() != 5) {
    std::cout << "Invalid pos command" << std::endl;
    return false;
  }

  try {
    auto participant = vals[1];
    auto x = std::stod(vals[2]);
    auto y = std::stod(vals[3]);
    auto z = std::stod(vals[4]);

    std::cout << "Adding spatial position: " << participant << " (" << x << ";" << y << ";" << z << ")" << std::endl;
    batch_update.set_spatial_position(participant, {x, y, z});
  } catch (...) {
    std::cerr << "Improper spatial positonal values, please try again!\n";
    return false;
  }
  return true;
}

bool sdk_interactions::get_spatial_direction(dolbyio::comms::spatial_audio_batch_update& batch_update,
                                             const std::string& msg) {
  auto vals = split(msg);
  if (vals.size() != 4) {
    std::cout << "Invalid dir command" << std::endl;
    return false;
  }
  try {
    auto x = std::stod(vals[1]);
    auto y = std::stod(vals[2]);
    auto z = std::stod(vals[3]);
    std::cout << "Adding spatial direction: (" << x << ";" << y << ";" << z << ")" << std::endl;

    batch_update.set_spatial_direction({x, y, z});
  } catch (...) {
    std::cerr << "Improper spatial directional values, please try again!\n";
    return false;
  }
  return true;
}

bool sdk_interactions::get_spatial_environment(dolbyio::comms::spatial_audio_batch_update& batch_update,
                                               const std::string& msg) {
  auto vals = split(msg);
  if (vals.size() != 13) {
    std::cout << "Invalid dir command" << std::endl;
    return false;
  }
  try {
    auto scale_x = std::stod(vals[1]);
    auto scale_y = std::stod(vals[2]);
    auto scale_z = std::stod(vals[3]);
    auto for_x = std::stod(vals[4]);
    auto for_y = std::stod(vals[5]);
    auto for_z = std::stod(vals[6]);
    auto up_x = std::stod(vals[7]);
    auto up_y = std::stod(vals[8]);
    auto up_z = std::stod(vals[9]);
    auto right_x = std::stod(vals[10]);
    auto right_y = std::stod(vals[11]);
    auto right_z = std::stod(vals[12]);

    std::cout << "Adding spatial environemnt: scale (" << scale_x << ";" << scale_y << ";" << scale_z << ")"
              << " forward: (" << for_x << ";" << for_y << ";" << for_z << ")"
              << " up: (" << up_x << ";" << up_y << ";" << up_z << ")"
              << " right: (" << right_x << ";" << right_y << ";" << right_z << ")" << std::endl;

    batch_update.set_spatial_environment({scale_x, scale_y, scale_z}, {for_x, for_y, for_z}, {up_x, up_y, up_z},
                                         {right_x, right_y, right_z});
  } catch (...) {
    std::cerr << "Improper spatial environment settings, please try again!\n";
    return false;
  }
  return true;
}

dolbyio::comms::spatial_audio_batch_update sdk_interactions::get_update_spatial_settings() {
  dolbyio::comms::spatial_audio_batch_update batch_update{};

  bool done = false;
  std::cin.ignore();
  while (!done) {
    std::cout << "Setting spatial config. Available commands:\n"
              << "    pos <participant_id> <x> <y> <z>\n"
              << "    dir <x> <y> <z>\n"
              << "    env (scale)<x> <y> <z> (forward)<x> <y> <z> (up)<x> <y> "
                 "<z> (right)<x> <y> <z>\n"
              << "    done - input this command when done!" << std::endl;
    std::string msg;
    std::getline(std::cin, msg);

    if (msg.find("done") == 0) {
      done = true;
    } else if (msg.find("pos") == 0) {
      if (!get_spatial_position(batch_update, msg))
        continue;
    } else if (msg.find("dir") == 0) {
      if (!get_spatial_direction(batch_update, msg))
        continue;
    } else if (msg.find("env") == 0) {
      if (!get_spatial_environment(batch_update, msg))
        continue;
    } else {
      std::cout << "Invalid spatial command: " << msg << std::endl;
    }
  }
  return batch_update;
}

std::future<void> sdk_interactions::set_local_spatial_position() {
  auto prom = std::make_shared<std::promise<void>>();
  auto fut = prom->get_future();
  auto user_info = wait(sdk_->session().session_info());
  if (!user_info.participant_id) {
    prom->set_exception(std::make_exception_ptr("No participant ID can't set initial position!"));
    return fut;
  }

  auto participant = user_info.participant_id.value();

  dolbyio::comms::spatial_audio_batch_update update;
  std::cout << "Adding initial spatial position {0,0,0} for local participant:" << participant << std::endl;
  update.set_spatial_position(participant, {0, 0, 0});
  set_spatial_configuration(std::move(update), prom);
  return fut;
}

void sdk_interactions::set_spatial_configuration(spatial_audio_batch_update&& batch_update,
                                                 std::shared_ptr<std::promise<void>> waiter) {
  sdk_->conference()
      .update_spatial_audio_configuration(std::move(batch_update))
      .then([waiter]() {
        std::cout << "Spatial positions updated" << std::endl;
        if (waiter)
          waiter->set_value();
      })
      .on_error([waiter](std::exception_ptr&& e) {
        try {
          std::rethrow_exception(e);
        } catch (const std::exception& e) {
          std::cout << "Failed to update spatial positions: " << e.what() << std::endl;
        }
        if (waiter)
          waiter->set_exception(e);
      });
}

};  // namespace dolbyio::comms::sample
