
#include "Kernel.h"
#include "protocolRF.h"
extern "C" int LoadPlugins (ydle::Kernel &) ;

using namespace ydle ;


int	LoadPlugins (Kernel & k)
{
	k.RegisterProtocol (new protocolRF) ;

	return 1 ;
}
