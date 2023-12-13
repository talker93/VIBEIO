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
 * @brief The type of participant.
 */
enum class DLB_COMMS_EXPORT participant_type {
  user,     /**< A participant who can send and receive an audio and video stream
               during a conference. */
  listener, /**<  A participant who can receive audio and video streams, but
               cannot send any stream to a conference. */
  /// @cond DO_NOT_DOCUMENT
  speaker,
  pstn,
  mixer,
  dvcs,
  none,
  robot,
  robot_speaker,
  robot_listener,
  robot_pstn,
  robot_mixer,
  robot_none,
  /// @endcond
};

}  // namespace dolbyio::comms
