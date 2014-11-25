
#ifndef _IProtocol_H_
#define _IProtocol_H_

#include "INode.h"
#include "SettingsParser.h"
#include <boost/signals2.hpp>
#include <boost/bind.hpp>
#include <sys/time.h>



namespace ydle {
class IProtocol
{
public:
	IProtocol () {} ;
	virtual ~IProtocol () {} ;
	virtual std::string Name ()  = 0 ;
	virtual void Start ()  = 0 ;
	virtual void debugMode(bool mode = true) {} ;
	virtual void InitPlugin () = 0 ;
	virtual void SendMsg (Frame_t & frame) = 0 ;

	template <class T>
	void	Subscribe (T* o, void (T::*func) (Frame_t*))
	{
		_signalNewFrame.connect(bind(func, o,  _1));
	}

	static int		GetTimeMs() {
		struct timeval localTime;
		gettimeofday(&localTime, NULL); 
		int iTime=localTime.tv_sec * 1000000;
		iTime+=localTime.tv_usec;
		return (iTime/1000);
	}

protected:
	void Notify (Frame_t *pFrame)
	{
		_signalNewFrame(pFrame);
	}
protected:
	boost::signals2::signal<void (Frame_t*) > _signalNewFrame;
	
	
} ;

} ; // namspace ydle 

#endif // _IProtocol_H_
