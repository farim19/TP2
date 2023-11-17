#ifndef MAQUINAESTADOS_H
#define MAQUINAESTADOS_H
#include <stdint.h>
typedef enum {stHeader=0,
              stExit,
              stSuma,
              stEnd} statesValues;

 typedef struct
{
    statesValues actState;
    unsigned char suma;
} states;

typedef bool (*fcnState)(states *, unsigned char);

bool stateHeader(states *s, unsigned char newVal);
bool stateGetExit(states *s, unsigned char newVal);
bool stateGetSum(states *s, unsigned char newVal);
bool stateGetend(states *s, unsigned char newVal);


#endif // MAQUINAESTADOS_H
