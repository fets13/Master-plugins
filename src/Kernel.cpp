#include "Kernel.h"
#include "DynLoad.h"
#include <list>
#include <stdexcept>
#include <iostream>
#include <dlfcn.h>
#include <dirent.h>
#include <fnmatch.h>

using namespace std ;
using namespace ydle ;

Kernel::~Kernel ()
{
	this->Free (_nodes) ;
	this->Free (_protocols) ;
}



void	Kernel::LoadPlugins (std::string & dir)
{
	list<string> files = list<string>();

	// open directory
	DIR *dp;
	struct dirent *dirp;
	if((dp = opendir(dir.c_str())) == NULL) {
		cout << "Error(" << errno << ") opening " << dir << endl;
		return ;
	}
	// while there is an entry
	while ((dirp = readdir(dp)) != NULL) {
		// is the entry a regular file ?
		if (dirp->d_type != DT_REG) continue ;
		char * name = dirp->d_name ;
		// ignore file not matching 'so' extension
		if (fnmatch("*.so", name, FNM_CASEFOLD) != 0) continue ;

			files.push_back(string(name));
		string full = dir + "/" + dirp->d_name ;
		Plugin * p = new Plugin (full) ;
		if (p->Register(*this) ) {
			this->_loadedPlugins.insert( PluginMap::value_type(dirp->d_name, p)) ;
		}
		else {
			delete p ;
		}
	}
#if 0
	for( list<string>::iterator it = files.begin(); it != files.end(); ++it) {
		string full = dir + "/" + it->c_str() ;
		Plugin * p = new Plugin (full) ;
		if (p->Register(*this) ) {
			this->_loadedPlugins.insert( PluginMap::value_type(*it, p)) ;
		}
		else {
			delete p ;
		}
	}
#endif
}

Kernel::NodeList & Kernel::Nodes ()
{
	return _nodes ;
}


Kernel::ProtocolList & Kernel::Protocols ()
{
	return _protocols ;
}

Kernel::PluginMap & Kernel::Plugins ()
{
	return _loadedPlugins ;
}

INode *	 Kernel::Node (string name)
{
	for(NodeList::iterator it = _nodes.begin(); it != _nodes.end(); ++it) {
		INode * p = *it ;
		if (name == p->Name()) return p ;
	}
	return NULL ;
}

IProtocol * Kernel::Protocol (string name)
{
	for(ProtocolList::iterator it = _protocols.begin(); it != _protocols.end(); ++it) {
		IProtocol * p = *it ;
		if (name == p->Name()) return p ;
	}
	return NULL ;
}



Plugin::Plugin(string & path) : mDyn(path.c_str())
{
}

bool Plugin::Register (Kernel & k)
{
		return mDyn.ExecFuncTpl<Kernel&>  ("LoadPlugins", (Kernel &)k) ;
}
#if 0
  /// <summary>Shared library loading and access on windows</summary>
  class SharedLibrary {

    /// <summary>Handle by which shared objects are referenced</summary>
    public: typedef void * HandleType;

    /// <summary>Loads the shared object from the specified path</summary>
    /// <param name="path">
    ///   Path of the shared object that will be loaded
    /// </param>
    public: static HandleType Load(const std::string &path) {
      std::string pathWithExtension = std::string("./lib") + path + ".so";

      void *sharedObject = ::dlopen(pathWithExtension.c_str(), RTLD_NOW);
      if(sharedObject == NULL) {
        throw std::runtime_error(
          std::string("Could not load '") + pathWithExtension + "'"
        );
      }

      return sharedObject;
    }

    /// <summary>Unloads the shared object with the specified handle</summary>
    /// <param name="sharedLibraryHandle">
    ///   Handle of the shared object that will be unloaded
    /// </param>
    public: static void Unload(HandleType sharedLibraryHandle) {
      int result = ::dlclose(sharedLibraryHandle);
      if(result != 0) {
        throw std::runtime_error("Could not unload shared object");
      }
    }

    /// <summary>Looks up a function exported by the shared object</summary>
    /// <param name="sharedLibraryHandle">
    ///   Handle of the shared object in which the function will be looked up
    /// </param>
    /// <param name="functionName">Name of the function to look up</param>
    /// <returns>A pointer to the specified function</returns>
    public: template<typename TSignature>
    static TSignature *GetFunctionPointer(
      HandleType sharedLibraryHandle, const std::string &functionName
    ) {
      ::dlerror(); // clear error value

      void *functionAddress = ::dlsym(
        sharedLibraryHandle, functionName.c_str()
      );

      const char *error = ::dlerror(); // check for error
      if(error != NULL) {
        throw std::runtime_error("Could not find exported function");
      }

      return reinterpret_cast<TSignature *>(functionAddress);
    }

  };
#endif
