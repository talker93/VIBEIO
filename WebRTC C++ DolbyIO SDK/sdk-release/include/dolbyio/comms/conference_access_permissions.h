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
 * @brief Conference Access Permissions provided to a particpant who is invited
 * to a conference.
 */
enum class DLB_COMMS_EXPORT conference_access_permissions {
  invite,             /**< Allows a participant to invite participants to a conference. */
  join,               /**< Allows a participant to join a conference. */
  send_audio,         /**< Allows a participant to send an audio stream during a
                         conference. */
  send_video,         /**< Allows a participant to send a video stream during a
                         conference. */
  share_screen,       /**< Allows a participant to share their screen during a
                         conference. */
  share_video,        /**< Allows a participant to share a video during a conference.
                       */
  share_file,         /**< Allows a participant to share a file during a conference. */
  send_message,       /**< Allows a participant to send a message to other
                         participants during a conference. Message size is limited to
                         16KB. */
  record,             /**< Allows a participant to record a conference. */
  stream,             /**< Allows a participant to stream a conference. */
  kick,               /**< Allows a participant to kick other participants from a conference.
                       */
  update_permissions, /**< Allows a participant to update other participants'
                        permissions. */
};

}  // namespace dolbyio::comms
