#include "repository_interface.hpp"
#include "sword_status_reporter.hpp"
#include "module_store.hpp"
#include "text_processor.hpp"
#include "module_helper.hpp"
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
    cout << "German MODULES of CrossWire:" << endl;
    vector<SWModule*> modules = repoInterface.getRepoModulesByLang("CrossWire", "de");
    for (unsigned int i = 0; i < modules.size(); i++) {
        SWModule* currentModule = modules[i];

        cout << currentModule->getName();

        if (currentModule->getConfigEntry("Abbreviation")) {
            string moduleShortcut = currentModule->getConfigEntry("Abbreviation");
            cout << " " << moduleShortcut;
        }

        if (currentModule->getConfigEntry("InstallSize")) {
            string installSize = currentModule->getConfigEntry("InstallSize");
            cout << " " << installSize;
        }

        string moduleVersion = currentModule->getConfigEntry("Version");
        cout << " " << moduleVersion << " ";

        string moduleLocked = currentModule->getConfigEntry("CipherKey") ? "LOCKED" : "";
        cout << moduleLocked << " ";

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

void get_module_text(TextProcessor& text_processor)
{
    cout << "Text:" << endl;
    text_processor.enableMarkup();
    vector<Verse> verses = text_processor.getBookText("NASB", "Ps");
    cout << "Got " << verses.size() << " verses!" << endl;
    /*for (int i = 0; i < verses.size(); i++) {
        cout << verses[i].reference << "|" << verses[i].content << endl;
    }*/
}

void get_book_intro(TextProcessor& text_processor)
{
    cout << "Text:" << endl;
    text_processor.enableMarkup();
    string bookIntro = text_processor.getBookIntroduction("GerNeUe", "John");
    cout << bookIntro << endl;
}

void get_book_list(ModuleHelper& module_helper)
{
    vector<string> bookList = module_helper.getBookList("GerNeUe");

    for (int i = 0; i < bookList.size(); i++) {
        cout << bookList[i] << endl;
    }
}

void test_unlock_key(ModuleInstaller& module_installer, ModuleStore& module_store, TextProcessor& text_processor)
{
    module_installer.uninstallModule("NA28");
    module_installer.installModule("NA28");
    module_installer.saveModuleUnlockKey("NA28", "");
    SWModule* m = module_store.getLocalModule("NA28");
    cout << "Module readable: " << text_processor.isModuleReadable(m) << endl;
}

void get_strongs_entry(TextProcessor& text_processor)
{
    StrongsEntry* entry = text_processor.getStrongsEntry("G2766");
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

int main(int argc, char** argv)
{
    ModuleStore moduleStore;
    ModuleHelper moduleHelper(moduleStore);
    SwordStatusReporter statusReporter;
    RepositoryInterface repoInterface(statusReporter, moduleHelper);
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

    /*show_repos(repoInterface);

    show_modules(repoInterface);*/

    /*int error = moduleInstaller.installModule("UKJV");

    if (error) {
        cout << "Error installing module (write permissions?)\n";
    }
    
    error = moduleInstaller.uninstallModule("UKJV");

    if (error) {
        cout << "Error uninstalling module (write permissions?)\n";
    }*/

    /*get_local_module(moduleStore);
    
    get_repo_module(repoInterface);*/

    /*sword_facade.installModule("StrongsHebrew");
    sword_facade.installModule("StrongsGreek");*/

    //get_strongs_entry(textProcessor);

    get_module_text(textProcessor);

    //get_book_intro(textProcessor);

    //get_book_list(moduleHelper);

    //string translation = sword_facade.getSwordTranslation(string("/usr/share/sword/locales.d"), string("de"), string("locales"));
    //cout << translation << endl;

    /*vector<Verse> searchResults = moduleSearch.getModuleSearchResults("NASB", "Jesus faith", SearchType::multiWord, false, true);
    cout << "Got " << searchResults.size() << " results!" << endl;
    for (unsigned int i=0; i < searchResults.size(); i++) {
        cout << searchResults[i].reference << endl;
    }*/

    return 0;
}

