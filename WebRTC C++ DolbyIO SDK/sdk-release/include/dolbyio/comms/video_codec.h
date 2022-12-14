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
 * @brief The preferred video codec, either H264 or VP8.
 */
enum class DLB_COMMS_EXPORT video_codec {
  h264, /**< The H264 codec. */
  vp8,  /**< The VP8 codec. */
};

}  // namespace dolbyio::comms
