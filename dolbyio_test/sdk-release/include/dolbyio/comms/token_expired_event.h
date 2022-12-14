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
 * @brief The event notifying about the access token expiration.
 *
 * Upon the reception of this event, the application will know that the access token used by the SDK has expired. Note
 * that the application will receive two token refresh requests before that. If the application does not deliver new,
 * valid token, and the old one expires, the application can expect the conference to be closed, and most API calls will
 * just return the errors.
 */
struct DLB_COMMS_EXPORT token_expired_event {};

}  // namespace dolbyio::comms
