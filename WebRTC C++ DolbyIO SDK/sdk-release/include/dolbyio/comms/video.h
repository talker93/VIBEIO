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
#include <dolbyio/comms/media_engine/media_engine.h>

#include <variant>

namespace dolbyio::comms::services {

/**
 * @brief The local video service.
 *
 * This service is used to control local participant's video capture
 * and sending into conference.
 *
 * @attention  The local video interface contains methods that return
 * #async_result. Each function that returns #async_result is asynchronous and
 * the operation is executed during the SDK event loop. The caller can block the
 * calling thread until the operation completes using the #wait helper. The
 * caller can also chain consecutive operations, which are dependent on the
 * completion of this method, using the async_result::then calls. Each
 * async_result chain needs to be terminated with an async_result::on_error.
 *
 */
class DLB_COMMS_EXPORT local_video {
 public:
  /**
   * @brief Starts local video capture.
   *
   * This method may be called at any time, regardless of the conference state. If this method is invoked when there's
   * no active conference, it will still select the camera device and set the video frame handler. If the video frame
   * handler returns a non-null video sink, camera will start delivering frames to the sink.
   *
   * This method can also be used to switch cameras at any point. If you have passed in a video_frame_handler to the
   * previous start call and would like to continue using this handler, you must pass the same handler into the
   * subsequent call used to switch cameras. This will have the effect of just switching cameras, keeping the rest of
   * the pipeline in tact.
   *
   * The ownership of the frame handler remains with the application. The application must not delete the handler, its
   * sink and source, until it invokes the stop() method and the stop() method execution is finished.
   *
   * If the application uses a default-constructed camera_device, then a first camera found in the system will be used.
   *
   * If this method returns an error, the provided frame handler can be safely deleted by the application.
   *
   * If the application starts the video while not in the conference, and later joins the conference, the conference's
   * local video state is determined by the media_constraints passed to the conference::join() method. It is possible to
   * start local camera preview, but join the conference without video; in order to enable video later in the
   * conference, the start() method should be used again. It is not possible to disable sending video into the
   * conference but keep the local camera preview once the conference started video.
   *
   * @param device Camera device to start capturing from.
   * @param handler the camera stream's video frame handler.
   *
   * @returns async operation result.
   */
  virtual async_result<void> start(const camera_device& device = {}, video_frame_handler* handler = nullptr) = 0;

  /**
   * @brief Stops local video capture.
   *
   * @returns The result object producing the operation status asynchronously.
   */
  virtual async_result<void> stop() = 0;
};

/**
 * @brief The remote video service.
 *
 * @attention  The remote video interface contains methods that return
 * #async_result. Each function that returns #async_result is asynchronous and
 * the operation is executed during the SDK event loop. The caller can block the
 * calling thread until the operation completes using the #wait helper. The
 * caller can also chain consecutive operations, which are dependent on the
 * completion of this method, using the async_result::then calls. Each
 * async_result chain needs to be terminated with an async_result::on_error.
 *
 */
class DLB_COMMS_EXPORT remote_video {
 public:
  /**
   * @brief Sets the video sink to be used by all conferences.
   *
   * The video sink passed to this method will be used for passing the decoded video frames to the application. The
   * ownership of the sink remains with the application, and the SDK will not delete it. The application should set null
   * sink and ensure that the set_video_sink() call returned before deleting the previously set sink object.
   *
   * @param sink the video sink or nullptr.
   * @return the result object producing the operation status asynchronously.
   */
  virtual async_result<void> set_video_sink(video_sink* sink) = 0;
};

/**
 * @brief The video service.
 */
class DLB_COMMS_EXPORT video {
 public:
  /**
   * @brief Gets the local video service instance.
   * @return the local video service.
   */
  virtual local_video& local() = 0;

  /**
   * @brief Gets the remote video service instance.
   * @return the remove video service.
   */
  virtual remote_video& remote() = 0;
};

}  // namespace dolbyio::comms::services
