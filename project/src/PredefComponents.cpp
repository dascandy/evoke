#include "PredefComponents.h"

#include "Component.h"

#include <map>
#include <string>

static std::map<std::string, Component> predefComponents = {
    {"boost_system", Component("boost_system", true)},
    {"boost_filesystem", Component("boost_filesystem", true)},
    {"boost_process", Component("boost_process")},
    {"EGL", Component("EGL", true)},
    {"GLESv1_CM", Component("GLESv1_CM ", true)},
    {"dl", Component("dl", true)},
    {"log", Component("log ", true)},
    {"android", Component("android", true)},
    {"crypto", Component("crypto", true)},
    {"ssl", Component("ssl", true)},
    {"z", Component("z ", true)},
    {"mysqlclient", Component("mysqlclient", true)},
    {"SDL2", Component("SDL2", true)},
    {"GL", Component("GL", true)},
    {"GLEW", Component("GLEW", true)},
};

std::map<std::string, Component *> PredefComponentList()
{
    std::map<std::string, Component *> list;
    list["boost/filesystem.hpp"] = &predefComponents.find("boost_filesystem")->second;
    list["boost/filesystem/fstream.hpp"] = &predefComponents.find("boost_filesystem")->second;
    list["boost/process.hpp"] = &predefComponents.find("boost_process")->second;
    predefComponents.find("boost_filesystem")->second.pubDeps.insert(&predefComponents.find("boost_system")->second);

    // Enable Android app
    list["egl/egl.h"] = &predefComponents.find("EGL")->second;
    list["gles/gl.h"] = &predefComponents.find("GLESv1_CM")->second;
    list["dlfcn.h"] = &predefComponents.find("dl")->second;

    list["android/log.h"] = &predefComponents.find("log")->second;

    list["android/sensor.h"] = &predefComponents.find("android")->second;
    list["android/native_activity.h"] = &predefComponents.find("android")->second;
    list["android/looper.h"] = &predefComponents.find("android")->second;
    list["android/configuration.h"] = &predefComponents.find("android")->second;

    // Enable ssl/md5 use
    predefComponents.find("ssl")->second.pubDeps.insert(&predefComponents.find("crypto")->second);
    list["md5.h"] = &predefComponents.find("crypto")->second;
    list["openssl/conf.h"] = &predefComponents.find("ssl")->second;
    list["openssl/ssl.h"] = &predefComponents.find("ssl")->second;
    list["openssl/engine.h"] = &predefComponents.find("ssl")->second;
    list["openssl/dh.h"] = &predefComponents.find("ssl")->second;
    list["openssl/err.h"] = &predefComponents.find("ssl")->second;
    list["openssl/rsa.h"] = &predefComponents.find("ssl")->second;
    list["openssl/x509v3.h"] = &predefComponents.find("ssl")->second;
    list["openssl/x509_vfy.h"] = &predefComponents.find("ssl")->second;

    list["zlib.h"] = &predefComponents.find("z")->second;

    list["mysql/mysql.h"] = &predefComponents.find("mysqlclient")->second;

    list["sdl2/sdl.h"] = &predefComponents.find("SDL2")->second;
    list["sdl2/sdl_opengl.h"] = &predefComponents.find("GL")->second;
    list["gl/glew.h"] = &predefComponents.find("GLEW")->second;

    return list;
}
