#include "Component.h"
#include "File.h"
#include "PendingCommand.h"
#include "Project.h"
#include "Toolset.h"
#include "dotted.h"

#include <fstream>
#include <map>
#include <set>
#include <stack>

struct androidconfig
{
    struct target
    {
        std::set<std::string> systemincludepaths;
        std::set<std::string> systemlibrarypaths;
        std::string ccflags;
        std::string ld;
        std::string sofoldername;
    };
    std::string compiler(const target &t)
    {
        std::string accum = ndkpath + clangpp + " " + t.ccflags + " --sysroot " + ndkpath + sysroot;
        for(auto &p : t.systemincludepaths)
        {
            accum += " -I" + ndkpath + p;
        }
        return accum;
    }
    std::string linker(const target &t)
    {
        std::string accum = ndkpath + t.ld;
        for(auto &p : t.systemlibrarypaths)
        {
            accum += " -L" + ndkpath + p;
        }
        return accum + " " + ldflags;
    }
    std::string manifest(const std::string &appName)
    {
        return manifest1 + appName + manifest2 + appName + manifest3 + appName + manifest4;
    }
    std::string aapt(const std::string &apkName, const std::string &manifestFile)
    {
        return "aapt package -f -I " + android_jar + " -M " + manifestFile + " -S res -F apk/unsigned_" + apkName + ".apk so";
    }
    std::string apksigner(const std::string &apkName)
    {
        return "apksigner sign --ks /home/pebi/.ssh/keystore.jks --ks-key-alias androidkey --ks-pass pass:android --key-pass pass:android --out apk/" + apkName + ".apk apk/unsigned_" + apkName + ".apk";
    }
    const std::string ndkpath = "/home/pebi/android-ndk-r18";
    const std::string clangpp = "/toolchains/llvm/prebuilt/linux-x86_64/bin/clang++";
    const std::string sysroot = "/sysroot";
    const std::string ldflags = "-shared -lc -llog -landroid -lc++_static -lc++abi";
    const std::string android_jar = "/home/pebi/Android/Sdk/platforms/android-25/android.jar";
    std::map<std::string, target> targets = {{"aarch64", {
                                                             {"/sources/cxx-stl/llvm-libc++/include", "/sysroot/usr/include/aarch64-linux-android"},
                                                             {"/platforms/android-28/arch-arm64/usr/lib", "/sources/cxx-stl/llvm-libc++/libs/arm64-v8a"},
                                                             "-std=c++17 -target aarch64-linux-android -DANDROID",
                                                             "/toolchains/aarch64-linux-android-4.9/prebuilt/linux-x86_64/bin/aarch64-linux-android-ld",
                                                             "arm64-v8a",
                                                         }},
                                             {"armv7", {
                                                           {"/sources/cxx-stl/llvm-libc++/include", "/sysroot/usr/include/arm-linux-androideabi"},
                                                           {"/platforms/android-28/arch-arm/usr/lib", "/sources/cxx-stl/llvm-libc++/libs/armeabi-v7a", "/toolchains/arm-linux-androideabi-4.9/prebuilt/linux-x86_64/arm-linux-androideabi/lib"},
                                                           "-std=c++17 -target armv7-linux-android -DANDROID",
                                                           "/toolchains/arm-linux-androideabi-4.9/prebuilt/linux-x86_64/bin/arm-linux-androideabi-ld",
                                                           "armeabi-v7a",
                                                       }},
                                             {"x86", {
                                                         {"/sources/cxx-stl/llvm-libc++/include", "/sysroot/usr/include/i686-linux-android"},
                                                         {"/platforms/android-28/arch-x86/usr/lib", "/sources/cxx-stl/llvm-libc++/libs/x86"},
                                                         "-std=c++17 -target i686-linux-android -DANDROID",
                                                         "/toolchains/x86-4.9/prebuilt/linux-x86_64/bin/i686-linux-android-ld",
                                                         "x86",
                                                     }},
                                             {"x86_64", {
                                                            {"/sources/cxx-stl/llvm-libc++/include", "/sysroot/usr/include/x86_64-linux-android"},
                                                            {"/platforms/android-28/arch-x86_64/usr/lib64", "/sources/cxx-stl/llvm-libc++/libs/x86_64"},
                                                            "-std=c++17 -target x86_64-linux-android -DANDROID",
                                                            "/toolchains/x86_64-4.9/prebuilt/linux-x86_64/bin/x86_64-linux-android-ld",
                                                            "x86_64",
                                                        }}};
    static constexpr const char *manifest1 =
        "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
        "<!-- BEGIN_INCLUDE(manifest) -->\n"
        "<manifest xmlns:android=\"http://schemas.android.com/apk/res/android\"\n"
        "          package=\"com.evoke.";

    static constexpr const char *manifest2 =
        "\"\n"
        "          android:versionCode=\"1\"\n"
        "          android:versionName=\"1.0\">\n"
        "\n"
        "  <!-- This .apk has no Java code itself, so set hasCode to false. -->\n"
        "  <application\n"
        "      android:allowBackup=\"false\"\n"
        "      android:fullBackupContent=\"false\"\n"
        "      android:hasCode=\"false\">\n"
        "\n"
        "    <!-- Our activity is the built-in NativeActivity framework class.\n"
        "         This will take care of integrating with our NDK code. -->\n"
        "    <activity android:name=\"android.app.NativeActivity\"\n"
        "              android:label=\"";

    static constexpr const char *manifest3 =
        "\"\n"
        "              android:configChanges=\"orientation|keyboardHidden\">\n"
        "      <!-- Tell NativeActivity the name of our .so -->\n"
        "      <meta-data android:name=\"android.app.lib_name\"\n"
        "                 android:value=\"";

    static constexpr const char *manifest4 =
        "\" />\n"
        "      <intent-filter>\n"
        "        <action android:name=\"android.intent.action.MAIN\" />\n"
        "        <category android:name=\"android.intent.category.LAUNCHER\" />\n"
        "      </intent-filter>\n"
        "    </activity>\n"
        "  </application>\n"
        "\n"
        "</manifest>\n"
        "<!-- END_INCLUDE(manifest) -->\n";
};

std::string AndroidToolset::getObjNameFor(const File &file)
{
    return file.path.generic_string() + ".o";
}

std::string AndroidToolset::getExeNameFor(const Component &component)
{
    return "lib" + getNameFor(component) + ".so";
}

std::string AndroidToolset::getLibNameFor(const Component &component)
{
    return "lib" + getNameFor(component) + ".a";
}

void AndroidToolset::SetParameter(const std::string& key, const std::string& value) {}

void AndroidToolset::CreateCommandsForUnity(Project &project) {}

void AndroidToolset::CreateCommandsFor(Project &project)
{
    for(auto &p : project.components)
    {
        auto &component = p.second;
        std::string includes;
        for(auto &d : getIncludePathsFor(component))
        {
            includes += " -I" + d;
        }

        androidconfig config;
        std::vector<File *> libraries;
        for(auto &p : config.targets)
        {
            filesystem::path outputFolder = component.root;
            std::vector<File *> objects;
            for(auto &f : component.files)
            {
                if(!File::isTranslationUnit(f->path))
                    continue;
                filesystem::path outputFile = ("obj/" + p.first) / outputFolder / (f->path.string().substr(component.root.string().size()) + ".o");
                File *of = project.CreateFile(component, outputFile);
                std::shared_ptr<PendingCommand> pc = std::make_shared<PendingCommand>(config.compiler(p.second) + " -c -o " + outputFile.string() + " " + f->path.string() + " " + includes);
                objects.push_back(of);
                pc->AddOutput(of);
                std::unordered_set<File *> d;
                std::stack<File *> deps;
                deps.push(f);
                size_t index = 0;
                while(!deps.empty())
                {
                    File *dep = deps.top();
                    deps.pop();
                    pc->AddInput(dep);
                    for(File *input : dep->dependencies)
                        if(d.insert(input).second)
                            deps.push(input);
                    index++;
                }
                pc->Check();
                component.commands.push_back(pc);
            }
            if(!objects.empty())
            {
                std::string command;
                filesystem::path outputFile;
                std::shared_ptr<PendingCommand> pc;
                if(component.type == "library")
                {
                    outputFile = "lib/" + p.first + "/" + getLibNameFor(component);
                    command = "ar rcs " + outputFile.string();
                    for(auto &file : objects)
                    {
                        command += " " + file->path.string();
                    }
                    pc = std::make_shared<PendingCommand>(command);
                }
                else
                {
                    outputFile = "so/lib/" + p.second.sofoldername + "/" + getExeNameFor(component);
                    command = config.linker(p.second) + " -o " + outputFile.string();

                    for(auto &file : objects)
                    {
                        command += " " + file->path.string();
                    }
                    command += " -Llib";
                    std::vector<std::vector<Component *>> linkDeps = GetTransitiveAllDeps(component);
                    std::reverse(linkDeps.begin(), linkDeps.end());
                    for(auto d : linkDeps)
                    {
                        size_t index = 0;
                        while(index < d.size())
                        {
                            if(d[index] == &component || d[index]->isHeaderOnly())
                            {
                                d[index] = d.back();
                                d.pop_back();
                            }
                            else
                            {
                                ++index;
                            }
                        }
                        if(d.empty())
                            continue;
                        if(d.size() == 1)
                        {
                            command += " -l" + d[0]->root.string();
                        }
                        else
                        {
                            command += " -Wl,--start-group";
                            for(auto &c : d)
                            {
                                if(!c->isHeaderOnly())
                                {
                                    command += " -l" + c->root.string();
                                }
                            }
                            command += " -Wl,--end-group";
                        }
                    }
                    pc = std::make_shared<PendingCommand>(command);
                    for(auto &d : linkDeps)
                    {
                        for(auto &c : d)
                        {
                            if(c != &component && !c->isHeaderOnly())
                            {
                                pc->AddInput(project.CreateFile(*c, "lib/" + p.first + "/" + getLibNameFor(*c)));
                            }
                        }
                    }
                }
                File *libraryFile = project.CreateFile(component, outputFile);
                pc->AddOutput(libraryFile);
                libraries.push_back(libraryFile);
                for(auto &file : objects)
                {
                    pc->AddInput(file);
                }
                pc->Check();
                component.commands.push_back(pc);
            }
        }
        if(component.type != "library")
        {
            // Find manifest, if not then make one
            std::string manifest = "AndroidManifest.xml";
            if(!filesystem::is_regular_file(manifest))
            {
                std::ofstream(manifest)
                    << androidconfig::manifest1 << getNameFor(component)
                    << androidconfig::manifest2 << getNameFor(component)
                    << androidconfig::manifest3 << getNameFor(component)
                    << androidconfig::manifest4;
            }

            // Create apk from manifest & shared libraries
            std::string outputName = component.root.filename().string();
            std::shared_ptr<PendingCommand> pc = std::make_shared<PendingCommand>(config.aapt(outputName, manifest));
            File *uapkfile = project.CreateFile(component, "apk/unsigned_" + outputName + ".apk");
            pc->AddOutput(uapkfile);
            for(auto &file : libraries)
            {
                pc->AddInput(file);
            }
            pc->Check();
            component.commands.push_back(pc);

            // create signed apk from unsigned apk
            pc = std::make_shared<PendingCommand>(config.apksigner(outputName));
            File *apkfile = project.CreateFile(component, "apk/" + outputName + ".apk");
            pc->AddOutput(apkfile);
            pc->AddInput(uapkfile);
            pc->Check();
            component.commands.push_back(pc);
        }
    }
}
