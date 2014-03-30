#include "DataAccess.h"
	
using namespace ydle ;
using namespace std ;


DataAccess DataAccess::data ;

bool DataAccess::Get (string nom, double &val) 
{
	iterator it = find (nom) ;
	if (it == end()) {
		printf ("DataAccess::Get : nom %s not found\n", nom.c_str());
		return false ;
	}
	val = it->second ;
	return true ;

}
