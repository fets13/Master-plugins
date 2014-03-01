#include <stdio.h>
#include <stdlib.h>
#include <stdexcept>
#include <string>
#include "DynLoad.h"

#include <dlfcn.h>

using namespace std ;


DynLoad::DynLoad (PCSTR dll)
{
	mHandle = NULL ;
	mDllName.empty() ;
	SetDllName (dll) ;
}

DynLoad::~DynLoad()
{
	if (mHandle) {
		dlclose (mHandle) ;
		mHandle = NULL ;
	}
}


void DynLoad::SetDllName (PCSTR dll)
{
    if (dll == NULL) return ;
    if (mHandle != NULL) {
	   string s = "DynLoad::SetDllName(" ;
	   s += string(dll) + ") FAILED  already allocated " ;
	 printf ("Allocator::SetDllName(%s) FAILED mHandle already allocated ", dll) ;
      throw std::runtime_error(s);
    }

	mHandle = dlopen (dll, RTLD_NOW|RTLD_GLOBAL);
    if (! mHandle) {
	   string s = "DynLoad::SetDllName(" ;
	   s += string(dll) + ").dlopen FAILED : " ;
	   s += dlerror() ;
      throw std::runtime_error(s);
    }
    mDllName = dll ;
}
tDynLoadFunc DynLoad::GetSymbol (PCSTR funcName)
{
    if (! mHandle) {
		printf ("DynLoad::GetSymbol(%s) FAILED : handle==NULL \n", funcName) ;
	return NULL;
    }
	tDynLoadFunc allocator = (tDynLoadFunc)dlsym (mHandle, funcName);
    if (allocator == NULL)  {
		PCSTR erreur = dlerror() ;
	   printf	("DynLoad::GetSymbol.dlsym(%s) FAILED <%s>  dllName<%s> \n", funcName, erreur, mDllName.c_str()) ;
	return NULL ;
    }

    return allocator ;
}
void * DynLoad::ExecFunc (PCSTR funcName)
{
     typedef void * (* tPtrFunc) () ;
    tPtrFunc allocator = (tPtrFunc)GetSymbol (funcName) ;
    if (allocator == NULL) {
	   printf ("DynLoad::ExecFunc(%s) FAILED \n", funcName) ;
	return NULL;
    }
    printf ("DynLoad(%s) %d\n", funcName, __LINE__) ;
    return (void*)((*allocator)()) ;
}
