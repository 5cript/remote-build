#pragma once

#include "forward.hpp"
#include "dll.hpp"

#include <vector>
#include <string>
#include <memory>
#include <functional>

namespace RemoteBuild
{
    struct DLL_EXPORT Difference
    {
        std::vector <std::string> clientUpload;
        std::vector <std::string> serverOnly;
    };

    struct DLL_EXPORT Context
    {
        std::unique_ptr <Project> project;
        std::unique_ptr <Config> config;

        Context();
        Context(std::unique_ptr <Project>&& project, std::unique_ptr <Config>&& config);
        ~Context();

        Context& operator=(Context&&);
        Context(Context&&);

        bool isValid() const;
    };

    /**
     *  Load a config from a file and create a context. Does authorization on the server already.
     */
    DLL_EXPORT Context makeContext(std::string const& configPath, bool ignoreUploadErrors);

    /**
     *  Do prebuild steps. Does nothing if there aren't any.
     */
    DLL_EXPORT void prebuildSteps(Context const& ctx);

    /**
     *  Clean server side.
     */
    DLL_EXPORT void clean(Context const& ctx);

    /**
     *  Create directory structure.
     *  @param ctx The necessary project context.
     *  @param clean Make directories even if they exist already.
     */
    DLL_EXPORT void makeDirectories(Context const& ctx, bool clean);

    /**
     *  Return files that should be uploaded.
     */
    DLL_EXPORT Difference getFileDifference(Context const& ctx);

    /**
     *  synchronize client with server.
     *  @param ctx The necessary project context.
     *  @param clean Upload all files no matter if difference or not.
     *  @param killServerFiles purge files that are on the server, but not the client?
     *  @param printDiffOnly Only print the difference, but do not perform deletions or uploads
     */
    DLL_EXPORT void synchronize(Context const& ctx, bool clean, bool killServerFiles, bool printDiffOnly);

    /**
     *  Do postbuild steps. Does nothing if there aren't any.
     */
    DLL_EXPORT void postbuildSteps(Context const& ctx);

    /**
     *  Run build process.
     */
    DLL_EXPORT void build(Context const& ctx);

    /**
     *  Busy waits for build to finish.
     *  @param ctx The necessary project context.
     *  @param sink The data sink that gets all data received from the server. The sink should not add newlines itself.
     *  @param buildTimeoutSec Maximum amount of seconds
     *
     *  @return ExitStatus of process.
     */
    DLL_EXPORT int waitForBuild(Context const& ctx, std::function <void(std::string const&)> const& sink, int buildTimeoutSec);
}
