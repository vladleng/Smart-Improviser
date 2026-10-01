#include "ara/SharedManualTensions.h"
#include "ara/PluginViewState.h"
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
    for (int level : {0, 1, 2, 3})
    {
        PluginViewState saved;
        saved.width = 1500; saved.height = 1200;
        saved.degreeLabels = true; saved.tensionFilter = level;
        saved.labels = {{"kept", 3}, {"cleared", 0}};
        juce::MemoryBlock data;
        saved.write(data);
        const auto restored = PluginViewState::read(data.getData(), static_cast<int>(data.getSize()));
        require(restored && restored->tensionFilter == level && restored->degreeLabels
            && restored->width == 1500 && restored->height == 1200
            && restored->labels == saved.labels, "V5 host state roundtrip preserves filter and existing preferences");
        // Losing the optional tail falls back to All, not an arbitrary level.
        const auto shortState = PluginViewState::read(data.getData(), static_cast<int>(data.getSize()) - 1);
        require(shortState && shortState->tensionFilter == 0, "missing filter defaults to All");
        auto* bytes = static_cast<unsigned char*>(data.getData());
        bytes[data.getSize() - 1] = 99;
        require(PluginViewState::read(data.getData(), static_cast<int>(data.getSize()))->tensionFilter == 0,
            "invalid stored filter defaults to All");
    }
    for (int version : {2, 3, 4})
    {
        juce::MemoryBlock data;
        juce::MemoryOutputStream stream(data, false);
        stream.writeString("SmartImproviserARAStateV" + juce::String(version));
        stream.writeInt(1120); stream.writeInt(1000); stream.writeInt(1);
        stream.writeString("old-label"); stream.writeInt(2);
        if (version >= 3) stream.writeByte(1);
        const auto state = PluginViewState::read(data.getData(), static_cast<int>(data.getSize()));
        require(state && state->tensionFilter == 0 && state->labels.at("old-label") == 2
            && state->degreeLabels == (version >= 3), "legacy V2/V3/V4 defaults to All and preserves preferences");
    }
    require(!PluginViewState::read(nullptr, 0), "empty state is rejected");
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
