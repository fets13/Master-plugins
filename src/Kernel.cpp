#include "Kernel.h"
#include "DynLoad.h"
#include "Dir.h"
#include <stdexcept>
#include <iostream>
#include <dlfcn.h>

using namespace std ;
using namespace ydle ;

Kernel::~Kernel ()
{
}



void	Kernel::LoadPlugins (std::string & dir)
{
	StringList files ;
	ListPlugins (dir.c_str(), files) ;

	for( StringList::iterator it = files.begin(); it != files.end(); ++it) {
		string full = dir + "/" + it->c_str() ;
		Plugin * p = new Plugin (full) ;
		if (p->Register(*this) ) {
			this->_loadedPlugins.insert( PluginMap::value_type(*it, p)) ;
		}
		else {
			delete p ;
		}
	}
	
}

Kernel::NodeList & Kernel::Nodes ()
{
	return _nodes ;
}

Kernel::ProtocolList & Kernel::Protocols ()
{
	return _protocols ;
}

Kernel::FeatureList & Kernel::Features ()
{
	return _features ;
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

IFeature * Kernel::Feature (string name)
{
	for(FeatureList::iterator it = _features.begin(); it != _features.end(); ++it) {
		IFeature * p = *it ;
		if (name == p->Name()) return p ;
	}
	return NULL ;
}



Plugin::Plugin(string & path) : mDyn(path.c_str())
{
}

int Plugin::Register (Kernel & k)
{
		return mDyn.ExecFuncTpl<int, Kernel&>  ("LoadPlugins", (Kernel &)k) ;
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
