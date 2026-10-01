#include "ara/SharedManualTensions.h"
#include <cstdlib>
#include <iostream>
#include <thread>

static void require(bool condition, const char* message)
{
    if (!condition) { std::cerr << message << '\n'; std::exit(1); }
}

int main(int argc, char** argv)
{
    if (argc == 4)
    {
        SharedManualTensions writer(juce::File(juce::String::fromUTF8(argv[1])));
        for (int i = 0; i < 20; ++i)
            require(writer.assign(std::string(argv[2]) + std::to_string(i), 2), "child writer");
        return 0;
    }
    const auto dir = juce::File::getSpecialLocation(juce::File::tempDirectory)
        .getNonexistentChildFile("SmartImproviserSharedTensions", "", false);
    const auto file = dir.getChildFile("nested/preferences.xml");
    {
        SharedManualTensions projectA(file), projectB(file);
        require(projectA.snapshot().has_value() && projectA.snapshot()->empty(), "new profile is empty");
        require(projectA.assign("relative-v1:minor-V", 2), "save A");
        require(projectB.snapshot()->at("relative-v1:minor-V") == 2, "B sees A without host save");
        require(projectB.assign("relative-v1:major-V", 1), "save B");
        require(projectA.snapshot()->size() == 2, "B preserves A's keys");
        require(projectB.assign("relative-v1:minor-V", 3), "update B");
        require(projectA.importMissing({{"relative-v1:minor-V", 1}, {"legacy-other", 2}}), "migrate old project");
        require(projectA.snapshot()->at("relative-v1:minor-V") == 3, "old project cannot revert latest assignment");
        require(projectA.snapshot()->at("legacy-other") == 2, "old project contributes missing assignment");
        require(projectB.assign("relative-v1:minor-V", 0), "clear shared label");
        require(projectA.importMissing({{"relative-v1:minor-V", 2}}), "load old label after clear");
        require(projectB.snapshot()->at("relative-v1:minor-V") == 0, "cleared label is not resurrected");
        require(projectA.importMissing({}), "empty project load");
        require(projectB.snapshot()->at("relative-v1:major-V") == 1, "empty project cannot clear preferences");
        require(!projectA.assign("", 1) && !projectA.assign("bad", 4), "reject invalid assignment");
        require(projectA.assign("UTF8:минор & <source>", 2), "UTF8/XML key");
        require(projectB.snapshot()->at("UTF8:минор & <source>") == 2, "UTF8/XML roundtrip");
        std::thread a([&] { for (int i = 0; i < 20; ++i) require(projectA.assign("a" + std::to_string(i), 1), "writer A"); });
        std::thread b([&] { for (int i = 0; i < 20; ++i) require(projectB.assign("b" + std::to_string(i), 3), "writer B"); });
        a.join(); b.join();
        require(projectA.snapshot()->size() == 44, "concurrent instances do not lose assignments");
    }
    juce::ChildProcess childA, childB;
    const auto executable = juce::File::getSpecialLocation(juce::File::currentExecutableFile).getFullPathName();
    require(childA.start(juce::StringArray{executable, file.getFullPathName(), "processA", "write"}), "start process A");
    require(childB.start(juce::StringArray{executable, file.getFullPathName(), "processB", "write"}), "start process B");
    require(childA.waitForProcessToFinish(20000) && childB.waitForProcessToFinish(20000), "process writers finish");
    require(childA.getExitCode() == 0 && childB.getExitCode() == 0, "process writers succeed");
    SharedManualTensions restarted(file);
    require(restarted.snapshot()->size() == 84, "concurrent host processes do not lose labels");
    require(restarted.snapshot()->at("relative-v1:major-V") == 1, "profile survives restart");
    require(restarted.snapshot()->at("relative-v1:minor-V") == 0, "clear survives restart");
    require(file.replaceWithText("broken XML"), "corrupt test file");
    require(!restarted.snapshot() && !restarted.assign("new", 1), "corrupt file is not overwritten");
    require(file.loadFileAsString() == "broken XML", "corrupt bytes preserved");
    require(file.replaceWithText("<SmartImproviserManualTensions version=\"99\"/>"), "future file");
    require(!restarted.importMissing({{"new", 1}}), "future schema is not overwritten");
    const auto blocker = dir.getChildFile("blocker");
    require(blocker.replaceWithText("not a directory"), "blocked path");
    SharedManualTensions unwritable(blocker.getChildFile("preferences.xml"));
    require(!unwritable.assign("new", 1), "write failure is reported");
    require(dir.deleteRecursively(), "cleanup");
    std::cout << "Shared tension persistence checks passed\n";
}
