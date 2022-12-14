#pragma once

#if defined(__APPLE__) || defined(DOXYGEN)

#include <dolbyio/comms/media_engine/media_engine.h>

#import <CoreVideo/CoreVideo.h>

namespace dolbyio {
namespace comms {
/**
 * @brief MacOS Video Frames containing Texture data.
 */
class DLB_COMMS_MEDIA_EXPORT video_frame_macos {
 public:
  /**
   * @brief Gets the underlying <a
   * href="https://developer.apple.com/documentation/corevideo/cvpixelbufferref">CVPixelBUfferRef</a> .
   * @return Reference to the underling CVPixerlBuffer.
   */
  virtual CVPixelBufferRef get_buffer() = 0;
};
}  // namespace comms
}  // namespace dolbyio

#endif  // Apple
