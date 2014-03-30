#ifndef _Dir_H_
#define _Dir_H_

#include <list>
#include <string>

namespace ydle {

typedef std::list<std::string> StringList ;

void ListFiles (const char * dir, const char * pattern, StringList & files) ;
void ListPlugins (const char * dir, StringList & files) ;

}

#endif // _Dir_H_
