/***************************************************************************
 * This program is licensed by the accompanying "license" file. This file is
 * distributed "AS IS" AND WITHOUT WARRANTY OF ANY KIND WHATSOEVER, either
 * express or implied. See the License for the specific language governing
 * permissions and limitations under the License.
 *
 *                Copyright (C) 2022 by Dolby Laboratories.
 ***************************************************************************/

#include "comms/sample/media_source/file/libav_wrapper/frame.h"

namespace dolbyio::comms::sample {

frame::frame() : frame_(av_frame_alloc()) {}

frame::~frame() {
  if (frame_->data[0]) {
    av_freep(&frame_->data[0]);
  }
  av_frame_free(&frame_);
}

void frame::unref() {
  av_frame_unref(frame_);
}

};  // namespace dolbyio::comms::sample
