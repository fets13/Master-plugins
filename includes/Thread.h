#ifndef _Thread_H_
#define _Thread_H_

#include <thread>
#include "Ident.h"
#include "time.h"

namespace ydle {

class Thread : public Ident {
public:
	Thread (std::string n = "???") ;
	virtual ~Thread () ;

	void	Start() ;
	void	Stop() ;
	void	Pause() ;
	void	SetPauseMs (uint32_t val) ;
protected:
	virtual void ThreadBegin () {}
	virtual void ThreadAction () ;
	virtual void ThreadEnd () {}
	virtual void ThreadMain () ;
protected:
	std::thread *thread_t;
	bool running;
	struct timespec valPause ;
} ;

} ; // namespace ydle
#endif // _Thread_H_
