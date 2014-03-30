#include "Thread.h"

using namespace ydle ;
using namespace std ;

Thread::Thread (string nom) : Ident (nom)
{
	running = false ;
	SetPauseMs (1000) ;
}

Thread::~Thread ()
{
}

void Thread::SetPauseMs (uint32_t val)
{
	uint32_t s = val / 1000 ;
	valPause.tv_sec = s ;
	valPause.tv_nsec = (val - (s*1000)) * 1000000L;
	printf ("Thread::SetPauseMs(%d) : s:%ld nano:%ld\n", val, valPause.tv_sec, valPause.tv_nsec) ;
}

void Thread::Start()
{
	thread_t = new std::thread(&Thread::ThreadMain, this);
}

void Thread::Stop()
{
	running = false;
}

void Thread::ThreadMain()
{
	running = true ;
	ThreadBegin () ;
	while (running) {
		ThreadAction () ;
	}
	ThreadEnd () ;
	running = false ;
}

// No action, sleep only
void Thread::ThreadAction()
{
		Pause ();
}

void Thread::Pause()
{
		nanosleep(&valPause, NULL);
}

