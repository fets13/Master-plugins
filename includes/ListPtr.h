#ifndef _ListPtr_H_
#define _ListPtr_H_
#include <list>
template <class T>
class ListPtr : public std::list<T*>
{
public:
	virtual ~ListPtr () {
		Free (*this) ;
	}

	template <class Type>
    static void Free (std::list<Type*> & l) {
		typename std::list<Type*>::reverse_iterator it ;
		for (it = l.rbegin(); it != l.rend(); ++it) {
			delete *it;
		}
    }

} ;
#endif // _ListPtr_H_
