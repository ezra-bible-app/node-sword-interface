#include "sword_facade.hpp"

#include <vector>
#include <iostream>
#include <swmodule.h>
#include <localemgr.h>

using namespace std;
using namespace sword;

void show_repos(SwordFacade& sword_facade)
{
    cout << "REPOSITORIES:" << endl;
    vector<string> repoNames = sword_facade.getRepoNames();
    for (unsigned int i = 0; i < repoNames.size(); i++) {
        cout << repoNames[i] << endl;
    }
    cout << endl;
}

void show_modules(SwordFacade& sword_facade)
{
    cout << "German MODULES of CrossWire:" << endl;
    vector<SWModule*> modules = sword_facade.getRepoModulesByLang("CrossWire", "de");
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

void get_local_module(SwordFacade& sword_facade)
{
    SWModule* m = sword_facade.getLocalModule("germenge");
    string path = m->getConfigEntry("AbsoluteDataPath");
    cout << "germenge path: " << path << endl;
    cout << sword_facade.isModuleInUserDir("germenge") << endl;
}

void get_repo_module(SwordFacade& sword_facade)
{
    string module = "GerSch";
    SWModule* m = sword_facade.getRepoModule(module);

    if (m != 0) {
        string description = m->getConfigEntry("About");
        cout << module << " description: " << description << endl;
        cout << sword_facade.isModuleInUserDir(module) << endl;
    } else {
        cout << "Could not find " << module << "!" << endl;
    }
}

void get_module_text(SwordFacade& sword_facade)
{
    cout << "Text:" << endl;
    sword_facade.enableMarkup();
    vector<string> verses = sword_facade.getBookText("GerNeUe", "Gen");
    cout << "Got " << verses.size() << " verses!" << endl;
    for (int i = 0; i < verses.size(); i++) {
        cout << verses[i] << endl;
    }
}

void get_book_intro(SwordFacade& sword_facade)
{
    cout << "Text:" << endl;
    sword_facade.enableMarkup();
    string bookIntro = sword_facade.getBookIntroduction("GerNeUe", "Gen");
    cout << bookIntro << endl;
}

void get_strongs_entry(SwordFacade& sword_facade)
{
    StrongsEntry* entry = sword_facade.getStrongsEntry("G2766");
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

    entry = sword_facade.getStrongsEntry("H3069");
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
    SwordFacade sword_facade;

    /*std::vector<sword::SWModule*> localModules = sword_facade.getAllLocalModules();
    for (int i = 0; i < localModules.size(); i++) {
        sword::SWModule* module = localModules[i];
        cout << module->getName() << endl;
    }*/

    //sword_facade.refreshRemoteSources(true);

    cout << "SWORD version: " << sword_facade.getSwordVersion() << endl;

    /*show_repos(sword_facade);

    show_modules(sword_facade);*/

    /*int error = sword_facade.uninstallModule("KJV");

    if (error) {
        cout << "Error uninstalling module (write permissions?)\n";
    }*/

    /*int error = sword_facade.installModule("WLC");

    if (error) {
        cout << "Error installing module (write permissions?)\n";
    }*/

    //get_local_module(sword_facade);
    
    //get_repo_module(sword_facade);

    /*sword_facade.installModule("StrongsHebrew");
    sword_facade.installModule("StrongsGreek");*/

    //get_strongs_entry(sword_facade);

    //get_module_text(sword_facade);

    get_book_intro(sword_facade);

    //string translation = sword_facade.getSwordTranslation(string("/usr/share/sword/locales.d"), string("de"), string("locales"));
    //cout << translation << endl;

    /*vector<string> searchResults = sword_facade.getModuleSearchResults("ESV2001", "Lord");
    cout << "Got " << searchResults.size() << " results!" << endl;*/
    /*for (unsigned int i=0; i < searchResults.size(); i++) {
        cout << searchResults[i] << endl;
    }*/

    return 0;
}

