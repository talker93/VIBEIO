#pragma once

/***************************************************************************
 * This program is licensed by the accompanying "license" file. This file is
 * distributed "AS IS" AND WITHOUT WARRANTY OF ANY KIND WHATSOEVER, either
 * express or implied. See the License for the specific language governing
 * permissions and limitations under the License.
 *
 *                Copyright (C) 2021-2022 by Dolby Laboratories.
 ***************************************************************************/

#include <dolbyio/comms/audio.h>
#include <dolbyio/comms/conference.h>
#include <dolbyio/comms/device_management.h>
#include <dolbyio/comms/exports.h>
#include <dolbyio/comms/log_level.h>
#include <dolbyio/comms/media_io.h>
#include <dolbyio/comms/session.h>
#include <dolbyio/comms/video.h>

#include <functional>
#include <memory>
#include <string>

#if defined(__APPLE__)
#if !__has_feature(cxx_rtti) && !defined(DOLBYIO_COMMS_SUPPRESS_APPLE_NO_RTTI_WARNING)
#warning Using the Dolby.io SDK in the environment compiled with RTTI disabled on macos is supported, but you should \
take additional precautions. If the application only ever catches the exceptions using the std::exception& type, the \
dolbyio::comms::exception typeinfo will not be accessible; dolbyio::comms::exception inherits the \
std::exception, but the typeinfo for std::exception is a different object in the RTTI-enabled and RTTI-disabled \
code, and the SDK exceptions will not be caught. The application which attempts to catch using the dolbyio::comms:\
:exception& type even once is able to also catch all the SDK exceptions using the std::exception& type. Non-RTTI \
macos applications MUST ensure that their code attempts to catch the dolbyio::comms::exception& at least once. If \
this condition is satisfied, the code including this header can define \
DOLBYIO_COMMS_SUPPRESS_APPLE_NO_RTTI_WARNING macro to suppress this warning.
#endif  // Apple RTTI warning
#endif  // Apple

#ifdef WIN32
#include <dolbyio/comms/allocator.h>
#endif  // Windows

namespace dolbyio::comms {
/**
 * @defgroup sdk C++ SDK
 * The SDK Interface is a starting point that allows initializing the
 * C++ SDK and accessing the underlying services. To use the SDK, follow
 * these steps:
 *
 * 1. Acquire the necessary access tokens from an application server.
 * 2. Provide logging settings using the sdk::set_log_settings method.
 * 3. Create the SDK instance using the sdk::create method. The create call
 * needs to contain a valid access token in order to initialize the SDK.
 * 4. Log in to the Dolby.io backend by using the
 * [Session Service](doc:group__session__service). Retrieve a reference to
 * the Session Interface through the sdk::session method.
 * 5. To create and join a conference, retrieve a reference to the
 * [Conference Service](doc:group__conference__service) through the
 * sdk::conference method.
 */

/**
 * @ingroup sdk
 * @brief The interface for the function object, which is passed by the SDK to
 * the applications' refresh token callback. The application needs to fetch a
 * new token and invoke the function call operator that passes the token.
 */
class DLB_COMMS_EXPORT refresh_token {
 public:
  /**
   * @brief The destructor of the refresh token object.
   */
  virtual ~refresh_token();

  /**
   * @brief Overloaded function call operator which will be passed the newly
   *        acquired token.
   * @param token R-value reference to the token string.
   */
  virtual void operator()(std::string&& token) = 0;
};

/**
 * @ingroup sdk
 * @brief The SDK interface is the entry point for the CoreSDK API.
 * @attention  The SDK interface contains methods that return #async_result.
 * Each function that returns #async_result is asynchronous and executes
 * operations on the SDK event loop. The caller can block the calling thread
 * until the operation completes  and can use the #wait helper. The caller can
 * also chain consecutive operations which depend on the completion of this
 * method using async_result::then calls. When you create an async_result chain,
 * terminate it using async_result::on_error.
 */
class DLB_COMMS_EXPORT sdk {
 public:
#if defined(WIN32) || defined(DOXYGEN)
  /**
   * Set the instance of app_allocator.
   *
   * @attention This is only available on Windows.
   *
   * This method causes the global operators new and delete in the SDK to use the application-provided allocator. If the
   * application wishes to override the SDK's memory allocator, it must do so prior to calling any other
   * SDK APIs, otherwise there is a risk of overriding memory deallocation functions and using the overriden functions
   * to free memory that has been allocated using the default memory allocator. The SDK restores the default allocator
   * upon application exit (not the SDK destruction) to allow freeing memory allocated in global objects using the
   * system allocator.
   *
   * If the allocator has already been overridden, and the new one uses different memory management functions, this
   * method will throw. If the allocator does not contain all four memory allocation functions, this method will also
   * throw. The application should not attempt to restore the default allocator before shutdown; the SDK does it
   * automatically.
   *
   * @param allocator the allocator to be used in the SDK.
   */
  static void set_app_allocator(const app_allocator& allocator);
#endif  // Windows

  /**
   * @brief Settings describing what and how to log.
   */
  struct log_settings {
    /**
     * @brief Log level for SDK logs.
     * Default is INFO.
     */
    log_level sdk_log_level = log_level::INFO;
    /**
     * @brief Log level for Media Engine logs.
     * It is recommended to keep the Media Engine log level at OFF, ERROR or
     * WARNING to avoid spam and only enable more detailed logs when necessary.
     * Default is OFF.
     */
    log_level media_log_level = log_level::OFF;
    /**
     * @brief Directory to which SDK logs should be saved.
     * The application must have write access to the directory or it must be
     * able to create such a directory. Providing a valid directory implies
     * starting logging to a timestamped file. Providing no value or an
     * empty string has no effect. Default is an empty string.
     *
     * @attention
     * SDK logs are always also output to stdout regardless of this setting.
     */
    std::string log_directory{};
  };
  /**
   * @brief Sets what and how to log.
   * @attention Calling more than once has no effect.
   * @param settings Structure of type log_settings.
   * @throws dolbyio::comms::exception
   * Thrown when:
   * - provided invalid log directory
   *
   * @code{.cpp}
   * dolbyio::comms::sdk::log_settings log_settings;
   * log_settings.sdk_log_level = dolbyio::comms::log_level::INFO;
   * log_settings.media_log_level = dolbyio::comms::log_level::OFF;
   * log_settings.log_directory = ".";
   * dolbyio::comms::sdk::set_log_settings(log_settings);
   * @endcode
   */
  static void set_log_settings(const log_settings& settings);

  /**
   * @brief Creates an instance of the SDK. This method is synchronous and is
   * available when the SDK is created and initialized. Otherwise, SDK triggers
   * errors and exceptions.
   * @param access_token The access token that is required to send requests to
   * the Dolby.io backed. The token needs to be fetched externally and provided
   * in the create call.
   * @param refresh_token_callback The function object that is called to refresh
   * the access token. It fetches a new and valid access token and passes the
   * token to the SDK. To do it, the function object is passed as a pointer to
   * the SDK's refresh_token callable, callable, which needs to be invoked with
   * the newly acquired access token. This callback is invoked twice before the
   * access token expiration.
   *
   * @attention The SDK invokes refresh_token_callback on the SDKs event loop
   * thread. Since fetching a token requires a network request, we recommend not
   * blocking the SDK thread. You can use the #async_result and #solver APIs to
   * create an asynchronous method and chain the SDK refresh_token callback
   * invocation when the token is fetched.
   * @return The unique pointer to the initialized SDK object.
   * @exception dolbyio::comms::exception The SDK throws the object when an
   * error occurs.
   *
   * \code{.cpp}
   * std::unique_ptr<sdk> sdk =
   *  dolbyio::comms::sdk::create(
   *    access_token,
   *    [](std::function<void(std::unique_ptr<dolbyio::comms::refresh_token>)>&&
   * cb) { std::shared_ptr<refresh_token> shared_cb = std::move(cb);
   *      users_fetch_token_method().then([cb{std::move(shared_cb)}](auto&&token)
   *      {
   *        (*cb)(std::move(token));
   *      });
   *   });
   * \endcode
   */
  static std::unique_ptr<sdk> create(const std::string& access_token,
                                     std::function<void(std::unique_ptr<refresh_token>)>&& refresh_token_callback);

  /**
   * @brief The destructor that shuts down the SDK.
   */
  virtual ~sdk();

  /**
   * @brief The Media IO Service accessor.
   * @return References to the servies::media_io service.
   *
   * \code{.cpp}
   * auto conference_service = sdk->media_io();
   * \endcode
   */
  virtual services::media_io& media_io() = 0;

  /**
   * @brief The Audio Service accessor.
   * @return Reference to the services::audio service.
   */
  virtual services::audio& audio() = 0;

  /**
   * @brief The Video Service accessor.
   * @return Reference to the services::video service.
   */
  virtual services::video& video() = 0;

  /**
   * @brief The Session Service accessor.
   * @return Reference to the services::session service.
   *
   * \code{.cpp}
   * auto session_service = sdk->session();
   * \endcode
   */
  virtual services::session& session() = 0;

  /**
   * @brief The Conference Service accessor.
   * @return The reference to the services::conference service.
   *
   * \code{.cpp}
   * auto conference_service = sdk->conference();
   * \endcode
   */
  virtual services::conference& conference() = 0;

  /**
   * @brief The Device Management Service accessor.
   * @return The reference to the services::device_management service.
   *
   * \code{.cpp}
   * auto device_management_service = sdk->device_management();
   * \endcode
   */
  virtual services::device_management& device_management() = 0;

  /**
   * @brief Adds a listener for the signaling_channel_exception events.
   *
   * @param callback The function object that is invoked when the WebSocket
   * responsible for signaling detects a failure.
   *
   * @returns The result object producing the #event_handler_id asynchronously.
   *
   * \code{.cpp}
   * // The method invokes via wait blocks until it succeeds or fails
   * handlers.push_back(wait(sdk->add_event_handler(
   *     [](const dolbyio::comms::signalling_channel_exception& e) {
   *     // Process the event
   * })));
   *
   *  // Invoking the method directly requires chaining successive operations
   *  // via the `then` call
   * sdk->add_event_handler(
   *   [](const dolbyio::comms::signalling_channel_exception& e) {
   *     // Process the event
   *   })
   *   .then([this](event_handler_id&& handler) {
   *     this->handlers.push_back(std::move(handler);
   *   })
   *   .on_error([](auto&& e) {
   *     // Handle exception
   *   });
   * \endcode
   */
  virtual async_result<event_handler_id> add_event_handler(event_handler<signaling_channel_exception>&& callback) = 0;

  /**
   * @brief Adds a listener for the invalid_token_exception events.
   * @param callback The function object that is invoked in the case of
   * using an invalid token.
   *
   * @returns The result object producing the #event_handler_id asynchronously.
   *
   * \code{.cpp}
   * // The method invokes via wait blocks until it succeeds or fails
   * handlers.push_back(wait(sdk->add_event_handler(
   *   [](const dolbyio::comms::invalid_token_exception& e) {
   *     // Process the event
   *   })));
   *
   * // Invoking the method directly requires chaining successive operations
   * // via the `then` call
   * sdk->add_event_handler(
   *   [](const dolbyio::comms::invalid_token_exception& e) {
   *     // Process the event
   *   })
   *   .then([this](event_handler_id&& handler) {
   *     this->handlers.push_back(std::move(handler);
   *   })
   *   .on_error([](auto&& e) {
   *     // Handle exception
   *   });
   * \endcode
   */
  virtual async_result<event_handler_id> add_event_handler(event_handler<invalid_token_exception>&& callback) = 0;
};

}  // namespace dolbyio::comms
