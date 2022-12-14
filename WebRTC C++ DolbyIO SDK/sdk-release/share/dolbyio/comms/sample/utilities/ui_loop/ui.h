#pragma once

#include <dolbyio/comms/sdk.h>

#include <dolbyio/comms/multimedia_streaming/injector.h>
#include <dolbyio/comms/multimedia_streaming/recorder.h>
#include <dolbyio/comms/sample/media_source/file/source_capture.h>
#include <dolbyio/comms/sample/utilities/sdk/device_manager/interactions.h>
#include "comms/sample/utilities/media/media_io_interactions.h"

#include "comms/sample/utilities/commands_handler.h"
#include "comms/sample/utilities/sdk/events.h"
#include "comms/sample/utilities/sdk/interactions.h"

#include <thread>

namespace dolbyio::comms::sample {

class ui_interface {
 public:
  // Parses the command line; the constructor sets up the command handler and parses the command line. This is not
  // extensible, and subclasses can not add their own behaviours.
  ui_interface(int argc, char** argv);

  // The destructor should do nothing.
  virtual ~ui_interface();

  // Spawns the helper thread, and runs the ui_loop_on_helper_thread(). Runs the application_loop_on_main_thread().
  // Joins the helper thread. The ui_loop_on_helper_thread() decides itself when to return and the thread becomes
  // joinable, so the run() method is blocking for as long as the ui loop is running; the
  // application_loop_on_main_thread() may be blocking or not. If it blocks, then the ui_loop_on_helper_thread() must
  // ensure unblocking it before it exits.
  virtual void run(dolbyio::comms::sdk* sdk);

  const auto& sdk_params() const { return sdk_inter->get_params(); }

 protected:
  // Helper method which will create a conference and join it
  bool create_join_conference();

  // opens the session, joins the conference, runs the ui loop until quit, and then leaves the conference. Run on helper
  // thread.
  virtual void ui_loop_on_helper_thread();

  // runs the application-specific message loop.
  virtual void application_loop_on_main_thread() {}
  dolbyio::comms::sdk* sdk_ = nullptr;

 private:
  std::unique_ptr<std::thread> ui_thread_{};

  bool quit_ = false;
  std::shared_ptr<sample::sdk_interactions> sdk_inter = std::make_shared<sdk_interactions>();
  std::shared_ptr<media_io_interactions> inject_inter =
      std::make_shared<media_io_interactions>(sdk_inter->get_params());
  std::unique_ptr<plugin::recorder> recorder{};
  std::shared_ptr<device_interactions> dev_inter = std::make_shared<device_interactions>();

  commands_handler cmd_handler{};
  std::unique_ptr<event_logger> event_inter{};
};

#ifndef __APPLE__
using ui_impl = ui_interface;
#endif

}  // namespace dolbyio::comms::sample
