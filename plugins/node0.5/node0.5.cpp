
#include "node0.5.h"

using namespace ydle ;
class Node0_5 : public INode 
{
public:
	std::string Name () { return "NODE VERSION 0.5" ; }
	virtual void FormatCmd (int target, int sender, int param, int cmd) {};
	virtual int GetData (Frame_t *, tNodeDataList & list) ;
private:
	int ExtractData(uint8_t * & ptr, int &itype, float &fvalue) ;
} ;

int	LoadPlugins (Kernel & k)
{
	k.RegisterNode (new Node0_5) ;

	return 1 ;
}

int Node0_5::GetData (Frame_t *frame, tNodeDataList & l)
{
	int length = frame->taille - 1 ; // length without CRC16
	if (length < 1) {
//FETS			YDLE_DEBUG << "Node0_5::GetData FAILED received : From "<< frame->sender << " Type : "<< frame->type << " Length : " << frame->taille << "\n";
		return 0 ;
	}

	uint8_t * pData = frame->data ;
	uint8_t * pFin = pData + length ;
	int	type = 0 ;
	int index = 0 ;
	float	fValue ;
	do {
		ExtractData (pData, type, fValue) ;
		printf ("%s:%d  index=%d type:%d value:%g\n", __FILE__, __LINE__, index, type, fValue) ;

		sNodeData nodeData ;
		nodeData.type = type ;
		nodeData.val = fValue ;
		l.push_back (nodeData) ;

index++ ;
	} while (pData < pFin) ;

	return l.size() ;
}
/*
 * Extract the value from the frame
 * Yes, I know this function should not be here. Denia.
 * */
int Node0_5::ExtractData(uint8_t * & ptr, int &itype, float &fvalue)
{
	int ivalue ;


	itype=(uint8_t)*ptr>>4;

	switch(itype)
	{
	// 4 bits no signed
	case DATA_ETAT :
		ivalue=*ptr&0x0F;
		ptr++;
		break;

		// 12 bits signed
	case DATA_DEGREEC:
	case DATA_DEGREEF :
	case DATA_PERCENT :
		{
			bool bifValueisNegativ = (*ptr&0x8) ;
			ivalue=(*ptr&0x07)<<8;
			ptr++;
			ivalue+=*ptr;
			ptr++;
			if(bifValueisNegativ) ivalue=ivalue *(-1);
			fvalue = float(ivalue) / 20.0 ;
		}
		break;

		// 12 bits no signed
	case DATA_DISTANCE:
	case DATA_PRESSION:
	case DATA_HUMIDITY:
		ivalue=(*ptr&0x0F)<<8;
		ptr++;
		ivalue+=*ptr;
		ptr++;
		fvalue = float(ivalue) ;
		if (itype == DATA_HUMIDITY) fvalue /= 40.0 ;
		break;

		// 20 bits no signed
	case DATA_WATT  :
		ivalue=(*ptr&0x0F)<<16;
		ptr++;
		ivalue+=(*ptr)<<8;
		ptr++;
		ivalue+=*ptr;
		ptr++;
		fvalue = float(ivalue) ;
		break;
	}


	return 0;
}



