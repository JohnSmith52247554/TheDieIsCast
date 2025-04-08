#include "File.h"

#ifdef __APPLE__
#include <CoreFoundation/CoreFoundation.h>
#endif

namespace File 
{

    namespace fs = std::filesystem;

    fs::path getExecutableDir()
    {
#if defined(WIN32)
        char path[MAX_PATH];
        GetModuleFileNameA(NULL, path, MAX_PATH);
        return fs::path(path).parent_path();
#elif defined(__APPLE__)
        uint32_t size = 0;
        _NSGetExecutablePath(nullptr, &size);
        std::string buffer(size, '\0');
        _NSGetExecutablePath(buffer.data(), &size);
        auto exe_path = fs::path(buffer).parent_path().parent_path();
        return exe_path / "Resources";
#else
        char path[PATH_MAX];
        ssize_t len = ::readlink("/proc/self/exe", path, sizeof(path) - 1);
        if (len != -1)
        {
            path[len] = '\0';
            return fs::path(path).parent_path();
        }
        else 
        {
            throw std::logic_error("Failed to get executable path");
        }
#endif
    }

#ifdef __APPLE__
    // 获取资源目录的路径
    fs::path getResourceDirectory()
    {
        // 获取主Bundle
        CFBundleRef mainBundle = CFBundleGetMainBundle();
        if (!mainBundle)
        {
            std::cerr << "Failed to get main bundle" << std::endl;
            return "";
        }

        CFURLRef resourcesURL = CFBundleCopyResourcesDirectoryURL(mainBundle);
        if (!resourcesURL)
        {
            std::cerr << "Failed to get resources directory URL" << std::endl;
            return "";
        }

        char path[PATH_MAX];
        if (!CFURLGetFileSystemRepresentation(resourcesURL, true, (UInt8*)path, PATH_MAX))
        {
            std::cerr << "Failed to convert URL to file system representation" << std::endl;
            CFRelease(resourcesURL);
            return "";
        }

        CFRelease(resourcesURL);

        auto resources_path = fs::path(path).parent_path() / "Resources";
        std::cout << resources_path << std::endl;
        return resources_path;
    }
#endif

}