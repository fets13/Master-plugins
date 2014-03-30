#include "INode.h"
#include "DataAccess.h"

#include <sstream>

using namespace ydle ;
using namespace std ;

void INode::SetVal (int node, const char *name, double val)
{
	stringstream str ;
	str << node << "." << name ;
	DATA[str.str()] = val ;
}
