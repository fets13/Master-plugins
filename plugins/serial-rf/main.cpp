
#include "Kernel.h"
#include "serial-rf.h"
extern "C" int LoadPlugins (ydle::Kernel &) ;

using namespace ydle ;


int	LoadPlugins (Kernel & k)
{
	k.RegisterProtocol (new SerialRF) ;

	return 1 ;
}
