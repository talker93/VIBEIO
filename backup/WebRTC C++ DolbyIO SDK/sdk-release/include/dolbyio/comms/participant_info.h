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

#include <dolbyio/comms/participant_status.h>
#include <dolbyio/comms/participant_type.h>

#include <optional>
#include <string>

namespace dolbyio::comms {

/**
 * @brief Contains the current status of a conference participant and
 * information whether the participant's audio is enabled.
 * @ingroup conference_service
 */
struct DLB_COMMS_EXPORT participant_info {
  using id = std::string;

  /**
   * @brief Information about a conference participant.
   */
  struct info {
    std::optional<std::string> name{};        /**< The participant's name. */
    std::optional<std::string> external_id{}; /**< The external unique identifier that the customer's
                                                 application can add to the participant while opening a
                                                 session. If a participant uses the same external ID in
                                                 conferences, the participant's ID also remains the
                                                 same across all sessions.*/
    std::optional<std::string> avatar_url{};  /**< The URL of the participant's avatar. */
  };

  participant_info() = default;
  participant_info(id&& user_id,
                   std::optional<participant_type> type,
                   std::optional<participant_status> status,
                   info info,
                   std::optional<bool> is_sending_audio,
                   std::optional<bool> audible_locally)
      : user_id(std::move(user_id)),
        type(type),
        status(status),
        info(std::move(info)),
        is_sending_audio(is_sending_audio),
        audible_locally(audible_locally) {}

  id user_id{};                               /**< The unique identifier of the participant. */
  std::optional<participant_type> type{};     /**< The type of the participant. */
  std::optional<participant_status> status{}; /**< The current status of the participant. */
  info info;                                  /**< Additional information about the participant. */
  std::optional<bool> is_sending_audio{};     /**< A boolean that informs whether
                                      the participant is sending audio into conference. */
  std::optional<bool> audible_locally{};      /**< Indicates whether a remote participant is audible
                                                 locally. This will always be false for the local
                                                 participant. */
};

}  // namespace dolbyio::comms
