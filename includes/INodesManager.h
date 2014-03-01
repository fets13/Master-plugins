/*
 * NodesManager.h
 *
 *  Created on: Fev 24, 2014
 *      Author: fets
 */


#ifndef INodesManager_H
#define INodesManager_H

namespace ydle {


class INodesManager
{
public:
	virtual ~INodesManager () {}
	virtual int	SendCmd (int target, int sender, int param, int cmd) = 0 ;
} ;

} // namespace ydle

#endif //  INodesManager_H
