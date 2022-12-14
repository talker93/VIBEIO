#pragma once

/***************************************************************************
 * This program is licensed by the accompanying "license" file. This file is
 * distributed "AS IS" AND WITHOUT WARRANTY OF ANY KIND WHATSOEVER, either
 * express or implied. See the License for the specific language governing
 * permissions and limitations under the License.
 *
 *                Copyright (C) 2021-2022 by Dolby Laboratories.
 ***************************************************************************/

#include <dolbyio/comms/exception.h>
#include <dolbyio/comms/exports.h>

#include <string>

namespace dolbyio {

namespace comms {

/**
 * @brief The base exception class for exceptions originating from the
 * media_engine.
 * @ingroup exceptions
 */
class DLB_COMMS_MEDIA_EXPORT media_exception : public exception {
 public:
  /**
   * @brief The constructor that takes a description of the exception.
   * @param description The description of the exception.
   */
  media_exception(std::string&& description) : exception(description) {}
};

/**
 * @brief Describes exceptions originating from the DVC library.
 * @ingroup exceptions
 */
class DLB_COMMS_MEDIA_EXPORT dvc_exception : public media_exception {
 public:
  /**
   * @brief The constructor that takes a description of the exception.
   * @param description The description of the exception.
   */
  dvc_exception(std::string&& description) : media_exception(std::move(description)) {}
};

/**
 * @brief Describes the exception thrown if WebRTC fails to generate a
 * certificate.
 * @ingroup exceptions
 */
class DLB_COMMS_MEDIA_EXPORT certificate_exception : public exception {
 public:
  /**
   * @brief The constructor that takes a description of the exception.
   * @param description The description of the exception.
   */
  certificate_exception(std::string&& description) : exception(description) {}
};

}  // namespace comms
}  // namespace dolbyio
