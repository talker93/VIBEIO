#pragma once

/***************************************************************************
 * This program is licensed by the accompanying "license" file. This file is
 * distributed "AS IS" AND WITHOUT WARRANTY OF ANY KIND WHATSOEVER, either
 * express or implied. See the License for the specific language governing
 * permissions and limitations under the License.
 *
 *                Copyright (C) 2022 by Dolby Laboratories.
 ***************************************************************************/

#include "comms/sample/media_source/file/libav_wrapper/frame.h"

extern "C" {
#include <libavformat/avformat.h>
}

namespace dolbyio::comms::sample {

class decoder {
 public:
  explicit decoder(AVStream* stream, bool refcount) noexcept(false);
  ~decoder();

  int send(AVPacket* packet);
  int receive(frame* frame);

  AVCodecContext* codec_context() { return context_; }
  int channels() { return context_->channels; }
  int sample_rate() { return context_->sample_rate; }

 private:
  AVStream* stream_;
  AVCodec* codec_ = nullptr;
  AVCodecContext* context_ = nullptr;
  AVDictionary* options = nullptr;
};

};  // namespace dolbyio::comms::sample
