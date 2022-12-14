/***************************************************************************
 * This program is licensed by the accompanying "license" file. This file is
 * distributed "AS IS" AND WITHOUT WARRANTY OF ANY KIND WHATSOEVER, either
 * express or implied. See the License for the specific language governing
 * permissions and limitations under the License.
 *
 *                Copyright (C) 2021-2022 by Dolby Laboratories.
 ***************************************************************************/

#include <dolbyio/comms/sdk.h>

#include "comms/sample/utilities/commands_handler.h"
#include "comms/sample/utilities/sdk/device_manager/interactions.h"
#include "comms/sample/utilities/sdk/events.h"
#include "comms/sample/utilities/sdk/interactions.h"
#include "comms/sample/utilities/ui_loop/ui.h"
#ifdef __APPLE__
#include "comms/sample/utilities/ui_loop/macos_ui.h"
#endif  // Apple

#include <iostream>
#include <sstream>

namespace sample = dolbyio::comms::sample;

int main(int argc, char** argv) {
  // On MacOS the mainloop is needed for camera capture and UI rendering to work. Hence this is the
  // reason for this UI abstraction which will create the mainloop on MacOS and run the SDK loop in a separate
  // thread.
  std::unique_ptr<dolbyio::comms::sdk> sdk{};
  std::unique_ptr<sample::ui_interface> ui{};

  try {
    ui = std::make_unique<sample::ui_impl>(argc, argv);

    // Set log settings
    dolbyio::comms::sdk::set_log_settings(
        {ui->sdk_params().sdk_log_level, ui->sdk_params().me_log_level, ui->sdk_params().log_dir});

    // Create the SDK passing in the token and a refresh token callback
    sdk = dolbyio::comms::sdk::create(
        ui->sdk_params().access_token,
        [](std::unique_ptr<dolbyio::comms::refresh_token>&& refresh_token) { (void)refresh_token; });

    // Run the UI loop abstraction
    ui->run(sdk.get());
  } catch (const std::exception& ex) {
    std::cerr << "Failure: " << ex.what() << std::endl;
  }

  return 0;
}
