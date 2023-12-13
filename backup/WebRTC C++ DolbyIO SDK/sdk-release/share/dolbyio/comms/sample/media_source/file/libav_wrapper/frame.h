#pragma once

/***************************************************************************
 * This program is licensed by the accompanying "license" file. This file is
 * distributed "AS IS" AND WITHOUT WARRANTY OF ANY KIND WHATSOEVER, either
 * express or implied. See the License for the specific language governing
 * permissions and limitations under the License.
 *
 *                Copyright (C) 2022 by Dolby Laboratories.
 ***************************************************************************/

extern "C" {
#include <libavcodec/avcodec.h>
}

namespace dolbyio::comms::sample {

class frame {
 public:
  frame();
  ~frame();

  void unref();
  AVFrame* operator->() { return frame_; }
  AVFrame* raw() { return frame_; }
  operator bool() { return frame_ != nullptr; }

 private:
  AVFrame* frame_;
};

};  // namespace dolbyio::comms::sample
