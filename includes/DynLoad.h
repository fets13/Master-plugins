#ifndef __DYN_LOAD_H__
#define  __DYN_LOAD_H__

#include <string>
#include <stdexcept>

typedef	void *	tDynLoadHandle ;
typedef	void *	tDynLoadFunc ;
typedef	const char * PCSTR ;

class DynLoad
{

public:
	DynLoad (PCSTR dll = NULL) ;
	virtual ~DynLoad () ;
	void	SetDllName (PCSTR dll) ;
	void *	ExecFunc (PCSTR funcName) ;
	tDynLoadFunc	GetSymbol (PCSTR funcName) ;

	template <typename tParam>
	bool	ExecFuncTpl (PCSTR funcName, tParam param)
	{
		typedef int (* tPtrFunc) (tParam) ;
		tPtrFunc allocator = (tPtrFunc)GetSymbol (funcName) ;
		if (allocator == NULL) {
			std::string s ("ExecFuncTpl :") ;
			s += std::string(funcName) + " FAILED " ;
			throw std::runtime_error(s);
			return false ;
		}
	    allocator (param) ;
		return true ;
	}

protected:
protected:
    std::string		mDllName ;
	tDynLoadHandle 	mHandle ;
    
} ;


#endif //  __DYN_LOAD_H__
