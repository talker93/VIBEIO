/***************************************************************************
 * This program is licensed by the accompanying "license" file. This file is
 * distributed "AS IS" AND WITHOUT WARRANTY OF ANY KIND WHATSOEVER, either
 * express or implied. See the License for the specific language governing
 * permissions and limitations under the License.
 *
 *                Copyright (C) 2022 by Dolby Laboratories.
 ***************************************************************************/

#include "custom_video_processor.h"
#include <dolbyio/comms/media_engine/video_frame_macos.h>
#include <dolbyio/comms/media_engine/video_utils.h>

#if defined(__APPLE__)
#include <CoreVideo/CoreVideo.h>
#endif

using video_format_converter = dolbyio::comms::video_utils::format_converter;

custom_video_processor::custom_video_processor() : frame_queue_thread_([this]() { frame_queue_loop(); }) {}

custom_video_processor::~custom_video_processor() {
  {
    std::lock_guard<std::mutex> lock(frame_queue_lock_);
    frame_queue_.clear();
    exit_ = true;
  }
  frame_queue_cond_.notify_one();
  frame_queue_thread_.join();
}

// Receive frame from the SDK and place onto the frame queue. In a proper implementation
// this frame queue should have some size limitation.
void custom_video_processor::handle_frame(const std::string&,
                                          const std::string&,
                                          std::unique_ptr<dolbyio::comms::video_frame> frame) {
  {
    std::lock_guard<std::mutex> lock(frame_queue_lock_);
    frame_queue_.push_back(std::move(frame));
  }
  frame_queue_cond_.notify_one();
}

// Called by the SDK after application has called dolbyio::comms::services::local_video::start
// and passed in the custom processor. The SDK will provide the processor a sink where it can
// further inject frames.
void custom_video_processor::set_sink(video_sink* sink, const config&) {
  {
    std::lock_guard<std::mutex> lock(sink_lock_);
    sdk_video_sink_ = sink;
  }
  frame_queue_cond_.notify_one();
}

void custom_video_processor::frame_queue_loop() {
  std::vector<std::unique_ptr<dolbyio::comms::video_frame>> frame_queue;
  while (true) {
    {
      // While holding the lock check if there are any frames added to the queue
      std::unique_lock<std::mutex> lock(frame_queue_lock_);
      if (frame_queue_.empty() && !exit_) {
        frame_queue_cond_.wait(lock, [this]() { return !frame_queue_.empty() || exit_; });
      }
      if (exit_)
        break;

      // Move the current frames to our temporary queue and unlock
      frame_queue = std::move(frame_queue_);
    }

    // Process the current frames in place and then inject them back to the SDK
    for (std::size_t i = 0; i < frame_queue.size(); ++i) {
      std::unique_ptr<dolbyio::comms::video_frame> frame = std::move(frame_queue[i]);

      // On MacOS you should really be doing processing of copied frames. Otherwise
      // you may affect the frames other applications are capturing from the same
      // video device because the textures the camera driver writes too are shared
      // by application using this device.
      process_frame_in_place(*frame);

      // If a SDK video sink exists inject back processed frame into the SDK
      std::lock_guard<std::mutex> lock(sink_lock_);
      if (sdk_video_sink_)
        sdk_video_sink_->handle_frame("", "", std::move(frame));
    }
    frame_queue.clear();
  }
}

// This is just a sample, itchanges the UV values to 0 for the captured camera frames.
void custom_video_processor::process_frame_in_place(dolbyio::comms::video_frame& frame) {
  if (frame.get_i420_frame()) {
    auto* yuv_frame = frame.get_i420_frame();
    auto* u_buffer = const_cast<uint8_t*>(yuv_frame->get_u());
    auto* v_buffer = const_cast<uint8_t*>(yuv_frame->get_v());

    // Zero out the UV pixels, causing green tint on the image.
    video_format_converter::set_plane_buffer_value(u_buffer, yuv_frame->stride_u(), yuv_frame->stride_u(),
                                                   frame.height() / 2, 0);
    video_format_converter::set_plane_buffer_value(v_buffer, yuv_frame->stride_v(), yuv_frame->stride_v(),
                                                   frame.height() / 2, 0);
  } else {
#if defined(__APPLE__)
    if (frame.get_native_frame()) {
      // Note that changing pixels without copying the underlying CVPixelBuffer will change the
      // pixels in memory of the original frame, which happens to be shared with all processes
      // reading from the camera. So other applications using the same camera device will be affected.
      // To do this properly one should make a copy of the CVPixelBuffer.
      auto buf = frame.get_native_frame()->get_buffer();
      CVPixelBufferLockBaseAddress(buf, 0);

      // Sanity check for ensuring we are capturing NV12 from camera
      auto format_type = CVPixelBufferGetPixelFormatType(buf);
      if (format_type != kCVPixelFormatType_420YpCbCr8BiPlanarVideoRange &&
          format_type != kCVPixelFormatType_420YpCbCr8BiPlanarFullRange)
        return;

      // Get the Y and UV planes of the CVPixelBuffer
      auto* uv_plane = static_cast<uint8_t*>(CVPixelBufferGetBaseAddressOfPlane(buf, 1));
      auto bytes_uv_plane_row = CVPixelBufferGetBytesPerRowOfPlane(buf, 1);
      auto stride_uv = bytes_uv_plane_row / 2;
      auto height_uv_plane = CVPixelBufferGetHeightOfPlane(buf, 1);

      // Convert the NV12 frames to i420 by splitting the UV plane buffer
      video_format_converter::split_uv_planes(uv_plane, bytes_uv_plane_row, pixel_conversion_buffers_[0], stride_uv,
                                              pixel_conversion_buffers_[1], stride_uv, stride_uv, height_uv_plane);

      // Zero out all the pixels in the U and V plane buffers
      for (uint8_t i = 0; i < (sizeof(pixel_conversion_buffers_) / sizeof(pixel_conversion_buffers_[0])); ++i)
        video_format_converter::set_plane_buffer_value(pixel_conversion_buffers_[i], stride_uv, stride_uv,
                                                       height_uv_plane, 0);

      // Convert back to NV12 frame by merging the U and V plane buffers
      video_format_converter::merge_uv_plane(pixel_conversion_buffers_[0], stride_uv, pixel_conversion_buffers_[1],
                                             stride_uv, uv_plane, bytes_uv_plane_row, stride_uv, height_uv_plane);
      CVPixelBufferUnlockBaseAddress(buf, 0);
    }
#endif
  }
}
