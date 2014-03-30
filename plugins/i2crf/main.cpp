
#include "Kernel.h"
#include "protocol.h"
extern "C" int LoadPlugins (ydle::Kernel &) ;

using namespace ydle ;


int	LoadPlugins (Kernel & k)
{
	k.RegisterProtocol (new protocolI2CRF) ;

	return 1 ;
}
