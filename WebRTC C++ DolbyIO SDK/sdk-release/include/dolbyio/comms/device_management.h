#pragma once

/***************************************************************************
 * This program is licensed by the accompanying "license" file. This file is
 * distributed "AS IS" AND WITHOUT WARRANTY OF ANY KIND WHATSOEVER, either
 * express or implied. See the License for the specific language governing
 * permissions and limitations under the License.
 *
 *                Copyright (C) 2022 by Dolby Laboratories.
 ***************************************************************************/

#include <dolbyio/comms/async_result.h>
#include <dolbyio/comms/event_handling.h>
#include <dolbyio/comms/exports.h>
#include <dolbyio/comms/media_engine/media_engine.h>

#include <optional>
#include <ostream>
#include <string>
#include <vector>

namespace dolbyio::comms::services {
/**
 * @defgroup device_management Device Management Service
 * The Device Management Service provides an interface for setting
 * the input and output audio devices, as well as getting notifications about
 * the added and removed devices.
 *
 * To use the Device Management Service, follow these steps:
 *  1. Get all current audio devices using the
 *   device_management::get_audio_devices method.
 *  2. Set the desired input audio device by calling the
 *   device_management::set_preferred_input_audio_device.
 *  3. Set the desired output audio device by calling the
 *   device_management::set_preferred_output_audio_device.
 *  4. Subscribe to audio_device_added, audio_device_removed, and audio_device_changed events
 * using the respective device_management::add_event_handler method.
 *
 */

/**
 * @note The Device Management service is not intended to be used when media_io service
 * is being used as the Media IO Service implies using external media sources..
 *
 * @ingroup device_management
 * @brief Provides methods of the Device Management Service.
 *
 * @attention  The device_management interface contains methods that return
 * #async_result. Each function that returns #async_result is asynchronous and
 * the operation is executed during the SDK event loop. The caller can block the
 * calling thread until the operation completes using the #wait helper. The
 * caller can also chain consecutive operations, which are dependent on the
 * completion of this method, using the async_result::then calls. Each
 * async_result chain needs to be terminated with an async_result::on_error.
 */
class DLB_COMMS_EXPORT device_management {
 public:
  /**
   * @brief Sets the preferred input audio device.
   * @param device Structure containing information about the desired input
   * device.
   * @returns The result object producing the operation status asynchronously.
   * \code{.cpp}
   * // Wait device to be set
   * wait(sdk->device_management().set_preferred_input_audio_device(device));
   *
   * // Invoking the method directly requires chaining successive operations
   * // via the `then` call
   * sdk->device_management().set_preferred_input_audio_device(device)
   *   .then([]() {
   *     // device is now set
   *   })
   *   .on_error([](auto&& e) {
   *     // Handle exception
   *   });
   * \endcode
   */
  virtual async_result<void> set_preferred_input_audio_device(const dvc_device& device) = 0;

  /**
   * @brief Sets the preferred output audio device.
   * @param device Structure containing information about the desired output
   * device.
   * @returns The result object producing the operation status asynchronously.
   * \code{.cpp}
   * // Wait device to be set
   * wait(sdk->device_management().set_preferred_input_audio_device(device));
   *
   * // Invoking the method directly requires chaining successive operations
   * // via the `then` call
   * sdk->device_management().set_preferred_output_audio_device(device)
   *   .then([]() {
   *     // Device is now set
   *   })
   *   .on_error([](auto&& e) {
   *     // Handle exception
   *   });
   * \endcode
   */
  virtual async_result<void> set_preferred_output_audio_device(const dvc_device& device) = 0;

  /**
   * @brief Gets a list of the currently available audio devices in the system.
   * @returns The result object producing a vector containing the audio
   * devices asynchronously.
   * \code{.cpp}
   * auto devices = wait(sdk->device_management().get_audio_devices());
   *
   * // Invoking the method directly requires chaining successive operations
   * // via the `then` call
   * sdk->device_management().get_audio_devices()
   *   .then([](std::vector<dvc_device>&& dev) {
   *     // devices are now available
   *   })
   *   .on_error([](auto&& e) {
   *     // Handle exception
   *   });
   * \endcode
   */
  virtual async_result<std::vector<dvc_device>> get_audio_devices() = 0;

  /**
   * @brief Gets the audio input device that is currently used by the system.
   * @returns The result object producing the audio device asynchronously.
   * #async_result fails if no audio input device is set.
   * \code{.cpp}
   * // Wait device to be set
   * auto devices =
   * wait(sdk->device_management().get_current_audio_input_device());
   *
   * // Invoking the method directly requires chaining successive operations
   * // via the `then` call
   * sdk->device_management().get_current_audio_input_device()
   *   .then([](dvc_device&& dev) {
   *     // Devices are now available
   *   })
   *   .on_error([](auto&& e) {
   *     // Currently no audio input device set
   *   });
   * \endcode
   */
  virtual async_result<std::optional<dvc_device>> get_current_audio_input_device() = 0;

  /**
   * @brief Gets the audio output device currently used by the system.
   * @returns The result object producing the audio device asynchronously.
   * #async_result fails if no audio output device is set.
   * \code{.cpp}
   * // Wait device to be set
   * auto devices =
   * wait(sdk->device_management().get_current_audio_output_device());
   *
   * // Invoking the method directly requires chaining successive operations
   * // via the `then` call
   * sdk->device_management().get_current_audio_output_device()
   *   .then([](dvc_device&& dev) {
   *     // Current output device now available
   *   })
   *   .on_error([](auto&& e) {
   *     // Currently no audio input device set
   *   });
   * \endcode
   */
  virtual async_result<std::optional<dvc_device>> get_current_audio_output_device() = 0;

  /**
   * @brief Gets a list of the currently available video devices in the system.
   * @returns The result object producing a vector containing the audio
   * devices asynchronously.
   * \code{.cpp}
   * auto devices = wait(sdk->device_management().get_video_devices());
   *
   * // Invoking the method directly requires chaining successive operations
   * // via the `then` call
   * sdk->device_management().get_video_devices()
   *   .then([](std::vector<camera_device>&& dev) {
   *     // Devices are now available
   *   })
   *   .on_error([](auto&& e) {
   *     // Handle exception
   *   });
   * \endcode
   */
  virtual async_result<std::vector<camera_device>> get_video_devices() = 0;

  /**
   * @brief Gets the video device currently used by the system.
   * @returns The result object producing the audio device asynchronously.
   * #async_result fails if no audio output device is set.
   * \code{.cpp}
   * // Wait device to be set
   * auto devices =
   * wait(sdk->device_management().get_current_video_device());
   *
   * // Invoking the method directly requires chaining successive operations
   * // via the `then` call
   * sdk->device_management().get_current_video_device()
   *   .then([](camera_device&& dev) {
   *     // Current output device now available
   *   })
   *   .on_error([](auto&& e) {
   *     // Currently no audio input device set
   *   });
   * \endcode
   */
  virtual async_result<std::optional<camera_device>> get_current_video_device() = 0;

  /**
   * @brief Adds a listener for the audio_device_added events.
   *
   * @param callback The function object that is invoked when
   * an audio device is added to the system.
   *
   * @returns The result object producing the #event_handler_id asynchronously.
   *
   * \code{.cpp}
   * // Invoke the method via wait blocks until the method succeeds or fails
   * handlers.push_back(wait(sdk->device_management().add_event_handler(
   *   [](const dolbyio::comms::audio_device_added& e) {
   *     // Process the event
   *   })));
   *
   * // Invoking the method directly requires chaining successive operations
   * // via the `then` call
   * sdk->conference().add_event_handler(
   *   [](const dolbyio::comms::audio_device_added& e) {
   *     // Process the event
   *   }))
   *   .then([this](event_handler_id&& handler) {
   *     this->handlers.push_back(std::move(handler));
   *   })
   *   .on_error([](auto&& e) {
   *     // Handle exception
   *   });
   * \endcode
   */
  virtual async_result<event_handler_id> add_event_handler(event_handler<audio_device_added>&& callback) = 0;

  /**
   * @brief Adds a listener for the audio_device_removed events.
   *
   * @param callback The function object that is invoked when an
   * audio device is removed from the system.
   *
   * @returns The result object producing the #event_handler_id asynchronously.
   *
   * \code{.cpp}
   * // Invoke the method via wait blocks until the method succeeds or fails
   * handlers.push_back(wait(sdk->device_management().add_event_handler(
   *   [](const dolbyio::comms::audio_device_removed& e) {
   *     // Process the event
   *   })));
   *
   * // Invoking the method directly requires chaining successive operations
   * // via the `then` call
   * sdk->conference().add_event_handler(
   *   [](const dolbyio::comms::audio_device_removed& e) {
   *     // Process the event
   *   }))
   *   .then([this](event_handler_id&& handler) {
   *     this->handlers.push_back(std::move(handler));
   *   })
   *   .on_error([](auto&& e) {
   *     // Handle exception
   *   });
   * \endcode
   */
  virtual async_result<event_handler_id> add_event_handler(event_handler<audio_device_removed>&& callback) = 0;

  /**
   * @brief Adds a listener for the audio_device_changed events.
   *
   * @param callback The function object that is invoked when a
   * audio_device_changed event occurs.
   *
   * @returns The result object producing the #event_handler_id asynchronously.
   *
   * \code{.cpp}
   * // Invoke the method via wait blocks until the method succeeds or fails
   * handlers.push_back(wait(sdk->device_management().add_event_handler(
   *   [](const dolbyio::comms::audio_device_changed& e) {
   *     // Process the event
   *   })));
   *
   * // Invoking the method directly requires chaining successive operations
   * // via the `then` call
   * sdk->conference().add_event_handler(
   *   [](const dolbyio::comms::audio_device_changed& e) {
   *     // Process the event
   *   }))
   *   .then([this](event_handler_id&& handler) {
   *     this->handlers.push_back(std::move(handler));
   *   })
   *   .on_error([](auto&& e) {
   *     // Handle exception
   *   });
   * \endcode
   */
  virtual async_result<event_handler_id> add_event_handler(event_handler<audio_device_changed>&& callback) = 0;

  /**
   * @brief Adds a listener for the video_device_added events.
   *
   * @param callback The function object that is invoked when
   * an video device is added to the system.
   *
   * @returns The result object producing the #event_handler_id asynchronously.
   *
   * \code{.cpp}
   * // Invoke the method via wait blocks until the method succeeds or fails
   * handlers.push_back(wait(sdk->device_management().add_event_handler(
   *   [](const dolbyio::comms::video_device_added& e) {
   *     // Process the event
   *   })));
   *
   * // Invoking the method directly requires chaining successive operations
   * // via the `then` call
   * sdk->conference().add_event_handler(
   *   [](const dolbyio::comms::video_device_added& e) {
   *     // Process the event
   *   }))
   *   .then([this](event_handler_id&& handler) {
   *     this->handlers.push_back(std::move(handler));
   *   })
   *   .on_error([](auto&& e) {
   *     // Handle exception
   *   });
   * \endcode
   */
  virtual async_result<event_handler_id> add_event_handler(event_handler<video_device_added>&& callback) = 0;

  /**
   * @brief Adds a listener for the video_device_removed events.
   *
   * @param callback The function object that is invoked when an
   * video device is removed from the system.
   *
   * @returns The result object producing the #event_handler_id asynchronously.
   *
   * \code{.cpp}
   * // Invoke the method via wait blocks until the method succeeds or fails
   * handlers.push_back(wait(sdk->device_management().add_event_handler(
   *   [](const dolbyio::comms::video_device_removed& e) {
   *     // Process the event
   *   })));
   *
   * // Invoking the method directly requires chaining successive operations
   * // via the `then` call
   * sdk->conference().add_event_handler(
   *   [](const dolbyio::comms::video_device_removed& e) {
   *     // Process the event
   *   }))
   *   .then([this](event_handler_id&& handler) {
   *     this->handlers.push_back(std::move(handler));
   *   })
   *   .on_error([](auto&& e) {
   *     // Handle exception
   *   });
   * \endcode
   */
  virtual async_result<event_handler_id> add_event_handler(event_handler<video_device_removed>&& callback) = 0;

  /**
   * @brief Adds a listener for the video_device_changed events.
   *
   * @param callback The function object that is invoked when an
   * video device is removed from the system.
   *
   * @returns The result object producing the #event_handler_id asynchronously.
   */
  virtual async_result<event_handler_id> add_event_handler(event_handler<video_device_changed>&& callback) = 0;

  /**
   * @brief Adds a listener for the audio_device_timeout_failure events.
   *
   * @param callback The function object that is invoked when a
   * audio_device_timeout_failure event occurs.
   *
   * @returns The result object producing the #event_handler_id asynchronously.
   *
   * \code{.cpp}
   * // Invoke the method via wait blocks until the method succeeds or fails
   * handlers.push_back(wait(sdk->device_management().add_event_handler(
   *   [](const dolbyio::comms::audio_device_timeout_failure& e) {
   *     // Process the event
   *   })));
   *
   * // Invoking the method directly requires chaining successive operations
   * // via the `then` call
   * sdk->conference().add_event_handler(
   *   [](const dolbyio::comms::audio_device_timeout_failure& e) {
   *     // Process the event
   *   }))
   *   .then([this](event_handler_id&& handler) {
   *     this->handlers.push_back(std::move(handler));
   *   })
   *   .on_error([](auto&& e) {
   *     // Handle exception
   *   });
   * \endcode
   */
  virtual async_result<event_handler_id> add_event_handler(event_handler<audio_device_timeout_failure>&& callback) = 0;
};

};  // namespace dolbyio::comms::services
