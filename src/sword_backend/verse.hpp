#ifndef _VERSE
#define _VERSE

#include <string>

class Verse
{
public:
    Verse() {}
    virtual ~Verse() {}

    std::string reference;
    int absoluteVerseNumber;
    std::string content;
};

#endif // _VERSE