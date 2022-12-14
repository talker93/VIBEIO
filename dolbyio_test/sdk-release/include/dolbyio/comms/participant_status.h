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
 * @brief The possible statuses of conference participants.
 */
enum class DLB_COMMS_EXPORT participant_status {
  reserved,   /**< A participant is invited to a conference and is waiting for
                 an invitation. */
  connecting, /**< A participant received a conference invitation and is
                 connecting to the conference. */
  on_air,     /**< A participant successfully connected to the conference. */
  decline,    /**< An invited participant declined the conference invitation. */
  inactive,   /**< A participant does not send any audio, video, or screen-share
                 stream to the conference. */
  left,       /**< A participant left the conference. */
  warning,    /**< A participant experiences a peer connection problem. */
  error,      /**< A participant cannot connect to the conference due to a peer
                 connection failure. */
};

}  // namespace dolbyio::comms
