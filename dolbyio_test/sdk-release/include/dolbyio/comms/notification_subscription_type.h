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

namespace dolbyio::comms {

/**
 * @brief The subscription type.
 */
enum class DLB_COMMS_EXPORT notification_subscription_type {
  conference_created,   ///< Subscribe to conference created events.
  conference_ended,     ///< Subscribe to conference ended events.
  participant_joined,   ///< Subscribe to participant joining the conference.
  participant_left,     ///< Subscribe to participant leaving the conference.
  active_participants,  ///< Subscribe to periodic notifications about the
                        ///< active participants in the conference.
};

}  // namespace dolbyio::comms
