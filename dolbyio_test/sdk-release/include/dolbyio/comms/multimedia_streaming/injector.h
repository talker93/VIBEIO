#pragma once

/***************************************************************************
 * This program is licensed by the accompanying "license" file. This file is
 * distributed "AS IS" AND WITHOUT WARRANTY OF ANY KIND WHATSOEVER, either
 * express or implied. See the License for the specific language governing
 * permissions and limitations under the License.
 *
 *                Copyright (C) 2022 by Dolby Laboratories.
 ***************************************************************************/

#include <dolbyio/comms/exports.h>
#include <dolbyio/comms/media_engine/media_engine.h>

#include <chrono>
#include <condition_variable>
#include <functional>
#include <memory>
#include <mutex>
#include <thread>

namespace dolbyio::comms::plugin {

/**
 * @defgroup default_injector Default Media Injector Plugin
 *
 * The Default Media Injector plugin is a module that allows injecting locally
 * captured media into the conference. The Default Media Injector provides
 * interfaces for injecting decoded video and audio streams. A media source
 * must be implemented from where this media is to be injected. This source
 * can be a media file, live stream, etc..
 */

/**
 * @brief The current media injection status is expressed via this structure.
 * This structure can be passed to the application by the injector to describe
 * the current status.
 * @ingroup default_injector
 */
struct DLB_COMMS_ADDON_EXPORT media_injection_status {
  /**
   *  @brief Enum describing the state of the media.
   */
  enum state { STOPPED = 0, INJECTING, ERROR };
  /**
   * @brief The type of media this status is for.
   */
  enum type { AUDIO = 1, VIDEO };

  /**
   * @brief Constructor taking only the media type
   * @param type Enum value describing the type of media.
   */
  media_injection_status(type type);

  /**
   * @brief Explicit constructor taking the type, state and extra
   * description.
   * @param type Enum value describing the type of media.
   * @param state Enum value describing the state.
   * @param description Extra description of current media state.
   */
  media_injection_status(type type, state state, const std::string& description = {});

  /**
   * Default destructor.
   */
  ~media_injection_status();

  type type_;                 /**< Holds the value of the type of media */
  state state_{STOPPED};      /**<  Holds the value of current state for single media */
  std::string description_{}; /**< Holds the value of any extra description of
                               current state for single media */
};

/**
 * @brief The base abstract class for the Default Media Injector's.
 * @ingroup default_injector
 */
class DLB_COMMS_ADDON_EXPORT injector : public video_frame_handler, public video_source, public audio_source {
 public:
  using media_injection_status_cb = std::function<void(const media_injection_status&)>;
  using has_video_sink_cb = std::function<void(bool)>;
  /**
   * @brief Constructor for the injector base abstract class.
   *
   * @param status_cb Status callback function object, which the application
   * should provide. The injector invokes the function object for any status
   * changes it encounters and provide the media_injection_status object as a
   * parameter. This callback is invoked on either of the injector's media
   * injection thread, or possibly on the injection sources thread.
   * Either way the application should ensure this callback is thread-safe.
   */
  injector(media_injection_status_cb&& status_cb);

  /**
   * @brief The destructor of the injector.
   */
  virtual ~injector();

  /**
   * @brief Sets the callback notifying about the video sink presence.
   *
   * The application may use this callback to start/stop injection, but can be configured manually,
   * regardless of the injector's video sink status.
   *
   * @param cb
   */
  void set_has_video_sink_cb(has_video_sink_cb&& cb);

  /**
   * @brief Inject an audio frame into the conference. The audio frame must
   * contain 10ms of PCM data.
   *
   * When this function returns, the audio_frame is be deleted and the audio
   * data is memcpy'd in the WebRtcAudioSendStream::OnData function (which
   * implements AudioSource::Sink::OnData).
   *
   * @param frame Audio frame containing raw PCM data.
   *
   * @retval true On successful push of frame.
   * @retval false On failure to push frame.
   */
  virtual bool inject_audio_frame(std::unique_ptr<audio_frame>&& frame) = 0;

  /**
   * @brief Inject a raw video frame into the conference. The video frame must
   * contain YUV pixels.
   *
   * This function returns once the raw video frame has been placed onto
   * the encoder_queue_ by VideoStreamEncoderOnFrame. The video frame is only
   * actually free once the encoder is done with it.
   *
   * @param frame The video frame.
   *
   * @retval true On successful push of frame.
   * @retval false On failure to push frame.
   */
  virtual bool inject_video_frame(std::unique_ptr<video_frame>&& frame) = 0;

 private:
  /// @cond DO_NOT_DOCUMENT
  video_source* source() override { return this; }
  video_sink* sink() override { return nullptr; }
  void register_audio_frame_rtc_source(rtc_audio_source* source) override;
  void deregister_audio_frame_rtc_source() override;
  void set_sink(video_sink* sink, const video_source::config& config) override;

 protected:
  void report_status(const media_injection_status& status);
  media_injection_status_cb status_cb_{};
  std::mutex cb_lock_{};
  has_video_sink_cb has_video_sink_cb_{};
  media_injection_status audio_status_{media_injection_status::AUDIO};
  media_injection_status video_status_{media_injection_status::VIDEO};

  std::mutex audio_lock_{};
  std::mutex video_lock_{};
  video_sink* video_sink_{nullptr};
  rtc_audio_source* rtc_audio_source_{nullptr};
  /// @endcond
};

/**
 * @brief The interface for the Default Passthrough Media Injector.
 * @ingroup default_injector
 */
class DLB_COMMS_ADDON_EXPORT injector_passthrough : public injector {
 public:
  /**
   * @brief Constructor for the passthrough injector.
   *
   * @param status_cb Status callback function object, which the application
   * should provide. The injector invokes the function object for any status
   * changes it encounters and provides the media_injection_status object as a
   * parameter. This callback is invoked on either of the injector's media
   * injection thread, or possibly on the injection sources thread. Either way
   * the application should ensure this callback is thread-safe.
   */
  injector_passthrough(media_injection_status_cb&& status_cb);

  /**
   * @brief Destructor for passthrough injector.
   */
  ~injector_passthrough();

  /**
   * @brief Inject an audio frame into the conference. The audio frame must
   * contain 10ms of PCM data.
   *
   * This method directly passes the frames to WebRTC. The application is
   * responsible for providing frames in steady 10ms intervals.
   *
   * @param frame Audio frame containing raw PCM data.
   *
   * @retval true On successful push of frame.
   * @retval false On failure to push frame.
   */
  bool inject_audio_frame(std::unique_ptr<audio_frame>&& frame) override;

  /**
   * @brief Inject a raw video frame into the conference. The video frame must
   * contain YUV pixels.
   *
   * This method directly passes the frames to WebRTC. The application is
   * responsible for delivering frames at a proper rate.
   *
   * @param frame The video frame.
   *
   * @retval true On successful push of frame.
   * @retval false On failure to push frame.
   */
  bool inject_video_frame(std::unique_ptr<video_frame>&& frame) override;
};

/**
 * @brief The interface for the Default Paced Media Injector.
 * @ingroup default_injector
 */
class DLB_COMMS_ADDON_EXPORT injector_paced : public injector {
 public:
  /**
   * @brief Constructor for the passthrough injector.
   *
   * @param status_cb Status callback function object, which the application
   * should provide. The injector invokes the function object for any status
   * changes it encounters and provide the media_injection_status object as a
   * parameter. This callback is invoked on either of the injector's media
   * injection thread, or possibly on the injection sources thread. Either way
   * the application should ensure this callback is thread-safe.
   */
  injector_paced(media_injection_status_cb&& status_cb);

  /**
   * Destructor of the paced injector.
   */
  ~injector_paced();

  /**
   * @brief Inject an audio frame into the conference. The audio frame must
   * contain 10ms of PCM data.
   *
   * This method can be called more often than every 10ms. The audio frames are
   * queued and provided to WebRTC every 10ms. The queue can hold up to one
   * second of audio; once the queue fills, this function blocks the calling
   * thread until space on the queue is available.
   *
   * @param frame Audio frame containing raw PCM data.
   *
   * @retval true On successful push of frame.
   * @retval false On failure to push frame.
   */
  bool inject_audio_frame(std::unique_ptr<audio_frame>&& frame) override;

  /**
   * @brief Inject a raw video frame into the conference. The video frame must
   * contain YUV pixels.
   *
   * The desired frame interval for injecting the frames into WebRTC conference
   * should be configured via the #set_video_frame_interval method. The video
   * frames are queued and provided to WebRTC every frame interval. The
   * queue can hold up to 10 video frames; once the queue fills this function
   * blocks the calling thread until space on the queue is available.
   *
   * @attention This method blocks the calling thread when the video queue
   * is full.
   *
   * @param frame The video frame.
   *
   * @retval true On successful push of frame.
   * @retval false On failure to push frame.
   */
  bool inject_video_frame(std::unique_ptr<video_frame>&& frame) override;

  /**
   * @brief Sets the video frame interval for injecting video frames into the
   * conference. The pacing injector provides frames to WebRTC at the
   * interval specified by this method, provided the frames are available on the
   * queue.
   *
   * @param interval The frame interval in milliseconds.
   */
  void set_video_frame_interval(std::chrono::milliseconds interval);

  /**
   * @brief Enables the video injection pacing thread for taking video
   * frames from the queue and injecting them into the conference.
   *
   */
  void start_video_injection();

  /**
   * @brief Stops the video injection pacing thread from taking video
   * frames from the queue and injecting them into the conference. There are two
   * options for stopping injection; the default option, which is considered forceful
   * stoppage, stops the video pacing thread immediately. Frames that are on
   * the queue when a force stop is invoked remain. The second option, which is considered
   * graceful stoppage allows the caller to wait until all the frames on the
   * queue are popped off by the video pacing thread, then stops the thread.
   *
   * @param force Boolean indicating whether to forcefully shutdown the
   * video injection or wait for queue to become empty.
   *
   */
  void stop_video_injection(bool force = true);

  /**
   * @brief Enables the audio injection pacing thread for taking audio
   * frames from the queue and injecting them into the conference.
   *
   */
  void start_audio_injection();

  /**
   * @brief Start injecting audio silence into the conference. This should be
   * used if the injection source wants to pause the injection of media while
   * not removing the audio track. In order to keep AV sync on point when the
   * media injection is resumed, calling this function will start the audio
   * injection pacing thread to provide WebRTC with silent audio frames. The
   * silence injection is stopped the same way as regular injection via the
   * stop_audio_injection method.
   *
   * @param sample_rate The sample rate of  the audio stream which was being
   * injected before being paused.
   * @param channels The number of channels of the audio stream which was being
   * injected before being paused.
   */
  void start_audio_silence_injection(int sample_rate, int channels);

  /**
   * @brief Stops the audio injection pacing thread from taking audio
   * frames from the queue and injecting them into the conference. There are two
   * options for stopping injection; the default option, which is considered forceful
   * stoppage, stops the audio pacing thread immediately. Frames that are on
   * the queue when a force stop is invoked remain. The second option, which is considered
   * graceful stoppage allows the caller to wait until all the frames on the
   * queue are popped off by the audio pacing thread, then stops the thread.
   *
   * @param force Boolean indicating whether to forcefully shutdown the
   * audio injection or wait for queue to be cleared.
   *
   */
  void stop_audio_injection(bool force = true);

  /**
   * @brief Clears the audio queue that is holding audio frames
   * awaiting their injection into conference. This method unblocks
   * any threads that are blocked with the inject_audio_frame method.
   */
  void clear_audio_queue();

  /**
   * @brief Clears the video queue that is holding video frames
   * awaiting their injection into conference. This method unblocks
   * any threads that are blocked with the inject_video_frame method.

   */
  void clear_video_queue();

 private:
  /// @cond DO_NOT_DOCUMENT
  class audio_queue;
  class video_queue;
  struct audio_queue_deleter {
    void operator()(audio_queue*) const;
  };
  struct video_queue_deleter {
    void operator()(video_queue*) const;
  };

  std::condition_variable audio_cond_{};
  std::condition_variable video_cond_{};
  std::unique_ptr<std::thread> audio_thread_{};
  std::unique_ptr<std::thread> video_thread_{};
  std::unique_ptr<audio_queue, audio_queue_deleter> audio_queue_{};
  std::unique_ptr<video_queue, video_queue_deleter> video_queue_{};
  std::chrono::milliseconds video_frame_interval_{33};
  /// @endcond
};

}  // namespace dolbyio::comms::plugin
