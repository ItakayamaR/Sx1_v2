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
#define LORA_PW               20            //Potencia de transmisión (dBm)


byte e;
char message_received[100];
char message_sent[]="hola";
int delay_time=1;
int counter=0;

//Declaración de funciones
void Ini_module_spi(void); 
void End_module_spi();
uint8_t send_message(char *message, uint8_t seconds, boolean control=false);
uint8_t receive_message(char seconds, boolean control=false);

void setup()
{
  delay(10);

  //Inicializamos el pin de led
  pinMode(LED, OUTPUT);

  //Inicializamos pines del módulo 1
  pinMode(DIO0_1, INPUT);
  pinMode(DIO1_1, INPUT);
  pinMode(DIO2_1, INPUT);
  pinMode(RST1, OUTPUT);
  pinMode(SS1, OUTPUT);
  digitalWrite(SS1,HIGH);

  // Abrimos comunicacion serial
  Serial.begin(115200); 

  //Iniciamos los modulos en reset
  digitalWrite(RST1,0);
  
  //Imprimimos configuraciones actuales
  Serial.print("BW: ");
  Serial.println(LORA_BW);
  Serial.print("SP: ");
  Serial.println(LORA_SP);
  Serial.print("CR: 4/");
  Serial.println(LORA_CR);

  //Configuramos la conexión SPI
  Ini_module_spi();

}

void loop(void)
{ 
  uint8_t status;
  
  //Comentar o descomentar para los módulos en modo de transmisión/recepción
  //status=send_message(message_sent, delay_time, false); //(Modulo de emision, mensaje a enviar, delay entre mensajes, con/sin mensaje de confirmación)
  status=receive_message(20, false);


  //Serial.println(status);
  delay(100);
}

uint8_t send_message(char *message, uint8_t seconds, boolean control){
  uint8_t status = 0;
  uint8_t i = 0;
  
  // Enviamos un mensaje   
  Serial.println("Start sending message"); 

  //Prendemos el led
  digitalWrite(LED,1);
  LoRa.beginPacket();
  
  LoRa.print("N°: ");
  LoRa.print(counter);
  LoRa.print(" ");
  LoRa.print("Msg: ");
  LoRa.print(message);

  //LoRa.print("A");
  LoRa.endPacket();
  digitalWrite(LED,0);

  Serial.print("Message sent: ");
  Serial.println(message);
  Serial.print("Message n° ");
  Serial.println(counter);
  status=1;

  if (control == true){
    LoRa.receive();
    for(i=0; i<10; i++) {
      int packetSize = LoRa.parsePacket();
      if (packetSize) {
        String LoRaData = LoRa.readString();
        if (LoRaData=="OK"){
          Serial.println("Message confirmed");
          break;
        }
      } else {
        Serial.println("Waiting for confirmation");
      } 
      if (i==10) Serial.println("No confirmation");
      delay(1000);
    }  
  }  

  Serial.println(""); 
  counter++;
  delay(seconds*1000); 
  return status;
}

uint8_t receive_message(char seconds, boolean control){
  uint8_t i=0;
  uint8_t status=0;
  
  LoRa.receive();
  //Esperamos a recibir un mensaje
  while(i < seconds ){
    int packetSize = LoRa.parsePacket();
    if (packetSize) {
      // received a packet
      Serial.print("N° of bytes received: ");
      Serial.println(packetSize);
      Serial.print("Received packet: '");
      // read packet
      while (LoRa.available()) { 
        Serial.print(LoRa.readString()); 
      }

      // Imprimimos el RSSI
      Serial.println("'");
      Serial.print("With RSSI: ");
      Serial.print(LoRa.packetRssi());

      // Imprimimos el SNR
      Serial.print(" and SNR: ");
      Serial.println(LoRa.packetSnr());
      
      if(control == true){
        delay(1000);
        Serial.println("Sending confirmation"); 
        LoRa.beginPacket();
        LoRa.print("OK");
        LoRa.endPacket();
      }

      //Prendemos el led por 0.5s
      digitalWrite(LED,1);
      delay(500);
      digitalWrite(LED,0);

      status=1;
      break;

    } else{
      Serial.println("Waiting for message");
    }
    delay(1000);
    i++;
  }

  

  if (status == 0) { Serial.println("No message received"); }
  Serial.println("");
  return status;
}

void Ini_module_spi(void) 
{
  //Inicializamos SPI en los pines correspondientes
  LoRa.setPins(SCK, MISO, MOSI, SS1, RST1, DIO0_1);

  //Seteamos la frecuencia deseada y los canales  y esperamos que se configure
  while (!LoRa.begin(LORA_CHANNEL)) {
    Serial.println(".");
    delay(500);
  }

  LoRa.setSyncWord(LORA_SYNCWORD);              //Seteamos la dirección de sincronización
  LoRa.setSpreadingFactor(LORA_SP);             //Seteamos el Spreading Factor (SP)
  LoRa.setSignalBandwidth(LORA_BW);             //Seteamos El ancho de banda
  LoRa.setCodingRate4(LORA_CR);                 //Seteamos el Coding rate (4/(x-4))
  LoRa.setPreambleLength(LORA_PL);              //Seteamos la longitud del preambulo (x+4)
  LoRa.setTxPower(LORA_PW);                     //Configuramos la potencia de transmisión

  // Mensaje de comprobación
  Serial.println(F("Module configured finished"));
  Serial.println();
}


void End_module_spi(){
  LoRa.end();
  digitalWrite(RST1,1);
}
