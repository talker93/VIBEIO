#pragma once

/***************************************************************************
 * This program is licensed by the accompanying "license" file. This file is
 * distributed "AS IS" AND WITHOUT WARRANTY OF ANY KIND WHATSOEVER, either
 * express or implied. See the License for the specific language governing
 * permissions and limitations under the License.
 *
 *                Copyright (C) 2022 by Dolby Laboratories.
 ***************************************************************************/

#include <dolbyio/comms/exception.h>
#include <dolbyio/comms/exports.h>
#include <dolbyio/comms/log_level.h>

#include <dolbyio/comms/media_engine/media_exceptions.h>

#include <memory>
#include <string>

namespace dolbyio {
namespace comms {
namespace video_utils {

/**
 * @brief Class which can be used as helper convert various frame formats.
 * For now it supports to/from NV12, i420, RGB as well as helper for merging/splitting
 * UV planes.
 */
class DLB_COMMS_MEDIA_EXPORT format_converter {
 public:
  /**
   * @brief Convert from NV12 to i420 video format.
   * @param src_y Source Y Plane Buffer.
   * @param src_stride_y Source Y Plane Stride.
   * @param src_vu Source UV Plane Buffer.
   * @param src_stride_vu Source UV Plane Stride.
   * @param dst_y Destination Y plane buffer.
   * @param dst_stride_y Destination Y plane stride.
   * @param dst_u Destination U plane buffer.
   * @param dst_stride_u Destination U plane stride.
   * @param dst_v Destination V plane buffer.
   * @param dst_stride_v Destination V plane stride.
   * @param width Frame width.
   * @param height Frame height.
   * @retval 0 On success.
   * @retval 1 On failure.
   */
  static int nv12_to_i420(const uint8_t* src_y,
                          int src_stride_y,
                          const uint8_t* src_vu,
                          int src_stride_vu,
                          uint8_t* dst_y,
                          int dst_stride_y,
                          uint8_t* dst_u,
                          int dst_stride_u,
                          uint8_t* dst_v,
                          int dst_stride_v,
                          int width,
                          int height);

  /**
   * @brief Split the UV plane buffers into respective U and V buffers.
   * This can be used to go from NV12 to i420 if you want to keep the
   * same Y buffer and just get separated U and V buffers.
   *
   * @warning Caller must ensure that height * width does not exceed buffer size.
   *
   * @param src_vu Source UV Plane Buffer.
   * @param src_stride_vu Source UV Plane Stride.
   * @param dst_u Destination U plane buffer.
   * @param dst_stride_u Destination U plane stride.
   * @param dst_v Destination V plane buffer.
   * @param dst_stride_v Destination V plane stride.
   * @param width Width of the plane the plane buffer (not frame width).
   * @param height Height of the plane buffer (not frame width).
   */
  static void split_uv_planes(const uint8_t* src_vu,
                              int src_stride_vu,
                              uint8_t* dst_u,
                              int dst_stride_u,
                              uint8_t* dst_v,
                              int dst_stride_v,
                              int width,
                              int height);

  /**
   * @brief Convert i420 to NV12 video format.
   * @param src_y Source Y Plane Buffer.
   * @param src_stride_y Source Y Plane Stride.
   * @param src_u Source U Plane Buffer.
   * @param src_stride_u Source U Plane Stride.
   * @param src_v Source V Plane Buffer.
   * @param src_stride_v Source V Plane Stride.
   * @param dst_y Destination Y plane buffer.
   * @param dst_stride_y Destination Y plane stride.
   * @param dst_uv Destination UV plane buffer.
   * @param dst_stride_uv Destination UV plane stride.
   * @param width Frame width.
   * @param height Frame height.
   * @retval 0 On success.
   * @retval 1 On failure.
   */
  static int i420_to_nv12(const uint8_t* src_y,
                          int src_stride_y,
                          const uint8_t* src_u,
                          int src_stride_u,
                          const uint8_t* src_v,
                          int src_stride_v,
                          uint8_t* dst_y,
                          int dst_stride_y,
                          uint8_t* dst_uv,
                          int dst_stride_uv,
                          int width,
                          int height);

  /**
   * @brief Merge the U and V plane buffers into a single UV plane buffer.
   * This can be used to convert from i420 to NV12, if you want to keep
   * using the same Y buffer qnd just get merged UV buffer.
   *
   * @warning Caller must ensure that height * width does not exceed buffer size.
   *
   * @param src_u Source U Plane Buffer.
   * @param src_stride_u Source U Plane Stride.
   * @param src_v Source V Plane Buffer.
   * @param src_stride_v Source V Plane Stride.
   * @param dst_uv Destination UV plane buffer.
   * @param dst_stride_uv Destination UV plane stride.
   * @param width Width of the plane the plane buffer (not frame width).
   * @param height Height of the plane buffer (not frame width).
   */
  static void merge_uv_plane(const uint8_t* src_u,
                             int src_stride_u,
                             const uint8_t* src_v,
                             int src_stride_v,
                             uint8_t* dst_uv,
                             int dst_stride_uv,
                             int width,
                             int height);

  /**
   * @brief Convert i420 to ARGB video format.
   * @param src_y Source Y Plane Buffer.
   * @param src_stride_y Source Y Plane Stride.
   * @param src_u Source U Plane Buffer.
   * @param src_stride_u U Plane Stride.
   * @param src_v Source V Plane Buffer.
   * @param src_stride_v Source V Plane Stride.
   * @param dst_argb Destination ARGB buffer.
   * @param dst_stride_argb Destination ARGB stride.
   * @param width Frame width.
   * @param height Frame height.
   * @retval 0 On success.
   * @retval 1 On failure.
   */
  static int i420_to_argb(const uint8_t* src_y,
                          int src_stride_y,
                          const uint8_t* src_u,
                          int src_stride_u,
                          const uint8_t* src_v,
                          int src_stride_v,
                          uint8_t* dst_argb,
                          int dst_stride_argb,
                          int width,
                          int height);

  /**
   * @brief Convert ARGB to i420 video format.
   * @param src_argb Source ARGB buffer.
   * @param src_stride_argb Source ARGB stride.
   * @param dst_y Destination Y Plane Buffer.
   * @param dst_stride_y Destination Y Plane Stride.
   * @param dst_u Destination U Plane Buffer.
   * @param dst_stride_u Destination U Plane Stride.
   * @param dst_v Destination V Plane Buffer.
   * @param dst_stride_v Destination V Plane Stride.
   * @param width Frame width.
   * @param height Frame height.
   * @retval 0 On success.
   * @retval 1 On failure.
   */
  static int argb_to_i420(const uint8_t* src_argb,
                          int src_stride_argb,
                          uint8_t* dst_y,
                          int dst_stride_y,
                          uint8_t* dst_u,
                          int dst_stride_u,
                          uint8_t* dst_v,
                          int dst_stride_v,
                          int width,
                          int height);

  /**
   * @brief Convert ARGB to NV12 video format.
   * @param src_argb Source ARGB buffer.
   * @param src_stride_argb Source ARGB stride.
   * @param dst_y Destination Y plane buffer.
   * @param dst_stride_y Destination Y plane stride.
   * @param dst_uv Destination UV plane buffer.
   * @param dst_stride_uv Destination UV plane stride.
   * @param width Frame width.
   * @param height Frame height.
   * @retval 0 On success.
   * @retval 1 On failure.
   */
  static int argb_to_nv12(const uint8_t* src_argb,
                          int src_stride_argb,
                          uint8_t* dst_y,
                          int dst_stride_y,
                          uint8_t* dst_uv,
                          int dst_stride_uv,
                          int width,
                          int height);

  /**
   * @brief Convert NV12 to ARGB video format.
   * @param src_y Source Y plane buffer.
   * @param src_stride_y Source Y plane stride.
   * @param src_vu Source UV plane buffer.
   * @param src_stride_vu Source UV plane stride.
   * @param dst_argb Source ARGB buffer.
   * @param dst_stride_argb Source ARGB stride.
   * @param width Frame width.
   * @param height Frame height.
   * @retval 0 On success.
   * @retval 1 On failure.
   */
  static int nv12_to_argb(const uint8_t* src_y,
                          int src_stride_y,
                          const uint8_t* src_vu,
                          int src_stride_vu,
                          uint8_t* dst_argb,
                          int dst_stride_argb,
                          int width,
                          int height);

  /**
   * @brief Sets a plane buffer to specified 32bit value. This
   * can be used to zero-out a plane for instance.
   *
   * @warning Caller must ensure that height * width does not exceed buffer size.
   *
   * @param dst Destination plane buffer.
   * @param dst_stride Destination plane stride;
   * @param width Width of buffer the plane in bytes.
   * @param height Height of the plane buffer in bytes.
   * @param value Value to be set.
   */
  static void set_plane_buffer_value(uint8_t* dst, int dst_stride, int width, int height, uint32_t value);
};

}  // namespace video_utils
}  // namespace comms
}  // namespace dolbyio
