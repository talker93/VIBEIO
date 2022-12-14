/***************************************************************************
 * This program is licensed by the accompanying "license" file. This file is
 * distributed "AS IS" AND WITHOUT WARRANTY OF ANY KIND WHATSOEVER, either
 * express or implied. See the License for the specific language governing
 * permissions and limitations under the License.
 *
 *                Copyright (C) 2022 by Dolby Laboratories.
 ***************************************************************************/

#include "comms/sample/media_source/file/utils/media_frame.h"

#include <cassert>

namespace dolbyio::comms::sample {

audio_frame_impl::audio_frame_impl(std::unique_ptr<frame_from_pool<audio_buffer>> audio_buf)
    : frame_(std::move(audio_buf)) {}

audio_frame_impl::~audio_frame_impl() = default;

video_frame_impl::video_frame_impl(std::unique_ptr<frame_from_pool<frame>> frame) : frame_(std::move(frame)) {
  assert((*frame_->val())->format == AV_PIX_FMT_YUV420P);
}

video_frame_impl::~video_frame_impl() = default;

int video_frame_impl::width() const {
  return (*frame_->val())->width;
}

int video_frame_impl::height() const {
  return (*frame_->val())->height;
}

int video_frame_impl::stride_y() const {
  return (*frame_->val())->linesize[0];
}

int video_frame_impl::stride_u() const {
  return (*frame_->val())->linesize[1];
}

int video_frame_impl::stride_v() const {
  return (*frame_->val())->linesize[2];
}

const uint8_t* video_frame_impl::get_y() const {
  return (*frame_->val())->data[0];
}

const uint8_t* video_frame_impl::get_u() const {
  return (*frame_->val())->data[1];
}

const uint8_t* video_frame_impl::get_v() const {
  return (*frame_->val())->data[2];
}

};  // namespace dolbyio::comms::sample
