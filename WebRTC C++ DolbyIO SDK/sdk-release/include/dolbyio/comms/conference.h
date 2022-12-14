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
#include <dolbyio/comms/conference_access_permissions.h>
#include <dolbyio/comms/event_handling.h>
#include <dolbyio/comms/exports.h>
#include <dolbyio/comms/media_engine/media_engine.h>
#include <dolbyio/comms/participant_events.h>
#include <dolbyio/comms/participant_info.h>
#include <dolbyio/comms/spatial_audio_style.h>
#include <dolbyio/comms/spatial_audio_types.h>
#include <dolbyio/comms/video_codec.h>

#include <optional>
#include <ostream>
#include <string>
#include <unordered_map>
#include <vector>

namespace dolbyio::comms {

/// @cond DO_NOT_DOCUMENT

// A small helper type to deprecate public API structure fields safely.
// The application, which only sets them, continues to compile with deprecation warnings.
// Only the value constructor is deprecated, the default and copy constructors are not, so deprecating the field does
// not cause warnings in implicitly-generated copy-constructors.
template <typename T>
struct deprecated_field {
  deprecated_field() = default;
  [[deprecated("Setting this field is deprecated. Refer to the documentation for replacement.")]] deprecated_field(
      const T& val)
      : has_value_(true), val_(val) {}
  deprecated_field(const deprecated_field&) = default;
  deprecated_field& operator=(const deprecated_field&) = default;

  T& operator*() { return val_; }
  operator std::optional<T>() const {
    if (has_value_)
      return val_;
    return {};
  }
  const T& operator*() const { return val_; }
  bool has_value() const { return has_value_; }

 private:
  bool has_value_ = false;
  T val_{};
};
/// @endcond

/**
 * @defgroup conference_service Conference Service
 * The Conference Service allows joining and leaving conferences as well as
 * subscribing to events. To use the Conference Service, follow these steps:
 * 1. Open a session using services::session::open.
 * 2. Create the Media Recording Module instance using the Default Recorder and
 * the plugin::recorder interface.
 * 3. Set the Recording Module instance as the Media Sink for the Conference
 * Service via the services::conference::set_media_sink method.
 * 4. Create the Media Injector Module instance using the Default Injector and
 * the plugin::injector interface.
 * 5. Create a Media Injection Source (something with the ability to provide raw
 * frames to the Media Injector Module). This can be an application that reads
 * from a file, gets remote stream etc... and provides the media frames to the
 * injector. This source should be passed a reference to the Media Injector
 * Module.
 * 6. Set the Injector Module instance as the Media Source for the Conference
 * Service via the services::conference::set_media_source method.
 * 7. Subscribe to the events exposed through the service using the
 * services::conference::add_event_handler methods for the respective event.
 * 8. Create a conference via the services::conference::create method.
 * 9. Join the created conference via the services::conference::join or
 * services::conference::listen method.
 * 10. The SDK returns the #async_result class that contains the conference_info
 * structure, which describes the conference.
 * 11. Leave the conference using the services::conference::leave method.
 */

/**
 * @brief Possible values representing the current status of a conference.
 */
enum class DLB_COMMS_EXPORT conference_status {
  creating,  /**< The SDK is creating a new conference. */
  created,   /**< The conference is created. */
  joining,   /**< The local participant is joining a conference. */
  joined,    /**< The local participant successfully joined the conference. */
  leaving,   /**< The local participant is leaving the conference. */
  left,      /**< The local participant left the conference. */
  destroyed, /**< The conference is destroyed on the server. */
  error,     /**< A conference error occurred. */
};

/**
 * @brief Contains the conference information. This structure provides
 * conference details that are required to join a specific conference. The SDK
 * returns conference_info to describe the created or joined conference.
 * @ingroup conference_service
 */
struct DLB_COMMS_EXPORT conference_info {
  std::string id;                     /**< The unique conference identifier. */
  std::optional<std::string> alias{}; /**< The conference alias. The alias is optional in the case of
                                           using the conference ID. */
  bool is_new = false;                /**< Indicates whether the conference represented by the
                                         object has just been created. */
  conference_status status;           /**< The current status of the conference. */
  std::vector<conference_access_permissions> permissions{}; /**< Permissions that allow a conference participant to
                                                                 perform limited actions during a protected conference.
                                                             */
  std::unordered_map<participant_info::id, participant_info> participants{}; /**< Conference participants */

  /**
   * @since 2.1.0
   * @brief The spatial audio style used in the joined conference. This is
   * only set when the conference has been successfully joined.
   */
  std::optional<enum spatial_audio_style> spatial_audio_style{};
};

/**
 * @brief Emitted whenever conference status changes.
 * @ingroup conference_service
 * @ingroup events
 */
struct DLB_COMMS_EXPORT conference_status_updated {
  /**
   * @brief A constructor that takes the status of the event.
   * @param value The current status of a conference.
   * @param conf_id The ID of the conference for which the status changed.
   */
  conference_status_updated(conference_status value, const std::string& conf_id = std::string()) {
    status = value;
    id = conf_id;
  };
  conference_status status; /**< The conference status. */
  std::string id;           /**< The unique identifier of the conference. */
};

/**
 * @brief Emitted whenever the active speaker changes.
 * An empty list of speakers indicates that there is no active speaker.
 * @ingroup conference_service
 * @ingroup events
 */
struct DLB_COMMS_EXPORT active_speaker_change {
  std::string conference_id;                /**< The unique identifier for the current conference. */
  std::vector<std::string> active_speakers; /**< The list of all active speakers. */
};

/**
 * @brief Emitted when a new message is received in the current conference.
 * @ingroup conference_service
 * @ingroup events
 */
struct DLB_COMMS_EXPORT conference_message_received {
  std::string conference_id;                 /**< The unique identifier of the current conference. */
  std::string user_id;                       /**< The unique identifier of the participant who sent
                                                the message. */
  struct participant_info::info sender_info; /**< Additional information about
                                                the sender of the message. */
  std::string message;                       /**< The message. */
};

/**
 * @brief Emitted when a new conference invitation is received.
 * @ingroup conference_service
 * @ingroup events
 */
struct DLB_COMMS_EXPORT conference_invitation_received {
  std::string conference_id;                 /**< The unique identifier of the conference. */
  std::string conference_alias;              /**< The Alias for the conference. */
  struct participant_info::info sender_info; /**< Additional information about
                                                the sender of the invitation. */
};

namespace services {

/**
 * @ingroup conference_service
 * @brief Provides methods of the Conference Service.
 *
 * @attention  The conference interface that contains methods that return
 * #async_result. Each function that returns #async_result is asynchronous and
 * the operation is executed during the SDK event loop. The caller can block the
 * calling thread until the operation completes using the #wait helper. The
 * caller can also chain consecutive operations, which are dependent on the
 * completion of this method, using the async_result::then calls. Each
 * async_result chain needs to be terminated with an async_result::on_error.
 */
class DLB_COMMS_EXPORT conference {
 public:
  /**
   * @ingroup conference_service
   * @brief The conference options structure that provides additional
   * information about a conference.
   */
  struct conference_options {
    /**
     * @brief The conference parameters.
     */
    struct params {
      bool dolby_voice = true;                          /**< A boolean that indicates whether the SDK
                                                           should create a Dolby Voice conference where
                                                           each participant receives one audio stream. */
      bool stats = false;                               /**< A boolean that indicates whether the conference
                                                           should include additional statistics. */
      enum video_codec video_codec = video_codec::h264; /**< The preferred video codec. */

      enum spatial_audio_style spatial_audio_style = spatial_audio_style::individual; /**< An enum that defines how the
                                                                                         spatial location is
                                                                                         communicated between the SDK
                                                                                         and the Dolby.io server. */
    };
    std::optional<std::string> alias; /**< The alias of the conference. */
    params params;                    /**< The conference parameters. */
  };

  /**
   * @ingroup conference_service
   * @brief The local media constraints for application joining a conference.
   */
  struct media_constraints {
    bool audio = false;     /**< A boolean that indicates whether the application should
                                 capture local audio and send it to a conference. */
    bool video = false;     /**< A boolean that indicates whether the application should
                                 capture local video and send it to a conference. */
    bool send_only = false; /**< Allows the user to join a conference as a sender only. This
                                 is strictly intended for applications that want to inject media
                                 without recording. Applications which set this flag will not receive
                                 media.*/
  };

  /**
   * @ingroup conference_service
   * @brief The options that define how the application expects to join a
   * conference in terms of media preference.
   */
  struct connection_options {
    std::optional<int> max_video_forwarding;            /**< Sets the maximum number of video streams that
                                                           may be transmitted to the SDK. Valid parameter
                                                           values are between 0 and 25. If this
                                                           parameter is not set, the default value for the maximum
                                                           number of video streams that can be transmitted to the
                                                           SDK is 25.*/
    std::optional<std::string> conference_access_token; /**< The conference access token that is
                                                           required to join a protected conference if
                                                           the conference is created using the
                                                           [create](ref:conference#operation-create-conference)
                                                           REST API. The application needs to
                                                           externally fetch and provide the token to
                                                           the SDK when it calls the join or listen
                                                           method. */
    bool spatial_audio = false;                         /**< Enables spatial audio for the joining participant. This
                                                           boolean should be set to true if the spatial_style is not
                                                           disabled. For more information, refer to our sample
                                                           application code.*/
    bool simulcast = false;                             /**< Enables simulcast support in the conference. */
  };

  /**
   * @ingroup conference_service
   * @brief The options for joining a conference as an active user.
   */
  struct join_options {
    connection_options connection; /**< The options for connecting to the conference. */
    media_constraints constraints; /**< The media constraints for the active user. */
  };

  /**
   * @ingroup conference_service
   * @brief The options for listening to a conference as listener.
   */
  struct listen_options {
    connection_options connection; /**< The options for connecting to the conference. */
  };

  /**
   * @brief Creates a demo conference and joins to it upon completion.
   *
   * @param spatialAudio A boolean flag enabling spatial audio for the joining
   *  participant. The default value is true.
   *
   * @returns The result object producing the conference_info asynchronously.
   *
   * \code{.cpp}
   * // Wait for the conference creation
   * auto conf_info = wait(sdk->conference().demo(false));
   *
   *  // Invoking the method directly requires chaining successive operations
   *  // via the `then` call
   * sdk->conference().demo())
   *    .then[](auto&& info) {
   *      // Do something with the returned conf info;
   *    })
   *    .on_error([](auto&& e) {
   *      // Handle exception
   *    });
   * \endcode
   */
  virtual async_result<conference_info> demo(bool spatialAudio = true) = 0;

  /**
   * @brief Creates a conference and returns information about the conference
   * upon completion.
   *
   * @param options The conference options.
   *
   * @returns The result object producing the conference_info asynchronously.
   *
   * \code{.cpp}
   * // Wait for the conference creation
   * auto conf_info = wait(sdk->conference().create(options));
   *
   *  // Invoking the method directly requires chaining successive operations
   *  // via the `then` call
   * sdk->conference().create(options))
   *    .then[](auto&& info) {
   *      // Do something with the returned conf info;
   *    })
   *    .on_error([](auto&& e) {
   *      // Handle exception
   *    });
   * \endcode
   */
  virtual async_result<conference_info> create(const conference_options& options) = 0;

  /**
   * @brief Joins an existing conference as an active user who can both receive
   * media from the conference and inject media into the conference.
   *
   * @param conf The conference options that need to contain conference_id.
   * @param join The join options for the SDK user.
   *
   * @returns The result object producing the conference_info asynchronously.
   *
   * \code{.cpp}
   * // Wait for the conference creation
   * auto conf_info = wait(sdk->conference().join(conf_options, join_options));
   *
   *  // Invoking the method directly requires chaining successive operations
   *  // via the `then` call
   * sdk->conference().join(conf_options, join_options))
   *    .then[](auto&& info) {
   *      // Do something with the returned conf info;
   *    })
   *    .on_error([](auto&& e) {
   *      // Handle exception
   *    });
   * \endcode
   */
  virtual async_result<conference_info> join(const conference_info& conf, const join_options& join) = 0;

  /**
   * @brief Joins an existing conference as a listener who can receive audio and
   * video streams, but cannot send any stream to the conference.
   *
   * @param conf The conference options that need to contain conference_id.
   * @param listen The join options for the SDK user.
   *
   * @returns The result object producing the conference_info asynchronously.
   *
   * \code{.cpp}
   * // Invoke the method via wait blocks until the method succeeds or fails
   * wait(sdk->conference().listen(conf, join));
   *
   * // Invoking the method directly requires chaining successive operations
   * // via the `then` call
   * sdk->conference().listen(conf, join)
   *   .then([](conference_info&&) {
   *     // The user joined the conference
   *   })
   *   .on_error([](auto&& e) {
   *     // Handle exception
   *   });
   * \endcode
   */
  virtual async_result<conference_info> listen(const conference_info& conf, const listen_options& listen) = 0;

  /**
   * @brief Leaves a conference.
   *
   * @returns The result object producing the operation status asynchronously.
   *
   * \code{.cpp}
   * // Invoke the method via wait blocks until the method succeeds or fails
   * wait(sdk->conference().leave());
   *
   * // Invoking the method directly requires chaining successive operations
   * // via the `then` call
   * sdk->conference().leave()
   *   .then([]() {
   *     // The user left the conference
   *   })
   *   .on_error([](auto&& e) {
   *     // Handle exception
   *   });
   * \endcode
   */
  virtual async_result<void> leave() = 0;

  /**
   * @brief Sends a message to the current conference. The message size is
   * limited to 16KB.
   *
   * @param message The message.
   *
   * @returns The result object producing the operation status asynchronously.
   *
   * @code{.cpp}
   *  // Invoke the method via wait blocks until the method succeeds or fails
   *  wait(sdk->conference().send("some message"));
   *
   *  // Invoking the method directly requires chaining successive operations
   *  // via the `then` call
   *   sdk->conference().send("some message")
   *   .then([]() {
   *      // The message was sent
   *   })
   *   .on_error([](auto&& e){
   *      // Rethrow and handle the exception
   *   });
   * @endcode
   */
  virtual async_result<void> send(const std::string& message) = 0;

  /**
   * @brief Mutes and un-mutes the local participant's microphone.
   *
   * @param muted A boolean value that indicates the required mute state. True
   * mutes the microphone, false un-mutes the microphone.
   *
   * @returns The result object producing the operation status asynchronously.
   *
   * @code{.cpp}
   *  // Invoke the method via wait blocks until the method succeeds or fails
   *  wait(sdk->conference().mute(true));
   *
   *  // Invoking the method directly requires chaining successive operations
   *  // via the `then` call
   *   sdk->conference().mute(true)
   *   .then([]() {
   *      // The microphone is now muted
   *   })
   *   .on_error([](auto&& e){
   *      // The operation failed, and microphone is not muted
   *   });
   * @endcode
   */
  virtual async_result<void> mute(bool muted) = 0;

  /**
   * @brief Mutes and un-mutes a specified remote participant.
   *
   * @param muted A boolean value that indicates the required mute state. True
   * mutes the remote participant, false un-mutes the remote participant.
   *
   * @param participant_id The ID of the remote participant to be muted.
   *
   * @attention This method is only available for non-Dolby Voice conferences.
   *
   * @returns The result object producing the operation status asynchronously.
   * If attempted for a Dolby Voice Conference, the async_result will fail.
   *
   * @code{.cpp}
   *  // Invoke the method via wait blocks until the method succeeds or fails
   *  wait(sdk->conference().remote_mute(true, participant_id));
   *
   *  // Invoking the method directly requires chaining successive operations
   *  // via the `then` call
   *   sdk->conference().remote_mute(true, participant_id)
   *   .then([]() {
   *      // The remote participant is now muted
   *   })
   *   .on_error([](auto&& e){
   *      // The operation failed, and remote participant is not muted
   *   });
   * @endcode
   */
  virtual async_result<void> remote_mute(bool muted, const std::string& participant_id) = 0;

  /**
   * @brief Mutes and un-mutes the output audio device.
   *
   * @attention This method is only available in Dolby Voice conferences.
   *
   * @param muted A boolean value that indicates the required mute state. True
   * mutes the output device, false un-mutes the output device.
   *
   * @returns The result object producing the operation status asynchronously.
   *
   * @code{.cpp}
   *  // Invoke the method via wait blocks until the method succeeds or fails
   *  wait(sdk->conference().mute_output(true));
   *
   *  // Invoking the method directly requires chaining successive operations
   *  // via the `then` call
   *   sdk->conference().mute_output(true)
   *   .then([]() {
   *      // The output is now muted
   *   })
   *   .on_error([](auto&& e){
   *      // The operation failed, and output is not muted
   *   });
   * @endcode
   */
  virtual async_result<void> mute_output(bool muted) = 0;

  /**
   * @brief Updates the spatial audio configuration to enable experiencing
   * spatial audio during a conference. This method contains information
   * about participants' locations, the direction the local participant
   * is facing in space, and a spatial environment of an application,
   * so the audio renderer understands which directions the application
   * considers forward, up, and right and which units it uses for distance.
   * This method is available only for participants who joined a conference
   * using the join method with the spatial_audio parameter enabled.
   * Depending on the selected spatial_audio_style, the method requires either
   * providing only the position of the local participant or the positions of
   * all participants. When using the individual spatial_audio_style, remote
   * participants' audio is disabled until the participants are assigned
   * to specific locations and each time new participants join the conference,
   * the positions need to be updated.
   *
   * @param configuration The object with the new configuration set.
   *
   * @returns The result object producing the operation status asynchronously.
   *
   * \code{.cpp}
   * // Invoke the method via wait blocks until the method succeeds or fails
   * wait(sdk->conference().update_spatial_audio_configuration(configuration));
   *
   * // Invoking the method directly requires chaining successive operations
   * // via the `then` call
   * sdk->conference().update_spatial_audio_configuration(configuration)
   *   .then([]() {
   *     // Configuration is now updated
   *   })
   *   .on_error([](auto&& e) {
   *     // Handle exception
   *   });
   * \endcode
   */
  virtual async_result<void> update_spatial_audio_configuration(
      dolbyio::comms::spatial_audio_batch_update&& configuration) = 0;

  /**
   * @brief Updates the spatial audio configuration to enable experiencing
   * spatial audio during a conference. This method contains information
   * about the participant's location.
   * This method is available only for participants who joined a conference
   * using the join method with the spatial_audio parameter enabled.
   * Depending on the selected spatial_audio_style, the method requires either
   * providing only the position of the local participant or the positions of
   * all participants. When using the individual spatial_audio_style, remote
   * participants' audio is disabled until the participants are assigned
   * to specific locations and each time new participants join the conference,
   * the positions need to be updated.
   *
   * @param participant_id The participant whose position is updated.
   * @param position The location of given participant.
   *
   * @returns The result object producing the operation status asynchronously.
   *
   * \code{.cpp}
   * // Invoke the method via wait blocks until the method succeeds or fails
   * wait(sdk->conference().set_spatial_position(participant_id, position));
   *
   * // Invoking the method directly requires chaining successive operations
   * // via the `then` call
   * sdk->conference().set_spatial_position(participant_id, position)
   *   .then([]() {
   *     // Position is now updated
   *   })
   *   .on_error([](auto&& e) {
   *     // Handle exception
   *   });
   * \endcode
   */
  virtual async_result<void> set_spatial_position(const std::string& participant_id,
                                                  const spatial_position& position) = 0;

  /**
   * @brief Updates the spatial audio configuration to enable experiencing
   * spatial audio during a conference. This method contains information
   * about the direction the local participant is facing in space.
   * This method is available only for participants who joined a conference
   * using the join method with the spatial_audio parameter enabled.
   *
   * @param direction The direction the local participant is facing.
   *
   * @returns The result object producing the operation status asynchronously.
   *
   * \code{.cpp}
   * // Invoke the method via wait blocks until the method succeeds or fails
   * wait(sdk->conference().set_spatial_direction(direction));
   *
   * // Invoking the method directly requires chaining successive operations
   * // via the `then` call
   * sdk->conference().set_spatial_direction(direction)
   *   .then([]() {
   *     // Direction is now updated
   *   })
   *   .on_error([](auto&& e) {
   *     // Handle exception
   *   });
   * \endcode
   */
  virtual async_result<void> set_spatial_direction(const spatial_direction& direction) = 0;
  /**
   * @brief Updates the spatial audio configuration to enable experiencing
   * spatial audio during a conference. This method contains information
   * about a spatial environment of an application, so the audio renderer
   * understands which directions the application considers forward, up, and
   * right and which units it uses for distance. This method is available only
   * for participants who joined a conference using the join method with the
   * spatial_audio parameter enabled.
   *
   * @param scale A scale that defines how to convert units from the
   * coordinate system of an application (pixels or centimeters) into meters
   * used by the spatial audio coordinate system.
   * @param forward A vector describing the direction the application
   * considers as forward. The value can be either +1, 0, or -1 and must be
   * orthogonal to up and right.
   * @param up A vector describing the direction the application considers as
   * up. The value can be either +1, 0, or -1 and must be orthogonal to
   * forward and right.
   * @param right A vector describing the direction the application considers
   * as right. The value can be either +1, 0, or -1 and must be orthogonal to
   * forward and up.
   *
   * @returns The result object producing the operation status asynchronously.
   *
   * \code{.cpp}
   * // Invoke the method via wait blocks until the method succeeds or fails
   * wait(sdk->conference().set_spatial_environment(scale, forward, up, right));
   *
   * // Invoking the method directly requires chaining successive operations
   * // via the `then` call
   * sdk->conference().set_spatial_environment(scale, forward, up, right)
   *   .then([]() {
   *     // Environment is now updated
   *   })
   *   .on_error([](auto&& e) {
   *     // Handle exception
   *   });
   * \endcode
   */

  virtual async_result<void> set_spatial_environment(const spatial_scale& scale,
                                                     const spatial_position& forward,
                                                     const spatial_position& up,
                                                     const spatial_position& right) = 0;

  /**
   * @brief Gets the full information about the currently active conference.
   *
   * @returns The result object producing the operation status asynchronously.
   *
   * @code{.cpp}
   *  // Invoke the method via wait blocks until the method succeeds or fails
   *  wait(sdk->conference().get_current_conference());
   *
   *  // Invoking the method directly requires chaining successive operations
   *  // via the `then` call
   *   sdk->conference().get_current_conference()
   *   .then([](conference_info&& info) {
   *      // Process conference information
   *   })
   *   .on_error([](auto&& e){
   *      // The operation failed, no active conference
   *   });
   * @endcode
   */
  virtual async_result<conference_info> get_current_conference() const = 0;

  /**
   * @brief Declines a conference invitation.
   *
   * @param conf_id The conference ID.
   *
   * @returns The result object producing the operation status asynchronously.
   *
   * \code{.cpp}
   * // Invoke the method via wait blocks until the method succeeds or fails
   * wait(sdk->conference().decline_invitation(conf_id));
   *
   * // Invoking the method directly requires chaining successive operations
   * // via the `then` call
   * sdk->conference().decline_invitation(conf_id)
   *   .then([]() {
   *     // Invitation declined successfully
   *   })
   *   .on_error([](auto&& e) {
   *     // Failed to decline invitation
   *   });
   * \endcode
   */
  virtual async_result<void> decline_invitation(const std::string& conf_id) = 0;

  /**
   * @brief Gets a specific participant's audio level.
   *
   * @param participant_id The ID of the participant.
   *
   * @returns The object producing the audio level (between 0.0 and 1.0)
   * asynchronously.
   *
   * \code{.cpp}
   * // Invoke the method via wait blocks until the method succeeds or fails
   * float audio_level = wait(sdk->conference().get_audio_level(participant_id));
   *
   * // Invoking the method directly requires chaining successive operations
   * // via the `then` call
   * sdk->conference().get_audio_level(participant_id)
   *   .then([](float&& audio_level) {
   *     // Use the returned audio level
   *   })
   *   .on_error([](auto&& e) {
   *     // Handle exception
   *   });
   * \endcode
   */
  virtual async_result<float> get_audio_level(const std::string& participant_id) = 0;

  /**
   * @brief Gets audio levels for all participants whose audio level
   * is greater than or equal to 0.05.
   *
   * @returns The object producing the vector of audio_level structs
   * {participant_id, audio_level (between 0.0 and 1.0)} asynchronously.
   *
   * \code{.cpp}
   * // Invoke the method via wait blocks until the method succeeds or fails
   * auto audio_levels = wait(sdk->conference().get_all_audio_levels());
   *
   * // Invoking the method directly requires chaining successive operations
   * // via the `then` call
   * sdk->conference().get_all_audio_levels()
   *   .then([](std::vector<dolbyio::comms::audio_level>&& audio_levels) {
   *     // Process the returned audio levels
   *   })
   *   .on_error([](auto&& e) {
   *     // Handle exception
   *   });
   * \endcode
   */
  virtual async_result<std::vector<dolbyio::comms::audio_level>> get_all_audio_levels() = 0;

  /**
   * @brief Adds a listener for the conference_status_updated events.
   *
   * @param callback The function object that is invoked when the
   * conference status changes.
   *
   * @returns The result object producing the #event_handler_id
   * asynchronously.
   *
   * \code{.cpp}
   * // Invoke the method via wait blocks until the method succeeds or fails
   * handlers.push_back(wait(sdk->conference().add_event_handler(
   *   [](const dolbyio::comms::conference_status_updated& e) {
   *     // Process the event
   *   })));
   *
   * // Invoking the method directly requires chaining successive operations
   * // via the `then` call
   * sdk->conference().add_event_handler(
   *   [](const dolbyio::comms::conference_status_updated& e) {
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
  virtual async_result<event_handler_id> add_event_handler(event_handler<conference_status_updated>&& callback) = 0;

  /**
   * @brief Adds a listener for the participant added events.
   *
   * @param callback The function object that is invoked when a participant
   * is added to the conference.
   *
   * @returns The result object producing the #event_handler_id asynchronously.
   *
   * \code{.cpp}
   *  // Invoke the method via wait blocks until the method succeeds or fails
   *  handlers.push_back(wait(sdk->conference().add_event_handler(
   *    [](const dolbyio::comms::participant_added& e) {
   *      // Process the event
   *    })));
   *
   * // Invoking the method directly requires chaining successive operations
   * // via the `then` call
   * sdk->conference().add_event_handler(
   *   [](const dolbyio::comms::participant_added& e) {
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
  virtual async_result<event_handler_id> add_event_handler(event_handler<participant_added>&& callback) = 0;

  /**
   * @brief Adds a listener for participant_updated events.
   *
   * @param callback The function object that is invoked when a conference
   * participant changes status.
   *
   * @returns The result object producing the #event_handler_id asynchronously.
   *
   * \code{.cpp}
   * // Invoke the method via wait blocks until the method succeeds or fails
   * handlers.push_back(wait(sdk->conference().add_event_handler(
   *   [](const dolbyio::comms::participant_updated& e) {
   *     // Process the event
   *   })));
   *
   * // Invoking the method directly requires chaining successive operations
   * // via the `then` call
   * sdk->conference().add_event_handler(
   *   [](const dolbyio::comms::participant_updated& e) {
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
  virtual async_result<event_handler_id> add_event_handler(event_handler<participant_updated>&& callback) = 0;

  /**
   * @brief Adds a listener for active_speaker_changed events.
   *
   * @param callback The function object that is invoked when the active
   * speaker changes.
   *
   * @returns The result object producing the #event_handler_id asynchronously.
   *
   * \code{.cpp}
   * // Invoke the method via wait blocks until the method succeeds or fails
   * handlers.push_back(wait(sdk->conference().add_event_handler(
   *   [](const dolbyio::comms::active_speaker_change& e) {
   *     // Process the event
   *   })));
   *
   * // Invoking the method directly requires chaining successive operations
   * // via the `then` call
   * sdk->conference().add_event_handler(
   *   [](const dolbyio::comms::active_speaker_change& e) {
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
  virtual async_result<event_handler_id> add_event_handler(event_handler<active_speaker_change>&& callback) = 0;

  /**
   * @brief Adds a listener for video_track_added events.
   *
   * @param callback The function object that is invoked when a new video
   * track is added to a conference.
   *
   * @returns The result object producing the #event_handler_id asynchronously.
   *
   * \code{.cpp}
   * // Invoke the method via wait blocks until the method succeeds or fails
   * handlers.push_back(wait(sdk->conference().add_event_handler(
   *   [](const dolbyio::comms::video_track_added& e) {
   *     // Process the event
   *   })));
   *
   * // Invoking the method directly requires chaining successive operations
   * // via the `then` call
   * sdk->conference().add_event_handler(
   *   [](const dolbyio::comms::video_track_added& e) {
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
  virtual async_result<event_handler_id> add_event_handler(
      event_handler<dolbyio::comms::video_track_added>&& callback) = 0;

  /**
   * @brief Adds a listener for video_track_removed events.
   *
   * @param callback The function object that is invoked when a video track
   * is removed from a conference.
   *
   * @returns The result object producing the #event_handler_id asynchronously.
   *
   * \code{.cpp}
   * // Invoke the method via wait blocks until the method succeeds or fails
   * handlers.push_back(wait(sdk->conference().add_event_handler(
   *   [](const dolbyio::comms::video_track_removed& e) {
   *     // Process the event
   *   })));
   *
   * // Invoking the method directly requires chaining successive operations
   * // via the `then` call
   * sdk->conference().add_event_handler(
   *   [](const dolbyio::comms::video_track_removed& e) {
   *     // Process the event
   *   }))
   *   .then([this](event_handler_id&& handler) {
   *     this->handlers.push_back(std::move(handler));
   *   })
   *   .on_error([](auto&& e) {
   *     // handle exception
   *   });
   * \endcode
   */
  virtual async_result<event_handler_id> add_event_handler(
      event_handler<dolbyio::comms::video_track_removed>&& callback) = 0;

  /**
   * @brief Adds a listener for audio_track_added events.
   *
   * @param callback The function object that is invoked when a new audio
   * track is added to a conference.
   *
   * @returns The result object producing the #event_handler_id asynchronously.
   *
   * \code{.cpp}
   * // Invoke the method via wait blocks until the method succeeds or fails
   * handlers.push_back(wait(sdk->conference().add_event_handler(
   *   [](const dolbyio::comms::audio_track_added& e) {
   *     // Process the event
   *   })));
   *
   * // Invoking the method directly requires chaining successive operations
   * // via the `then` call
   * sdk->conference().add_event_handler(
   *   [](const dolbyio::comms::audio_track_added& e) {
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
  virtual async_result<event_handler_id> add_event_handler(
      event_handler<dolbyio::comms::audio_track_added>&& callback) = 0;

  /**
   * @brief Adds a listener for audio_track_removed events.
   *
   * @param callback The function object that is invoked when an audio
   * track is removed from a conference.
   *
   * @returns The result object producing the #event_handler_id asynchronously.
   *
   * \code{.cpp}
   * // Invoke the method via wait blocks until the method succeeds or fails
   * handlers.push_back(wait(sdk->conference().add_event_handler(
   *   [](const dolbyio::comms::audio_track_removed& e) {
   *     // Process the event
   *   })));
   *
   * // Invoking the method directly requires chaining successive operations
   * // via the `then` call
   * sdk->conference().add_event_handler(
   *   [](const dolbyio::comms::audio_track_removed& e) {
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
  virtual async_result<event_handler_id> add_event_handler(
      event_handler<dolbyio::comms::audio_track_removed>&& callback) = 0;

  /**
   * @brief Uses the add_event_handler method for dvc_error_exception to
   * handle DVC errors when the media engine encounters an error from the DVC
   * library.
   * @param callback The function object that is called with the
   * dvc_error_exception.
   *
   * @returns The result object producing the #event_handler_id asynchronously.
   *
   * \code{.cpp}
   * // Invoke the method via wait blocks until the method succeeds or fails
   * handlers.push_back(wait(sdk->conference().add_event_handler(
   *   [](const dolbyio::comms::dvc_error_exception& e) {
   *     // Process the event
   *   })));
   *
   * // Invoking the method directly requires chaining successive operations
   * // via the `then` call
   * sdk->conference().add_event_handler(
   *   [](const dolbyio::comms::dvc_error_exception& e) {
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
  virtual async_result<event_handler_id> add_event_handler(
      event_handler<dolbyio::comms::dvc_error_exception>&& callback) = 0;

  /**
   * @brief Uses the add_event_handler method for
   * peer_connection_failed_exception to handle errors which are generated when
   * the PeerConnection has entered a failed state.
   * @param callback The function object that is called with the
   * peer_connection_failed_exception.
   *
   * @returns The result object producing the #event_handler_id asynchronously.
   *
   * \code{.cpp}
   * // Invoke the method via wait blocks until the method succeeds or fails
   * handlers.push_back(wait(sdk->conference().add_event_handler(
   *   [](const dolbyio::comms::peer_connection_failed_exception& e) {
   *     // Process the event
   *   })));
   *
   * // Invoking the method directly requires chaining successive operations
   * // via the `then` call
   * sdk->conference().add_event_handler(
   *   [](const dolbyio::comms::peer_connection_failed_exception& e) {
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
  virtual async_result<event_handler_id> add_event_handler(
      event_handler<dolbyio::comms::peer_connection_failed_exception>&& callback) = 0;

  /**
   * @brief Adds a listener for conference_message_received events.
   *
   * @param callback The function object that is invoked when a message is
   * received in the conference.
   *
   * @returns The #async_result class that wraps a #solver that can contain the
   * #event_handler_id object.
   *           - SUCCESS: The #solver returns a void operator and the callback
   * attached via the async_result::then method is invoked.
   *           - FAILURE: The #solver fails with the std::exception_ptr
   * exception that describes the error. The exception pointer is passed to the
   * callback and can be set and handled via async_result::on_error,
   * async_result::then, or async_result::consume_errors.
   *
   * \code{.cpp}
   *  // Invoke the method via wait blocks until the method succeeds or fails
   *  handlers.push_back(wait(sdk->conference().add_event_handler(
   *      [](const dolbyio::comms::conference_message_received& e) {
   *        // Process the event
   *      })));
   *
   * // Invoking the method directly requires chaining successive operations
   * // via the `then` call
   *  sdk->conference().add_event_handler(
   *      [](const dolbyio::comms::conference_message_received& e) {
   *        // Process the event
   *      })
   *      .then([this](event_handler_id&& handler) {
   *        this->handlers.push_back(std::move(handler);
   *      })
   *      .on_error([](auto&& e){
   *        // Handle an exception
   *      });
   * \endcode
   */
  virtual async_result<event_handler_id> add_event_handler(event_handler<conference_message_received>&& callback) = 0;

  /**
   * @brief Adds a listener for conference invitation events.
   *
   * @param callback The function object that is invoked when a conference
   * invitation is received.
   *
   * @returns The #async_result class that wraps a #solver that can contain the
   * #event_handler_id object.
   *           - SUCCESS: The #solver returns a void operator and the callback
   * attached via the async_result::then method is invoked.
   *           - FAILURE: The #solver fails with the std::exception_ptr
   * exception that describes the error. The exception pointer is passed to the
   * callback and can be set and handled via async_result::on_error,
   * async_result::then, or async_result::consume_errors.
   *
   * \code{.cpp}
   *  // Invoke the method via wait blocks until the method succeeds or fails
   *  handlers.push_back(wait(sdk->conference().add_event_handler(
   *      [](const dolbyio::comms::conference_invitation_received& e) {
   *        // Process the event
   *      })));
   *
   * // Invoking the method directly requires chaining successive operations
   * // via the `then` call
   *  sdk->conference().add_event_handler(
   *      [](const dolbyio::comms::conference_invitation_received& e) {
   *        // Process the event
   *      })
   *      .then([this](event_handler_id&& handler) {
   *        this->handlers.push_back(std::move(handler);
   *      })
   *      .on_error([](auto&& e){
   *        // Handle an exception
   *      });
   * \endcode
   */
  virtual async_result<event_handler_id> add_event_handler(
      event_handler<conference_invitation_received>&& callback) = 0;
};

}  // namespace services

}  // namespace dolbyio::comms
