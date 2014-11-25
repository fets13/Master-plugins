/**************************************************

**************************************************/

#ifndef __SERIAL_RF_H__
#define __SERIAL_RF_H__
#include "ISerial.h"
#include <string>
#include <pthread.h>
#include "rs232.h"
#include "IProtocol.h"

namespace ydle {

#define Err	printf
typedef const char * PCSTR ;

typedef enum {
	MSG_NORMAL = 0
		, MSG_ACK_OK
		, MSG_ACK_NOK
		, MSG_COUNT
} tMsgType ;
typedef struct SerialHeader
{
	uint8_t	begin ;
	uint8_t	type ;	// 0: message, 1:ACK_OK, 2:ACK_NOK
	uint8_t	size ;
	uint8_t	crc ;
} SerialHeader ;




#define SERIAL_MSG_MAX	64
#define SERIAL_MSG_BEGIN	0xAA
class SerialRF  : public IProtocol
{

	typedef struct tMsgMemo
	{
		Frame_t frame_;
		int		time_;
		int		count_;
		bool	sent_ ;
		tMsgMemo() {
			sent_ =	false ;
			count_ = 0 ;
			time_ = IProtocol::GetTimeMs() ;
		}
		void Update () {
			time_ = IProtocol::GetTimeMs() ;
			count_ ++ ;
		}
		bool	CheckTime (int timeout) {
			int time = IProtocol::GetTimeMs() ;
			return ( (time - time_) > timeout ||  time_ > time ) ;
			 
		}
	} tMsgMemo ;
	class MsgMemoList : public std::list <tMsgMemo *>
	{
		public:
			~MsgMemoList () {
				iterator it;
				for(it = begin(); it != end(); ++it) {
						Erase(it);
				}
			}
			void Erase (iterator it)
			{
				delete *it ;
				erase(it);
			}
			bool CheckErase (uint8_t receptor, uint8_t sender) {
				iterator it;
				for(it=begin(); it != end(); ++it) {
					tMsgMemo * msg = *it ;
					if(receptor == msg->frame_.receptor && sender == msg->frame_.sender) {
	//FETS							YDLE_DEBUG << "Remove ACK from pending list";
						Erase(it);
						return true ; // remove only one ACK at a time.
					}
				}
				return false ;

			}
			tMsgMemo * AddNew (Frame_t & frame) {
				tMsgMemo *newMsg = new tMsgMemo;
				memcpy(&newMsg->frame_, &frame, sizeof(Frame_t));
				push_back(newMsg);
				return newMsg ;
			}
	} ;
private:
	RS232	mRS ;
	SerialHeader mHeaderIn, mHeaderOut ;
	uint8_t		mDataIn[SERIAL_MSG_MAX] ;
	uint8_t		mDataOut[SERIAL_MSG_MAX] ;
public:
	SerialRF () ; 
	bool		InitSerial (PCSTR device, int baudrate) ;
	bool		ReadHeader () ;
	bool		ReadMsg () ;
	uint8_t		GetType () { return mHeaderIn.type ; }
	uint8_t		GetSize () { return mHeaderIn.size ; }
	puint8_t	GetData () { return mDataIn ; }
	bool		WriteMsg (puint8_t buf, int size) ;
	bool		SendAck(uint8_t status) ;
	bool		Loop() ;
	static
		void*	Loop (void* pParam);

	bool		Send() ;
protected:
	void 		ManageNewMsg (puint8_t msg, uint8_t sz) ;
	void		CheckACK() ;
public:
	virtual void Start () ;
	virtual std::string Name () { return "Protocol Serial-RF" ; }
	virtual void InitPlugin ()  ;
	virtual void SendMsg (Frame_t & frame) ;
	bool		mWaitAck ;
	bool		mSomething2Send ;
	int			mTimeOutAck, mTimeAck ;
	pthread_t	mThread ;
	Frame_t		mSendFrame ;
	MsgMemoList	mListMsg;


} ;

}; //namespace ydle

#endif // __SERIAL_RF_H__
