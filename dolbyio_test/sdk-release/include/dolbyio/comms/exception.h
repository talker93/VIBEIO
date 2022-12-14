#pragma once

/***************************************************************************
 * This program is licensed by the accompanying "license" file. This file is
 * distributed "AS IS" AND WITHOUT WARRANTY OF ANY KIND WHATSOEVER, either
 * express or implied. See the License for the specific language governing
 * permissions and limitations under the License.
 *
 *                Copyright (C) 2021-2022 by Dolby Laboratories.
 ***************************************************************************/

#include <dolbyio/comms/exports.h>

#include <stdexcept>
#include <string>

namespace dolbyio {

namespace comms {

/**
 * @defgroup exceptions Exceptions
 * The Exceptions group gathers all possible C++ SDK exceptions.
 *
 */

/**
 * @class exception
 * @brief The base class for the SDK exceptions.
 * @ingroup exceptions
 */
class DLB_COMMS_MEDIA_EXPORT exception : public std::exception {
 public:
  /**
   * @brief The constructor that takes the description of the exception.
   * @param description A string indicating a reason for the exception.
   */
  exception(const std::string& description) : full_description_(description) {}

  /**
   * @brief Overrides to std::exception that returns a description of the
   * exception.
   * @return A string that describes the exception.
   */
  const char* what() const noexcept override { return full_description_.c_str(); }

 protected:
  std::string full_description_;
};
}  // namespace comms
}  // namespace dolbyio
