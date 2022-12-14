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
#include <dolbyio/comms/exports.h>

#include <variant>

namespace dolbyio::comms {

/**
 * @brief The audio noise reduction level.
 */
enum class DLB_COMMS_EXPORT noise_reduction {
  high, /**< Reduce all noise and isolate voice. */
  low,  /**< Remove steady/stationary background noise. */
};

namespace audio_capture_mode {

/**
 * @brief Capture background noise and non-voice sounds.
 *
 * This mode uses echo suppression. No noise suppression is applied, and so a low end microphone or a noisy environment
 * may introduce the noise into the conference.
 */
struct DLB_COMMS_EXPORT unprocessed {};

/**
 * @brief Capture the voice and reduce non-voice sounds.
 *
 * This mode focuses on best voice quality. Echo suppression is always enabled when using this mode. Additionally, it
 * offers a tunable setting for the level of voice isolation and non-voice noise reduction, and supports the following
 * settings:
 *
 * - when using noise_reduction::high, provides a strong voice isolation and attempts to remove all non-voice sound from
 * input.
 * - when using noise_reduction::low, removes only steady background noise from the input.
 */
struct DLB_COMMS_EXPORT standard {
  /**
   * @brief The constructor.
   * @param reduce_noise the noise reduction level.
   */
  constexpr standard(noise_reduction reduce_noise) : reduce_noise(reduce_noise) {}

  /**
   * @brief The noise reduction level.
   */
  noise_reduction reduce_noise;
};

/**
 * A type-safe tagged union capable of holding any of the supported audio capture modes.
 */
using variant = std::variant<unprocessed, standard>;
}  // namespace audio_capture_mode

namespace services {

/**
 * @brief The local audio service.
 *
 * This service is used to control local participant's input audio properties, and enabling or disabling audio capture
 * and sending into conference.
 *
 * The start and stop audio methods of the local audio service should only be used when the conference is active.
 *
 * @attention  The local_audio interface contains methods that return
 * #async_result. Each function that returns #async_result is asynchronous and
 * the operation is executed during the SDK event loop. The caller can block the
 * calling thread until the operation completes using the #wait helper. The
 * caller can also chain consecutive operations, which are dependent on the
 * completion of this method, using the async_result::then calls. Each
 * async_result chain needs to be terminated with an async_result::on_error.
 */
class DLB_COMMS_EXPORT local_audio {
 public:
  /**
   * @brief Opens the input audio device and starts processing audio.
   *
   * This method creates a WebRTC AudioTrack and attaches the AudioTrack to an active Peer Connection. This method also
   * connects the Audio Sink of the media_source_interface with the WebRTC Audio Source, creating the audio delivery
   * pipeline. When the async_result of this method resolves, the application can start an audio stream from any media
   * source it chooses, pass the raw audio PCM data to the injector module from this source, and inject the audio data
   * into a conference.
   *
   * @attention This method does not start the capturing from a media source; this action needs to be performed
   * separately, when the async_result of this method is resolved.
   *
   * @return The result object producing the operation status asynchronously.
   */
  virtual async_result<void> start() = 0;

  /**
   * @brief Closes the input audio device and stops processing audio.
   *
   * This method destructs a WebRTC AudioTrack and detaches it from an active Peer Connection. This method also
   * disconnects the Audio Sink of the media_source_interface from the WebRTC Audio Source, which deconstructs the audio
   * delivery pipeline. All frames captured after the return of stop_audio are discarded, so an application should stop
   * capturing audio from the local media source when async_result of this method resolves.
   *
   * @attention This method does not stop the capturing from a media source; this action needs to be performed
   * separately by an application before or after calling stop_audio.
   *
   * @return The result object producing the operation status asynchronously.
   */
  virtual async_result<void> stop() = 0;

  /**
   * @brief Sets the unprocessed audio capture mode.
   *
   * @param mode The audio capture mode.
   * @return The result object producing the operation status asynchronously.
   */
  virtual async_result<void> set_capture_mode(const audio_capture_mode::unprocessed& mode) = 0;

  /**
   * @brief Sets the standard audio capture mode.
   *
   * @attention The Standard capture mode with low noise reduction is not supported in server-side use-cases or
   * non-Dolby Voice conferences. Trying to set standard capture mode in the aforementioned cases will result in
   * actual setting of Default capture mode.
   *
   * @param mode The audio capture mode.
   * @return The result object producing the operation status asynchronously.
   */
  virtual async_result<void> set_capture_mode(const audio_capture_mode::standard& mode) = 0;

  /**
   * @brief Reads the current audio capture mode.
   *
   * @attention This method is only available for Dolby Voice Conferences.
   *
   * @attention This method is not supported in server-side use-cases.
   *
   * @return The result object producing the variant holding audio capture mode asynchronously.
   */
  virtual async_result<audio_capture_mode::variant> get_capture_mode() = 0;
};

/**
 * @brief The remote audio service.
 *
 * This service is used to control the local properties of the remote participants' audio, that is the audio which is
 * played in the conference on the current client's endpoint.
 *
 * All methods of the remote audio service should only be used when the conference is active.
 *
 * @attention  The remote_audio interface contains methods that return
 * #async_result. Each function that returns #async_result is asynchronous and
 * the operation is executed during the SDK event loop. The caller can block the
 * calling thread until the operation completes using the #wait helper. The
 * caller can also chain consecutive operations, which are dependent on the
 * completion of this method, using the async_result::then calls. Each
 * async_result chain needs to be terminated with an async_result::on_error.
 */
class DLB_COMMS_EXPORT remote_audio {
 public:
  /**
   * @brief Enables the participant's audio to be played locally.
   *
   * This method resumes the reception of audio coming from the remote participant, whose audio has been previously
   * stopped locally. By default, remote participants' audio tracks are enabled, and attempting to start their audio
   * locally will do nothing. This method allows an audio track from the desired remote participant to be mixed into the
   * Dolby Voice audio stream received by the SDK. If the participant does not have their audio enabled, this method
   * does not enable their audio track.
   *
   * @attention This method is only available for Dolby Voice Conferences.
   *
   * @param participant_id The ID of the participant whose audio track you would like to hear.
   * @return The result object producing the operation status asynchronously.
   */
  virtual async_result<void> start(const std::string& participant_id) = 0;

  /**
   * @brief Disables the local playback of the remote participant's audio.
   *
   * This method allows the local participant to not receive an audio track from a selected remote participant. This
   * method only impacts the local participant; the rest of conference participants will still hear the participant's
   * audio. This method does not allow the audio track of the selected remote participant to be mixed into the Dolby
   * Voice audio stream that the SDK receives.
   *
   * @attention This method is only available for Dolby Voice Conferences.
   *
   * @param participant_id The ID of the participant whose audio track you would like to stop.
   *
   * @returns The result object producing the operation status asynchronously.
   */
  virtual async_result<void> stop(const std::string& participant_id) = 0;
};

/**
 * @brief The audio service.
 */
class DLB_COMMS_EXPORT audio {
 public:
  /**
   * @brief Gets the local audio service instance.
   * @return the local audio service.
   */
  virtual local_audio& local() = 0;

  /**
   * @brief Gets the remote audio service instance.
   * @return the remote audio service.
   */
  virtual remote_audio& remote() = 0;
};

}  // namespace services
}  // namespace dolbyio::comms
