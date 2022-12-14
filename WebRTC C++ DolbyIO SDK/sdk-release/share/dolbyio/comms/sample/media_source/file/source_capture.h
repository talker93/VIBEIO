#pragma once

/***************************************************************************
 * This program is licensed by the accompanying "license" file. This file is
 * distributed "AS IS" AND WITHOUT WARRANTY OF ANY KIND WHATSOEVER, either
 * express or implied. See the License for the specific language governing
 * permissions and limitations under the License.
 *
 *                Copyright (C) 2022 by Dolby Laboratories.
 ***************************************************************************/

#include <dolbyio/comms/multimedia_streaming/injector.h>

#include "comms/sample/media_source/file/libav_wrapper/avcontext.h"
#include "comms/sample/media_source/file/source_context.h"
#include "comms/sample/media_source/file/utils/audio_buffer.h"
#include "comms/sample/media_source/file/utils/frame_pool.h"
#include "comms/sample/media_source/file/utils/media_frame.h"

#include <condition_variable>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

namespace dolbyio::comms::sample {

enum class source_state { STOPPED, RESTARTED, CONTINUE, RESUMED, PAUSED };

struct file_source_status {
  bool capturing_audio = false;
  bool capturing_video = false;
  source_state current_state = source_state::STOPPED;
};

using audio_pool_frame_ptr = std::unique_ptr<frame_from_pool<audio_buffer>>;
using video_pool_frame_ptr = std::unique_ptr<frame_from_pool<frame>>;

class file_source {
 public:
  static std::unique_ptr<file_source> create(std::vector<std::string>&& files,
                                             bool loop,
                                             dolbyio::comms::plugin::injector_paced& injection,
                                             std::function<void(const file_source_status& status)>&& status_cb);

  file_source(std::vector<std::string>&& files,
              bool loop,
              comms::plugin::injector_paced& injector,
              std::function<void(const file_source_status&)>&& status_cb);
  ~file_source();

  // Public Interface
  bool set_audio_capture(bool enable);
  bool set_video_capture(bool enable);
  bool seek(int64_t time);
  bool pause();
  bool resume();
  bool loop_current_file(bool enables);
  void play_new_file(const std::string& file);
  void add_file_playlist(const std::string& file);

 private:
  bool start_capture();
  bool stop_capture(std::unique_lock<std::mutex>& lock);
  bool restart_capture(bool init_context);
  bool initialize_av_context();
  bool playlist_not_finished();
  bool wait_thread_stopped(std::unique_lock<std::mutex>& lock);

  int queue_audio_frame(audio_pool_frame_ptr&& value);
  int queue_video_frame(video_pool_frame_ptr&& value);
  void allocate_audio_frame_pool();
  audio_pool_frame_ptr process_audio(audio_pool_frame_ptr&& curr_buff, frame& aframe);

  // Managing the capturing thread
  void start_thread();
  void thread_function();
  void capture_loop();
  void capture_loop_exited();

  dolbyio::comms::plugin::injector_paced& injector_;
  std::unique_ptr<frame_pool<audio_buffer>> audio_pool_{};
  std::unique_ptr<frame_pool<frame>> video_pool_{};
  std::unique_ptr<libav_context> libav_context_;

  file_state file_state_;
  std::vector<std::string> input_files_;
  std::vector<std::string>::iterator curr_file_;

  struct capture_state {
    bool capture_audio = false;
    bool capture_video = false;
    bool running = false;
    bool running_silence = false;
    bool looping = false;
  };
  capture_state capture_state_;

  struct thread_state {
    bool start_ = false;
    bool exit_ = false;
    bool waiting_ = false;
    bool stopped_ = false;
  };
  thread_state thread_state_;

  std::mutex capture_lock_;
  std::thread capture_thread_;
  std::condition_variable wait_to_start_;
  std::condition_variable wait_to_stop_;
  std::function<void()> capture_executor_;

  std::function<void(const file_source_status& status)> source_status_;
};

};  // namespace dolbyio::comms::sample
