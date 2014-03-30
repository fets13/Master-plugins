#ifndef _Ident_H_
#define _Ident_H_

#include <string>

class Ident
{
public:
	Ident (std::string n="") : mName(n) {}
	virtual std::string Name ()  { return mName ;}
	const char * NameStr ()  { return mName.c_str() ;}
private:
	std::string		mName ;
} ;

#endif // _Ident_H_
