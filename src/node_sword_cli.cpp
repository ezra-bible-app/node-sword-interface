#include "repository_interface.hpp"
#include "sword_status_reporter.hpp"
#include "module_store.hpp"
#include "text_processor.hpp"
#include "module_helper.hpp"
#include "dict_helper.hpp"
#include "module_installer.hpp"
#include "strongs_entry.hpp"
#include "module_search.hpp"

#include <vector>
#include <iostream>
#include <swmodule.h>
#include <swmgr.h>
#include <localemgr.h>

using namespace std;
using namespace sword;

void show_repos(RepositoryInterface& repoInterface)
{
    cout << "REPOSITORIES:" << endl;
    vector<string> repoNames = repoInterface.getRepoNames();
    for (unsigned int i = 0; i < repoNames.size(); i++) {
        cout << repoNames[i] << endl;
    }
    cout << endl;
}

void show_modules(RepositoryInterface& repoInterface)
{
    string repo = "CrossWire";
    cout << "DICT MODULES of " << repo << ":" << endl;
    vector<SWModule*> modules = repoInterface.getRepoModulesByLang(repo, "en", ModuleType::dict);

    for (unsigned int i = 0; i < modules.size(); i++) {
        SWModule* currentModule = modules[i];

        cout << currentModule->getName();

        /*if (currentModule->getConfigEntry("Abbreviation")) {
            string moduleShortcut = currentModule->getConfigEntry("Abbreviation");
            cout << " " << moduleShortcut;
        }

        if (currentModule->getConfigEntry("InstallSize")) {
            string installSize = currentModule->getConfigEntry("InstallSize");
            cout << " " << installSize;
        }

        string moduleVersion = currentModule->getConfigEntry("Version");
        cout << " " << moduleVersion << " ";*/

        //string moduleLocked = currentModule->getConfigEntry("CipherKey") ? "LOCKED" : "";
        //cout << moduleLocked << " ";

        /*ConfigEntMap::const_iterator begin = currentModule->getConfig().lower_bound("GlobalOptionFilter");
        ConfigEntMap::const_iterator end = currentModule->getConfig().upper_bound("GlobalOptionFilter");
        for(; begin !=end; ++begin) {
            cout << begin->first.c_str() << " = " << begin->second.c_str() << endl;
        }*/

        string path = currentModule->getConfigEntry("AbsoluteDataPath");
        cout << path << endl;
    }
    cout << endl;
}

void get_local_module(ModuleStore& module_store)
{
    SWModule* m = module_store.getLocalModule("GerNeUe");
    string path = module_store.getModuleDataPath(m);
    cout << "GerNeUe path: " << path << endl;
    cout << module_store.isModuleInUserDir("GerNeUe") << endl;
}

void get_repo_module(RepositoryInterface& repoInterface)
{
    string module = "GerSch";
    SWModule* m = repoInterface.getRepoModule(module);

    if (m != 0) {
        string description = m->getConfigEntry("About");
        cout << module << " description: " << description << endl;
        // FIXME
        //cout << sword_facade.isModuleInUserDir(module) << endl;
    } else {
        cout << "Could not find " << module << "!" << endl;
    }
}

void get_updated_repo_modules(RepositoryInterface& repoInterface)
{
    ModuleStore moduleStore;
    ModuleHelper moduleHelper(moduleStore);

    vector<SWModule*> updatedModules = repoInterface.getUpdatedRepoModules();

    for (int i = 0; i < updatedModules.size(); i++) {
        cout << updatedModules[i]->getName() << " : " << updatedModules[i]->getConfigEntry("Version") << endl;
    }
}

void get_module_text(TextProcessor& text_processor)
{
    cout << "Text:" << endl;
    text_processor.enableMarkup();
    vector<Verse> verses = text_processor.getBookText("NET", "Luke");
    cout << "Got " << verses.size() << " verses!" << endl;
    for (int i = 0; i < 10; i++) {
        cout << verses[i].reference << "|" << verses[i].content << endl;
    }
}

void get_reference_text(ModuleStore& module_store, TextProcessor& text_processor)
{
    cout << "RAW Text:" << endl;
    SWModule* swordModule = module_store.getLocalModule("NETnote");
    swordModule->setKey("1 Tim 3:2");
    string rawText = swordModule->getRawEntry();
    cout << rawText << endl;
  
    cout << "Processed Text: " << endl;
    text_processor.enableMarkup();
    Verse text = text_processor.getReferenceText("NETnote", "1 Tim 3:2");
    cout << text.content << endl;
}

void get_book_intro(TextProcessor& text_processor)
{
    cout << "Text:" << endl;
    text_processor.enableMarkup();
    string bookIntro = text_processor.getBookIntroduction("NET", "Luke");
    cout << bookIntro << endl;
}

void get_book_list(ModuleHelper& module_helper)
{
    vector<string> bookList = module_helper.getBookList("GerNeUe");

    for (int i = 0; i < bookList.size(); i++) {
        cout << bookList[i] << endl;
    }
}

void get_dict_key_list(DictHelper& dict_helper)
{
    vector<string> keyList = dict_helper.getKeyList("Vines");

    cout << "Got " << keyList.size() << " entries!" << endl;

    for (int i = 0; i < keyList.size(); i++) {
        cout << keyList[i] << endl;
    }
}

void test_unlock_key(ModuleInstaller& module_installer, ModuleStore& module_store, TextProcessor& text_processor)
{
    module_installer.uninstallModule("NA28");
    module_installer.installModule("Deutsche Bibelgesellschaft", "NA28");
    module_installer.saveModuleUnlockKey("NA28", "");
    SWModule* m = module_store.getLocalModule("NA28");
    cout << "Module readable: " << text_processor.isModuleReadable(m) << endl;
}

void get_strongs_entry(TextProcessor& text_processor)
{
    //StrongsEntry* entry = text_processor.getStrongsEntry("G2443");
    //StrongsEntry* entry = text_processor.getStrongsEntry("G3762");
    StrongsEntry* entry = text_processor.getStrongsEntry("G4572");
    if (entry != 0) {
        cout << entry->key << endl;
        cout << entry->transcription << endl;
        cout << entry->phoneticTranscription << endl;
        cout << entry->definition << endl;
        for (unsigned int i = 0; i < entry->references.size(); i++) {
            cout << entry->references[i].text << endl;
        }
        cout << endl;
    }

    entry = text_processor.getStrongsEntry("H3069");
    if (entry != 0) {
        cout << entry->key << endl;
        cout << entry->transcription << endl;
        cout << entry->phoneticTranscription << endl;
        cout << entry->definition << endl;
        for (unsigned int i = 0; i < entry->references.size(); i++) {
            cout << entry->references[i].text << endl;
        }
    }
}

void install_if_missing(ModuleInstaller& module_installer, ModuleStore& module_store, const string& repo, const string& moduleName)
{
    SWModule* m = module_store.getLocalModule(moduleName);
    if (m == 0) {
        cout << "Installing module " << moduleName << " from " << repo << " ..." << endl;
        int error = module_installer.installModule(repo, moduleName);
        if (error) {
            cout << "  ERROR: Failed to install " << moduleName << endl;
        } else {
            cout << "  Successfully installed " << moduleName << endl;
            module_store.refreshMgr();
        }
    } else {
        cout << "Module " << moduleName << " already installed." << endl;
    }
}

void test_update_single_repository_config(RepositoryInterface& repoInterface)
{
    string repo = "CrossWire";
    cout << "=== Update Single Repository Config Test ===" << endl;
    cout << "Refreshing remote source: " << repo << " ..." << endl;
    int ret = repoInterface.refreshIndividualRemoteSource(repo, nullptr);
    if (ret == 0) {
        cout << "Successfully refreshed " << repo << endl;
    } else {
        cout << "ERROR: Failed to refresh " << repo << " (return code: " << ret << ")" << endl;
    }
    cout << "=== End of Update Single Repository Config Test ===" << endl;
}

void test_verse_reference_mapping(ModuleInstaller& module_installer, ModuleStore& module_store, TextProcessor& text_processor)
{
    cout << "=== Verse Reference Mapping Test ===" << endl;

    // Install required modules for the tests
    // KJV uses KJV versification
    install_if_missing(module_installer, module_store, "CrossWire", "KJV");
    // RusSynodal uses Synodal versification
    install_if_missing(module_installer, module_store, "CrossWire", "RusSynodal");
    // VulgClementine uses Vulg versification
    install_if_missing(module_installer, module_store, "CrossWire", "VulgClementine");

    cout << endl;

    // Test 1: Same versification (should return unchanged)
    string result1 = text_processor.mapVerseReference("Gen.1.1", "KJV", "KJV");
    cout << "KJV Gen.1.1 -> KJV: " << result1 << endl;

    // Test 2: KJV to Synodal (Psalm numbering and superscription shift)
    string result2 = text_processor.mapVerseReference("Ps.13.1", "KJV", "RusSynodal");
    cout << "KJV Ps.13.1 -> RusSynodal (Synodal): " << result2 << endl;

    // Test 3: KJV to Vulg (Psalm 51 -> Psalm 50, default allowRange=false returns first verse only)
    string result3 = text_processor.mapVerseReference("Ps.51.1", "KJV", "VulgClementine");
    cout << "KJV Ps.51.1 -> VulgClementine (Vulg, no range): " << result3 << endl;

    // Test 3b: Same mapping with allowRange=true (returns full range)
    string result3b = text_processor.mapVerseReference("Ps.51.1", "KJV", "VulgClementine", true);
    cout << "KJV Ps.51.1 -> VulgClementine (Vulg, with range): " << result3b << endl;

    // Test 3c: Reverse - take the second verse of the range (Ps.50.3) and map back to KJV
    string result3c = text_processor.mapVerseReference("Ps.50.3", "VulgClementine", "KJV");
    cout << "VulgClementine Ps.50.3 -> KJV: " << result3c << endl;

    // Test 4: Synodal to KJV (reverse mapping)
    string result4 = text_processor.mapVerseReference("Ps.12.2", "RusSynodal", "KJV");
    cout << "RusSynodal Ps.12.2 -> KJV: " << result4 << endl;

    // Test 5: Synodal to Vulg (non-KJV to non-KJV, routes through KJVA hub)
    string result5 = text_processor.mapVerseReference("Ps.50.1", "RusSynodal", "VulgClementine");
    cout << "RusSynodal Ps.50.1 -> VulgClementine (Vulg): " << result5 << endl;

    // --- NT mapping tests ---
    cout << endl << "--- NT Mapping Tests ---" << endl;

    // Test 6: KJV Rev.13.1 -> Vulg (default: first verse only)
    string result6 = text_processor.mapVerseReference("Rev.13.1", "KJV", "VulgClementine");
    cout << "KJV Rev.13.1 -> VulgClementine (Vulg, no range): " << result6 << endl;

    // Test 6b: Same with allowRange=true
    string result6b = text_processor.mapVerseReference("Rev.13.1", "KJV", "VulgClementine", true);
    cout << "KJV Rev.13.1 -> VulgClementine (Vulg, with range): " << result6b << endl;

    // Test 6c: Reverse - take the second verse of the range (Rev.12.19) and map back to KJV
    string result6c = text_processor.mapVerseReference("Rev.12.19", "VulgClementine", "KJV");
    cout << "VulgClementine Rev.12.19 -> KJV: " << result6c << endl;

    // Test 7: KJV Mark.9.1 -> Vulg (Vulg has Mark 9:1 as Mark 8:39)
    string result7 = text_processor.mapVerseReference("Mark.9.1", "KJV", "VulgClementine");
    cout << "KJV Mark.9.1 -> VulgClementine (Vulg): " << result7 << endl;

    // Test 8: KJV Acts.19.41 -> Vulg (Vulg merges Acts 19:40-41)
    string result8 = text_processor.mapVerseReference("Acts.19.41", "KJV", "VulgClementine");
    cout << "KJV Acts.19.41 -> VulgClementine (Vulg): " << result8 << endl;

    // Test 9: KJV 3John.1.14 -> Synodal (default: first verse only)
    string result9 = text_processor.mapVerseReference("3John.1.14", "KJV", "RusSynodal");
    cout << "KJV 3John.1.14 -> RusSynodal (Synodal, no range): " << result9 << endl;

    // Test 9b: Same with allowRange=true (Synodal splits 3 John 1:14 into 14-15)
    string result9b = text_processor.mapVerseReference("3John.1.14", "KJV", "RusSynodal", true);
    cout << "KJV 3John.1.14 -> RusSynodal (Synodal, with range): " << result9b << endl;

    // Test 9c: Reverse - take the second verse of the range (3John.1.15) and map back to KJV
    string result9c = text_processor.mapVerseReference("3John.1.15", "RusSynodal", "KJV");
    cout << "RusSynodal 3John.1.15 -> KJV: " << result9c << endl;

    // Test 10: KJV Rom.16.25 -> Synodal (Synodal moves the doxology to Rom.14.24)
    string result10 = text_processor.mapVerseReference("Rom.16.25", "KJV", "RusSynodal");
    cout << "KJV Rom.16.25 -> RusSynodal (Synodal): " << result10 << endl;

    // Test 11: KJV Mark.4.41 -> Vulg (Vulg merges Mark 4:40-41 into one verse)
    string result11 = text_processor.mapVerseReference("Mark.4.41", "KJV", "VulgClementine");
    cout << "KJV Mark.4.41 -> VulgClementine (Vulg): " << result11 << endl;

    cout << "=== End of Mapping Test ===" << endl;
}

int test_net_1sam_1_5(ModuleInstaller& module_installer, ModuleStore& module_store, TextProcessor& text_processor)
{
    cout << "=== NET Passage Test (1Sam.1.5) ===" << endl;

    install_if_missing(module_installer, module_store, "Xiphos", "NET");

    SWModule* netModule = module_store.getLocalModule("NET");
    if (netModule == 0) {
        cout << "ERROR: NET module is unavailable after install attempt." << endl;
        return 1;
    }

    text_processor.enableMarkup();
    Verse verse = text_processor.getReferenceText("NET", "1Sam.1.5");

    cout << "Requested Reference: 1Sam.1.5" << endl;
    cout << "Resolved Reference: " << verse.reference << endl;

    if (verse.content.empty()) {
        cout << "ERROR: Empty verse content for NET 1Sam.1.5" << endl;
        return 2;
    }

    cout << "Verse Content Start" << endl;
    cout << verse.content << endl;
    cout << "Verse Content End" << endl;
    cout << "=== End NET Passage Test ===" << endl;

    return 0;
}

int main(int argc, char** argv)
{
    ModuleStore moduleStore;
    ModuleHelper moduleHelper(moduleStore);
    DictHelper dictHelper(moduleStore);
    SwordStatusReporter statusReporter;
    long timeoutMillis = 20000;
    RepositoryInterface repoInterface(statusReporter, moduleHelper, moduleStore, "", timeoutMillis);
    ModuleInstaller moduleInstaller(repoInterface, moduleStore);
    TextProcessor textProcessor(moduleStore, moduleHelper);
    ModuleSearch moduleSearch(moduleStore, moduleHelper, textProcessor);

    /*std::vector<sword::SWModule*> localModules = moduleStore.getAllLocalModules();
    for (int i = 0; i < localModules.size(); i++) {
        sword::SWModule* module = localModules[i];
        cout << module->getName() << endl;
    }*/

    /*for (unsigned int i = 0; i < 20; i++) {
        cout << "Refreshing remote sources [" << i << "]" << endl;
        repoInterface.refreshRemoteSources(true);
    }*/

    //test_unlock_key(moduleInstaller, moduleStore, textProcessor);

    /*show_repos(repoInterface);*/

    //show_modules(repoInterface);

    int result = test_net_1sam_1_5(moduleInstaller, moduleStore, textProcessor);
    if (result != 0) {
        return result;
    }

    //test_verse_reference_mapping(moduleInstaller, moduleStore, textProcessor);

    /*int error = moduleInstaller.installModule("CrossWire", "UKJV");

    if (error) {
        cout << "Error installing module (write permissions?)\n";
    }*/
    
    /*int error = moduleInstaller.uninstallModule("YLT");

    if (error) {
        cout << "Error uninstalling module (write permissions?)\n";
    }*/

    /*get_local_module(moduleStore);
    
    get_repo_module(repoInterface);*/

    /*sword_facade.installModule("StrongsHebrew");
    sword_facade.installModule("StrongsGreek");*/

    //get_strongs_entry(textProcessor);

    //get_module_text(textProcessor);

    //get_reference_text(moduleStore, textProcessor);

    //get_book_intro(textProcessor);

    //get_book_list(moduleHelper);

    //get_dict_key_list(dictHelper);

    //string translation = sword_facade.getSwordTranslation(string("/usr/share/sword/locales.d"), string("de"), string("locales"));
    //cout << translation << endl;

    /*vector<Verse> searchResults = moduleSearch.getModuleSearchResults("GerNeUe", "Glaube", SearchType::multiWord, SearchScope::NT, true);
    cout << "Got " << searchResults.size() << " results!" << endl;
    for (unsigned int i=0; i < searchResults.size(); i++) {
        cout << searchResults[i].reference << endl;
    }*/

    //get_updated_repo_modules(repoInterface);

    return 0;
}

