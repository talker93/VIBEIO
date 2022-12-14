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

namespace dolbyio::comms {

/**
 * @brief The base class that describes exceptions that are thrown when an
 * asynchronous operation is abruptly canceled.
 * @ingroup exceptions
 */
class DLB_COMMS_EXPORT async_operation_canceled : public exception {
 public:
  /**
   * @brief The constructor that takes a reason for failure.
   * @param reason A string that indicates a reason for failure.
   */
  async_operation_canceled(const std::string& reason);
};

/**
 * @brief The base class for the JSON and depicts exceptions that are thrown
 * when there is a JSON parsing error. This means an error in serializing or
 * deserializing JSON string or object.
 * @ingroup exceptions
 */
class DLB_COMMS_EXPORT json_exception : public exception {
 public:
  /**
   * @brief The constructor that takes a description of the exception.
   * @param description The description of the exception.
   * @param path JSON object path
   */
  json_exception(std::string&& description, std::string&& path);

  /**
   * @brief The accessor that returns the full path of the exception.
   * @return A string that contains a full path of the exception.
   */
  const std::string& path() const noexcept { return path_; }

  /**
   * @brief The accessor that returns a description of the exception string.
   * @return A string that contains a description of the exception.
   */
  const std::string& desc() const noexcept { return description_; }

 protected:
  std::string description_;
  std::string path_;
};

/**
 * @brief Describes the JSON web token exceptions indicating that the
 * authentication web token is not correct.
 * @ingroup exceptions
 */
class DLB_COMMS_EXPORT jwt_exception : public exception {
 public:
  /**
   * @brief The constructor that takes the body of the token.
   * @param jwt_body The body of the web token.
   */
  jwt_exception(const std::string& jwt_body);
};

/**
 * @brief The base class for all IO exceptions that occur in the case of
 * problems with opening sockets or files.
 * @ingroup exceptions
 */
class DLB_COMMS_EXPORT io_exception : public exception {
 public:
  /**
   * @brief The constructor that takes the description of the exception.
   * @param description A string that describes the exception.
   */
  io_exception(const std::string& description);
};

using network_exception = io_exception;

/**
 * @brief The class for exceptions indicating that the CA certificates are not
 * properly loaded.
 * @ingroup exceptions
 */
class DLB_COMMS_EXPORT security_check_exception : public io_exception {
 public:
  /**
   * @brief The security_check_exception constructor.
   * @param message A string that describes the error.
   */
  security_check_exception(const std::string& message);
};

/**
 * @brief The class that describes IO errors that occur on the signaling
 * WebSocket. The user can also set a listener for this exception object using
 * sdk::add_event_handler to emitted the event whenever a signaling error
 * occurs.
 * @ingroup exceptions
 * @ingroup events
 */
class DLB_COMMS_EXPORT signaling_channel_exception : public io_exception {
 public:
  /**
   * @brief The signaling_channel_exception constructor.
   * @param message A string that describes the error.
   */
  signaling_channel_exception(const std::string& message);
};

/**
 * @brief The class that describes the HTTP errors.
 * @ingroup exceptions
 */
class DLB_COMMS_EXPORT http_exception : public io_exception {
 public:
  /**
   * @brief The constructor that takes the HTTP error code.
   * @param code The HTTP error code.
   */
  http_exception(int code);

  /**
   * @brief The accessor that returns the HTTP error code of the exception.
   * @return An integer value that represents the HTTP error code.
   */
  int http_code() const noexcept { return code_; }

 private:
  int code_;
};

/**
 * @brief The class that describes errors received from the backend service
 * during REST API calls.
 * @ingroup exceptions
 */
class DLB_COMMS_EXPORT restapi_exception : public io_exception {
 public:
  /**
   * @brief The constructor that takes the error code, reason, and description
   * of the error from a server response.
   * @param http_code The HTTP code.
   * @param rest_code The REST API code.
   * @param err_reason The reason for the error.
   * @param err_description The error description.
   */
  restapi_exception(int http_code, int rest_code, const std::string& err_reason, const std::string& err_description);

  /**
   * @brief The accessor for the HTTP code of the exception.
   * @return An integer that represents the HTTP code.
   */
  int http_code() const noexcept { return http_; }

  /**
   * @brief The accessor for the REST code of the exception.
   * @return An integer that represents the REST error code.
   */
  int rest_code() const noexcept { return rest_; }

  /**
   * @brief The accessor for the reason of the error.
   * @return A string that contains the reason for the error returned in the
   * REST response.
   */
  const std::string& reason() const noexcept { return reason_; }

  /**
   * @brief The accessor for the error description.
   * @return A string that contains the description of the error returned in the
   * REST response.
   */
  const std::string& description() const noexcept { return desc_; }

 private:
  int http_;
  int rest_;
  std::string reason_;
  std::string desc_;
};

/**
 * @brief The class that describes the rejected REST API requests caused by an
 * invalid access token. The user can also set a listener for this exception
 * object using sdk::add_event_handler. In such a case, the event will be
 * emitted whenever a signaling error occurs.
 * @ingroup exceptions
 * @ingroup events
 */
class DLB_COMMS_EXPORT invalid_token_exception : public restapi_exception {
 public:
  /**
   * @brief The constructor that takes the error codes, reasons, and
   * descriptions of the error that is in the server response.
   * @param http_code The HTTP code.
   * @param rest_code The REST API code.
   * @param err_reason The reason for the error.
   * @param err_description The error description.
   */
  invalid_token_exception(int http_code,
                          int rest_code,
                          const std::string& err_reason,
                          const std::string& err_description);
};

/**
 * @brief The class that describes errors related to the Session Service.
 * @ingroup exceptions
 */
class DLB_COMMS_EXPORT session_exception : public exception {
 public:
  /**
   * @brief The constructor that takes the error description.
   * @param description A string that describes the exception.
   */
  session_exception(const std::string& description);
};

/**
 * @brief The class that describes errors that occur in an active conference.
 * @ingroup exceptions
 */
class DLB_COMMS_EXPORT conference_exception : public exception {
 public:
  /**
   * @brief The constructor that takes the exception description.
   * @param description The description of the exception.
   */
  conference_exception(const std::string& description);
};

/**
 * @brief The class that describes errors for attempted operations that require
 * a different conference state.
 * @ingroup exceptions
 */
class DLB_COMMS_EXPORT conference_state_exception : public conference_exception {
 public:
  /**
   * @brief The constructor that takes the current the required state.
   * @param current The current state of a conference.
   * @param required The required state of a conference.
   */
  conference_state_exception(const std::string& current, const std::string& required);

  /**
   * @brief Returns the current state of a conference that is stored in the
   * exception.
   * @return The current session state.
   */
  const std::string& current_state() const noexcept { return current_; }

  /**
   * @brief Returns the required state of a conference that is stored in the
   * exception.
   * @return A string that contains the required state of a conference.
   */
  const std::string& required_state() const noexcept { return required_; }

 private:
  std::string current_;
  std::string required_;
};

/**
 * @brief The base class describing exceptions stemming from media engine.
 * @ingroup exceptions
 */
class DLB_COMMS_EXPORT media_engine_exception : public conference_exception {
 public:
  /**
   * @brief media_engine_exception constructor
   * @param reason describing the media engine exception.
   */
  media_engine_exception(const std::string& reason);
};

/**
 * @brief The exception that is emitted when there is a
 * failure detected with the peer connection to the server. Meaning One or more
 * of the ICE transports on the connection is in the failed state. This error is
 * strict and indicates that peer connection will not return to normal state.
 * This exception can also be listened for by attaching a handler for the
 * peer_connection_failed_exception type through the Conference Service.
 * @ingroup exceptions
 */
class DLB_COMMS_EXPORT peer_connection_failed_exception : public media_engine_exception {
 public:
  /**
   * @brief The constructor that takes a reason for failure.
   * @param reason Describes in detail why failure occurred.
   */
  peer_connection_failed_exception(const std::string& reason);
};

/**
 * @brief The class that describes a conference error which
 * originates in the DVC library. This exception can also be listened for
 * by attaching a handler for the dvc_error_exception type through the
 * Conference Service.
 * @ingroup exceptions
 */
class DLB_COMMS_EXPORT dvc_error_exception : public media_engine_exception {
 public:
  /**
   * @brief The constructor that takes a reason for the DVC error.
   * @param reason Describes in detail why the error occurred.
   */
  dvc_error_exception(const std::string& reason);
};

/**
 * @brief The class that describes conference error where
 * the WebRTC is unable to create an answer for the received offer.
 * @ingroup exceptions
 */
class DLB_COMMS_EXPORT create_answer_exception : public media_engine_exception {
 public:
  /**
   * @brief The constructor that takes a reason for error.
   * @param reason Describes in detail the reason for failing to create
   * answer.
   */
  create_answer_exception(const std::string& reason);
};

/**
 * @brief The class that describes conferencing errors
 * that occur when setting a remote description fails. This means that
 * there is an incompatibility in the offer, for example, that the offer
 * contains not supported codecs.
 * @ingroup exceptions
 */
class DLB_COMMS_EXPORT remote_description_exception : public media_engine_exception {
 public:
  /**
   * @brief The constructor that takes a reason for error.
   * @param reason Describes in detail the reason for remote description setup
   * failure.
   */
  remote_description_exception(const std::string& reason);
};

/**
 * @brief The class that describes a conferencing
 * error which may occur during a creation of an initial peer connection.
 * @ingroup exceptions
 */
class DLB_COMMS_EXPORT create_peer_connection_exception : public media_engine_exception {
 public:
  /**
   * @brief The default constructor.
   */
  create_peer_connection_exception();
};

/**
 * @brief The class that describes a conferencing error which
 * can occur during an initial setting of candidates for a connection when
 * the remote description is successfully applied.
 * @ingroup exceptions
 */
class DLB_COMMS_EXPORT ice_candidate_exception : public media_engine_exception {
 public:
  /**
   * @brief The constructor that takes the reason for error.
   * @param reason Describes in detail the reason for the ice candidate error.
   */
  ice_candidate_exception(const std::string& reason);
};

/**
 * @brief The exception that indicates an issue with the local media
 * stream.
 * @ingroup exceptions
 */
class DLB_COMMS_EXPORT media_stream_exception : public media_engine_exception {
 public:
  /**
   * @brief The default constructor for the error.
   */
  media_stream_exception();
};

/**
 * @brief The class that describes a conference error that results from a
 * network connection issue. This error has a direct correlation with the
 * disconnected PeerConnectionState. According to the WebRTC documentation, this
 * state may be triggered intermittently and resolve without any action on less
 * reliable networks. Therefore, this error is not considered fatal or
 * non-recoverable like the PeerConnectionFailed state.
 * @ingroup exceptions
 */
class DLB_COMMS_EXPORT peer_connection_disconnected_exception : public media_engine_exception {
 public:
  /**
   * @brief The constructor that takes the reason for error.
   * @param reason Describes in detail the reason for the
   * peer_connection_disconnected_exception.
   */
  peer_connection_disconnected_exception(const std::string& reason);
};

/**
 * @brief An error setting spatial coordinates.
 * @ingroup exceptions
 */
class DLB_COMMS_EXPORT spatial_placement_exception : public exception {
 public:
  /**
   * @brief The constructor that takes the description of the exception.
   * @param description A string indicating a reason for the exception.
   */
  spatial_placement_exception(const std::string& description);
};

}  // namespace dolbyio::comms
