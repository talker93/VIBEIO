/***************************************************************************
 * This program is licensed by the accompanying "license" file. This file is
 * distributed "AS IS" AND WITHOUT WARRANTY OF ANY KIND WHATSOEVER, either
 * express or implied. See the License for the specific language governing
 * permissions and limitations under the License.
 *
 *                Copyright (C) 2022 by Dolby Laboratories.
 ***************************************************************************/

#include "comms/sample/media_source/file/libav_wrapper/decoder.h"

#include <iostream>
#include <stdexcept>
#include <string>

namespace dolbyio::comms::sample {

decoder::decoder(AVStream* stream, bool refcount) noexcept(false)
    : stream_(stream) {
  int av_return = 0;

  if (!stream_)
    throw std::runtime_error("Stream does not exist!");

  codec_ = avcodec_find_decoder(stream_->codecpar->codec_id);
  if (!codec_) {
    throw std::runtime_error("Failed to find codec in stream!");
  }
  context_ = avcodec_alloc_context3(codec_);
  if (!context_) {
    throw std::runtime_error("Failed to create codec context!");
  }

  if ((av_return = avcodec_parameters_to_context(context_, stream_->codecpar)) < 0) {
    throw std::runtime_error("Failed to copy codec parameters to decoder context! code:" + std::to_string(av_return));
  }
  if ((av_return = av_dict_set(&options, "refcounted_frames", refcount ? "1" : "0", 0)) < 0) {
    std::cerr << "Failed to add " << (refcount ? "refcounted" : "non-refcounted")
              << " option to dictionary:" + std::to_string(av_return) << ". Not fatal, continuing!\n";
  }
  if ((av_return = avcodec_open2(context_, codec_, &options)) < 0) {
    throw std::runtime_error("Failed to open codec! code:" + std::to_string(av_return));
  }
}

decoder::~decoder() {
  if (context_)
    avcodec_free_context(&context_);
}

int decoder::send(AVPacket* packet) {
  int ret = avcodec_send_packet(context_, packet);
  // Do some error depiction here and then return something
  // more useful to caller.
  return ret;
}

int decoder::receive(dolbyio::comms::sample::frame* frame) {
  int ret = avcodec_receive_frame(context_, frame->raw());
  return ret;
}

};  // namespace dolbyio::comms::sample
