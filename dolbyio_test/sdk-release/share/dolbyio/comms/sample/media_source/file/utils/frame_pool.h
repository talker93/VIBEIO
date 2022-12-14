#pragma once

#include <condition_variable>
/***************************************************************************
 * This program is licensed by the accompanying "license" file. This file is
 * distributed "AS IS" AND WITHOUT WARRANTY OF ANY KIND WHATSOEVER, either
 * express or implied. See the License for the specific language governing
 * permissions and limitations under the License.
 *
 *                Copyright (C) 2022 by Dolby Laboratories.
 ***************************************************************************/

#include <functional>
#include <iostream>
#include <mutex>
#include <vector>

namespace dolbyio::comms::sample {

template <typename T>
class frame_pool {
 public:
  explicit frame_pool(int size, std::function<T*()>&& cb, std::function<void(T*)> db)
      : create_cb_(std::move(cb)), destroy_cb_(std::move(db)) {
    for (int i = 0; i < size; ++i)
      pool_.push_back(create_cb_());
    total_ = size;
  }

  ~frame_pool() { destroy_all_frames(); }

  T* get_frame() {
    std::lock_guard<std::mutex> lock(lock_);
    T* frame = nullptr;
    if (!pool_.empty()) {
      frame = pool_.back();
      pool_.pop_back();
    } else {
      ++total_;
      frame = create_cb_();
    }
    return frame;
  }

  void return_frame(T* frame) {
    std::unique_lock<std::mutex> lock(lock_);
    pool_.push_back(frame);
    if (destroy_wait) {
      if (total_ == pool_.size()) {
        lock.unlock();
        cond_.notify_one();
      }
    }
  }

  void destroy_all_frames() {
    std::unique_lock<std::mutex> lock(lock_);
    if (pool_.size() < total_) {
      destroy_wait = true;
      cond_.wait(lock, [this]() { return pool_.size() >= total_; });
    }
    for (const auto& frame : pool_) {
      destroy_cb_(frame);
    }
  }

 private:
  std::mutex lock_;
  std::condition_variable cond_;
  bool destroy_wait = false;

  std::vector<T*> pool_;

  std::function<T*()> create_cb_;
  std::function<void(T*)> destroy_cb_;
  unsigned int total_ = 0;
};

template <typename T>
class frame_from_pool {
 public:
  template <typename U>
  frame_from_pool(T* val, frame_pool<T>& pool, U&& deleter = {}) : val_(val), pool_(pool), delete_cb_(deleter) {}
  ~frame_from_pool() {
    if (delete_cb_)
      delete_cb_(val_);
    if (val_)
      pool_.return_frame(val_);
  }

  T* val() { return val_; }

 private:
  T* val_;
  frame_pool<T>& pool_;
  int id_;
  void (*delete_cb_)(T*);
};

}  // namespace dolbyio::comms::sample
