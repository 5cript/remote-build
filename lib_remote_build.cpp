#include "lib_remote_build.hpp"

#include "config.hpp"
#include "project.hpp"

#include <sstream>
#include <thread>
#include <chrono>
#include <fstream>

namespace RemoteBuild
{
//#####################################################################################################################
    struct GlobbingEssentials
    {
        std::vector <std::string> fileFilter;
        std::vector <std::string> dirFilter;
        std::vector <std::string> masks;

        GlobbingEssentials(Context const& ctx)
            : fileFilter{}
            , dirFilter{}
            , masks{"*.?pp"}
        {
            if (ctx.config->fileFilter)
                fileFilter = ctx.config->fileFilter.get();
            if (ctx.config->directoryFilter)
                dirFilter = ctx.config->directoryFilter.get();
            if (ctx.config->globExpressions)
                masks = ctx.config->globExpressions.get();
        }
    };
//#####################################################################################################################
    Config loadConfig(std::string const& configPath)
    {
        std::ifstream reader(configPath, std::ios_base::binary);
        if (!reader.good())
            return {};

        return Internal::loadConfig(reader);
    }
//#####################################################################################################################
    bool Context::isValid() const
    {
        return !config->id.empty() && !config->serverAddress.empty() && !config->local.empty();
    }
//---------------------------------------------------------------------------------------------------------------------
    Context::Context(std::unique_ptr <Project>&& project, std::unique_ptr <Config>&& config)
        : project{std::move(project)}
        , config{std::move(config)}
    {
    }
//---------------------------------------------------------------------------------------------------------------------
    Context::Context() = default;
//---------------------------------------------------------------------------------------------------------------------
    Context& Context::operator=(Context&&) = default;
//---------------------------------------------------------------------------------------------------------------------
    Context::Context(Context&&) = default;
//---------------------------------------------------------------------------------------------------------------------
    Context::~Context() = default;
//#####################################################################################################################
    DLL_EXPORT Context makeContext(std::string const& configPath, bool ignoreUploadErrors)
    {
        auto config = loadConfig(configPath);
        if (config.id.empty() || config.serverAddress.empty() || config.local.empty())
            throw std::invalid_argument("Config is invalid");

        return {
            std::make_unique <Project> (config.local, config.id, config.serverAddress, config.user, config.password, ignoreUploadErrors),
            std::make_unique <Config> (config) // by copy, because order of execution might not be determined.
        };
    }
//---------------------------------------------------------------------------------------------------------------------
    DLL_EXPORT void prebuildSteps(Context const& ctx)
    {
        if (ctx.config->localPreUploadSteps)
            system(ctx.config->localPreUploadSteps.get().c_str());
    }
//---------------------------------------------------------------------------------------------------------------------
    DLL_EXPORT void clean(Context const& ctx)
    {
        ctx.project->clean();
    }
//---------------------------------------------------------------------------------------------------------------------
    DLL_EXPORT void makeDirectories(Context const& ctx, bool clean)
    {
        std::vector <std::string> directoryFilter;
        if (ctx.config->directoryFilter)
            directoryFilter = ctx.config->directoryFilter.get();

        ctx.project->createDirectoryStructure(
            directoryFilter,
            !clean
        );
    }
//---------------------------------------------------------------------------------------------------------------------
    DLL_EXPORT Difference getFileDifference(Context const& ctx)
    {
        GlobbingEssentials essentials{ctx};
        auto pair = ctx.project->getDiff(essentials.fileFilter, essentials.dirFilter, essentials.masks);
        return {
            pair.first,
            pair.second
        };
    }
//---------------------------------------------------------------------------------------------------------------------
    DLL_EXPORT void synchronize(Context const& ctx, bool clean, bool killServerFiles, bool printDiffOnly)
    {
        GlobbingEssentials essentials{ctx};
        ctx.project->unidirectional_sychronize(
            essentials.fileFilter,
            essentials.dirFilter,
            !clean,
            essentials.masks,
            printDiffOnly,
            killServerFiles
        );
    }
//---------------------------------------------------------------------------------------------------------------------
    DLL_EXPORT void postbuildSteps(Context const& ctx)
    {
        if (ctx.config->localPostUploadSteps)
            system(ctx.config->localPostUploadSteps.get().c_str());
    }
//---------------------------------------------------------------------------------------------------------------------
    DLL_EXPORT void build(Context const& ctx)
    {
        ctx.project->build();
    }
//---------------------------------------------------------------------------------------------------------------------
    DLL_EXPORT int waitForBuild(Context const& ctx, std::function <void(std::string const&)> const& sink, int buildTimeoutSec)
    {
        sink("waiting for build to finish...\n");
        std::this_thread::sleep_for(std::chrono::seconds{1});
        std::ofstream writer{ctx.config->log, std::ios_base::binary};
        for (int i = 0; buildTimeoutSec == 0 || i < buildTimeoutSec; i += 1)
        {
            std::stringstream sstr;
            ctx.project->saveBuildLog(sstr, writer);
            sink(sstr.str());

            if (!ctx.project->isBuilding())
            {
                std::stringstream sstr;
                ctx.project->saveBuildLog(sstr, writer);
                sink(sstr.str());
                return ctx.project->getExitStatus();
            }
            std::this_thread::sleep_for(std::chrono::seconds{1});
        }
        sink("waiting for build to finish timed out, here is the build log so far\n");
        std::stringstream sstr;
        ctx.project->saveBuildLog(sstr, writer);
        sink(sstr.str());
        return 255;
    }
//#####################################################################################################################
}
