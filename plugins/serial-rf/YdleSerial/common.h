
#define DEBUG_SERIAL
#ifdef DEBUG_SERIAL
#define D1(a) Serial.println(a) ;
#define D2(a,b) Serial.print(a); Serial.println(b) ;
#define D3(a,b,c) Serial.print(a); D2(b,c) ;
#define D4(a,b,c,d) Serial.print(a); D3(b,c,d) ;
#define D2arg(a,b,c) Serial.print(a); Serial.println(b,c) ;
#else // DEBUG_SERIAL
#define D1(a) 
#define D2(a,b) 
#define D3(a,b,c) 
#define D4(a,b,c,d)
#define D2arg(a,b,c) 
#endif // DEBUG_SERIAL

