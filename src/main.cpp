/*
 * Programa para prueba de modulos LoRa SxV2
 * 
 * Empresa: DIACSA
 * Autor: Ismael Takayama
 * Fecha: 23/02/2020
 * 
 * Programa para probar la comunicación de los módulos Lora RFM95W, LORA1276V2.0 y E32-433T30D
 * Manuales:
 * - https://cdn.sparkfun.com/assets/learn_tutorials/8/0/4/RFM95_96_97_98W.pdf (Lora RFM95W)
 * - https://fccid.io/2AD66-LORAV2/User-Manual/User-Manual-3379111 (LORA1276V2.0)
 * - https://www.ebyte.com/en/product-view-news.aspx?id=108 (E32-433T30D)
 *
 * Se usan las librerías LoRa de Sandeep Mistry y LoRa_E32 de Renzo Mischianti (www.mischianti.org)
 *
 * 
 * 
 */

#include <Arduino.h>
#include <SPI.h>
#include <stdio.h>
#include "Pines.h"
#include "LoRa_E32.h"
#include "LoRa.h"

//Definiciones para los módulos 1 y 2
#define LORA_BW               125E3             // Ancho de banda (BW)
#define LORA_SP               10                // Spreading Factor (SP)
#define LORA_CHANNEL          915E6             // Canal de transmisión
#define LORA_SYNCWORD         0x12              // Código de sincronización
#define LORA_ADDRESS          4                 // Dirección del nodo transmisor
#define LORA_SEND_TO_ADDRESS  2                 // Dirección del nodo receptor

//Definiciones para el módulo 3


byte MODO = 0;                                  // Variable usada para identificar el modulo usado actualmente
byte MODO_ANT = 0;                              

char message_received[100];                     // Mensaje recibido
char message_sent[]="hola";                     // Mensaje enviado
int counter=0;                                  // Contador de mensajes enviados

//Configuramos la clase para el módulo 3
LoRa_E32 E32_433(RX, TX, &Serial1, AUX, M0, M1);  // e32 TX e32 RX 

void Ini_module_spi(byte m); 
void Ini_module3();
void End_module3();
void End_module_spi();
void EnableDevice(byte m);

void setup()
{
    //Inicializamos los pines de LED y selección
  pinMode(LED, OUTPUT);
  pinMode(SEL1, INPUT);
  pinMode(SEL2, INPUT);

  //Inicializamos pines del módulo 1
  pinMode(DIO0_1, INPUT);
  pinMode(DIO1_1, INPUT);
  pinMode(DIO2_1, INPUT);
  pinMode(RST1, OUTPUT);
  pinMode(SS1, OUTPUT);
  digitalWrite(SS1,HIGH);

  //Inicializamos pines del módulo 2
  pinMode(DIO0_2, INPUT);
  pinMode(DIO1_2, INPUT);
  pinMode(DIO2_2, INPUT);
  pinMode(RST2, OUTPUT);
  pinMode(SS2, OUTPUT);
  digitalWrite(SS2,HIGH);

  //Iniciamos comunicación para el Módulo 3
  E32_433.begin();

  // Abrimos comunicaciones seriales
  Serial.begin(115200); 

  //Iniciamos los modulos en reset
  digitalWrite(RST1,0);
  digitalWrite(RST2,0);
  digitalWrite(M0,1);
  digitalWrite(M1,1);

  //Seed para valor rand
  srand(millis());
}

void loop(void)
{ 
  uint8_t i = 0;

  MODO = ( (digitalRead(SEL2)<<1) + digitalRead(SEL1) );        // Leemos el modo
  
  if (MODO_ANT != MODO){
    EnableDevice(MODO);         //Habilitamos el modulo según la posición de los jumpers
    MODO_ANT=MODO;
    counter=0;                  //Reiniciamos el contador
  }

  if (MODO==1 || MODO==2){
    // Enviamos un mensaje 
    Serial.println("Start sending message"); 
    LoRa.beginPacket();
    LoRa.println(message_sent);
    LoRa.print("Message n° ");
    LoRa.println(counter);
    LoRa.endPacket();
    Serial.println("Finish sending message"); 
    Serial.println(""); 

    counter++;
    
    //Serial.print("Message sent: ");
    //Serial.println(message_sent);
    //Serial.print("Message n° ");
    //Serial.println(counter);
    //Serial.println(""); 
    
    LoRa.receive();                                     //Ponemos a Lora en modo de recepcion
    delay((3+(rand() % 5))*1000);                      //Delay para evitar saturación de mensajes
    while(i < 15){
      Serial.println("Esperando mensaje");
      int packetSize = LoRa.parsePacket();
      if (packetSize) {                                 //Si es que se ha recibido un paquete
        Serial.print("Received packet: ");  
        while (LoRa.available()) {                      //Leemos el mensaje
          String LoRaData = LoRa.readString();
          Serial.print(LoRaData); 
        }

        Serial.print("With RSSI: ");                    // Imprimimos el RSSI
        Serial.println(LoRa.packetRssi());
        Serial.println("");
        
        digitalWrite(LED,1);                            //Prendemos el led por 0.5s
        delay(500);
        digitalWrite(LED,0);
        break;
      }
      delay(1000);
      i++; 
    }

  } else if (MODO == 3){
    char *Count = (char*)malloc(40);
    sprintf(Count, "N°: %u, Msj: ", counter);
  
    //enviamos un mensaje
    ResponseStatus rs = E32_433.sendMessage(strcat(Count, message_sent));
    if (rs.getResponseDescription() == "Success"){
      Serial.println("Mensaje enviado");
      counter++;
    } else {
      Serial.println("Error al enviar");
    }
    free(Count);
    Serial.println("");

    delay((3+(rand() % 5))*1000);                     //Delay para evitar saturación de mensajes
    //Esperamos a recibir un mensaje
    while (i<15) {
      //Serial.println("Esperando mensaje");
      if (E32_433.available()  > 1){                            //Si hay un mensaje 
        ResponseContainer rs = E32_433.receiveMessage();
        if (rs.status.getResponseDescription() == "Success"){
          Serial.print("Mensaje Recibido: ");
          Serial.println(rs.data);
        } else {
          Serial.println("Error al recibir mensaje");
        }

        digitalWrite(LED,1);                          //Prendemos el led por 0.5s
        delay(500);
        digitalWrite(LED,0);
        break;                                  
      } 
      delay(1000);
      i++;
    }
    Serial.println("");    
  }
  //delay(3000);
}

void Ini_module_spi(byte m) 
{
  //Inicializamos SPI en los pines correspondientes
  if (m==1){
    LoRa.setPins(SCK, MISO, MOSI, SS1, RST1, DIO0_1);

  } else if (m==2) {
    LoRa.setPins(SCK, MISO, MOSI, SS2, RST2, DIO0_2);
  }

  //Seteamos la frecuencia deseada y los canalies  y esperamos que se configure
  while (!LoRa.begin(LORA_CHANNEL)) {
    Serial.println(".");
    delay(500);
  }

  LoRa.setSyncWord(LORA_SYNCWORD);              //Seteamos la dirección de sincronización
  LoRa.setSpreadingFactor(LORA_SP);             //Seteamos el Spreading Factor (SP)
  LoRa.setSignalBandwidth(LORA_BW);             //Seteamos El ancho de banda
  LoRa.setCodingRate4(5);                       //Seteamos el Coding rate (4/(x-4))
  LoRa.setPreambleLength(8);                    //Seteamos la longitud del preambulo (x+4)


  // Mensaje de comprobación
  Serial.println(F("Module configured finished"));
  Serial.println();
}


void Ini_module3(){
  //Activamos el modo normal de operaciones
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
	configuration.CHAN = 0x17;                  //Canal 433

	configuration.OPTION.fec = FEC_1_ON;
	configuration.OPTION.fixedTransmission = FT_TRANSPARENT_TRANSMISSION;
	configuration.OPTION.ioDriveMode = IO_D_MODE_PUSH_PULLS_PULL_UPS;
	configuration.OPTION.transmissionPower = POWER_30;
	configuration.OPTION.wirelessWakeupTime = WAKE_UP_1250;

	configuration.SPED.airDataRate = AIR_DATA_RATE_000_03;
	configuration.SPED.uartBaudRate = UART_BPS_9600;
	configuration.SPED.uartParity = MODE_00_8N1;

	// Seteamos la configuración para que no se guarde tras el apagado
	ResponseStatus rs = E32_433.setConfiguration(configuration, WRITE_CFG_PWR_DWN_LOSE);
	Serial.println(rs.getResponseDescription());
	Serial.println(rs.code);
  
	printParameters(configuration);
	c.close();
}

void End_module3(){
  digitalWrite(M0,1);
  digitalWrite(M1,1);  
}

void End_module_spi(){
  LoRa.end();
  digitalWrite(RST1,1);
  digitalWrite(RST2,1);
}

void EnableDevice(byte m){
  End_module_spi();
  End_module3();
  
  switch(m)
  {
    case 0:
      Serial.println("modo 0");
      break;

    case 1:
      Serial.println("modo 1");
      Ini_module_spi(m);
      break;
    case 2:
      Serial.println("modo 2");
      Ini_module_spi(m);
      break;

    case 3:
      Serial.println("modo 3");
      Ini_module3(); 
      break;
  }
}