#pragma once

/***************************************************************************
 * This program is licensed by the accompanying "license" file. This file is
 * distributed "AS IS" AND WITHOUT WARRANTY OF ANY KIND WHATSOEVER, either
 * express or implied. See the License for the specific language governing
 * permissions and limitations under the License.
 *
 *                Copyright (C) 2021-2022 by Dolby Laboratories.
 ***************************************************************************/

/**
 * @defgroup loglevel Logging Levels
 * The Logging Levels group gathers logging levels available in the C++ SDK.
 *
 */

namespace dolbyio {

namespace comms {

#ifdef DEBUG
#pragma push_macro("DEBUG")
#define DOLBYIO_COMMS_STORED_NONSTANDARD_DEBUG 1
#undef DEBUG
#endif

/**
 * @ingroup loglevel
 * @brief The logging levels to set. The logging levels allow classifying the
 * entries in the log files in terms of urgency to help to control the amount of
 * logged information.
 */
enum class log_level {
  OFF,     /**< Turns off logging. */
  ERROR,   /**< The error level logging generates logs when an error occurs and
              the SDK cannot properly function. */
  WARNING, /**< The warning level logging generates logs when the SDK detects an
              unexpected problem but is still able to work as usual. */
  INFO,    /**< The info level logging generates an informative number of logs. */
  DEBUG,   /**< The debug level logging generates a high number of logs to provide
              diagnostic information in a detailed manner.  */
  VERBOSE, /**< The verbose level logging generates the highest number of logs,
              including even the HTTP requests. */

  DOLBYIO_COMMS_DEBUG = DEBUG, /**< The debug level. Use this enumerator if the DEBUG enumerator
                                  conflicts with compiler's / toolkit's preprocessor macro. */
};

#ifdef DOLBYIO_COMMS_STORED_NONSTANDARD_DEBUG
#pragma pop_macro("DEBUG")
#undef DOLBYIO_COMMS_STORED_NONSTANDARD_DEBUG
#endif

}  // namespace comms
}  // namespace dolbyio
