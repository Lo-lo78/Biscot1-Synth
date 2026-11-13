// SPDX-License-Identifier: AGPL-3.0-or-later
#include "PresetBrowser.h"
#include "ScreenReaderAnnouncer.h"
#include "PluginProcessor.h"
#include <algorithm>

namespace biscot
{
namespace
{
constexpr int applicationMenuKeyCode = 0x5d;
constexpr int applicationMenuExtendedKeyCode = 0x1005d;
}

PresetBrowser::PresetBrowser (PresetManager& p) : manager (p)
{
    setWantsKeyboardFocus (false);
    folderLabel.setJustificationType (juce::Justification::centredLeft);
    folderLabel.setAccessible (false);
    folderLabel.setColour (juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible (folderLabel);
    promptLabel.setJustificationType (juce::Justification::centredLeft);
    promptLabel.setAccessible (false);
    promptLabel.setColour (juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible (promptLabel);
    entries.setRowHeight (30);
    entries.setMultipleSelectionEnabled (false);
    entries.setOutlineThickness (1);
    entries.setName ("Preset browser");
    entries.setDescription ({});
    addAndMakeVisible (entries);
    nameEdit.setMultiLine (false);
    nameEdit.setReturnKeyStartsNewLine (false);
    nameEdit.setInputRestrictions (120);
    nameEdit.setName ("Preset name");
    addAndMakeVisible (nameEdit);
    accept.onClick = [this] { confirmAction(); };
    cancel.onClick = [this]
    {
        if (mode == Mode::overwrite || mode == Mode::remove || mode == Mode::loadReplace)
            cancelConfirmation();
        else if (mode == Mode::rename || mode == Mode::folder)
        { showMode (Mode::browse); focusList(); }
        else close();
    };
    back.onClick = [this] { parentFolder(); };
    // MSVC requires the initializer-list elements to have one common type.
    // accept/cancel are ConfirmationButton while back is a juce::TextButton.
    // Register each component directly instead of deducing a mixed auto* list.
    addAndMakeVisible (accept);
    addAndMakeVisible (cancel);
    addAndMakeVisible (back);
    showMode (Mode::closed);
}
PresetBrowser::~PresetBrowser() { close (true); }

bool PresetBrowser::BrowserList::keyPressed (const juce::KeyPress& key)
{
    return parent.listKey (key) || juce::ListBox::keyPressed (key);
}
bool PresetBrowser::NameEdit::keyPressed (const juce::KeyPress& key)
{
    if (parent.textKey (key)) return true;
    const bool used = juce::TextEditor::keyPressed (key);
    // The focused name field still inserts the space. JUCE's ordinary UIA
    // text notifications may not speak it through NVDA, so echo it explicitly.
    if ((parent.mode == Mode::save || parent.mode == Mode::rename || parent.mode == Mode::folder)
        && key.getKeyCode() == 32
        && !key.getModifiers().isCtrlDown() && !key.getModifiers().isAltDown())
        biscot::announceToActiveScreenReader (*this, "spazio");
    return used;
}

void PresetBrowser::announce (const juce::String& text)
{
    juce::Component* source = &entries;
    if (mode == Mode::save || mode == Mode::rename || mode == Mode::folder)
        source = &nameEdit;
    else if (mode == Mode::overwrite || mode == Mode::remove || mode == Mode::loadReplace)
        source = confirmationYes ? static_cast<juce::Component*>(&accept) : static_cast<juce::Component*>(&cancel);
    biscot::announceToActiveScreenReader (*source, text);
}
void PresetBrowser::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0xff12161d));
    g.setColour (juce::Colour (0xffd9e7f7));
    g.setFont (juce::Font (juce::FontOptions (juce::Font::getDefaultSansSerifFontName(), 20.0f, juce::Font::bold)));
    g.drawText ("Biscot1 Presets", 16, 8, getWidth() - 32, 30,
                juce::Justification::centredLeft);
}
void PresetBrowser::resized()
{
    auto area = getLocalBounds().reduced (18);
    area.removeFromTop (40);
    folderLabel.setBounds (area.removeFromTop (27));
    area.removeFromTop (8);
    const auto bottom = area.removeFromBottom (42);
    auto buttons = bottom;
    back.setBounds (buttons.removeFromLeft (95));
    cancel.setBounds (buttons.removeFromRight (115));
    buttons.removeFromRight (10);
    accept.setBounds (buttons.removeFromRight (115));
    area.removeFromBottom (8);
    promptLabel.setBounds (area.removeFromTop (42));
    nameEdit.setBounds (area.removeFromTop (42));
    area.removeFromTop (6);
    entries.setBounds (area);
}

juce::File PresetBrowser::rememberedDirectory() const
{
    // Save As still honours the directory navigated to in this editor session.
    return directory.isDirectory() && manager.inside (directory)
        ? directory : manager.savedBrowserDirectory();
}
void PresetBrowser::rememberDirectory()
{
    // The folder is remembered across each browse/save invocation in this
    // editor session; the current .biscot file path is saved in plugin state.
    if (! manager.inside (directory)) directory = manager.root();
    manager.saveBrowserDirectory (directory);
}
void PresetBrowser::browse()
{
    if (active()) { close(); return; }
    if (auto result = manager.ensureLibrary(); result.failed()) { announce (result.getErrorMessage()); return; }
    // Only the last CONFIRMED load chooses the browser's start position.
    // Save-As folder history and uncommitted navigation never affect Browse.
    const auto target = manager.lastLoadedPresetAcrossSessions();
    directory = manager.validPreset (target) ? target.getParentDirectory() : manager.root();
    original = manager.capture();
    previewFile = {};
    hasPreview = false;
    incrementalSearch.reset();
    showMode (Mode::browse);
    int selected = 0;
    if (manager.validPreset (target) && target.getParentDirectory() == directory)
    {
        const auto matchingEntries = manager.list (directory);
        for (int row = 0; row < static_cast<int> (matchingEntries.size()); ++row)
            if (matchingEntries[static_cast<size_t> (row)].file == target)
            {
                selected = row;
                break;
            }
    }
    refresh (selected);
    juce::MessageManager::callAsync ([safe = juce::Component::SafePointer<PresetBrowser> (this)]
    {
        if (safe != nullptr && safe->mode == Mode::browse) safe->focusList();
    });
}
void PresetBrowser::saveAs()
{
    if (mode == Mode::browse) close();
    if (active()) return;
    if (auto result = manager.ensureLibrary(); result.failed()) { announce (result.getErrorMessage()); return; }
    directory = rememberedDirectory();
    showMode (Mode::save);
    const auto current = manager.currentFile();
    focusName (current.existsAsFile() ? current.getFileNameWithoutExtension() : "");
}
void PresetBrowser::close (bool revertPreview)
{
    if (! active()) return;
    if (revertPreview && original.valid() && hasPreview) manager.restore (original);
    original = {};
    hasPreview = false;
    previewFile = {};
    rememberDirectory();
    showMode (Mode::closed);
    if (onClose) onClose();
}

void PresetBrowser::showMode (Mode next)
{
    mode = next;
    setVisible (next != Mode::closed);
    const bool browsing = next == Mode::browse;
    const bool editing = next == Mode::save || next == Mode::rename || next == Mode::folder;
    const bool confirming = next == Mode::overwrite || next == Mode::remove || next == Mode::loadReplace;
    entries.setVisible (browsing);
    nameEdit.setVisible (editing);
    promptLabel.setVisible (editing || confirming);
    folderLabel.setVisible (next != Mode::closed);
    accept.setVisible (editing || confirming);
    cancel.setVisible (next != Mode::closed);
    back.setVisible (browsing);
    if (next != Mode::closed)
    {
        folderLabel.setText ("Folder: " + directory.getRelativePathFrom (manager.root()), juce::dontSendNotification);
        if (next == Mode::save) promptLabel.setText ("Save complete patch as .biscot", juce::dontSendNotification);
        if (next == Mode::rename) promptLabel.setText ("Rename preset or folder", juce::dontSendNotification);
        if (next == Mode::folder) promptLabel.setText ("New preset folder", juce::dontSendNotification);
        if (next == Mode::overwrite) promptLabel.setText ("Overwrite existing preset?", juce::dontSendNotification);
        if (next == Mode::remove) promptLabel.setText ("Delete selected preset?", juce::dontSendNotification);
        if (next == Mode::loadReplace) promptLabel.setText ("Replace complete current instrument state with this preset?", juce::dontSendNotification);
        accept.setButtonText (confirming ? "Yes" : (next == Mode::save ? "Save" : "OK"));
        cancel.setButtonText (confirming ? "No" : "Close");
        toFront (false);
    }
    resized();
    repaint();
}
void PresetBrowser::refresh (int selected)
{
    incrementalSearch.reset();
    const juce::ScopedValueSetter<bool> guard (suppressSelection, true);
    items = manager.list (directory);
    entries.updateContent();
    if (items.empty()) entries.deselectAllRows();
    else
    {
        const auto index = juce::jlimit (0, static_cast<int> (items.size()) - 1, selected);
        entries.selectRow (index);
        entries.scrollToEnsureRowIsOnscreen (index);
    }
}
void PresetBrowser::focusList()
{
    entries.grabKeyboardFocus();
    juce::Timer::callAfterDelay (1, [safe = juce::Component::SafePointer<PresetBrowser> (this)]
    {
        if (safe != nullptr && safe->mode == Mode::browse
            && safe->entries.hasKeyboardFocus (true))
            if (auto* handler = safe->entries.getAccessibilityHandler()) handler->grabFocus();
    });
    const int row = entries.getSelectedRow();
    if (juce::isPositiveAndBelow (row, static_cast<int> (items.size())))
    {
        announce ("Preset browser. " + getNameForRow (row));
    }
    else announce ("Preset browser. Empty folder");
}
void PresetBrowser::focusName (const juce::String& text)
{
    nameEdit.setText (text, juce::dontSendNotification);
    nameEdit.grabKeyboardFocus();
    nameEdit.selectAll();
}
int PresetBrowser::getNumRows() { return static_cast<int> (items.size()); }
juce::String PresetBrowser::getNameForRow (int row)
{
    if (! juce::isPositiveAndBelow (row, static_cast<int> (items.size()))) return {};
    const auto& entry = items[static_cast<size_t> (row)];
    return juce::String (row + 1) + ". "
        + (entry.directory ? "Folder. " : "Preset. ") + entry.name;
}
void PresetBrowser::paintListBoxItem (int row, juce::Graphics& g,
                                      int width, int height, bool selected)
{
    g.fillAll (selected ? juce::Colour (0xff395775) : juce::Colour (0xff202630));
    g.setColour (juce::Colours::white);
    g.setFont (juce::Font (juce::FontOptions (juce::Font::getDefaultSansSerifFontName(), 15.0f, juce::Font::plain)));
    g.drawText (getNameForRow (row), 8, 0, width - 14, height, juce::Justification::centredLeft);
}
void PresetBrowser::selectedRowsChanged (int row)
{
    if (suppressSelection || mode != Mode::browse ||
        ! juce::isPositiveAndBelow (row, static_cast<int> (items.size()))) return;
    // LJuno-style explicit UI Automation announcement, not merely a ListBox
    // selection event: NVDA can otherwise remain completely silent on arrows.
    // Moving between preset filenames must not silently load another synth,
    // Pattern bank or Song bank before the user answers Yes or No.
    announce (getNameForRow (row));
}
void PresetBrowser::listBoxItemDoubleClicked (int row, const juce::MouseEvent&)
{
    activateRow (row);
}
void PresetBrowser::previewRow (int row)
{
    if (isPreviewing || ! juce::isPositiveAndBelow (row, static_cast<int> (items.size()))) return;
    const auto& item = items[static_cast<size_t> (row)];
    if (item.directory || item.file == previewFile) return;
    const juce::ScopedValueSetter<bool> guard (isPreviewing, true);
    auto result = manager.preview (item.file);
    if (result.failed()) { announce (result.getErrorMessage()); return; }
    hasPreview = true;
    previewFile = item.file;
    if (onPresetLoaded) onPresetLoaded();
}
void PresetBrowser::activateRow (int row)
{
    if (! juce::isPositiveAndBelow (row, static_cast<int> (items.size()))) return;
    const auto entry = items[static_cast<size_t> (row)];
    if (entry.directory)
    {
        incrementalSearch.reset();
        directory = entry.file;
        refresh(); focusList(); return;
    }
    // Confirmation precedes the first mutation; arrow navigation is browse-only.
    pendingFile = entry.file;
    confirmReturn = Mode::browse;
    confirmationYes = false;
    showMode (Mode::loadReplace);
    cancel.grabKeyboardFocus();
    announce ("Load preset " + entry.name +
        "? This replaces the entire synth, Pattern and Song state. Yes or No. No selected");
}
void PresetBrowser::parentFolder()
{
    incrementalSearch.reset();
    if (directory == manager.root()) return;
    const auto old = directory;
    directory = directory.getParentDirectory();
    if (! manager.inside (directory)) directory = manager.root();
    manager.saveBrowserDirectory (directory);
    refresh();
    for (int i = 0; i < static_cast<int> (items.size()); ++i)
        if (items[static_cast<size_t> (i)].file == old)
        { entries.selectRow (i); break; }
    focusList();
}
bool PresetBrowser::selectByInitial (juce::juce_wchar character, bool backwards)
{
    if (items.empty()) return false;
    const int match = incrementalSearch.type (character, entries.getSelectedRow(),
        static_cast<int> (items.size()),
        [this] (int row) { return items[static_cast<size_t> (row)].name; }, backwards);
    if (match < 0) announce ("No matching preset for " + incrementalSearch.currentPrefix());
    else if (match != entries.getSelectedRow())
    {
        entries.selectRow (match);
        entries.scrollToEnsureRowIsOnscreen (match);
    }
    else announce (getNameForRow (match));
    return true;
}

void PresetBrowser::renameSelected()
{
    const auto row = entries.getSelectedRow();
    if (! juce::isPositiveAndBelow (row, static_cast<int> (items.size()))) return;
    editFile = items[static_cast<size_t> (row)].file;
    showMode (Mode::rename);
    focusName (items[static_cast<size_t> (row)].name);
}
void PresetBrowser::newFolder()
{
    showMode (Mode::folder);
    focusName ("");
}
void PresetBrowser::deleteSelected()
{
    const auto row = entries.getSelectedRow();
    if (! juce::isPositiveAndBelow (row, static_cast<int> (items.size()))) return;
    if (items[static_cast<size_t> (row)].directory)
    { announce ("Delete folders using Windows Explorer"); return; }
    pendingFile = items[static_cast<size_t> (row)].file;
    confirmReturn = Mode::browse;
    confirmationYes = false;
    showMode (Mode::remove);
    cancel.grabKeyboardFocus(); // No is the default, matching LJuno.
    announce ("Delete preset " + pendingFile.getFileNameWithoutExtension() + "? Yes or No");
}
void PresetBrowser::commitName()
{
    if (mode == Mode::save)
    {
        juce::File destination;
        auto result = manager.save (directory, nameEdit.getText(), destination, false);
        if (result.failed() && result.getErrorMessage() == "Preset exists")
        {
            pendingFile = destination;
            confirmReturn = Mode::save;
            confirmationYes = false;
            showMode (Mode::overwrite);
            cancel.grabKeyboardFocus(); // No is the default, matching LJuno.
            announce ("Overwrite preset " + pendingFile.getFileNameWithoutExtension() + "? Yes or No");
            return;
        }
        if (result.failed()) { announce (result.getErrorMessage()); return; }
        const auto message = "Saved preset " + destination.getFileNameWithoutExtension();
        announce (message); close (false);
    }
    else if (mode == Mode::rename)
    {
        juce::File renamed;
        auto result = manager.rename (editFile, nameEdit.getText(), renamed);
        if (result.failed()) { announce (result.getErrorMessage()); return; }
        showMode (Mode::browse);
        refreshAndSelectFile (renamed);
    }
    else if (mode == Mode::folder)
    {
        juce::File made;
        auto result = manager.newFolder (directory, nameEdit.getText(), made);
        if (result.failed()) { announce (result.getErrorMessage()); return; }
        showMode (Mode::browse);
        refreshAndSelectFile (made);
    }
}
void PresetBrowser::refreshAndSelectFile (const juce::File& file)
{
    refresh();
    for (int i = 0; i < static_cast<int> (items.size()); ++i)
        if (items[static_cast<size_t> (i)].file == file)
        { entries.selectRow (i); entries.scrollToEnsureRowIsOnscreen (i); break; }
    focusList();
}
void PresetBrowser::cancelConfirmation()
{
    confirmationYes = false;
    showMode (confirmReturn);
    if (mode == Mode::browse) focusList(); else focusName (nameEdit.getText());
}
void PresetBrowser::confirmAction()
{
    if (mode == Mode::loadReplace)
    {
        const auto target = pendingFile;
        // PresetManager::load validates the complete file before changing state.
        // This action never writes over the existing preset file on disk.
        auto result = manager.load (target);
        if (result.failed()) { cancelConfirmation(); announce (result.getErrorMessage()); return; }
        if (onPresetLoaded) onPresetLoaded();
        announce ("Loaded preset " + target.getFileNameWithoutExtension());
        close (false);
    }
    else if (mode == Mode::overwrite)
    {
        juce::File saved;
        auto result = manager.save (directory, nameEdit.getText(), saved, true);
        if (result.failed()) { announce (result.getErrorMessage()); return; }
        const auto message = "Saved preset " + saved.getFileNameWithoutExtension();
        announce (message); close (false);
    }
    else if (mode == Mode::remove)
    {
        auto result = manager.remove (pendingFile);
        if (result.failed()) { announce (result.getErrorMessage()); return; }
        const auto message = "Preset deleted";
        pendingFile = {};
        showMode (Mode::browse);
        refresh(); focusList(); announce (message);
    }
    else commitName();
}
void PresetBrowser::showMenu()
{
    juce::PopupMenu menu;
    menu.addItem (1, "Rename\tF2", ! items.empty());
    menu.addItem (2, "New folder\tCtrl+N");
    menu.addItem (3, "Delete\tDelete", ! items.empty());
    menu.showMenuAsync (juce::PopupMenu::Options().withTargetComponent (&entries),
         [safe = juce::Component::SafePointer<PresetBrowser> (this)] (int selected)
         {
             if (safe == nullptr || safe->mode != Mode::browse) return;
             if (selected == 1) safe->renameSelected();
             else if (selected == 2) safe->newFolder();
             else if (selected == 3) safe->deleteSelected();
         });
}
bool PresetBrowser::listKey (const juce::KeyPress& k)
{
    const auto code = k.getKeyCode();
    const auto mods = k.getModifiers();
    if (code == juce::KeyPress::escapeKey || (mods.isAltDown()
        && (juce::CharacterFunctions::toLowerCase (k.getTextCharacter()) == 'b'
            || juce::CharacterFunctions::toLowerCase (k.getTextCharacter()) == 'c')))
    { close(); return true; }
    if (code == juce::KeyPress::returnKey) { incrementalSearch.reset(); activateRow (entries.getSelectedRow()); return true; }
    if (code == juce::KeyPress::rightKey && !mods.isCtrlDown() && !mods.isAltDown())
    {
        const int row = entries.getSelectedRow();
        if (juce::isPositiveAndBelow (row, static_cast<int> (items.size()))
            && items[static_cast<size_t> (row)].directory)
            activateRow (row);
        else announce ("Right arrow enters folders. Press Enter to load a preset");
        return true;
    }
    // Mirror LJuno: explicitly select + announce on every keyboard step.
    if (! items.empty() && ! mods.isAltDown() && ! mods.isCtrlDown()
        && ! mods.isCommandDown())
    {
        int target = entries.getSelectedRow();
        if (code == juce::KeyPress::upKey) --target;
        else if (code == juce::KeyPress::downKey) ++target;
        else if (code == juce::KeyPress::pageUpKey) target -= 10;
        else if (code == juce::KeyPress::pageDownKey) target += 10;
        else if (code == juce::KeyPress::homeKey) target = 0;
        else if (code == juce::KeyPress::endKey) target = static_cast<int> (items.size()) - 1;
        else target = -1;
        if (target >= 0 || code == juce::KeyPress::upKey || code == juce::KeyPress::pageUpKey)
        {
            incrementalSearch.reset();
            const int bounded = juce::jlimit (0, static_cast<int> (items.size()) - 1, target);
            if (bounded != entries.getSelectedRow())
            {
                entries.selectRow (bounded); // selectedRowsChanged announces + loads preview
                entries.scrollToEnsureRowIsOnscreen (bounded);
            }
            return true; // silent edges
        }
    }
    if (code == juce::KeyPress::backspaceKey || code == juce::KeyPress::leftKey)
    { parentFolder(); return true; }
    if (code == juce::KeyPress::F2Key) { renameSelected(); return true; }
    if (code == juce::KeyPress::deleteKey) { deleteSelected(); return true; }
    if (mods.isCtrlDown() && (k.getTextCharacter() == 'n' || k.getTextCharacter() == 'N' || code == 'N'))
    { newFolder(); return true; }
    if ((code == juce::KeyPress::F10Key && mods.isShiftDown())
        || code == applicationMenuKeyCode || code == applicationMenuExtendedKeyCode)
    { showMenu(); return true; }
    if (! mods.isCtrlDown() && ! mods.isAltDown() && ! mods.isCommandDown())
    {
        auto needle = juce::CharacterFunctions::toLowerCase (k.getTextCharacter());
        if (mods.isShiftDown() && code >= '0' && code <= '9') needle = static_cast<juce::juce_wchar> (code);
        if (juce::CharacterFunctions::isLetterOrDigit (needle))
            return selectByInitial (needle, mods.isShiftDown());
    }
    return false; // ListBox handles arrows, Home, End, PageUp/Down.
}
bool PresetBrowser::textKey (const juce::KeyPress& k)
{
    if (k.getKeyCode() == juce::KeyPress::escapeKey)
    {
        if (mode == Mode::save) close();
        else { showMode (Mode::browse); focusList(); }
        return true;
    }
    if (k.getKeyCode() == juce::KeyPress::returnKey)
    { commitName(); return true; }
    return false;
}
bool PresetBrowser::keyPressed (const juce::KeyPress& key)
{
    return shortcut (key) || juce::Component::keyPressed (key);
}
bool PresetBrowser::shortcut (const juce::KeyPress& key)
{
    if (! active()) return false;
    if (mode == Mode::browse) return listKey (key);
    if (mode == Mode::save || mode == Mode::rename || mode == Mode::folder)
        return textKey (key);
    if (mode == Mode::overwrite || mode == Mode::remove || mode == Mode::loadReplace)
    {
        const auto code = key.getKeyCode();
        const auto mods = key.getModifiers();
        if (code == juce::KeyPress::escapeKey) { cancelConfirmation(); return true; }
        if (! mods.isCtrlDown() && ! mods.isAltDown() && ! mods.isCommandDown())
        {
            const auto character = juce::CharacterFunctions::toLowerCase (key.getTextCharacter());
            if (character == 'y' || code == 'Y') { confirmAction(); return true; }
            if (character == 'n' || code == 'N') { cancelConfirmation(); return true; }
            if (code == juce::KeyPress::leftKey || code == juce::KeyPress::rightKey ||
                code == juce::KeyPress::upKey || code == juce::KeyPress::downKey)
            {
                confirmationYes = code == juce::KeyPress::rightKey || code == juce::KeyPress::upKey;
                (confirmationYes ? accept : cancel).grabKeyboardFocus();
                announce (confirmationYes ? "Yes selected" : "No selected");
                return true;
            }
            if (code == juce::KeyPress::returnKey)
            {
                if (confirmationYes) confirmAction(); else cancelConfirmation();
                return true;
            }
        }
        return true;
    }
    return false;
}
void PresetBrowser::previousNext (int direction)
{
    if (active()) close();
    juce::String name;
    auto result = manager.relative (direction, name);
    if (result.failed()) announce (result.getErrorMessage());
    else { if (onPresetLoaded) onPresetLoaded(); announce ("Preset " + name); }
}
}
