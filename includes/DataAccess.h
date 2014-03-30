
#ifndef DataAccess_H
#define DataAccess_H
#include <map>
#include <string>

namespace ydle {

class DataAccess : public std::map <std::string, double>
{
public:
	bool	Get(std::string n, double & v) ;
	static DataAccess data ;
};

#define DATA DataAccess::data

} ;
#endif // DataAccess_H
