#pragma once

/***************************************************************************
 * This program is licensed by the accompanying "license" file. This file is
 * distributed "AS IS" AND WITHOUT WARRANTY OF ANY KIND WHATSOEVER, either
 * express or implied. See the License for the specific language governing
 * permissions and limitations under the License.
 *
 *                Copyright (C) 2021-2022 by Dolby Laboratories.
 ***************************************************************************/

#include <dolbyio/comms/exports.h>
#include <dolbyio/comms/media_engine/media_engine.h>
#include <dolbyio/comms/sdk.h>

#include <memory>

namespace dolbyio::comms::plugin {

/**
 * @defgroup default_recorder Default Media Recorder Plugin
 * The Default Media Recorder plugin is a module that allows capturing remote
 * audio and video data and storing them on a hard drive.
 */

/**
 * @brief The interface for the Default Media Recorder.
 * @ingroup default_recorder
 */
class DLB_COMMS_ADDON_EXPORT recorder : public dolbyio::comms::audio_sink,
                                        public dolbyio::comms::video_sink,
                                        public dolbyio::comms::video_sink_encoded {
 public:
  /**
   * @brief The available formats for capturing audio.
   */
  enum class audio_recording_config {
    NONE, /**< Disables audio capturing. */
    PCM,  /**< Enables capturing audio and storing the audio in the PCM format.
           */
    AAC,  /**< Enables capturing audio and storing the audio in the AAC format.
           */
  };

  /**
   * @brief The available configurations for capturing and storing video. The
   * captured video frames are stored in corresponding containers. The encoded
   * video is not transcoded during or after the recording, so the corresponding
   * container depends on what is negotiated in the SDP. Video encoded via the
   * H264 codec is stored in the MPEG-TS container and video encoded via VP8 is
   * stored in Matroska.
   *
   * @note If the configuration is set to ENCODED_OPTIMIZED, then the video frames are not decoded
   * after they are dumped. In this case if you call the dolbyio::comms::services::remote_video service::set_video_sink
   * method, the frames will never reach the set sink. If you would like to get decoded frames as well as dump encoded
   * frames to a file make sure to set the configuration to ENCODED format.
   */
  enum class video_recording_config {
    NONE,             /**< Disables video capturing. */
    YUV,              /**< Enables capturing of raw video frames that are stored in the
                         Matroska container. */
    ENCODED,          /**< Enables capturing of the video frames that are encoded via the
                         VP8 or H264 codec. */
    ENCODED_OPTIMIZED /**< Enables capturing of the video frames that are
                         encoded via the VP8 or H264 codec. Only the first frame of the
                         incoming video track will be decoded. */
  };

  /**
   * @brief Constructor taking the audio and video recording configurations.
   * @param sdk The reference to the SDK object.
   * @param audio Audio recording configuration.
   * @param video Video recording configuration.
   */
  recorder(dolbyio::comms::sdk& sdk, audio_recording_config audio, video_recording_config video);

  /**
   * @brief The destructor of the recorder.
   */
  virtual ~recorder();

  /**
   * @brief Creates the media recording module.
   * @param out_dir The location for storing the captured audio and video.
   * @param sdk The reference to the SDK object.
   * @param audio The audio configuration.
   * @param video The video configuration.
   * @return The pointer to the recorder module object.
   *
   * @exception dolbyio::comms::exception Occurs when a metadata file cannot
   * be created.
   * @exception dolbyio::comms::exception Occurs when an event handler cannot be
   * connected.
   * @exception std::exception Occurs in the case of a file system issue.
   *
   * @code
   * // When an output directory and the sdk instance are created
   * auto recorder = dolbyio::comms::plugin::recorder::create(outdir, sdk, audio_cfg, video_cfg);
   * @endcode
   */
  static std::unique_ptr<recorder> create(const std::string& out_dir,
                                          dolbyio::comms::sdk& sdk,
                                          audio_recording_config audio,
                                          video_recording_config video);

  decoder_config decoder_configuration() const override;

 protected:
  /**
   * @brief The reference to the sdk.
   */
  dolbyio::comms::sdk& sdk_;

  /**
   * @brief Video configuration set for the recorder.
   */
  video_recording_config video_config_;

  /**
   * @brief Audio configuration set for the recorder.
   */
  audio_recording_config audio_config_;
};

}  // namespace dolbyio::comms::plugin
