// SPDX-License-Identifier: AGPL-3.0-or-later
#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "PluginProcessor.h"
#include "BrowserIncrementalSearch.h"
#include <functional>
#include <vector>
#include <memory>

namespace biscot {
// Keyboard-first, in-plugin library browser. No system file chooser, external
// dependency files or mutation of the live banks during preview.
class TrackerLibraryBrowser final : public juce::Component {
public:
 enum class Mode { patterns, songs };
 explicit TrackerLibraryBrowser(BiscotProcessor&);
 void open(Mode);
 void close(bool imported=false,const juce::String& result={});
 bool keyPressed(const juce::KeyPress&) override;
 void resized() override;
 void paint(juce::Graphics&) override;
 std::unique_ptr<juce::AccessibilityHandler> createAccessibilityHandler() override;
 std::function<void(Mode,bool,const juce::String&)> onClose;
private:
 struct FileList final : juce::ComboBox {
  explicit FileList(TrackerLibraryBrowser& p):owner(p){}
  bool keyPressed(const juce::KeyPress& k) override {return owner.handle(k);}
  TrackerLibraryBrowser& owner;
 } list;
 struct Entry {juce::File path;bool directory=false;};
 BiscotProcessor& processor;
 juce::Label heading,help;
 Mode mode=Mode::patterns;
 juce::File root,folder;
 std::vector<Entry> entries;
 BrowserIncrementalSearch search;
 int selected=0,previewSteps=0,archiveSongSlot=0; // 0 = full original file
 bool refreshing=false,automaticPreview=false,confirming=false;
 juce::File confirmationFile;
 juce::File previewCacheFile;
 std::unique_ptr<TrackerSongBank> previewSongs;
 std::unique_ptr<TrackerPatternBank> previewPatterns;
 void scan(bool announceSummary=false);
 void navigate(int);
 void parentFolder();
 void enterFolder();
 void selectionChanged(bool speak,bool preview);
 void stopPreview();
 void preview();
 void activate();
 bool importSelected(const juce::File&,juce::String&);
 bool handle(const juce::KeyPress&);
 juce::String entryLabel(int) const;
 void announce(const juce::String&);
 JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TrackerLibraryBrowser)
};
}
