#include "comms/sample/utilities/sdk/events.h"

#include <iostream>

namespace dolbyio::comms::sample {

namespace {
auto timestamp() {
  return std::chrono::system_clock::now().time_since_epoch().count();
}
}  // namespace

event_logger::event_logger(sdk& sdk, sdk_interactor_common& sdk_inter, bool log_active_speaker)
    : sdk_(sdk), sdk_inter_(sdk_inter) {
  handlers_.push_back(wait(sdk_.add_event_handler([](const dolbyio::comms::signaling_channel_exception& e) {
    std::cerr << "Signaling channel error: " << e.what() << std::endl;
  })));
  handlers_.push_back(
      wait(sdk_.conference().add_event_handler([this](const dolbyio::comms::conference_status_updated& e) {
        std::cerr << "Current conference status updated to: " << static_cast<int>(e.status) << std::endl;
        sdk_inter_.update_conference_status(e.status);
      })));
  handlers_.push_back(
      wait(sdk_.conference().add_event_handler([](const dolbyio::comms::conference_message_received& e) {
        std::cout << "Received message from " << e.user_id << ", message body: " << e.message << std::endl;
      })));
  handlers_.push_back(
      wait(sdk_.conference().add_event_handler([this](const dolbyio::comms::conference_invitation_received& e) {
        std::cerr << "Received invitation for " << e.conference_alias << " " << e.conference_id << " from "
                  << e.sender_info.name.value_or("N/A") << " " << e.sender_info.external_id.value() << std::endl;
        sdk_inter_.add_invitation(e);
      })));
  handlers_.push_back(
      wait(sdk_.session().add_event_handler([](const dolbyio::comms::subscription_events::participant_joined& e) {
        std::cout << "(subscription) Participant joined: " << e.participant.user_id << "("
                  << e.participant.info.name.value_or("no name") << ") in conference " << e.conference.conference_alias
                  << std::endl;
      })));
  handlers_.push_back(
      wait(sdk_.session().add_event_handler([](const dolbyio::comms::subscription_events::participant_left& e) {
        std::cout << "(subscription) Participant left: " << e.participant.user_id << "("
                  << e.participant.info.name.value_or("no name") << ") from conference "
                  << e.conference.conference_alias << std::endl;
      })));
  handlers_.push_back(
      wait(sdk_.session().add_event_handler([](const dolbyio::comms::subscription_events::active_participants& e) {
        std::cout << "(subscription) Active participants in conference " << e.conference.conference_alias << ": "
                  << e.participants.size() << " (total participants count: " << e.participants_count << ")"
                  << std::endl;
      })));
  handlers_.push_back(
      wait(sdk_.session().add_event_handler([](const dolbyio::comms::subscription_events::conference_status& e) {
        std::cout << "(subscription) Conference status: " << e.conference.conference_alias << " is live? " << e.live
                  << std::endl;
      })));
  handlers_.push_back(
      wait(sdk_.session().add_event_handler([](const dolbyio::comms::subscription_events::conference_created& e) {
        std::cout << "(subscription) Conference created: " << e.conference.conference_alias << std::endl;
      })));
  handlers_.push_back(
      wait(sdk_.session().add_event_handler([](const dolbyio::comms::subscription_events::conference_ended& e) {
        std::cout << "(subscription) Conference ended: " << e.conference.conference_alias << std::endl;
      })));
  if (log_active_speaker) {
    handlers_.push_back(
        wait(sdk_.conference().add_event_handler([this](const dolbyio::comms::active_speaker_change& e) {
          if (e.active_speakers.size()) {
            if (current_active_speaker_ != e.active_speakers[0]) {
              std::cerr << "ActiveSpeakerChanged: timestamp: " << timestamp()
                        << " participant_id: " << e.active_speakers[0] << std::endl;
              current_active_speaker_ = e.active_speakers[0];
            }
          } else if (!current_active_speaker_.empty()) {
            std::cerr << "ActiveSpeakerChanged timestamp: " << timestamp() << " No one is Speaking!\n";
            current_active_speaker_.clear();
          }
        })));
  }
  handlers_.push_back(wait(sdk_.conference().add_event_handler([](const dolbyio::comms::participant_added& e) {
    std::cout << "participant_added id:" << e.participant.user_id
              << " sending_audio: " << e.participant.is_sending_audio.value_or(false)
              << " audible_locally: " << e.participant.audible_locally.value_or(false) << std::endl;
  })));
  handlers_.push_back(wait(sdk_.conference().add_event_handler([](const dolbyio::comms::participant_updated& e) {
    std::cout << "participant_updated id:" << e.participant.user_id
              << " sending_audio: " << e.participant.is_sending_audio.value_or(false)
              << " audible_locally: " << e.participant.audible_locally.value_or(false) << std::endl;
  })));
  handlers_.push_back(wait(sdk_.session().add_event_handler(
      [](const dolbyio::comms::token_expired_event&) { std::cout << "SESSION TOKEN IS NOW EXPIRED!" << std::endl; })));
}

event_logger::~event_logger() {
  for (const auto& handler : handlers_)
    wait(handler->disconnect());
}

};  // namespace dolbyio::comms::sample
