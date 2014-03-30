
#include "Kernel.h"
#include "Scripts.h"

using namespace ydle ;


extern "C" {
int	LoadPlugins (Kernel & k)
{
	k.RegisterFeature (new Scripts("lua-scripts")) ;

	return 1 ;
}
}
