
#ifndef _IFeature_H_
#define _IFeature_H_

#include <string>
#include "Ident.h"

namespace ydle {
class IFeature : public Ident
{
public:
	IFeature (std::string name) : Ident(name) {} ;
	virtual ~IFeature () {} ;
	virtual void Init ()  = 0 ;
	virtual void Start ()  = 0 ;

protected:
} ;

} ; // namspace ydle 

#endif // _IFeature_H_
