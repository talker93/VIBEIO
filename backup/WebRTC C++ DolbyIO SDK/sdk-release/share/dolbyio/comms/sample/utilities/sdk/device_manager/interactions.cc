#include "comms/sample/utilities/sdk/device_manager/interactions.h"
#include "comms/sample/utilities/commands_handler.h"

#include <iostream>

namespace dolbyio::comms::sample {

void device_interactions::enable() {
  devices_ = wait(sdk_->device_management().get_audio_devices());
  video_devices_ = wait(sdk_->device_management().get_video_devices());
  curr_input_device_ = wait(sdk_->device_management().get_current_audio_input_device());
  if (curr_input_device_)
    std::cerr << curr_input_device_->name() << std::endl;

  curr_output_device_ = wait(sdk_->device_management().get_current_audio_output_device());
  if (curr_output_device_)
    std::cerr << curr_output_device_->name() << std::endl;

  handlers_.push_back(
      wait(sdk_->device_management().add_event_handler([this](const dolbyio::comms::audio_device_added& e) {
        if (std::find(devices_.begin(), devices_.end(), e.device) == devices_.end())
          devices_.push_back(e.device);
      })));
  handlers_.push_back(
      wait(sdk_->device_management().add_event_handler([this](const dolbyio::comms::audio_device_removed& e) {
        devices_.erase(
            std::remove_if(devices_.begin(), devices_.end(), [&e](const auto& device) { return device == e.uid; }),
            devices_.end());
      })));
  handlers_.push_back(
      wait(sdk_->device_management().add_event_handler([this](const dolbyio::comms::audio_device_changed& e) {
        std::cerr << "Device changed event: " << e.device.name() << std::endl;
        if (e.no_device) {
          if (e.device.direction() & dolbyio::comms::dvc_device::input) {
            std::cerr << "No input devices\n";
            curr_input_device_ = {};
          }
          if (e.device.direction() & dolbyio::comms::dvc_device::output) {
            std::cerr << "No output devices\n";
            curr_output_device_ = {};
          }
        } else {
          if (e.utilized_direction & dolbyio::comms::dvc_device::input) {
            curr_input_device_ = e.device;
            std::cerr << "New input:" << curr_input_device_->name() << std::endl;
          }
          if (e.utilized_direction & dolbyio::comms::dvc_device::output) {
            curr_output_device_ = e.device;
            std::cerr << "New output:" << curr_output_device_->name() << std::endl;
          }
        }
      })));
  handlers_.push_back(
      wait(sdk_->device_management().add_event_handler([](const dolbyio::comms::audio_device_timeout_failure&) {
        std::cerr << "Prolonged audio device problem, you may have no audio." << std::endl;
      })));
  handlers_.push_back(
      wait(sdk_->device_management().add_event_handler([this](const dolbyio::comms::video_device_added& e) {
        if (std::find_if(video_devices_.begin(), video_devices_.end(), [device{e.device}](const auto& dev) {
              return device.unique_id == dev.unique_id;
            }) == video_devices_.end()) {
          std::cerr << "New video device added: " << e.device.display_name << std::endl;
          video_devices_.push_back(e.device);
        }
      })));
  handlers_.push_back(
      wait(sdk_->device_management().add_event_handler([this](const dolbyio::comms::video_device_removed& e) {
        video_devices_.erase(std::remove_if(video_devices_.begin(), video_devices_.end(),
                                            [&e](const auto& device) {
                                              if (device.unique_id == e.uid) {
                                                std::cerr << "Video device removed: " << device.display_name
                                                          << std::endl;
                                                return true;
                                              }
                                              return false;
                                            }),
                             video_devices_.end());
      })));
}

void device_interactions::disable() {
  for (auto& iter : handlers_)
    wait(iter->disconnect());
  handlers_.clear();
  devices_.clear();
}

void device_interactions::set_sdk(dolbyio::comms::sdk* sdk) {
  if (sdk_)
    disable();
  sdk_ = sdk;
  if (sdk_)
    enable();
}

void device_interactions::register_command_line_handlers(commands_handler&) {}

void device_interactions::register_interactive_commands(commands_handler& handler) {
  handler.add_interactive_command("g", "get audio devices", [this]() { get_audio_devices(); });
  handler.add_interactive_command("i", "set input audio device", [this]() { set_input_audio_device(); });
  handler.add_interactive_command("o", "set output audio device", [this]() { set_output_audio_device(); });
  handler.add_interactive_command("get-cameras", "get video devices", [this]() { get_video_devices(); });
}

void device_interactions::get_audio_devices() {
  devices_ = wait(sdk_->device_management().get_audio_devices());
  for (const auto& iter : devices_)
    std::cerr << iter.name() << std::endl;
}

void device_interactions::set_input_audio_device() {
  std::cout << "Enter input device name:" << std::endl;
  std::string msg;
  std::cin.ignore();
  std::getline(std::cin, msg);
  auto iter = std::find_if(devices_.begin(), devices_.end(), [&msg](const auto& device) {
    return device.name() == msg && (device.direction() & dolbyio::comms::dvc_device::input);
  });
  if (iter != devices_.end())
    wait(sdk_->device_management().set_preferred_input_audio_device(*iter));
  else
    std::cerr << "Device not found\n";
}

void device_interactions::set_output_audio_device() {
  std::cout << "Enter output device name:" << std::endl;
  std::string msg;
  std::cin.ignore();
  std::getline(std::cin, msg);
  auto iter = std::find_if(devices_.begin(), devices_.end(), [&msg](const auto& device) {
    return device.name() == msg && (device.direction() & dolbyio::comms::dvc_device::output);
  });
  if (iter != devices_.end())
    wait(sdk_->device_management().set_preferred_output_audio_device(*iter));
  else
    std::cerr << "Device not found\n";
}

void device_interactions::get_video_devices() {
  video_devices_ = wait(sdk_->device_management().get_video_devices());
  for (const auto& iter : video_devices_)
    std::cerr << iter.display_name << std::endl;
}

};  // namespace dolbyio::comms::sample
