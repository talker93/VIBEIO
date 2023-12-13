// This file is compiled with C++17 standard: CMake build scripts set the
// C++17 standard, and the proper flag is passed to the compiler of the
// Objective-C++ source file. The CMAKE_OBJCXX_STANDARD value is populated from
// the CMAKE_CXX_STANDARD variable. Unfortunately, the clangd language server,
// when indexing the code, always uses C++14 standard for objective-c++ files. A
// quick workaorund of redefining the macro allows at least for parsing the
// std:: types correctly, and it appears that the whole code indexes correctly:
#if defined(IAPI_CLANGD_INDEXER) && IAPI_CLANGD_INDEXER
#if __cplusplus < 201703L
#undef __cplusplus
#define __cplusplus 201703L
#endif  // C++ < 17
#endif  // Clangd indexer

#include "macos_ui.h"

#include <iostream>

#import <Cocoa/Cocoa.h>

namespace dolbyio::comms::sample {

// This class is used for rendering participant video streams. There is a single
// instance of this class created for each participant video track received by the
// SDK. The constructor of this class will create the UI window
struct macosui::track_data::impl : public macosui::track_data {
  impl* get_impl() override { return this; }
  impl(const std::string& peer_id, const std::string& title) : peer_id_(peer_id) {
    __block std::string title_copy(title);
    dispatch_barrier_async(dispatch_get_main_queue(), ^{
      NSString* win_name = [NSString stringWithUTF8String:title_copy.c_str()];
      if (win_name == nil) {
        win_name = @"(invalid string)";
      }
      window_ = [[NSWindow alloc] initWithContentRect:NSMakeRect(0, 0, 500, 500)
                                            styleMask:NSWindowStyleMaskTitled
                                              backing:NSBackingStoreBuffered
                                                defer:NO];
      [window_ cascadeTopLeftFromPoint:NSMakePoint(20, 20)];
      [window_ setTitle:win_name];
      [window_ makeKeyAndOrderFront:nil];

      view_ = [[NSView alloc] initWithFrame:NSMakeRect(0, 0, 500, 500)];
      video_layer_ = [CALayer layer];
      [view_ setLayer:video_layer_];
      [view_ setWantsLayer:YES];
      view_.layer.backgroundColor = [[NSColor yellowColor] CGColor];

      [window_.contentView addSubview:view_];
    });
  }

  ~impl() override {
    dispatch_barrier_async_and_wait(dispatch_get_main_queue(), ^{
      if (window_) {
        [window_ setReleasedWhenClosed:NO];
        [window_ close];
        window_ = nil;
        view_ = nil;
        video_layer_ = nil;
      }
    });
  }

  void set_title(const std::string& title) {
    __block std::string title_copy(title);
    dispatch_async(dispatch_get_main_queue(), ^{
      NSString* win_title = [NSString stringWithUTF8String:title_copy.c_str()];
      if (win_title == nil) {
        win_title = @"(invalid string)";
      }
      [window_ setTitle:win_title];
    });
  }

  // This method will actually draw the IOSurface extracted from the pixel buffer
  // onto the respective window.
  void set_frame(std::unique_ptr<dolbyio::comms::video_frame> unique_frame) {
    std::shared_ptr<dolbyio::comms::video_frame> frame(std::move(unique_frame));
    dispatch_async(dispatch_get_main_queue(), ^{
      if (frame->get_native_frame()) {
        auto buf = frame->get_native_frame()->get_buffer();
        auto iosurface = CVPixelBufferGetIOSurface(buf);
        if (iosurface) {
          set_size_on_main_thread(frame->width(), frame->height());

          video_layer_.contents = (__bridge id)(iosurface);
          cur_frame_ = frame;
        }
      } else if (frame->get_i420_frame()) {
        // VP8 frames not supported for now! :[
      }
    });
  }

  // Updates the size of the window based on the width/height of the incoming
  // video frame. This method is invoked on the mainQueue.
  void set_size_on_main_thread(int width, int height) {
    if (width == view_.frame.size.width && height == view_.frame.size.height)
      return;

    NSRect new_rect{};
    new_rect.size.height = height;
    new_rect.size.width = width;
    view_.frame = new_rect;

    new_rect = window_.frame;
    new_rect.size.height = height;
    new_rect.size.width = width;
    [window_ setFrame:new_rect display:YES animate:NO];
  }

  const std::string& peer_id() const { return peer_id_; }

 private:
  const std::string peer_id_{};
  std::shared_ptr<dolbyio::comms::video_frame> cur_frame_{};
  NSWindow* window_ = nullptr;
  NSView* view_ = nullptr;
  CALayer* video_layer_ = nullptr;
};

macosui::macosui(int argc, char** argv) : ui_interface(argc, argv) {}

macosui::~macosui() = default;

// The helper ui function which is invoked on the offloaded thread, so that main can run the
// macOS NSMainLoop. This function sets up macOS sample specific handlers and then invokeds
// the base class main ui loop helper function.
void macosui::ui_loop_on_helper_thread() {
  try {
    wait(sdk_->video().remote().set_video_sink(this));
    if (sdk_params().display_video) {
      ev_handlers_.emplace_back(
          wait(sdk_->conference().add_event_handler([this](const dolbyio::comms::video_track_removed& e) {
            std::unique_ptr<track_data> track{};
            {
              std::lock_guard<std::mutex> l(video_tracks_lock_);
              auto it = video_tracks_.find(e.stream_id);
              if (it != video_tracks_.end()) {
                track = std::move(it->second);
                video_tracks_.erase(it);
              }
            }
            // The track is now deleted, without holding the video_tracks_lock_, but still blocking with a
            // synchronisation with the main dispatch queue. TODO: make it async, non-blocking.
          })));
      ev_handlers_.emplace_back(
          wait(sdk_->conference().add_event_handler([this](const dolbyio::comms::video_track_added& e) {
            if (e.remote) {
              std::string title = e.peer_id;
              auto it = participants_.find(e.peer_id);
              if (it != participants_.end())
                title = it->second.info.name.value_or(e.peer_id);
              std::lock_guard<std::mutex> l(video_tracks_lock_);
              video_tracks_.insert(std::make_pair<std::string, std::unique_ptr<track_data>>(
                  std::string(e.stream_id), std::make_unique<track_data::impl>(e.peer_id, title)));
            }
          })));
      ev_handlers_.emplace_back(wait(sdk_->conference().add_event_handler(
          [this](const dolbyio::comms::participant_added& e) { update_participant(e.participant); })));
      ev_handlers_.emplace_back(wait(sdk_->conference().add_event_handler(
          [this](const dolbyio::comms::participant_updated& e) { update_participant(e.participant); })));
    }

    // Wait for the main loop to start running:
    __block std::promise<void> prom{};
    auto future = prom.get_future();

    dispatch_async(dispatch_get_main_queue(), ^{
      prom.set_value();
    });

    future.get();

    ui_interface::ui_loop_on_helper_thread();
  } catch (const std::exception& e) {
    std::cerr << "Failure: " << e.what() << std::endl;
  }

  // base's ui loop exitted: we're not in the conference any more:
  cleanup_before_exit();
}

void macosui::application_loop_on_main_thread() {
  [NSApp run];
}

void macosui::run(dolbyio::comms::sdk* sdk) {
  @autoreleasepool {
    [NSApplication sharedApplication];
    [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
    [NSApp activateIgnoringOtherApps:YES];

    ui_interface::run(sdk);
  }
}

void macosui::cleanup_before_exit() {
  try {
    for (auto& eh : ev_handlers_) {
      wait(eh->disconnect());
    }
    ev_handlers_.clear();
    video_tracks_.clear();  // safe to clear without mutex: conference is left, and event handlers are detached. FIXME:
                            // Each track's destructor syncs to the main thread!
    wait(sdk_->video().remote().set_video_sink(nullptr));
  } catch (const std::exception& e) {
    std::cerr << "Failure: " << e.what() << std::endl;
  }

  // The RTCVideoCapturer posts notification about start/stop video to main queue and then invokes the start/stop
  // mechanism on kCaptureSessionQueue from the mainqueue. Thus at the time of stoppage the mainqueue must exist.
  // Henceforth we ensure video capture is stopped prior to destroying the mainqueue.
  wait(sdk_->video().local().stop());

  dispatch_async(dispatch_get_main_queue(), ^{
    [NSApp stop:nil];

    // After stopping the application thread, the [NSApp run] still will not return until the loop handles some UI
    // event. Producing a fake, application-defined event for this purpose:
    NSEvent* ev = [NSEvent otherEventWithType:NSEventTypeApplicationDefined
                                     location:NSPoint{0, 0}
                                modifierFlags:0
                                    timestamp:0
                                 windowNumber:0
                                      context:nil
                                      subtype:0
                                        data1:0
                                        data2:0];
    [NSApp postEvent:ev atStart:FALSE];
  });
}

void macosui::update_participant(const dolbyio::comms::participant_info& participant) {
  participants_.insert_or_assign(participant.user_id, participant);

  // safe to access the video tracks without lock: the video_tracks_ collection is only mutated on the SDK thread, and
  // we're on the SDK thread here:
  for (auto& track : video_tracks_) {
    if (track.second->get_impl()->peer_id() == participant.user_id) {
      track.second->get_impl()->set_title(participant.info.name.value_or(participant.user_id));
      return;
    }
  }
}

// This is invoked by the SDK whenever a video frame has been decoded and is ready to be passed to a sink.
// This implementation receives the video frame, finds the appropriate track_data implementation and invokes
// its set_frame function which will perform the drawing of the frame.
void macosui::handle_frame(const std::string& stream_id,
                           const std::string&,
                           std::unique_ptr<dolbyio::comms::video_frame> frame) {
  track_data::impl* track = nullptr;
  {
    std::lock_guard<std::mutex> l(video_tracks_lock_);
    auto it = video_tracks_.find(stream_id);
    if (it == video_tracks_.end())
      return;
    track = it->second->get_impl();
  }

  // Safe to access without lock: video_tracks_ may be mutated, but the track in question will not be removed while it's
  // running. If we're in handle_frame(), the track stays until we return:
  track->set_frame(std::move(frame));
}

}  // namespace dolbyio::comms::sample
