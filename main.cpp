#include "sword_facade.hpp"

#include <vector>
#include <iostream>
#include <swmodule.h>

using namespace std;
using namespace sword;

int main(int argc, char** argv)
{
  SwordFacade sword_facade;

  //sword_facade.refreshRemoteSources(true);

  cout << "REPOSITORIES:" << endl;
  vector<string> repoNames = sword_facade.getRepoNames();
  for (unsigned int i = 0; i < repoNames.size(); i++) {
    cout << repoNames[i] << endl;
  }
  cout << endl;

  cout << "German MODULES of CrossWire:" << endl;
  vector<SWModule*> germanModules = sword_facade.getRepoModulesByLang("CrossWire", "de");
  for (unsigned int i = 0; i < germanModules.size(); i++) {
    SWModule* currentModule = germanModules[i];

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
    cout << moduleLocked;

    cout << endl;
  }
  cout << endl;

  return 0;
}
