#include <cstdint>
#include <thread>
#include "fw/filesystem.hpp"
#include <functional>
#include <map>
#include <atomic>
#include <iostream>

enum class Change {
  Changed,
  Created,
  Deleted
};

#ifdef __linux__
#include <unistd.h>
#include <sys/inotify.h>

struct FsWatcher {
  struct inotify_event {
    int      wd;       /* Watch descriptor */
    uint32_t mask;     /* Mask describing event */
    uint32_t cookie;   /* Unique cookie associating related
                          events (for rename(2)) */
    uint32_t len;      /* Size of name field */
    char     name[1];   /* Optional null-terminated name */
  };

  FsWatcher(fs::path path, std::function<void(fs::path, Change)> onEvent) 
  : onEvent(onEvent)
  {
    fd = inotify_init();

    add_watch_recursive(path);

    thread = std::thread([this]{ run(); });
  }
  ~FsWatcher() {
    close(fd);
    thread.detach();
  }
  void run() {
    while (!quit) {
      char buf[4096];
      long len = read(fd, buf, sizeof buf);
      if ((len == -1 && errno != EAGAIN) ||
          len == 0) {
        close(fd);
        fd = inotify_init();
        std::unordered_map<int, fs::path> x = std::move(paths);
        for (auto& [_, path] : x) {
          add_watch(path);
        }
      } else {
        for (inotify_event* p = (inotify_event*)buf; (char*)p < buf + len; p = (inotify_event*)((char*)p + p->len + 16)) {
          handle_raw_event(p);
        }
      }
    }
  }

  std::map<uint32_t, fs::path> moved_from_filenames;
  void handle_raw_event(struct inotify_event* event) {
    if (event->name == std::string(".evoke.db")) return;

    fs::path path = paths[event->wd];
    if (event->len) path /= event->name;
    fprintf(stderr, "Got event %08X %d   %s\n", event->mask, event->wd, path.c_str());

    if (event->mask & (IN_MOVED_FROM)) {
      moved_from_filenames[event->cookie] = path.filename();
    }

    if (event->mask & (IN_CLOSE_WRITE | IN_MODIFY)) {
      onEvent(path, Change::Changed);
    } else if (event->mask & (IN_CREATE | IN_MOVED_TO)) {
      if ((event->mask & IN_MOVED_TO) && event->len == 0) {
        path /= moved_from_filenames[event->cookie];
        moved_from_filenames.erase(event->cookie);
      }
      onEvent(path, Change::Created);
      if (fs::is_directory(path))
        add_watch_recursive(path);
    } else if (event->mask & (IN_MOVED_FROM | IN_DELETE)) {
      onEvent(path, Change::Deleted);
      if (event->len == 0)
        remove_watch(event->wd);
    } else if (event->mask & (IN_DELETE_SELF | IN_MOVE_SELF)) {
      remove_watch(event->wd);
    } else if (event->mask & IN_Q_OVERFLOW) {
      fprintf(stderr, "Queue overflow, events missed\n");
    }
  }
  void add_watch_recursive(fs::path path) {
    if (path.filename() == "build" || path.filename() == ".git") return;
    add_watch(path);
    for (auto& entry : fs::directory_iterator(path)) {
      if (fs::is_directory(entry.status())) {
        fprintf(stderr, "%s %s\n", path.c_str(), entry.path().c_str());
        add_watch_recursive(entry.path());
      }
    }
  }
  void add_watch(fs::path path) {
    int wd = inotify_add_watch(fd, path.c_str(), IN_MODIFY | IN_CLOSE_WRITE | IN_MOVED_FROM | IN_MOVED_TO | IN_CREATE | IN_DELETE | IN_DELETE_SELF | IN_MOVE_SELF);
    fprintf(stderr, "adding watch to %s -> %d\n", path.c_str(), wd);
    paths[wd] = path;
  }
  void remove_watch(int wd) {
    fprintf(stderr, "removing watch to %d\n", wd);
    inotify_rm_watch(fd, wd);
    paths.erase(wd);
  }
  int fd;
  std::atomic<bool> quit = false;
  std::thread thread;
  std::unordered_map<int, fs::path> paths;
  std::function<void(fs::path, Change)> onEvent;
};

void FsWatch(fs::path path, std::function<void(fs::path, Change)> onEvent) {
  static FsWatcher fsw(path, onEvent);
}

#else

void FsWatch(fs::path path, std::function<void(fs::path, Change)> onEvent) {
  std::cerr << "Fs Watching not implemented on this platform; cannot run daemon mode.\n";
}
#endif
