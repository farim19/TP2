#include <driver/gpio.h>
#include <driver/uart.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <esp_log.h>

#define ledPin GPIO_NUM_2

typedef enum {stHeader=0,
              stValon,
              stValoff,
              stSuma,
              stEnd} statesValues;
    

typedef struct{
uint16_t valon;
uint16_t valoff;
}vals;

typedef union{
uint16_t val;
uint8_t valvec[2];
}valor;

typedef struct{
valor value;
uint8_t indice;
}total;

typedef struct{
    statesValues actState;
    total ValorOn;
    total ValorOff;
    unsigned int sum;
}states;

typedef bool (*fcnState)(states *,  char);

bool stateHeader(states *s,  char newVal);
bool stateGetValon(states *s,  char newVal);
bool stateGetValoff(states *s,  char newVal);
bool stateGetSuma(states *s,  char newVal);
bool stateGetEnd(states *s,  char newVal);

QueueHandle_t colaDatos;

void hardware_init(void);

int ledOn();
int ledOff();



void procesar( void * cola )
{
        static fcnState listStates[6] = {
        stateHeader,
        stateGetValon,
        stateGetValoff,
        stateGetSuma,
        stateGetEnd};

    static states vStates = {stHeader, {{0},0}, {{0},0}, 0};
   vals xD;
    uint8_t sum,end=0xB0,head2=0x0B,exit=0xCC;
    char auxi;
    QueueHandle_t * aux = (QueueHandle_t *)cola; 
    while(1){
        fread((void *)&auxi, 1, 1, stdin);
        if(listStates[vStates.actState](&vStates, auxi)){
        sum=head2+exit;
        xD.valon=vStates.ValorOn.value.val;
        xD.valoff=vStates.ValorOff.value.val;
        xQueueSend((*aux), (void*)&xD, 0); 
        vTaskDelay(50/portTICK_PERIOD_MS);
        fwrite(&head2, 1, 1, stdout);
        fwrite(&exit, 1, 1, stdout);
        fwrite(&sum, 1, 1, stdout);
        fwrite(&end, 1, 1, stdout);
        }
        vTaskDelay(50/portTICK_PERIOD_MS);
    }
}
void leds( void * cola )
{
  uint16_t valon=1000, valoff=1000;
  vals xDD;
  QueueHandle_t * aux = (QueueHandle_t *)cola; 
   hardware_init();
    while(1){
         if(xQueueReceive(*aux, &xDD, 0)){
        valon=xDD.valon;
        valoff=xDD.valoff;
        }
        ledOn();
        vTaskDelay(valon/portTICK_PERIOD_MS);
        ledOff();
        vTaskDelay(valoff/portTICK_PERIOD_MS);
    }
}
void app_main() {
    colaDatos =  xQueueCreate(50, sizeof(vals)); 
    xTaskCreate(procesar, "Proceso de datos", configMINIMAL_STACK_SIZE + 4096, &colaDatos, 2, NULL);
    xTaskCreate(leds, "Uso de los leds", configMINIMAL_STACK_SIZE + 4096, &colaDatos, 2,NULL);
}
void hardware_init(void){
  gpio_config_t gpioLed =
  {
   .pin_bit_mask = 1ULL << ledPin,
   .mode = GPIO_MODE_DEF_OUTPUT,    
   .pull_up_en = GPIO_PULLUP_DISABLE,
   .pull_down_en = GPIO_PULLDOWN_DISABLE,
   .intr_type = GPIO_INTR_DISABLE
  };   
  ESP_ERROR_CHECK(gpio_config(&gpioLed));
ESP_ERROR_CHECK(gpio_set_level(ledPin, 0));
}
int ledOn(){
  ESP_ERROR_CHECK(gpio_set_level(ledPin, 1));
  return 0;
}
int ledOff(){
  ESP_ERROR_CHECK(gpio_set_level(ledPin, 0));
  return 0;
}
bool stateHeader(states *s,  char newVal)
{
    if(newVal == 0x0B)
    {
        s->ValorOn.indice = 0;
        s->ValorOff.indice = 0;
        s->sum = 0;
        s->actState = stValon;
    }
    else
        s->actState = stHeader;
    return false;
}
bool stateGetValon(states *s,  char newVal)
{
 s->ValorOn.value.valvec[s->ValorOn.indice]=newVal;
 s->ValorOn.indice++;
 s->sum+=newVal;
 if(s->ValorOn.indice==2){
 s->actState=stValoff;
 }
     return false;
}
bool stateGetValoff(states *s,  char newVal)
{
 s->ValorOff.value.valvec[s->ValorOff.indice]=newVal;
 s->ValorOff.indice++;
 s->sum+=newVal;
 if(s->ValorOff.indice==2){
 s->actState=stSuma;
 }
     return false;
}
bool stateGetSuma(states *s,  char newVal)
{
  if(newVal == (s->ValorOn.value.val+s->ValorOff.value.val)%256){
        s->actState=stEnd;
  }
  else{
    s->actState = stHeader;
  }
    return false;
}
bool stateGetEnd(states *s,  char newVal)
{bool ret=false;
  if(newVal == 0xB0){
     ret=true;
}
s->actState = stHeader;
    return ret;
}