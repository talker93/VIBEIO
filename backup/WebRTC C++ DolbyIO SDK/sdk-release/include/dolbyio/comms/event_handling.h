#pragma once

/***************************************************************************
 * This program is licensed by the accompanying "license" file. This file is
 * distributed "AS IS" AND WITHOUT WARRANTY OF ANY KIND WHATSOEVER, either
 * express or implied. See the License for the specific language governing
 * permissions and limitations under the License.
 *
 *                Copyright (C) 2021-2022 by Dolby Laboratories.
 ***************************************************************************/

#include <dolbyio/comms/async_result.h>
#include <dolbyio/comms/exports.h>

#include <functional>
#include <memory>

namespace dolbyio::comms {

/**
 * @defgroup event_handling Event Handling
 * The event handler connection interface handles subscribing to events. The
 * application users can either use sdk::add_event_handler or
 * services::conference::add_event_handler to subscribe to events. To use event
 * handlers and subscribe to events, use an #event_handler callback that will be
 * invoked by the SDK when the event occurs. After that, you should receive an
 * #event_handler_id for the event you subscribed to. This ID is required to
 * disconnect event listeners. When the subscription (asynchronous operation) is
 * finished, you should receive the event_handler_id.
 */

/**
 * @brief The function object that handles subscribing to events and returns a
 * void operator.
 * @tparam EventType The selected event.
 * @ingroup event_handling
 */
template <typename EventType>
using event_handler = std::function<void(const EventType&)>;

/**
 * @class event_handler_connection
 * @brief The interface that exposes a connection to the handler. The interface
 * is created for each subscribed event.
 * @ingroup event_handling
 */
class DLB_COMMS_EXPORT event_handler_connection {
 public:
  /**
   * @brief The destructor of the handler connection that does not disconnect
   * the handler.
   */
  virtual ~event_handler_connection();

  /**
   * @brief Disconnects a handler to unsubscribe from the subscribed event.
   *
   * @attention  This function is asynchronous and the operation is executed on the
   * event loop of the SDK. You can either block the calling thread until the
   * operation completes or chain consecutive operations that depend on the
   * completion of this method using the async_result::then call. The
   * async_result chain operations need to be terminated with an
   * async_result::on_error.
   *
   * @returns The result object producing the operation status asynchronously.
   *
   * \code{.cpp}
   * // Wait for the operation to complete
   * wait(handler->disconnect());
   *
   * // Continue in the `then` call
   * handler->disconnect()
   *   .then([]() {
   *     // the handler is disconnected
   *   })
   *   .on_error([](auto&& e) {
   *     // handle the disconnect error
   *   });
   * \endcode
   */
  virtual async_result<void> disconnect() = 0;
};

/**
 * @brief Defines a type of the unique pointer to
 * dolbyio::comms::event_handler_connection.
 * @ingroup event_handling
 */
using event_handler_id = std::unique_ptr<event_handler_connection>;
}  // namespace dolbyio::comms
