#include "maquinaestados.h"
#include "dialog.h"

bool stateHeader(states *s, unsigned char newVal)
{
    if(newVal == 0x0B)
    {
        s->actState = stExit;
        s->suma=0;
        s->suma+=newVal;
    }
    else
        s->actState = stHeader;
    return false;
}
bool stateGetExit(states *s, unsigned char newVal)
{
    if(newVal == 0xCC){
        s->actState=stSuma;
        s->suma+=newVal;
    }else
         s->actState=stHeader;
    return false;
}

bool stateGetSum(states *s, unsigned char newVal)
{
    if(newVal == (s->suma)%256)
        s->actState=stEnd;
     else
      s->actState=stHeader;
    return false;
}
bool stateGetend(states *s, unsigned char newVal)
{bool ret=false;
    if(newVal==0xB0)
          ret=true;
    s->actState = stHeader;
      return ret;
    }

