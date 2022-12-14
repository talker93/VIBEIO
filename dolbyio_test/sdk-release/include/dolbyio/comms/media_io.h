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
#include <dolbyio/comms/media_engine/media_engine.h>

namespace dolbyio::comms::services {

/**
 * @brief The Media IO service is intended for non-standard WebRTC utilization like recording the incoming
 * video and audio streams to files or injecting audio and video from a file into the conference. Application can
 * utilize this service to set encoded video and raw audio sinks as endpoints for remote video/audio streams or set
 * video/audio sources on the SDK where media will be injected. Using this service for audio injection or recording
 * forces the conference to use Opus.
 */
class media_io {
 public:
  /**
   *
   * @brief Sets the Encoded Video Sink on the SDK to start receiving the incoming frames before they
   * are decoded. The frames can then be passed to the decoder and regular VideoSinks for rendering. However,
   * if only encoded dumping is desired configuring the recorder implementation to dump ENCODED_OPTIMIZED frames will
   * make it so the frames are not decoded just dumped. This greatly saves CPU but can result in poor
   * video quality in the file in poor network conditions (if keyFrames are dropped there is no retry mechanism since
   * the frames are never decoded), this the optimized option is suitable best when using the SDK in Cloud Server
   * scenarios where bandwith/poor network are not an issue.
   *
   * @attention  The Encoded Sink can not be set when a conference is active.
   *
   * @param sink The pointer to the instance of the Encoded Video Sink interface, which can be either the default
   * recorder created through the plugin::recorder interface or an implemented customized recording module.
   *
   * @returns The result object producing the operation status asynchronously.
   *
   * \code{.cpp}
   * // Invoke the method via wait blocks until the method succeeds or fails
   * wait(sdk->media_io().set_encoded_sink(recorder));
   *
   * // Invoking the method directly requires chaining successive operations
   * // via the `then` call
   * sdk->media_io().set_encoded_sink(recorder, true)
   *   .then([]() {
   *     // Media capturer is now set
   *   })
   *   .on_error([](auto&& e) {
   *     // Handle exception
   *   });
   * \endcode
   */
  virtual async_result<void> set_encoded_video_sink(video_sink_encoded* sink) = 0;

  /**
   *
   * @brief Sets the Audio Sink on the SDK to start receiving the incoming Audio Frames after they
   * are decoded. Setting an Audio Sink means that audio sent to the SDK will be encoded with Opus codec.
   *
   * @attention  The Audio Sink can not be set when a conference is active.
   *
   * @param sink The pointer to the instance of the Audio Sink interface, which can be either the default recorder
   * created through the plugin::recorder interface or an implemented customized recording module.
   *
   * @returns The result object producing the operation status asynchronously.
   *
   * \code{.cpp}
   * // Invoke the method via wait blocks until the method succeeds or fails
   * wait(sdk->media_io().set_media_sink(recorder));
   *
   * // Invoking the method directly requires chaining successive operations
   * // via the `then` call
   * sdk->media_io().set_audio_sink(recorder)
   *   .then([]() {
   *     // The audio sink is now set
   *   })
   *   .on_error([](auto&& e) {
   *     // Handle exception
   *   });
   * \endcode
   */
  virtual async_result<void> set_audio_sink(audio_sink* sink) = 0;

  /**
   *
   * @brief Sets the Audio Source on the SDK to provide Audio Frames to the Conference. The Audio Source can be an
   * instance of the Default Injector or a Custom Injector. Injected audio will be encoded in Opus.
   *
   * @attention  The Audio Source can not be set when a conference is active.
   *
   * @param source The pointer to the instance of the Audio Source interface.
   *
   * @returns The result object producing the operation status asynchronously.
   *
   * \code{.cpp}
   * // Invoke the method via wait blocks until the method succeeds or fails
   * wait(sdk->media_io().set_audio_source(injector));
   *
   * // Invoking the method directly requires chaining successive operations
   * // via the `then` call
   * sdk->media_io().set_audio_source(injector)
   *   .then([]() {
   *     // Audio Source is now set
   *   })
   *   .on_error([](auto&& e) {
   *     // Handle exception
   *   });
   * \endcode
   */
  virtual async_result<void> set_audio_source(audio_source* source) = 0;
};

}  // namespace dolbyio::comms::services
