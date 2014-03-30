
#ifndef _INode_H_
#define _INode_H_

#include <string>
#include <list>

namespace ydle {
typedef enum {
	TYPE_ETAT =		1		// Node send data
	, TYPE_CMD				// ON/OFF sortie etc...
	, TYPE_ACK				// Acquit last command
	, TYPE_ETAT_ACK			// Node send data and whant ACK
} eMsgType ;

typedef enum {
	DATA_ETAT = 1			// On - OFF (4bits)
	, DATA_DEGREEC			// Degrée Celsius ** -204.7 à 204.7 (12bits)
	, DATA_DEGREEF			// Degrée Fahrenheit ** -204.7 à 204.7 (12bits)
	, DATA_PERCENT			// Pourcentage ** -100% à 100% (12bits)
	, DATA_DISTANCE			// Distance en Cm ** 0 à 4095 (12 bits)
	, DATA_WATT				// Watt ** 0 à 1048575 (20bits)
	, DATA_HUMIDITY			// Pourcentage humidité ** 0 à 100% (12bits)
	, DATA_PRESSION			// Pression en hpa 0 à 4095 (12bits)
} eDataType ;


typedef enum {
	CMD_LINK  = 0			//Link a node to the master 
	, CMD_ON				//Send a ON command to node data = N° output
	, CMD_OFF				//Send a OFF command to node data = N° output
	, CMD_RESET				//Ask a node to reset is configuration 
} eCmdType ;

// Définie un type de structure Signal_t
struct Frame_t
{
	uint8_t sender;
	uint8_t receptor;
	uint8_t type;
	uint8_t taille;	// data len + crc in BYTES
	uint8_t data[30];
	uint8_t crc;
	void	FormatCmd (uint8_t target, uint8_t sender, uint8_t param, uint8_t cmd)  ;
	void	DataToFrame(uint8_t transmitter, uint8_t recveiver, uint8_t type) ;
	void	AddCmd(uint8_t typeIn, uint8_t dataIn) ;
	void 	Dump(const char *) ;
};

class INode
{
public:
	typedef struct {
		int	type ;
		float val ;
	} sNodeData ;
	typedef  std::list<sNodeData> tNodeDataList ;
public:
	virtual ~INode () {} ;
	virtual std::string Name ()  = 0 ;
	virtual void FormatCmd (int target, int sender, int param, int cmd)  = 0 ;
	virtual int GetData (Frame_t *, tNodeDataList & list)  = 0 ;
protected:
	void SetVal (int node, const char *name, double val) ;
} ;


} ; // namespace ydle

#endif // _INode_H_
