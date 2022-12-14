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
#include <dolbyio/comms/event_handling.h>
#include <dolbyio/comms/exports.h>
#include <dolbyio/comms/notification_subscription_type.h>
#include <dolbyio/comms/participant_info.h>
#include <dolbyio/comms/token_expired_event.h>

#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace dolbyio::comms {

/**
 * Subscription information.
 *
 * Applications wishing to receive events about the conferences which are not
 * joined should use this structure in the services::session::subscribe() and
 * the services::session::unsubscribe() APIs to select the types of events and
 * the conference aliases of interest.
 **/
struct DLB_COMMS_EXPORT notification_subscription {
  /**
   * @brief The constructor.
   * @param conference_alias The alias of the conference.
   * @param type Type of subscription event.
   */
  notification_subscription(const std::string& conference_alias, notification_subscription_type type)
      : conference_alias(conference_alias), type(type) {}

  std::string conference_alias;         ///< The alias of the conference.
  notification_subscription_type type;  ///< The event type.
};

namespace subscription_events {
/**
 * @brief The conference struct used in all subscription events.
 */
struct DLB_COMMS_EXPORT conference {
  std::optional<std::string> conference_id;  ///< The conference ID. May have no value if the
                                             ///< conference with given alias has not been created.
  std::string conference_alias;              ///< The conference alias.
};

/**
 * Event notifying about participant joined to the conference.
 *
 * The application must use the services::session::subscribe() method to
 * subscribe on the notification_subscription_type::participant_joined event for
 * given conference in order to receive these events.
 **/
struct DLB_COMMS_EXPORT participant_joined {
  subscription_events::conference conference;  ///< The conference into which the participant joined.
  participant_info participant;                ///< The joined participant info.
};

/**
 * Event notifying about participant left the conference.
 *
 * The application must use the services::session::subscribe() method to
 * subscribe on the notification_subscription_type::participant_left event for
 * given conference in order to receive these events.
 **/
struct DLB_COMMS_EXPORT participant_left {
  subscription_events::conference conference;  ///< The conference from which the participant left.
  participant_info participant;                ///< The left participant info.
};

/**
 * Event notifying about list of active conference participants.
 *
 * The application must use the services::session::subscribe() method to
 * subscribe on the notification_subscription_type::active_participants event
 * for given conference in order to receive these events.
 **/
struct DLB_COMMS_EXPORT active_participants {
  subscription_events::conference conference;  ///< The conference from which the participant left.
  int participants_count;                      ///< The number of all participants, active and
                                               ///< listeners.
  std::vector<participant_info> participants;  ///< The collection of active participants.
};

/**
 * Event notifying about the conference status.
 *
 * This event is sent once when the application subscribes to the
 * notification_subscription_type::conference_created event and notifies about
 * the current status of the conference (which may or may not be created at this
 * point).
 *  */
struct DLB_COMMS_EXPORT conference_status {
  subscription_events::conference conference;  ///< The conference for which the status is reported.
  bool live;                                   ///< True if the conference is live.
  int start_timestamp;                         ///< Timestamp of the conference start; this field is
                                               ///< valid only if the conference is live.
  std::vector<participant_info> participants;  ///< The list of conference participants. Empty if the
                                               ///< conference is not live.
};

/**
 * Event notifying about the conference being created.
 *
 * The application must use the services::session::subscribe() method to
 * subscribe on the notification_subscription_type::conference_created event
 * for given conference in order to receive these events.
 **/
struct DLB_COMMS_EXPORT conference_created {
  subscription_events::conference conference;  ///< The conference being created.
};

/**
 * Event notifying about the conference ended.
 *
 * The application must use the services::session::subscribe() method to
 * subscribe on the notification_subscription_type::conference_ended event
 * for given conference in order to receive these events.
 **/
struct DLB_COMMS_EXPORT conference_ended {
  subscription_events::conference conference;  ///< The conference that ended.
};
}  // namespace subscription_events

namespace services {
/**
 * @defgroup session_service Session Service
 * The Session Service is responsible for connecting SDK with the Dolby.io
 * backend by opening and closing sessions. Opening a session is mandatory
 * before joining conferences.
 *
 * To use the Session Service, follow these steps:
 *  1. Open a session using the session::open method.
 *  2. Join a conference.
 *  3. Leave the conference and close the session using the
 *  session::close method.
 */

/**
 * @brief The session class.
 * @attention  The session_service interface contains methods that return
 * #async_result. Each function that returns #async_result is asynchronous and
 * executes operations on the SDK event loop. The caller can block the calling
 * thread until the operation completes  and can use the #wait helper. The
 * caller can also chain consecutive operations which depend on the completion
 * of this method using async_result::then calls. When you create an
 * async_result chain, terminate it using async_result::on_error.
 *
 * @ingroup session_service
 */
class DLB_COMMS_EXPORT session {
 public:
  /**
   * @brief The class that represents the participant who tries to open
   * a session.
   * @ingroup session_service
   */
  struct user_info {
    std::string name;       /**< The name of the participant. */
    std::string externalId; /**< The external unique identifier that the customer's
                               application can add to the participant while opening a
                               session. If a participant uses the same external ID in
                               conferences, the participant's ID also remains the same
                               across all sessions.*/
    std::string avatarUrl;  /**< The URL of the participant's avatar. */

    std::optional<std::string> participant_id{}; /**< The unique ID for the participant opening
                                       the session, this field is provided by the
                                       backend when session is opened */
  };

  /**
   * @brief The current state of a session.
   */
  enum class state {
    disconnected, /**< The session is disconnected, this is the initial state.
                   */
    connecting,   /**< The session is connecting. */
    connected,    /**< The session is connected. */
    reconnecting, /**< The session is reconnecting. */
  };

  /**
   * @brief The class that defines exceptions for invalid session states.
   * @ingroup exceptions
   */
  class DLB_COMMS_EXPORT state_exception : public session_exception {
   public:
    /**
     * @brief The constructor that takes the current and the required state of a
     * session.
     * @param current The current state of the session.
     * @param required The required state of the session.
     */
    state_exception(state current, state required);

    /**
     * @brief Returns the current state of a session that is stored in the
     * exception.
     * @return The current state of the session.
     */
    state current_state() const noexcept { return current_; }

    /**
     * @brief Returns the required state of a session that is stored in the
     * exception.
     * @return A string that contains the required state of a session.
     */
    state required_state() const noexcept { return required_; }

   private:
    state current_;
    state required_;
  };

  /**
   * @brief Opens a new session for the specified participant.
   *
   * @param identification Information about the participant who tries to open a
   * session.
   *
   * @returns The result object producing the user_info about opened session
   * asynchronously.
   *
   * \code{.cpp}
   * // Wait for a new session to open
   * auto info = wait(sdk->session().open(identification));
   *
   * // Invoking the method directly requires chaining successive operations
   * // via the `then` call
   * sdk->session().open(identification)
   *   .then([](auto&&) {
   *     // session is now opened and info available
   *   })
   *   .on_error([](auto&& e) {
   *     // handle exception
   *   });
   * \endcode
   */
  virtual async_result<user_info> open(user_info&& identification) = 0;

  /**
   * @brief Closes the current session.
   *
   * @returns The result object producing the operation status asynchronously.
   *
   * \code{.cpp}
   * // Wait for the session to close
   * wait(sdk->session().close());
   *
   * // Invoking the method directly requires chaining successive operations
   * // via the `then` call
   * sdk->session().close()
   *   .then([]() {
   *     // session is now closed
   *   })
   *   .on_error([](auto&& e) {
   *     // handle exception
   *   });
   * \endcode
   */
  virtual async_result<void> close() = 0;

  /**
   * @brief Gets the information about the current session.
   *
   * @returns The result object producing the user_info asynchronously.
   *
   * \code{.cpp}
   * // Wait for the session to close
   * auto info = wait(sdk->session().session_info());
   *
   * // Invoking the method directly requires chaining successive operations
   * // via the `then` call
   * sdk->session_info().
   *   .then([](auto&&) {
   *     // session info now available
   *   })
   *   .on_error([](auto&& e) {
   *     // handle exception
   *   });
   * \endcode
   */
  virtual async_result<user_info> session_info() const = 0;

  /**
   * Subscribes to all status updates for a specified conference, such as
   * added/removed participants and conference ended, within a notification and
   * without joinning the conference.
   *
   * @param subscriptions Turns on the subscription for the specified
   * notifications.
   *
   * @returns The result object producing the operation status asynchronously.
   */
  virtual async_result<void> subscribe(const std::vector<notification_subscription>& subscriptions) = 0;

  /**
   * Unsubscribes from status updates notifications for the specified
   * conference.
   *
   * @param subscriptions Turns off the subscription for the specified
   * notifications.
   *
   * @returns The result object producing the operation status asynchronously.
   */
  virtual async_result<void> unsubscribe(const std::vector<notification_subscription>& subscriptions) = 0;

  /**
   * @brief Adds a listener for subscription_events::participant_joined events.
   *
   * @param callback The function object that is invoked when a participant
   * joins the conference with enabled
   * notification_subscription_type::participant_joined subscription.
   *
   * @returns The result object producing the #event_handler_id asynchronously.
   **/
  virtual async_result<event_handler_id> add_event_handler(
      event_handler<subscription_events::participant_joined>&& callback) = 0;

  /**
   * @brief Adds a listener for subscription_events::participant_left events.
   *
   * @param callback The function object that is invoked when a participant
   * leaves the conference with enabled
   * notification_subscription_type::participant_left subscription.
   *
   * @returns The result object producing the #event_handler_id asynchronously.
   **/
  virtual async_result<event_handler_id> add_event_handler(
      event_handler<subscription_events::participant_left>&& callback) = 0;

  /**
   * @brief Adds a listener for subscription_events::active_participants events.
   *
   * @param callback The function object that is invoked periodically
   * passing the list of active participants for the conference with enabled
   * notification_subscription_type::active_participants subscription.
   *
   * @returns The result object producing the #event_handler_id asynchronously.
   **/
  virtual async_result<event_handler_id> add_event_handler(
      event_handler<subscription_events::active_participants>&& callback) = 0;

  /**
   * @brief Adds a listener for subscription_events::conference_status events.
   *
   * @param callback The function object that is invoked passing the status
   * of the conference with the enabled
   * notification_subscription_type::conference_created subscription.
   *
   * @returns The result object producing the #event_handler_id asynchronously.
   **/
  virtual async_result<event_handler_id> add_event_handler(
      event_handler<subscription_events::conference_status>&& callback) = 0;

  /**
   * @brief Adds a listener for subscription_events::conference_created events.
   *
   * @param callback The function object that is invoked when a conference
   * with enabled
   * notification_subscription_type::conference_created subscription is created.
   *
   * @returns The result object producing the #event_handler_id asynchronously.
   **/
  virtual async_result<event_handler_id> add_event_handler(
      event_handler<subscription_events::conference_created>&& callback) = 0;

  /**
   * @brief Adds a listener for subscription_events::conference_ended events.
   *
   * @param callback The function object that is invoked when a conference
   * with enabled
   * notification_subscription_type::conference_ended subscription ends.
   *
   * @returns The result object producing the #event_handler_id asynchronously.
   **/
  virtual async_result<event_handler_id> add_event_handler(
      event_handler<subscription_events::conference_ended>&& callback) = 0;

  /**
   * @brief Adds a listener for the token_expired_event.
   *
   * @param callback The function object that is invoked in the case of
   * using an invalid token.
   *
   * @returns The result object producing the #event_handler_id asynchronously.
   */
  virtual async_result<event_handler_id> add_event_handler(event_handler<token_expired_event>&& callback) = 0;
};
}  // namespace services
}  // namespace dolbyio::comms
