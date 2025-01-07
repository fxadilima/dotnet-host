#include <nethost.h>
#include <coreclr_delegates.h>
#include <hostfxr.h>
#include <iostream>
#include <string>
#include <cstdio>
#include <cstdlib>
#include <assert.h>
#include <chrono>
#include <iostream>
#include <thread>
#include <vector>

#include <dlfcn.h>
#include <limits.h>


#define STR(s) s
#define CH(c) c
#define DIR_SEPARATOR '/'
#define MAX_PATH PATH_MAX

#define string_compare strcmp

using string_t = std::basic_string<char_t>;

namespace
{
    // Globals to hold hostfxr exports
    hostfxr_initialize_for_dotnet_command_line_fn init_for_cmd_line_fptr;
    hostfxr_initialize_for_runtime_config_fn init_for_config_fptr;
    hostfxr_get_runtime_delegate_fn get_delegate_fptr;
    hostfxr_run_app_fn run_app_fptr;
    hostfxr_close_fn close_fptr;

    // Using the nethost library, discover the location of hostfxr and get exports
    bool load_hostfxr(const char_t *app);
    load_assembly_and_get_function_pointer_fn get_dotnet_load_assembly(const char_t *assembly);

    std::string dllname;
    std::string dotnetapp_path;
    std::string dotnetlib_path;
    std::string dotnet_type;
    int nargs = 1;
    char** ppargs = nullptr;
    int rundll(const string_t& root_path);
}


int main(int argc, char** argv)
{
    nargs = argc;
    ppargs = argv;
    if (argc < 4)
    {
        std::cerr << "usage: " <<
            argv[0] << " [dotnetapp_path] [dllname] [dotnet_typename]\n";
        return 0;
    }

    char_t hostpath[MAX_PATH];
    auto resolved = realpath(argv[0], hostpath);

    dllname = argv[2];
    dotnetapp_path = argv[1];
    dotnetlib_path = dotnetapp_path + "/" + dllname + ".dll";
    dotnet_type = argv[3];
    dotnet_type += ", " + dllname;

    std::cerr << "\x1b[1;33m[main]\x1b[0m argv[0] = " << argv[0] << "\n"
        << "resolved => " << resolved << "\n";

    string_t rootpath = hostpath;
    auto pos = rootpath.find_last_of(DIR_SEPARATOR);
    rootpath = rootpath.substr(0, pos + 1);

    std::cerr << "\x1b[1;33m[main]\x1b[0m find rootpath: " << rootpath << "\n";

    return rundll(rootpath);
}

namespace
{
    load_assembly_and_get_function_pointer_fn get_dotnet_load_assembly(const char_t *config_path);
    int rundll(const string_t& root_path)
    {
        //
        // STEP 1: Load HostFxr and get exported hosting functions
        //
        if (!load_hostfxr(nullptr))
        {
            assert(false && "Failure: load_hostfxr()");
            return EXIT_FAILURE;
        }

        std::cerr << "[main] load_hostfxr() succeeded...\n";

        //
        // STEP 2: Initialize and start the .NET Core runtime
        //
        const string_t config_path = root_path + dotnetapp_path + "/" + dllname + ".runtimeconfig.json";
        load_assembly_and_get_function_pointer_fn load_assembly_and_get_function_pointer = nullptr;
        load_assembly_and_get_function_pointer = get_dotnet_load_assembly(config_path.c_str());
        assert(load_assembly_and_get_function_pointer != nullptr && "Failure: get_dotnet_load_assembly()");

        // UnmanagedCallersOnly
        typedef int (CORECLR_DELEGATE_CALLTYPE *init_fn)(int argc, intptr_t argv);
        init_fn custom = nullptr;
        std::string libpath = root_path + dotnetlib_path; 

        /**
         *  rc = get_function_pointer(
            STR("App, App"),
            STR("IsWaiting"),
            UNMANAGEDCALLERSONLY_METHOD,
            nullptr, nullptr, (void**)&is_waiting);

         */
        int rc = load_assembly_and_get_function_pointer(
            libpath.c_str(),
            dotnet_type.c_str(),
            STR("Run") /*method_name*/,
            UNMANAGEDCALLERSONLY_METHOD,
            nullptr,
            (void**)&custom);

        if (!custom)
        {
            std::cerr << "\x1b[1;33mrundll\x1b[1;31m FAILED\x1b[0m: using dotnetlib_path = " << libpath
                << "\n\x1b[1;32mdotnet_type\x1b[0m = \x1b[1;34m" << dotnet_type << "\x1b[0m\n";
        }
        assert(rc == 0 && custom != nullptr && "Failure: load_assembly_and_get_function_pointer()");
        int nResult = custom(nargs, (intptr_t)ppargs);

        std::cerr << "\x1b[1;33m[rundll]\x1b[0m The .NET Application has exited with return value: " << nResult << "\n";
        return nResult;
    }

    void* load_library(const char* path)
    {
        void *h = dlopen(path, RTLD_LAZY | RTLD_LOCAL);
        assert(h != nullptr);
        return h;
    }

    void *get_export(void *h, const char *name)
    {
        void *f = dlsym(h, name);
        assert(f != nullptr);
        return f;
    }

    // <SnippetLoadHostFxr>
    // Using the nethost library, discover the location of hostfxr and get exports
    bool load_hostfxr(const char* app)
    {
        get_hostfxr_parameters params { sizeof(get_hostfxr_parameters), app, nullptr };

        // Pre-allocate a large buffer for the path to hostfxr
        char_t buffer[MAX_PATH];
        size_t buffer_size = sizeof(buffer) / sizeof(char_t);
        int rc = get_hostfxr_path(buffer, &buffer_size, &params);
        if (rc != 0)
            return false;

        std::cerr << "get_hostfxr_path returns: " << buffer << "\n";

        // Load hostfxr and get desired exports
        // NOTE: The .NET Runtime does not support unloading any of its native libraries. Running
        // dlclose/FreeLibrary on any .NET libraries produces undefined behavior.
        void *lib = load_library(buffer);
        init_for_cmd_line_fptr = (hostfxr_initialize_for_dotnet_command_line_fn)get_export(lib, "hostfxr_initialize_for_dotnet_command_line");
        init_for_config_fptr = (hostfxr_initialize_for_runtime_config_fn)get_export(lib, "hostfxr_initialize_for_runtime_config");
        get_delegate_fptr = (hostfxr_get_runtime_delegate_fn)get_export(lib, "hostfxr_get_runtime_delegate");
        run_app_fptr = (hostfxr_run_app_fn)get_export(lib, "hostfxr_run_app");
        close_fptr = (hostfxr_close_fn)get_export(lib, "hostfxr_close");

        return (init_for_config_fptr && get_delegate_fptr && close_fptr);
    }

    // <SnippetInitialize>
    // Load and initialize .NET Core and get desired function pointer for scenario
    load_assembly_and_get_function_pointer_fn get_dotnet_load_assembly(const char_t *config_path)
    {
        // Load .NET Core
        void *load_assembly_and_get_function_pointer = nullptr;
        hostfxr_handle cxt = nullptr;
        int rc = init_for_config_fptr(config_path, nullptr, &cxt);
        if (rc != 0 || cxt == nullptr)
        {
            std::cerr << "Init failed: " << std::hex << std::showbase << rc << std::endl;
            close_fptr(cxt);
            return nullptr;
        }

        // Get the load assembly function pointer
        rc = get_delegate_fptr(
            cxt,
            hdt_load_assembly_and_get_function_pointer,
            &load_assembly_and_get_function_pointer);
        if (rc != 0 || load_assembly_and_get_function_pointer == nullptr)
            std::cerr << "Get delegate failed: " << std::hex << std::showbase << rc << std::endl;

        close_fptr(cxt);
        return (load_assembly_and_get_function_pointer_fn)load_assembly_and_get_function_pointer;
    }
}
