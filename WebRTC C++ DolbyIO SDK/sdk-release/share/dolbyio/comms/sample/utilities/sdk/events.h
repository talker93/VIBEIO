#pragma once

/***************************************************************************
 * This program is licensed by the accompanying "license" file. This file is
 * distributed "AS IS" AND WITHOUT WARRANTY OF ANY KIND WHATSOEVER, either
 * express or implied. See the License for the specific language governing
 * permissions and limitations under the License.
 *
 *                Copyright (C) 2022 by Dolby Laboratories.
 ***************************************************************************/

#include <dolbyio/comms/sdk.h>
#include "comms/sample/utilities/sdk/interactions.h"

namespace dolbyio::comms::sample {

 // This class is used to subscribe to various events sent out by the SDK and log there data.
class event_logger {
 public:
  event_logger(sdk& sdk, sdk_interactor_common& sdk_inter, bool log_active_speaker = false);
  ~event_logger();

 private:
  sdk& sdk_;
  sdk_interactor_common& sdk_inter_;

  std::vector<event_handler_id> handlers_{};
  std::string current_active_speaker_{};
};

};  // namespace dolbyio::comms::sample
