#include <Arduino.h>
#include <SPI.h>
#include <stdio.h>
#include "Pines.h"
#include "LoRa_E32.h"
#include "LoRa.h"


//Definiciones para la libreria
#define LORA_BW               125E3         //Bandwith
#define LORA_SP               7             //Spreading Factor
#define LORA_CHANNEL          915E6         //Canal
#define LORA_SYNCWORD         0x12          
#define LORA_CR               5             //Coding rate (4/x)
#define LORA_PL               8             //Preamble length (x+4)
#define LORA_ADDRESS          4
#define LORA_SEND_TO_ADDRESS  2

byte MODO = 0;
byte MODO_ANT = 0;

byte e;
char message_received[100];
char message_sent[]="hola";
int delay_time=5;
int counter=0;

//Configuramos la clase para el módulo 3
LoRa_E32 E32_433(RX, TX, &Serial1, AUX, M0, M1);  // e32 TX e32 RX 

void Ini_module3();
void End_module3();
void EnableDevice(void);
uint8_t send_message(uint8_t module, char *message, uint8_t seconds, boolean control=false);
uint8_t receive_message(uint8_t module, char seconds, boolean control=false);

void setup()
{
  delay(10);

  //Inicializamos los pines de LED 
  pinMode(LED, OUTPUT);

  //Iniciamos comunicación para el Módulo 3
  E32_433.begin();

  // Abrimos comunicaciones para observar 
  Serial.begin(115200); 

  //Iniciamos los modulos en reset
  digitalWrite(M0,1);
  digitalWrite(M1,1);
  
  //Imprimimos configuraciones actuales
  Serial.print("BW: ");
  Serial.println(LORA_BW);
  Serial.print("SP: ");
  Serial.println(LORA_SP);
  Serial.print("CR: 4/");
  Serial.println(LORA_CR);

  EnableDevice();
}

void loop(void)
{ 
  uint8_t status;
  

  //Comentar o descomentar para los módulos en modo de transmisión/recepción
  //status=send_message(MODO, message_sent, delay_time, false); //(Modulo de emision, mensaje a enviar, delay entre mensajes, con/sin mensaje de confirmación)
  status=receive_message(MODO, 20, true);


  //Serial.println(status);
  delay(100);
}

uint8_t send_message(uint8_t module, char *message, uint8_t seconds, boolean control){
  uint8_t status = 0;
  uint8_t i = 0;

  //enviamos un mensaje
  char *Count = (char*)malloc(40);
  sprintf(Count, "N°: %u, Msg: ", counter);


  //enviamos un mensaje
  digitalWrite(LED,1);
  ResponseStatus rs = E32_433.sendMessage(strcat(Count,message));
  digitalWrite(LED,0);
  Serial.println(rs.getResponseDescription());
  if (rs.getResponseDescription() == "Success"){
    Serial.println("Messaje sent");
    Serial.println(message);
    Serial.print("Messaje N°: ");
    Serial.println(counter);
    status=1;
  } else{
    Serial.println("Error");
  }
  free(Count); 

  if(control == true){
    for (i=0; i < 10; i++) {
      if (E32_433.available() > 1){
        ResponseContainer rs = E32_433.receiveMessage();
        if (rs.status.getResponseDescription() == "Success"){
          if (rs.data == "OK")
          Serial.println("Message confirmed");
          break;
        } 
      } else {
          Serial.println("Waiting for confirmation");
      }
      delay(1000);
      if (i==10) Serial.println("No confirmation");
    } 

  }
  Serial.println(""); 
  counter++;
  delay(seconds*1000); 
  return status;
}

uint8_t receive_message(uint8_t module, char seconds, boolean control){
  uint8_t i=0;
  uint8_t status=0;

  //Esperamos a recibir un mensaje
  while (i < seconds) {
    if (E32_433.available() > 1){
      ResponseContainer rs = E32_433.receiveMessage();
      if (rs.status.getResponseDescription() == "Success"){
        Serial.print("Received packet: '");
        Serial.println(rs.data);

        //Enviamos confirmación
        if (control==true){
          Serial.println("Sending confirmation"); 
          E32_433.sendMessage("OK");
        }

        //Prendemos el led por 0.5s
        digitalWrite(LED,1);
        delay(500);
        digitalWrite(LED,0);
        status=1;
        break;
      } 
    } else {
        Serial.println("Waiting for message");
    }
    delay(1000);
    i++;
  }

  
  Serial.println(""); 
  
  if (status == 0) { Serial.println("No message received"); }
  Serial.println("");
  return status;
}



void Ini_module3(){
  digitalWrite(M0,1);
  digitalWrite(M1,1);
  
  ResponseStructContainer c;
	c = E32_433.getConfiguration();
	// It's important get configuration pointer before all other operation
	Configuration configuration = *(Configuration*) c.data;
	Serial.println(c.status.getResponseDescription());
	Serial.println(c.status.code);

	printParameters(configuration);
	configuration.ADDL = 0x0;
	configuration.ADDH = 0x1;
	configuration.CHAN = 0x19;

	configuration.OPTION.fec = FEC_1_ON;
	configuration.OPTION.fixedTransmission = FT_TRANSPARENT_TRANSMISSION;
	configuration.OPTION.ioDriveMode = IO_D_MODE_PUSH_PULLS_PULL_UPS;
	configuration.OPTION.transmissionPower = POWER_30;
	configuration.OPTION.wirelessWakeupTime = WAKE_UP_1250;

	configuration.SPED.airDataRate = AIR_DATA_RATE_000_03;
  //configuration.SPED.airDataRate =AIR_DATA_RATE_111_192;
	configuration.SPED.uartBaudRate = UART_BPS_9600;
	configuration.SPED.uartParity = MODE_00_8N1;

	// Set configuration changed and set to not hold the configuration
	ResponseStatus rs = E32_433.setConfiguration(configuration, WRITE_CFG_PWR_DWN_LOSE);
	Serial.println(rs.getResponseDescription());
	Serial.println(rs.code);
	printParameters(configuration);
	c.close();
}



void EnableDevice(void){

  Serial.println("modo 3");
  Ini_module3(); 

}