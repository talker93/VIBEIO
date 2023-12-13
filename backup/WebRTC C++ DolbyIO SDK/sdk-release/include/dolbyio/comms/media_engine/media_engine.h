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
#include <dolbyio/comms/log_level.h>

#include <dolbyio/comms/media_engine/media_exceptions.h>

#include <limits>
#include <memory>
#include <string>

#ifdef linux
#undef linux
#warning "Please fix your Compiler settings"
#endif

/**
 * @defgroup events SDK Event Types
 * The SDK Event Types group gathers all event types for the C++ SDK.
 *
 * @defgroup media_record_api Media Recorder API
 * The Media Recorder API gathers classes and functions for writing modules to
 * record audio and video.
 *
 * @defgroup media_injector_api Media Injector API
 * The Media Injector API gathers classes and functions for writing modules to
 * inject audio and video into a conference. This API provides everything
 * necessary to write an injector, which can be connected to the CoreSDK. Once
 * connected the injector establishes the pipelines which allow an application
 * to inject audio and video into a conference.
 */

namespace dolbyio {

namespace comms {

/**
 * @brief Event indicates that a video track is no longer being received.
 * @ingroup events
 */
struct DLB_COMMS_MEDIA_EXPORT video_track_removed {
  std::string peer_id;          /**< The ID of the participant to whom the track belonged. */
  std::string stream_id;        /**< The ID of the stream to which the video track belonged. */
  std::string track_id;         /**< The ID of the video track. */
  std::string sdp_track_id;     /**< The ID of the track in the SDP matching the
                                   sender side. */
  bool is_screenshare = false;  /**< A boolean that indicates whether the video
                                   track was a screen share track. */
  bool capturer_status = false; /**< A boolean that indicates whether there was
                                   a sink attached to the video track. */
  bool remote;                  /**< Boolean indicating whether the track is from a remote
                                   participant.*/
};

/**
 * @brief Emitted when a new video track is received.
 * @ingroup events
 */
struct DLB_COMMS_MEDIA_EXPORT video_track_added {
  std::string peer_id;          /**< The ID of the participant to whom the video track
                                   belongs.  */
  std::string stream_id;        /**< The ID of the stream to which the video track belongs. */
  std::string track_id;         /**<  The ID of the video track. */
  std::string sdp_track_id;     /**< The id of the track in the SDP matching the
                                   sender side */
  bool is_screenshare = false;  /**< A boolean that indicates whether the video
                                   track is a screen share track. */
  bool capturer_status = false; /**< A boolean that indicates whether there is a
                                   sink attached to the video track. */
  bool remote;                  /**< Boolean indicating whether the track is from a remote
                                   participant.*/
};

/**
 * @brief Emitted when an audio track is removed and no longer received.
 * @ingroup events
 */
struct DLB_COMMS_MEDIA_EXPORT audio_track_removed {
  std::string peer_id;   /**< The ID of the participant to whom the audio track
                            belonged. */
  std::string stream_id; /**< The ID of the stream to which the audio track belonged. */
  std::string track_id;  /**< The ID of the video track. */
  bool remote;           /**< Boolean indicating whether the track is from a remote
                            participant.*/
};

/**
 * @brief Emitted when a new audio track is received.
 * @ingroup events
 */
struct DLB_COMMS_MEDIA_EXPORT audio_track_added {
  std::string peer_id;   /**< The ID of the participant to whom the audio track
                            belongs. */
  std::string stream_id; /**< The ID of the stream to which the audio track belongs. */
  std::string track_id;  /**< The ID of the audio track. */
  bool remote;           /**< Boolean indicating whether the track is from a remote
                            participant.*/
};

struct DLB_COMMS_MEDIA_EXPORT audio_level {
  std::string participant_id; /**< The ID of the participant to whom the talking
                          level corresponds. */
  float level{0.0};           /**< Talking level of participant values from 0.0 to 1.0. */
};

/**
 * @brief The platform agnostic description of an camera device.
 * @ingroup device_management
 */
struct DLB_COMMS_MEDIA_EXPORT camera_device {
  std::string display_name;
  std::string unique_id;
};

/**
 * @brief The platform agnostic description of an audio device.
 * @ingroup device_management
 */
class DLB_COMMS_MEDIA_EXPORT dvc_device {
 public:
  /**
   * @brief Enum describing the possible directions of audio devices.
   */
  enum direction {
    none = 0,             /**< No direction, this is default. */
    input = 1,            /**< The device is used for input (microphone). */
    output = 2,           /**< The device is used for output (speakers). */
    input_and_output = 3, /**< The device can be used for both input and output. */
  };

  /**
   * @brief Enum describing the current platform. This is necessary
   * because devices have different native handle formats for different
   * platforms.
   */
  enum class DLB_COMMS_MEDIA_EXPORT platform {
    macos,
    linux,
    windows,
  };

  /**
   * @brief Constructor for Unix based platforms.
   * @param uid Unique id of the device. This is is a binary string and not a
   * null terminated C string.
   * @param name String name of the device.
   * @param direction Direction of the device.
   * @param platform The current platform.
   * @param id Native handle of the device.
   */
  dvc_device(const std::string& uid, const std::string& name, direction direction, platform platform, unsigned int id);

  /**
   * @brief Constructor for Windows based platforms.
   * @param uid Unique id of the device. This is is a binary string and not a
   * null terminated C string.
   * @param name String name of the device.
   * @param direction Direction of the device.
   * @param platform The current platform.
   * @param id Native handle of the device.
   */
  dvc_device(const std::string& uid,
             const std::string& name,
             direction direction,
             platform platform,
             const std::string& id);

  /**
   * @brief Move constructor.
   * @param dev Object moved from.
   */
  dvc_device(dvc_device&& dev);

  /**
   * @brief Copy constructor
   * @param dev Object being copied from.
   */
  dvc_device(const dvc_device& dev);

  /**
   * @brief Destructor.
   */
  ~dvc_device();

  /**
   * @brief Copy assignment operator.
   * @param dev Object copied from.
   * @return New object that was copied.
   */
  const dvc_device& operator=(const dvc_device& dev);

  /**
   * @brief Gets the name of the audio device.
   * @return String containing the name.
   */
  const std::string& name() const;

  /**
   * @brief Gets the uid of the audio device.
   * @return Unique id of the device. This is is a binary string and not a
   * null terminated C string.
   */
  const std::string& uid() const;

  /**
   * @brief Gets the direction of the audio device.
   * @return The direction of the device.
   */
  direction direction() const;

  /**
   * @brief Gets the platform type of the device.
   * @return The platform type.
   */
  platform platform_type() const;

  /**
   * @brief Gets native id for MacOS platforms. This will
   * throw if called on non-MacOS platform.
   * @return Unsigned integer representing the native device id.
   */
  unsigned int macos_native_id() const;

  /**
   * @brief Gets native id for Linux platforms. This will
   * throw if called on non-Linux platform.
   * @return Unsigned integer representing the native device id.
   */
  unsigned int linux_native_id() const;

  /**
   * @brief Gets native id for Windows platforms. This will
   * throw if called on non-Windows platform.
   * @return String representing the native device id.
   */
  const std::string& windows_native_id() const;

  /**
   * @brief Compare operator for comparison against other dvc_devices's.
   * @param other The DVC device to be compared against
   * @return Boolean indicating the two are the same.
   */
  bool operator==(const dvc_device& other) const;

  /**
   * @brief Compare operator for comparison of unique id.
   * @param uid The unique id to be compared
   * @return Boolean indicating the device has this unique id.
   */
  bool operator==(const std::string& uid) const;

 private:
  std::string uid_;
  std::string name_;
  enum direction dir_;
  platform platform_;

  /// @cond DO_NOT_DOCUMENT
  union {
    unsigned int macos_id;
    unsigned int linux_id;
    std::string windows_id;
  };
  ///@endcond
};

/**
 * @brief Emitted when a new audio device is added to the system.
 * @ingroup events
 * @ingroup device_management
 */
struct DLB_COMMS_MEDIA_EXPORT audio_device_added {
  dvc_device device; /**< The device that was added. */
};

/**
 * @brief Emitted when an audio device is removed from the system.
 * @ingroup events
 * @ingroup device_management
 */
struct DLB_COMMS_MEDIA_EXPORT audio_device_removed {
  std::string uid; /**< Unique id belonging to the removed device. */
};

/**
 * @brief Emitted when the current audio device has changed.
 * @ingroup events
 * @ingroup device_management
 */
struct DLB_COMMS_MEDIA_EXPORT audio_device_changed {
  dvc_device device; /**< The new current device if no device is false. */
  bool no_device;    /**< Flag indicating whether there is a device in use for the
                       current direction. True means no device currently in use,
                       false means device available.*/
  /**
   *  @brief If the new device has the capability to be both an input and output device,
   *  this field indicates the direction (input or output) for which the device is being used.
   */
  enum dvc_device::direction utilized_direction;
};

/**
 * @brief Emitted when a new video device is added to the system.
 * @ingroup events
 * @ingroup device_management
 */
struct DLB_COMMS_MEDIA_EXPORT video_device_added {
  camera_device device; /**< The device that was added. */
};

/**
 * @brief Emitted when an video device is removed from the system.
 * @ingroup events
 * @ingroup device_management
 */
struct DLB_COMMS_MEDIA_EXPORT video_device_removed {
  std::string uid; /**< Unique id belonging to the removed device. */
};

/**
 * @brief Emitted a video device is in use.
 * @ingroup events
 * @ingroup device_management
 */
struct DLB_COMMS_MEDIA_EXPORT video_device_changed {
  camera_device device{}; /**< The device that is in use, or default-constructed for no device. */
};

/**
 * @brief Emitted when an error is encountered with the video device.
 * @ingroup events
 * @ingroup device_management
 */
struct DLB_COMMS_MEDIA_EXPORT video_device_error {
  /**
   * @brief Enum describing the types of video device errors.
   */
  enum class error_type {
    start_camera,  /**< An error occurred when trying to start camera. */
    camera_failure /**< An unrecoverable error occurred with the camera used for capturing. */
  };

  std::string uid;                 /**< Unique id belonging to the device. */
  std::string description;         /** Error describing the issue encountered with device. */
  std::string recovery_suggestion; /** Suggestion for possible remedy of the situation, may be empty. */
  error_type type;                 /**< Type of video device error. */
};

/**
 * @brief Emitted when the audio device fails continuously for a prolonged time.
 * @ingroup events
 * @ingroup device_management
 */
struct DLB_COMMS_MEDIA_EXPORT audio_device_timeout_failure {};

class video_frame_i420;
#ifdef __APPLE__
class video_frame_macos;
#endif  // Apple

/**
 * @brief The interface that wraps decoded video frames received from and to be
 * injected into WebRTC.
 * @ingroup media_record_api
 * @ingroup media_injector_api
 */
class DLB_COMMS_MEDIA_EXPORT video_frame {
 public:
  virtual ~video_frame() = default;

  /**
   * @brief Gets the width of the frame.
   * @return The width of the frame.
   */
  virtual int width() const = 0;

  /**
   * @brief Gets the height of the video frame.
   * @return The height of the frame.
   */
  virtual int height() const = 0;

  /**
   * @brief Gets the timestamp of the video frame if it was set.
   *
   * @attention On frames passed from the SDK this will be set to the time when the frame was captured. This will be in
   * sync with the timestamp of the captured audio frame corresponding to this video frame. If the application plans to
   * process the frame and then inject the processed frame back to the SDK, it should reuse the timestamp it receives
   * from the SDK to ensure proper AV synchronization on the receiving end.
   *
   * @return The System monotonic clock timestamp of the video frame in microseconds.
   */
  virtual int64_t timestamp_us() const = 0;

  /**
   * @brief Gets the I420 (YUV) data of the frame.
   *
   * @return the instance of the YUV interface to the data frame, or nullptr if the video frame is not in YUV format.
   */
  virtual video_frame_i420* get_i420_frame() = 0;

#if defined(__APPLE__) || defined(DOXYGEN)
  /**
   * @brief Gets the Texture data of the frame.
   *
   * @attention This is currently only available on MacOS.
   *
   * @return the instance of the MacOS video frame interface to the data frame, or nullptr if the video frame is not a
   * texture.
   */
  virtual video_frame_macos* get_native_frame() = 0;
#endif  // Apple
};

/**
 * @brief The interface for obtainig I420 (YUV) memory pointers and info for i420 frames.
 * @ingroup media_record_api
 * @ingroup media_injector_api
 */
class DLB_COMMS_MEDIA_EXPORT video_frame_i420 {
 public:
  /**
   * @brief Gets the Y component.
   * @return The pointer to the Y data buffer.
   */
  virtual const uint8_t* get_y() const = 0;

  /**
   * @brief Gets the U component.
   * @return The pointer to the U data buffer.
   */
  virtual const uint8_t* get_u() const = 0;

  /**
   * @brief Gets the V component for YUV.
   * @return The pointer to the V data buffer.
   */
  virtual const uint8_t* get_v() const = 0;

  /**
   * @brief Returns the Y component stride.
   * @return An integer representing the Y component stride.
   */
  virtual int stride_y() const = 0;

  /**
   * @brief Returns the U component stride.
   * @return An integer representing the U component stride.
   */
  virtual int stride_u() const = 0;

  /**
   * @brief Returns the V component stride.
   * @return An integer representing the V component stride.
   */
  virtual int stride_v() const = 0;
};

/**
 * @brief The interface that wraps encoded video frames received from the
 * conference.
 * @ingroup media_record_api
 */
class DLB_COMMS_MEDIA_EXPORT encoded_video_frame {
 public:
  virtual ~encoded_video_frame() = default;

  /**
   * @brief Returns the binary data containing the frame payload.
   * @return The non-null pointer to the payload data.
   */
  virtual const uint8_t* data() const = 0;

  /**
   * @brief Returns the size of the payload.
   * @return The size of the payload in bytes.
   */
  virtual size_t size() const = 0;
  /**
   * @brief Gets the width of the frame.
   * @return The width of the frame.
   */
  virtual int width() const = 0;

  /**
   * @brief Gets the height of the video frame.
   * @return The height of the frame.
   */
  virtual int height() const = 0;

  /**
   * @brief Checks if the frame is a key frame.
   * @return true if keyframe, false otherwise.
   */
  virtual bool is_keyframe() const = 0;
};

/**
 * @brief Interface that wraps decoded audio frames to be injected into WebRTC.
 * @ingroup media_injector_api
 */
class DLB_COMMS_MEDIA_EXPORT audio_frame {
 public:
  /**
   * @brief Default destructor.
   */
  virtual ~audio_frame() = default;

  /**
   * @brief Gets the underlying s16 raw PCM audio data.
   * @return Pointer to data.
   */
  virtual const int16_t* data() const = 0;

  /**
   * @brief Gets the sample rate of the audio frame.
   * @return Sample rate.
   */
  virtual int sample_rate() const = 0;

  /**
   * @brief Gets the number of channels in the audio frame.
   * @return Channels.
   */
  virtual int channels() const = 0;

  /**
   * @brief Gets the number of sample in the audio frame.
   * @return Samples.
   */
  virtual int samples() const = 0;
};

/**
 * @brief The interface for receiving the raw video frames (YUV bitmaps, or platform-specific format).
 * @ingroup media_record_api
 */
class DLB_COMMS_MEDIA_EXPORT video_sink {
 public:
  /**
   * @brief The callback that is invoked when a video frame is decoded and ready
   * to be processed.
   * @param stream_id The ID of the media stream to which the video track belongs. In the event of a local camera
   * camera stream, this string may be empty.
   * @param track_id The ID of the video track to which the frame belongs. In the event of a local camera
   * camera stream, this string may be empty.
   * @param frame The pointer to the video frame.
   */
  virtual void handle_frame(const std::string& stream_id,
                            const std::string& track_id,
                            std::unique_ptr<video_frame> frame) = 0;
};

/**
 * @brief The interface for providing video frames.
 *
 * This interface must be implemented by the injector, it shall serve as
 * the source of video frames passed to the rtc_video_source.
 *
 * @ingroup media_injector_api
 */
class DLB_COMMS_MEDIA_EXPORT video_source {
 public:
  /**
   * @brief The video configuration wanted by the WebRTC track.
   *
   * The video_source is free to ignore parts of the configuration, or the whole configuration. The video coding will be
   * most efficient if the configuration is respected though.
   */
  struct DLB_COMMS_MEDIA_EXPORT config {
    bool rotation_applied = false;                          ///< Experimental configuration.
    bool black_frames = false;                              ///< True if the frames should be black.
    int max_pixel_count = std::numeric_limits<int>::max();  ///< The maximum number of pixels in each frame.
    int target_pixel_count = -1;  ///< The desired number of pixels in each frame. -1 means no preference, but the
                                  ///< source should attempt to fit below max_pixel_count.
    int max_framerate_fps = std::numeric_limits<int>::max();  ///< The maximum framerate.
  };
  /**
   * @brief Sets the video sink on the video source.
   *
   * This method is invoked when the video pipeline is ready to accept video frames from the source. After this method
   * is invoked with non-null sink, the source can start delivering frames on any thread. This method may be invoked
   * multiple times with the same, or changing sink. The source implementation should ensure, if the new sink pointer is
   * different than the previous one, that after this method returns, the previously used sink will not receive any more
   * frames (should sync with the thread which delivers frames). When this method is called with the null sink, the
   * source should stop producing video frames at all.
   *
   * @param sink The sink which will receive the injected video frames.
   * @param config the suggested config for the video properties.
   */
  virtual void set_sink(video_sink* sink, const config& config) = 0;
};

/**
 * @brief The interface for receiving the encoded video frames.
 * @ingroup media_record_api
 */
class DLB_COMMS_MEDIA_EXPORT video_sink_encoded {
 public:
  /**
   * @brief The desired configuration of the decoder passing frames to this
   * encoded sink.
   */
  enum class decoder_config {
    full_decoding,      /**< The decoder will decode every single frame. In this case applications
                             can connect both a video_sink and a video_sink_encoded to recieve video
                             streams. This allows them to both dump video to file and render it on
                             screen for instance. */
    optimized_decoding, /**< The decoder will only decode the first frame. In this case applications
                             will not receive any decoded video frames if they set a video_sink. This
                             decoder configuration greatly optimized CPU load by not decoding every frame
                             but is useful for applications which only want to dump encoded video to file.
                        */
  };

  /**
   * @brief The callback that is invoked when a new video track is available.
   * The callback allows setting the corresponding codec for the video track.
   * @param codec The codec of the video track.
   * @param track_id The ID of the video track.
   *
   * @retval true The sink would like to use optimized decoding, meaning that
   * all incoming video frames are discarded and not decoded after being passed
   * to the handle_frame_encoded function.
   * @retval false The sink would like to use full decoding, meaning that all
   * the incoming frames are decoded.
   */
  virtual decoder_config configure_encoded_sink(const std::string& codec, const std::string& track_id) = 0;

  /**
   * @brief The callback that is invoked to check the set decoder configuration.
   * This allows the SDK to check if optimized decoding has been configured, if
   * optimized decoding is set then the decoder will only decode the first frame.
   * This means that applications can only connect an encoded video sink. When using
   * full_decoding an application can connect a video_sink and an video_sink_encoded so it
   * can render video frames and also dump them to file for instance.
   *
   * @return The desired decoder configuration for the Encoded Video Sink.
   */
  virtual decoder_config decoder_configuration() const = 0;

  /**
   * @brief The callback that is invoked when a new encoded video frame is ready
   * to be processed.
   * @param track_id The ID of the video track.
   * @param frame The encoded video frame.
   */
  virtual void handle_frame_encoded(const std::string& track_id, std::unique_ptr<encoded_video_frame> frame) = 0;
};

/**
 * @brief The interface for receiving audio frames.
 * @ingroup media_record_api
 */
class DLB_COMMS_MEDIA_EXPORT audio_sink {
 public:
  /**
   * @brief The callback that is invoked when an audio frame is decoded and
   * ready to be processed.
   * @param stream_id The media stream ID to which the audio track belongs.
   * @param track_id The ID of the audio track.
   * @param data The pointer to the underlying PCM data.
   * @param n_data The size of data.
   * @param sample_rate The sample rate
   * @param channels The number of channels.
   */
  virtual void handle_audio(const std::string& stream_id,
                            const std::string& track_id,
                            const int16_t* data,
                            size_t n_data,
                            int sample_rate,
                            size_t channels) = 0;
};

/**
 * @brief The interface that is a sink for the Conference Service,
 * where audio and video data for a specific conference are stored.
 *
 * @ingroup media_record_api
 */
class DLB_COMMS_MEDIA_EXPORT media_sink_interface {
 public:
  /**
   * @brief Gets the raw audio sink interface. Override this method to provide
   * access to the Audio Sink implementation.
   *
   * @note Recording the audio in the conference is a server-side use-case.
   * Using a media_sink_interface which returns a non-null audio_sink has severe
   * implications for the media pipeline: the DVC codec is disabled, and the
   * DVC-enabled conferences will use Opus; a fake audio device is used for
   * handling conference audio - the real speakers and microphone will not be
   * used. It is not possible to record the conference audio and have the
   * input provided by the real microphone.
   *
   * @return The pointer to the audio sink.
   */
  virtual audio_sink* audio() = 0;

  /**
   * @brief Gets the encoded video sink interface. Override this method to
   * provide access to the Encoded Video Sink implementation.
   * @return The pointer to the encoded video sink.
   */
  virtual video_sink_encoded* video_enc() = 0;
};

/**
 * @brief The adapter which is used for providing Audio frames into WebRTC.
 * This interface is an Audio Sink from the perspective of the Injector.
 * It is an Audio Source from the perspective of WebRTC Audio Tracks, thus it
 * provides this connection in establishing the audio injection pipeline.
 *
 * This interface is NOT implemented by the injector, it is used to be the
 * injector to provide audio frames.
 *
 * @ingroup media_injector_api
 */
class DLB_COMMS_MEDIA_EXPORT rtc_audio_source {
 public:
  /**
   * @brief The callback that is invoked when 10ms of audio data is ready
   * to be passed to WebRTC.
   * @param audio_data The pointer to the PCM data
   * @param bits_per_sample Bits per sample.
   * @param sample_rate The sample rate of the audio.
   * @param number_of_channels The number of channels.
   * @param number_of_frames The total number of samples (channels *
   * sample_rate/100)
   *
   */
  virtual void on_data(const void* audio_data,
                       int bits_per_sample,
                       int sample_rate,
                       size_t number_of_channels,
                       size_t number_of_frames) = 0;
};

/**
 * @brief The interface for providing audio frames.
 *
 * This interface must be implemented by the injector, it shall serve as
 * the source of audio frames passed to the rtc_audio_source.
 *
 * @ingroup media_injector_api
 */
class DLB_COMMS_MEDIA_EXPORT audio_source {
 public:
  /**
   * @brief Connects the RTC Audio Source to the audio source, in essence
   * creating the audio injection pipeline. This method will be called by the
   * media_engine when an Audio Track is attached to the active Peer Connection.
   *
   * @param source The RTC Audio Source which will receive the injected audio
   * frames.
   */
  virtual void register_audio_frame_rtc_source(rtc_audio_source* source) = 0;

  /**
   * @brief Disconnects the RTC Audio Source from the Audio Source, in essence
   * destructing the audio pipeline. This method is called by the media_engine
   * whenever an Audio Track is to be detached from the active Peer Connection.
   *
   */
  virtual void deregister_audio_frame_rtc_source() = 0;
};

/**
 * @brief The interface that acts as a media source for the Conference Service.
 *
 * @ingroup media_injector_api
 */
class DLB_COMMS_MEDIA_EXPORT media_source_interface {
 public:
  /**
   * @brief Gets the audio source interface. Override this method to provide
   * access to the Audio Source implementation.
   *
   * @note Injecting the audio into conference is a server-side use-case. Using
   * a media_source_interface which returns a non-null audio_source has severe
   * implications for the media pipeline: the DVC codec is disabled, and the
   * DVC-enabled conferences will use Opus; a fake audio device is used for
   * handling conference audio - the real speakers and microphone will not be
   * used. It is not possible to inject audio into the conference and have the
   * output audio rendered to speakers.
   *
   * @return The pointer to the audio source.
   */
  virtual audio_source* audio() = 0;
};

/**
 * @brief The video frame handler for local video streams.
 *
 * The application can set the video frame handler when starting a local camera stream. The frame handler can be used to
 * capture the camera frames for local camera preview, and for delivering modified video frames back into the video
 * pipeline for encoding.
 *
 * There are four use-cases supported by the video_frame_handler:
 *
 * 1. No-op frame handler: the camera frames are not delivered to the application, and are being encoded by the video
 * pipeline and sent into conference. The frame handler may return null sink and source, or the frame handler pointer
 * passed to the media pipeline can be just null.
 *
 * 2. The local preview: the frame handler returns non-null sink, but a null source. The video frames captured from the
 * camera are passed both to the conference's video track, and to the frame handler sink.
 *
 * 3. Video processing: the frame handler returns non-null sink and source. The camera frames are passed to the frame
 * handler sink only. When the conference's video track starts sending data, it will connect the frame handler source to
 * the internal sink. The application is supposed to deliver the video frames, but it's not required to be synchronous
 * with frames delivered to the frame handler sink. The application can deliver frames on any thread.
 *
 * 4. Video injection: the frame handler returns null sink, and non-null source. In this scenario, the real camera is
 * not used at all. The application should deliver frames produced externally through the frame handler source
 * interface.
 *
 * In the local preview and video processing scenarios, the camera is open all the time, regardless of the video track
 * state in the conference. The local preview can be displayed even before joining the conference, and will remain open
 * after the conference is left. In the video injection scenario, the camera is not open at all. When a no-op frame
 * handler is used, the conference's video track presence enables the camera.
 */
class DLB_COMMS_MEDIA_EXPORT video_frame_handler {
 public:
  /**
   * @brief Get the frame handler's video sink.
   *
   * If the frame handler wishes to get raw video frames in the stream it's attached to, this method should return the
   * proper video sink pointer.
   *
   * @return a video sink pointer, or nullptr.
   */
  virtual video_sink* sink() = 0;

  /**
   * @brief Get the frame handler's video source.
   *
   * If the frame handler wishes to forward the processed frames down the pipeline, it should return non-null source.
   *
   * @return a video source pointer, or nullptr.
   */
  virtual video_source* source() = 0;
};

}  // namespace comms
}  // namespace dolbyio
