#include "FsWatcher.hpp"
#include <inotify-cpp/NotifierBuilder.h>


#include <thread>

using namespace inotify;

void FsWatch(filesystem::path path, std::function<void(filesystem::path, Change)> onEvent) {
  std::thread([=](){ 
    auto events = { Event::create,
                    Event::modify,
                    Event::remove,
                    Event::moved_to,
                    Event::moved_from };

    BuildNotifier()
      .watchPathRecursively(path)
      .onEvents(events, [onEvent = std::move(onEvent)](Notification n) {
        switch (n.event) {
        case Event::create:
        case Event::moved_to:
          onEvent(n.path, Change::Created);
          break;
        case Event::remove:
        case Event::moved_from:
          onEvent(n.path, Change::Deleted);
          break;
        case Event::modify:
          onEvent(n.path, Change::Changed);
          break;
        default:
          break;
        }
      }).run();
  }).detach();
}
