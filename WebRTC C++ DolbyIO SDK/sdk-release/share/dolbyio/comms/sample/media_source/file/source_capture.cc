/***************************************************************************
 * This program is licensed by the accompanying "license" file. This file is
 * distributed "AS IS" AND WITHOUT WARRANTY OF ANY KIND WHATSOEVER, either
 * express or implied. See the License for the specific language governing
 * permissions and limitations under the License.
 *
 *                Copyright (C) 2022 by Dolby Laboratories.
 ***************************************************************************/

#include "comms/sample/media_source/file/source_capture.h"



namespace dolbyio::comms::sample {

std::unique_ptr<file_source> file_source::create(std::vector<std::string>&& files,
                                                 bool loop,
                                                 dolbyio::comms::plugin::injector_paced& injector,
                                                 std::function<void(const file_source_status& status)>&& status_cb) {
  return std::make_unique<file_source>(std::move(files), loop, injector, std::move(status_cb));
}

file_source::file_source(std::vector<std::string>&& files,
                         bool loop,
                         dolbyio::comms::plugin::injector_paced& injector,
                         std::function<void(const file_source_status&)>&& status_cb)
    : injector_(injector),
      video_pool_(std::make_unique<frame_pool<frame>>(
          20,
          []() { return (new frame()); },
          [](frame* f) { delete f; })),
      input_files_(std::move(files)),
      curr_file_(input_files_.begin()),
      capture_thread_([this]() {
#if defined(__APPLE__)
        pthread_setname_np("injection_capture");
#elif defined(__linux__)
        pthread_setname_np(capture_thread_.native_handle(), "injection_capture");
#endif
        thread_function();
      }),
      capture_executor_([this]() { return capture_loop(); }),
      source_status_(std::move(status_cb)) {
  if (curr_file_ != input_files_.end())
    file_state_.new_file(*curr_file_);
  else
    std::cerr << "Warning: no media file provided, you must provide one before "
                 "starting injection!\n";

  capture_state_.looping = loop;
}

file_source::~file_source() {
  {
    std::unique_lock<std::mutex> lock(capture_lock_);
    if (capture_state_.running) {
      capture_state_.capture_audio = false;
      capture_state_.capture_video = false;
      stop_capture(lock);
    } else if (capture_state_.running_silence) {
      injector_.stop_audio_injection(true);
    }
    injector_.clear_audio_queue();
    injector_.clear_video_queue();
    thread_state_.exit_ = true;
  }
  wait_to_start_.notify_one();
  try {
    capture_thread_.join();
  } catch (const std::exception& e) {
    std::cerr << "Error: Joining capture_thread threw: " << e.what() << std::endl;
  }
  audio_pool_.reset();
  video_pool_.reset();
  libav_context_.reset();
}

bool file_source::set_audio_capture(bool enable) {
  std::unique_lock<std::mutex> lock(capture_lock_);
  if (capture_state_.capture_audio == enable) {
    std::cerr << "Audio state already set to " << (enable ? "Enable" : "Disable") << "!\n";
    return false;
  }
  capture_state_.capture_audio = enable;
  std::cerr << "Audio set to " << (enable ? "" : "Stop ") << "Capture!\n";

  if (enable) {
    injector_.start_audio_injection();
    return start_capture();
  } else
    return stop_capture(lock);
}

bool file_source::set_video_capture(bool enable) {
  std::unique_lock<std::mutex> lock(capture_lock_);
  if (capture_state_.capture_video == enable) {
    std::cerr << "Video state already set to " << (enable ? "Enable" : "Disable") << "!\n";
    return false;
  }
  capture_state_.capture_video = enable;
  std::cerr << "Video set to " << (enable ? "" : "Stop ") << "Capture!\n";

  if (enable) {
    injector_.start_video_injection();
    return start_capture();
  } else
    return stop_capture(lock);
}

bool file_source::seek(int64_t time) {
  std::lock_guard<std::mutex> lock(capture_lock_);
  if (!capture_state_.running || !libav_context_->set_next_seek_time(time)) {
    std::cerr << "Failed to Seek, Capture State " << (capture_state_.running ? "" : "Not") << " Running!\n";
    return false;
  }

  file_state_.seek();
  return true;
}

bool file_source::pause() {
  std::unique_lock<std::mutex> lock(capture_lock_);
  if (!capture_state_.running) {
    std::cerr << "Pause Failed, Capture is Not Running!\n";
    return false;
  }

  file_state_.pause();
  if (wait_thread_stopped(lock)) {
    capture_state_.running_silence = true;
    injector_.start_audio_silence_injection(file_state_.audio.sample_rate_, file_state_.audio.channels_);
    return true;
  }
  return false;
}

bool file_source::resume() {
  std::lock_guard<std::mutex> lock(capture_lock_);
  if (file_state_.state() != file_state::PAUSE) {
    std::cerr << "Resume Failed, Current state is not Paused!\n";
    return false;
  }

  // Stop injecting silence into the conference
  injector_.stop_audio_injection(true /*force stoppage*/);

  if (capture_state_.capture_audio)
    injector_.start_audio_injection();
  if (capture_state_.capture_video)
    injector_.start_video_injection();

  capture_state_.running_silence = false;
  capture_state_.running = true;

  start_thread();
  return true;
}

void file_source::play_new_file(const std::string& file) {
  std::lock_guard<std::mutex> lock(capture_lock_);
  auto iter = input_files_.begin();
  // Is our playlist empty
  if (iter == input_files_.end()) {
    input_files_.push_back(file);
    curr_file_ = input_files_.begin();
  } else {
    for (; iter != input_files_.end(); ++iter) {
      if (*iter == file)
        break;
    }
    // Check if the new file to play is already part of the playlist
    if (iter != input_files_.end())
      curr_file_ = iter;
    // If it is not check if adding a new file will invalidate
    else if (input_files_.size() < input_files_.capacity()) {
      input_files_.push_back(file);
      auto last_iter = std::prev(input_files_.end());
      std::iter_swap(curr_file_, last_iter);
    } else {
      std::string current = *curr_file_;
      input_files_.push_back(file);
      auto last_iter = std::prev(input_files_.end());
      auto curr_iter = input_files_.begin();
      for (; curr_iter != input_files_.end(); ++curr_iter) {
        if (*curr_iter == current)
          break;
      }
      std::iter_swap(curr_iter, last_iter);
      curr_file_ = curr_iter;
    }
  }
  file_state_.new_file(*curr_file_);
}

void file_source::add_file_playlist(const std::string& file) {
  std::lock_guard<std::mutex> lock(capture_lock_);
  auto iter = input_files_.begin();
  // Check if the playlist is empty
  if (iter == input_files_.end()) {
    input_files_.push_back(file);
    curr_file_ = input_files_.begin();
    return;
  }
  for (; iter != input_files_.end(); ++iter) {
    if (*iter == file)
      break;
  }
  // Check if the file is already part of the playlist
  if (iter != input_files_.end())
    return;
  else if (input_files_.size() < input_files_.capacity())
    input_files_.push_back(file);
  else {
    std::string current = *curr_file_;
    input_files_.push_back(file);
    for (iter = input_files_.begin(); iter != input_files_.end(); ++iter) {
      if (*iter == current) {
        curr_file_ = iter;
        break;
      }
    }
  }
}

bool file_source::start_capture() {
  if (capture_state_.running)
    return true;

  auto state = file_state_.state();
  if (state == file_state::STOP || state == file_state::NEW) {
    if (!initialize_av_context())
      return false;
  } else if (state != file_state::PAUSE)
    return false;

  capture_state_.running = true;

  start_thread();
  return true;
}

bool file_source::stop_capture(std::unique_lock<std::mutex>& lock) {
  if (!capture_state_.running) {
    std::cerr << "Capture from media file is already stopped!\n";
    return true;
  }
  if (capture_state_.capture_audio || capture_state_.capture_video) {
    std::cerr << (capture_state_.capture_audio ? "Still Capturing Audio! " : "Not Capturing Audio! ")
              << (capture_state_.capture_video ? "Still Capturing Video!" : "Not Capturing Video!") << std::endl;
    return false;
  }
  // Stop and wait for the thread to reach conditional in thread_function
  file_state_.stop();
  return wait_thread_stopped(lock);
}

bool file_source::restart_capture(bool init_context) {
  if (init_context && !initialize_av_context())
    return false;

  capture_state_.running = true;
  injector_.start_video_injection();
  injector_.start_audio_injection();

  return true;
}

bool file_source::playlist_not_finished() {
  if (std::next(curr_file_) == input_files_.end())
    return false;

  return true;
}

bool file_source::wait_thread_stopped(std::unique_lock<std::mutex>& lock) {
  if (!lock.owns_lock())
    return false;

  thread_state_.waiting_ = true;
  wait_to_stop_.wait(lock, [this]() { return thread_state_.stopped_; });
  thread_state_.waiting_ = false;
  return true;
}

bool file_source::initialize_av_context() {
  try {
    if (file_state_.name().empty())
      throw std::runtime_error("There is no media file set to read from!");

    libav_context_ = std::make_unique<libav_context>(file_state_.name());
    libav_context_->create_decoder(AVMEDIA_TYPE_VIDEO);
    libav_context_->create_decoder(AVMEDIA_TYPE_AUDIO);
  } catch (const std::exception& ex) {
    libav_context_.reset();
    std::cerr << "Failed for reason: " << ex.what() << std::endl;
    return false;
  }

  // If the format of the audio in file is different we need a new audio frame
  // pool.
  if (!file_state_.audio.compare(libav_context_->sample_rate(), libav_context_->channels()))
    allocate_audio_frame_pool();

  file_state_.audio.settings(libav_context_->sample_rate(), libav_context_->channels());
  injector_.set_video_frame_interval(libav_context_->frame_interval());
  return true;
}

int file_source::queue_audio_frame(audio_pool_frame_ptr&& value) {
  return injector_.inject_audio_frame(std::make_unique<audio_frame_impl>(std::move(value)));
}

int file_source::queue_video_frame(video_pool_frame_ptr&& value) {
  return injector_.inject_video_frame(std::make_unique<video_frame_impl>(std::move(value)));
}

void file_source::allocate_audio_frame_pool() {
  audio_pool_ = std::make_unique<frame_pool<audio_buffer>>(
      100,
      [samples{libav_context_->sample_rate() / 100}, channels{libav_context_->channels()},
       sample_rate{libav_context_->sample_rate()}]() -> audio_buffer* {
        return new audio_buffer(samples, sample_rate, channels);
      },
      std::default_delete<audio_buffer>());
}

// right now only f32lpp, handle other input formats for audio
audio_pool_frame_ptr file_source::process_audio(audio_pool_frame_ptr&& curr_buff, frame& aframe) {
  if (!curr_buff) {
    std::cerr << "Current audio buffer is null!\n";
    return nullptr;
  }
  if (!aframe) {
    std::cerr << "No AVFrame provided!\n";
    return std::move(curr_buff);
  }
  if (aframe->format != AV_SAMPLE_FMT_FLTP) {
    std::cerr << "Currently only support AVFrame format Float Planar!\n";
    return std::move(curr_buff);
  }

  float* channel_buffer[AV_NUM_DATA_POINTERS] = {0};
  for (int i = 0; i < aframe->channels; ++i)
    channel_buffer[i] = reinterpret_cast<float*>(aframe->data[i]);

  for (int i = 0; i < aframe->nb_samples; ++i) {
    if (curr_buff->val()->full()) {
      queue_audio_frame(std::move(curr_buff));
      curr_buff.reset(new frame_from_pool<audio_buffer>(audio_pool_->get_frame(), *audio_pool_,
                                                        [](audio_buffer* buf) { buf->reset(); }));
    }
    for (int j = 0; j < aframe->channels; ++j) {
      double val = channel_buffer[j][i] * std::numeric_limits<int16_t>::max();
      curr_buff->val()->push(val);
    }
  }
  if (curr_buff->val()->full()) {
    queue_audio_frame(std::move(curr_buff));
    curr_buff.reset(new frame_from_pool<audio_buffer>(audio_pool_->get_frame(), *audio_pool_,
                                                      [](audio_buffer* buf) { buf->reset(); }));
  }
  return std::move(curr_buff);
}

void file_source::thread_function() {
  while (!thread_state_.exit_) {
    {
      std::unique_lock<std::mutex> lock(capture_lock_);

      // Check if someone is waiting to be sure the thread has returned
      // to the stopped state.
      thread_state_.stopped_ = true;
      if (thread_state_.waiting_)
        wait_to_stop_.notify_one();

      wait_to_start_.wait(lock, [this]() { return thread_state_.start_ || thread_state_.exit_; });

      thread_state_.stopped_ = false;
      if (thread_state_.exit_)
        break;
    }
    capture_executor_();
  }
}

void file_source::start_thread() {
  thread_state_.start_ = true;
  wait_to_start_.notify_one();
}

/* This is called on the capture_thread_ after it exits the loop. We check the
 * current status of the file and act accordingly.
 */
void file_source::capture_loop_exited() {
  std::unique_lock<std::mutex> lock(capture_lock_);
  capture_state_.running = false;
  bool restart = false;

  // If the capture loop exited on it's own check if we need to loop single
  // file, play next file in play list or just exit.
  auto state = file_state_.state();
  if (state == file_state::PLAYING) {
    // If the file left capture loop in play state it means there are still
    // like frames on the queue, make sure to let them all be inejcted before
    // continuing.
    injector_.stop_audio_injection(false /*wait all frames to be injected*/);
    injector_.stop_video_injection(false /*wait all frames to be injected*/);

    if (capture_state_.looping)
      restart = restart_capture(true);
    else if (playlist_not_finished()) {
      // Go to the next file and set it to start playing once we enter the
      // capture loop
      ++curr_file_;
      file_state_.new_file(*curr_file_);
      restart = restart_capture(true);
    } else {
      file_state_.stop();
      libav_context_.reset();
      capture_state_.capture_audio = false;
      capture_state_.capture_video = false;

      if (source_status_)
        source_status_(file_source_status{false, false, source_state::STOPPED});
    }
  } else {
    // If file is no longer in the play state stop the injection threads and
    // then act accordingly based on the file state
    injector_.stop_audio_injection(true /*force stoppage*/);
    injector_.stop_video_injection(true /*force stoppage*/);

    if (state == file_state::SEEK) {
      libav_context_->seek_set_time();
      restart = restart_capture(false);
    } else if (state == file_state::NEW) {
      injector_.clear_audio_queue();
      injector_.clear_video_queue();
      restart = restart_capture(true);
    } else if (state == file_state::STOP) {
      injector_.clear_audio_queue();
      injector_.clear_video_queue();
      libav_context_.reset();
    }
  }
  thread_state_.start_ = restart;
}

void file_source::capture_loop() {
  {
    audio_pool_frame_ptr reference_audio_frame(new frame_from_pool<audio_buffer>(
        audio_pool_->get_frame(), *audio_pool_, [](audio_buffer* buf) { buf->reset(); }));

    auto audio_read_frame = std::make_unique<frame>();
    file_state_.playing();
    int ret = 0;
    while (libav_context_->read_single_packet() >= 0) {
      {
        // Check if the state of the file has changed externally
        std::lock_guard<std::mutex> lock(capture_lock_);
        file_state::state_change state = file_state_.state();
        if (state != file_state::PLAYING) {
          if (state != file_state::PAUSE)
            libav_context_->packet_finished();
          break;
        }
        ret = libav_context_->packet_to_decoder(capture_state_.capture_video, capture_state_.capture_audio);
      }
      while (ret >= 0) {
        if (libav_context_->is_video()) {
          auto vframe = video_pool_->get_frame();
          ret = libav_context_->frame_from_decoder<video>(vframe);
          if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
            video_pool_->return_frame(vframe);
            break;
          }
          if (vframe->raw()->format != AV_PIX_FMT_YUV420P) {
            video_pool_->return_frame(vframe);
            std::cerr << "Bad video frame format" << std::endl;
            break;
          }
          queue_video_frame(
              std::make_unique<frame_from_pool<frame>>(vframe, *video_pool_, [](frame* f) { f->unref(); }));
        } else if (libav_context_->is_audio()) {
          ret = libav_context_->frame_from_decoder<audio>(audio_read_frame.get());
          if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
            break;

          // this will return the current audio frame that contains the packets
          // which are left over from last fillage
          reference_audio_frame = process_audio(std::move(reference_audio_frame), *audio_read_frame);
          audio_read_frame->unref();
        }
      }
      libav_context_->packet_finished();
    }
  }
  // All capture_loop stack variables deleted before checking reason for exit
  capture_loop_exited();
}

};  // namespace dolbyio::comms::sample
