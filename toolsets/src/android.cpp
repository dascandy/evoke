
#!/bin/bash

set -e
rm app-debug.apk app-debug-unsigned.apk -f

CCFLAGS="-std=c++17 -isystem /home/pebi/android-ndk-r17b/sources/cxx-stl/llvm-libc++/include --sysroot /home/pebi/android-ndk-r17b/sysroot -I/home/pebi/android-ndk-r17b/sources/android/native_app_glue"

mkdir -p out/lib/arm64-v8a/  out/lib/armeabi-v7a/ out/lib/x86_64/ out/lib/x86/ obj/i686/ obj/x86_64/ obj/armv7/ obj/aarch64/

/home/pebi/android-ndk-r17b/toolchains/llvm/prebuilt/linux-x86_64/bin/clang++ -target aarch64-linux-android /home/pebi/android-ndk-r17b/sources/android/native_app_glue/android_native_app_glue.c $CCFLAGS -c -o obj/aarch64/native_app_glue.o -isystem /home/pebi/android-ndk-r17b/sysroot/usr/include/aarch64-linux-android
/home/pebi/android-ndk-r17b/toolchains/llvm/prebuilt/linux-x86_64/bin/clang++ -target aarch64-linux-android /home/pebi/android-example/android-ndk/native-activity/app/src/main/cpp/main.cpp $CCFLAGS -c -o obj/aarch64/main.o -isystem /home/pebi/android-ndk-r17b/sysroot/usr/include/aarch64-linux-android
/home/pebi/android-ndk-r17b/toolchains/aarch64-linux-android-4.9/prebuilt/linux-x86_64/bin/aarch64-linux-android-ld -shared -o out/lib/arm64-v8a/libnative-activity.so obj/aarch64/main.o obj/aarch64/native_app_glue.o -L /home/pebi/android-ndk-r17b/platforms/android-28/arch-arm64/usr/lib -lGLESv1_CM -lEGL -L/home/pebi/android-ndk-r17b/sources/cxx-stl/llvm-libc++/libs/arm64-v8a -lc -ldl -llog -landroid -lc++_static -lc++abi

/home/pebi/android-ndk-r17b/toolchains/llvm/prebuilt/linux-x86_64/bin/clang++ -target armv7-linux-android /home/pebi/android-ndk-r17b/sources/android/native_app_glue/android_native_app_glue.c $CCFLAGS -c -o obj/armv7/native_app_glue.o -isystem /home/pebi/android-ndk-r17b/sysroot/usr/include/arm-linux-androideabi
/home/pebi/android-ndk-r17b/toolchains/llvm/prebuilt/linux-x86_64/bin/clang++ -target armv7-linux-android /home/pebi/android-example/android-ndk/native-activity/app/src/main/cpp/main.cpp $CCFLAGS -c -o obj/armv7/main.o -isystem /home/pebi/android-ndk-r17b/sysroot/usr/include/arm-linux-androideabi
/home/pebi/android-ndk-r17b/toolchains/arm-linux-androideabi-4.9/prebuilt/linux-x86_64/bin/arm-linux-androideabi-ld -shared -o out/lib/armeabi-v7a/libnative-activity.so  obj/armv7/main.o obj/armv7/native_app_glue.o -L /home/pebi/android-ndk-r17b/platforms/android-28/arch-arm/usr/lib -L/home/pebi/android-ndk-r17b/toolchains/arm-linux-androideabi-4.9/prebuilt/linux-x86_64/arm-linux-androideabi/lib -lGLESv1_CM -lEGL -L/home/pebi/android-ndk-r17b/sources/cxx-stl/llvm-libc++/libs/armeabi-v7a -lc -ldl -llog -landroid -lc++_static -lc++abi

/home/pebi/android-ndk-r17b/toolchains/llvm/prebuilt/linux-x86_64/bin/clang++ -target x86_64-linux-android /home/pebi/android-ndk-r17b/sources/android/native_app_glue/android_native_app_glue.c $CCFLAGS -c -o obj/x86_64/native_app_glue.o -isystem /home/pebi/android-ndk-r17b/sysroot/usr/include/x86_64-linux-android
/home/pebi/android-ndk-r17b/toolchains/llvm/prebuilt/linux-x86_64/bin/clang++ -target x86_64-linux-android /home/pebi/android-example/android-ndk/native-activity/app/src/main/cpp/main.cpp $CCFLAGS -c -o obj/x86_64/main.o -isystem /home/pebi/android-ndk-r17b/sysroot/usr/include/x86_64-linux-android
/home/pebi/android-ndk-r17b/toolchains/x86_64-4.9/prebuilt/linux-x86_64/bin/x86_64-linux-android-ld -shared -o out/lib/x86_64/libnative-activity.so  obj/x86_64/main.o obj/x86_64/native_app_glue.o -L /home/pebi/android-ndk-r17b/platforms/android-28/arch-x86_64/usr/lib64 -lGLESv1_CM -lEGL -L/home/pebi/android-ndk-r17b/sources/cxx-stl/llvm-libc++/libs/x86_64 -lc -ldl -llog -landroid -lc++_static -lc++abi

/home/pebi/android-ndk-r17b/toolchains/llvm/prebuilt/linux-x86_64/bin/clang++ -target i686-linux-android /home/pebi/android-ndk-r17b/sources/android/native_app_glue/android_native_app_glue.c $CCFLAGS -c -o obj/i686/native_app_glue.o -isystem /home/pebi/android-ndk-r17b/sysroot/usr/include/i686-linux-android
/home/pebi/android-ndk-r17b/toolchains/llvm/prebuilt/linux-x86_64/bin/clang++ -target i686-linux-android /home/pebi/android-example/android-ndk/native-activity/app/src/main/cpp/main.cpp $CCFLAGS -c -o obj/i686/main.o -isystem /home/pebi/android-ndk-r17b/sysroot/usr/include/i686-linux-android
/home/pebi/android-ndk-r17b/toolchains/x86-4.9/prebuilt/linux-x86_64/bin/i686-linux-android-ld -shared -o out/lib/x86/libnative-activity.so  obj/i686/main.o obj/i686/native_app_glue.o -L /home/pebi/android-ndk-r17b/platforms/android-28/arch-x86/usr/lib -lGLESv1_CM -lEGL -L/home/pebi/android-ndk-r17b/sources/cxx-stl/llvm-libc++/libs/x86 -lc -ldl -llog -landroid -lc++_static -lc++abi

aapt package -f -M AndroidManifest.xml -S res -I "/home/pebi/Android/Sdk/platforms/android-25/android.jar" -F app-debug-unsigned.apk out

apksigner sign --ks keystore.jks --ks-key-alias androidkey --ks-pass pass:android --key-pass pass:android --out app-debug.apk app-debug-unsigned.apk


#include "Toolset.h"
#include "Component.h"
#include "PendingCommand.h"
#include "File.h"
#include "Project.h"
#include "filter.h"

static std::string getLibNameFor(Component& component) {
  // TODO: change commponent to dotted string before making
  return "lib" + component.root.string() + ".a";
}

static std::string getExeNameFor(Component& component) {
  if (component.root.string() != ".") {
    return component.root.string();
  }
  return boost::filesystem::canonical(component.root).filename().string();
}

void UbuntuToolset::CreateCommandsFor(Project& project, Component& component) {
  std::string includes;
  for (auto& d : getIncludePathsFor(component)) {
    includes += " -I" + d;
  }

  boost::filesystem::path outputFolder = component.root;
  std::vector<File*> objects;
  for (auto& f : filter(component.files, [&project](File*f){ return project.IsCompilationUnit(f->path.extension().string()); })) {
    boost::filesystem::path outputFile = std::string("obj") / outputFolder / (f->path.string().substr(component.root.string().size()) + ".o");
    File* of = project.CreateFile(component, outputFile);
    PendingCommand* pc = new PendingCommand("g++ -c -std=c++17 -o " + outputFile.string() + " " + f->path.string() + includes);
    objects.push_back(of);
    pc->AddOutput(of);
    std::unordered_set<File*> d;
    std::stack<File*> deps;
    deps.push(f);
    size_t index = 0;
    while (!deps.empty()) {
      File* dep = deps.top();
      deps.pop();
      pc->AddInput(dep);
      for (File* input : dep->dependencies)
        if (d.insert(input).second) deps.push(input);
      index++;
    }
    pc->Check();
    component.commands.push_back(pc);
  }
  if (!objects.empty()) {
    std::string command;
    boost::filesystem::path outputFile;
    PendingCommand* pc;
    if (component.type == "library") {
      outputFile = "lib/" + getLibNameFor(component);
      command = "ar rcs " + outputFile.string();
      for (auto& file : objects) {
        command += " " + file->path.string();
      }
      pc = new PendingCommand(command);
    } else {
      outputFile = "bin/" + getExeNameFor(component);
      command = "g++ -pthread -o " + outputFile.string();

      for (auto& file : objects) {
        command += " " + file->path.string();
      }
      command += " -Llib";
      std::vector<std::vector<Component*>> linkDeps = GetTransitiveAllDeps(component);
      std::reverse(linkDeps.begin(), linkDeps.end());
      for (auto d : linkDeps) {
        size_t index = 0;
        while (index < d.size()) {
          if (d[index]->isHeaderOnly()) {
            d[index] = d.back();
            d.pop_back();
          } else {
            ++index;
          }
        }
        if (d.empty()) continue;
        if (d.size() == 1 || (d.size() == 2 && (d[0] == &component || d[1] == &component))) {
          if (d[0] != &component) {
            command += " -l" + d[0]->root.string();
          } else if (d.size() == 2) {
            command += " -l" + d[1]->root.string();
          }
        } else {
          command += " -Wl,--start-group";
          for (auto& c : d) {
            if (c != &component) {
              command += " -l" + c->root.string();
            }
          }
          command += " -Wl,--end-group";
        }
      }
      pc = new PendingCommand(command);
      for (auto& d : linkDeps) {
        for (auto& c : d) {
          if (c != &component) {
            pc->AddInput(project.CreateFile(*c, "lib/" + getLibNameFor(*c)));
          }
        }
      }
    }
    File* libraryFile = project.CreateFile(component, outputFile);
    pc->AddOutput(libraryFile);
    for (auto& file : objects) {
      pc->AddInput(file);
    }
    pc->Check();
    component.commands.push_back(pc);
    if (component.type == "unittest") {
      command = outputFile.string();
      pc = new PendingCommand(command);
      outputFile += ".log";
      pc->AddInput(libraryFile);
      pc->AddOutput(project.CreateFile(component, outputFile.string()));
      pc->Check();
      component.commands.push_back(pc);
    }
  }
}


