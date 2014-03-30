#include "Dir.h"
	
using namespace ydle ;
using namespace std ;

#include <dirent.h>
#include <fnmatch.h>
#include <iostream>

using namespace ydle ;

void ydle::ListPlugins (const char * dir, StringList & files)
{
	ListFiles (dir, "*.so", files) ;
}

void ydle::ListFiles (const char * dir, const char * pattern, StringList & files)
{

	// open directory
	DIR *dp;
	struct dirent *dirp;
	if((dp = opendir(dir)) == NULL) {
		cout << "Error(" << errno << ") opening " << dir << endl;
		return ;
	}
	// while there is an entry
	while ((dirp = readdir(dp)) != NULL) {
		// is the entry a regular file ?
		if (dirp->d_type != DT_REG) continue ;
		char * name = dirp->d_name ;
		// ignore file not matching 'so' extension
		if ((pattern != NULL) && fnmatch(pattern, name, FNM_CASEFOLD) != 0) continue ;
		printf ("ListFiles : looking : %s\n", name) ;

		files.push_back(string(name));
	}
}




