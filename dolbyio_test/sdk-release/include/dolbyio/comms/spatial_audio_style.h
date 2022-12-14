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
 * @brief The possible spatial audio styles of the conference. Setting
 * spatial_audio_style is possible only if the dolby_voice flag is set to true.
 * You can either use the individual or shared option.
 */
enum class DLB_COMMS_EXPORT spatial_audio_style {
  disabled,   /**< Spatial audio is disabled. */
  individual, /**< The individual option sets the spatial location that is based
                   on the spatial scene, local participant's position, and
                 remote participants' positions. This allows a client to control
                 the position using the local, self-contained logic. However,
                 the client has to communicate a large set of requests
                 constantly to the server, which increases network traffic, log
                   subsystem pressure, and complexity of the client-side
                 application. */
  shared,     /**< Spatial audio for shared scenes, The shared option sets the
                   spatial location that is based on the spatial scene and the local
                 participant's     position, while the relative positions among
                 participants are calculated by the     Dolby.io server. This way, the
                 spatial scene is shared by all participants, so     that each client
                 can set its own position and participate in the shared scene.     This
                 approach simplifies communication between the client and the server
                 and     decreases network traffic. */
};

}  // namespace dolbyio::comms
