#pragma once

/***************************************************************************
 * This program is licensed by the accompanying "license" file. This file is
 * distributed "AS IS" AND WITHOUT WARRANTY OF ANY KIND WHATSOEVER, either
 * express or implied. See the License for the specific language governing
 * permissions and limitations under the License.
 *
 *                Copyright (C) 2022 by Dolby Laboratories.
 ***************************************************************************/

#include <dolbyio/comms/exports.h>

#include <dolbyio/comms/participant_info.h>

#include <optional>
#include <string>

namespace dolbyio::comms {

/**
 * @brief Emitted when a participant successfully joins the conference.
 * @ingroup conference_service
 * @ingroup events
 */
struct DLB_COMMS_EXPORT participant_added {
  participant_added() = default;
  participant_added(participant_info&& info, const std::string& conf_id) : participant(info), conference_id(conf_id) {}
  participant_info participant; /**< Information about the participant. */
  std::string conference_id;    /**< Conference ID. */
};

/**
 * @brief Emitted when a participant changes status.
 * @ingroup conference_service
 * @ingroup events
 */
struct DLB_COMMS_EXPORT participant_updated {
  participant_updated() = default;
  participant_updated(participant_info&& info, const std::string& conf_id)
      : participant(info), conference_id(conf_id) {}
  participant_info participant; /**< Updated information about the participant. */
  std::string conference_id;    /**< Conference ID. */
};

};  // namespace dolbyio::comms
