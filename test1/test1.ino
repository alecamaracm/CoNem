#include <Adafruit_NeoPixel.h>
   
#define PIN_DATA 6
#define LED_LENGHT 60


////////////////COMMAND TYPES/////////////////////////////////////
#define TYPE_NONE 0
#define TYPE_SND 1   //Tipos de comando
/////////////////////////////////////////////////////////////////


////////////////SND POSITIONS////////////////////////////////////
#define POS_NONE 0 //Posicion global
#define POS_SND_NONE 0   //En donde va de leido el comando SND
#define POS_SND_FROM 1
#define POS_SND_TO 2
#define POS_SND_RESEND 3
#define POS_SND_EXECUTE 4
////////////////////////////////////////////////////////////////

/////////////SND MESSAGES VARS/////////////////////////////////
#define TYPE_MSG_NONE 0
#define TYPE_MSG_STA 1
#define TYPE_MSG_STR 2
///////////////////////////////////////////////////////////////

#define DEBUG_MSG 0   //Si se mostrarán los mesajes de debug
#define BAUDRATE_RX 250000

/////////////MODULE IDENTIFICATION/////////////////////////
#define AB_ADDRESS "LEDTEST01"                  // 10 cifras máximo
#define AB_TYPE 1                                // 1 --> LED (60)
//////////////////////////////////////////////////////////


//MESSAGE     |SND-PC-LED0000001: command~

Adafruit_NeoPixel strip = Adafruit_NeoPixel(60, PIN_DATA, NEO_GRB + NEO_KHZ800);
//SoftwareSerial serial2;

long starttime;    //Time when the command starts

byte bytereceived;   //Ultimo byte recibido
unsigned int requesttime=0;   //Tiempo que lleva para que se agote el itempo de espera
bool commandmustend;    //Una función lo puede poner en true para forzar el acabado de comando

byte cmdtype=TYPE_NONE;   //SND,...
byte cmdpos=POS_NONE;       //Que esta leyendo: From  To   msg
byte cmdpositiondata;    //En que parte de lo que esta leyendo esta

char strfrom[15];                                                                                                         //Todos tienen una longitud máxima de 10!
char strto[15];
char tempstr[15];   //Buffer donde se almacena cualquier tipo de información temporal :Tipo de comando , color...
char tempstr2[15];
byte tempcolor[3]; //Contiene un color: 255,0,127 Lo puede usar quien quiera  + byte null

byte msgtype=TYPE_MSG_NONE;  //Sobre el mensaje dentro del comando SND
byte msgpositiondata;  //Después de identificar el nombre cualquier funcion(STA) lo puede manipular como quiera
unsigned int count1=0; //Counter que pueden usar las funciones

  
void setup() {
  memset( tempstr, 0 , sizeof(tempstr) ) ;
  
  strip.begin();
  Serial.begin(BAUDRATE_RX);
  strip.show(); // Initialize all pixels to 'off'

  Serial.print(F("Initialized module: "));
  Serial.print(AB_ADDRESS);
  Serial.print("     Type: ");
  Serial.println(AB_TYPE);
  Serial.print(F("Available SRAM memory: "));
  Serial.println(availableMemory());

  if (DEBUG_MSG==1) Serial.println(F("WARNING: Debug mode is ON. Long commands will not work properly!"));
  if (BAUDRATE_RX>9600) Serial.println(F("WARNING: RX Baud rate is set to a value greater than 9600."));                         

}


void loop() 
{
      if (Serial.available()>0)
      {
        bytereceived=Serial.read();
        if (bytereceived=='|')  //Si el primer caracter es un comienzo de comando (|)       
        {
            iniciodecomando:;            
            
            if (DEBUG_MSG==1) Serial.println("---Inicio de commando");                         

            resetcommand(); //Se resetean todas las variables orientadas al comando

            leerotrocaracter:;    //Todo el rato se vuelve aquí para leer el siguiente carácter si el anterior se ha leido bien  

            requesttime=0;   //Si inicializa el contador de tiempo
                    
            leerotrocaractertrasfallar:;
            
            if (Serial.available()>0)     //Si hay caracter disponible
            {
                bytereceived=Serial.read();
                if (bytereceived=='~') //Si se receibe el carácter de finalizar comando se acaba el comando
                {
                   if (cmdpos==POS_SND_RESEND) Serial.println("~");  //Si actualmente se estaba reenviando el comando, lo finaliza con un ~
                   if (DEBUG_MSG==1) Serial.println(F("Commando finalizado con exito"));
                  goto endcommand;
                }

                if (bytereceived=='|') //Si se receibe el carácter de empezar comando se cancela el actual y se empieza otro
                {
                  Serial.println(F("!!! Comando finalizado por la presencia de caracter de inicio de otro. Cancelando actual y pasando al otro"));
                  goto iniciodecomando;
                }
                
                //Serial.print("Caracter leido: ");
                //Serial.println((char)bytereceived);
               

                // --> Se hace todo lo que se tenga que hacer con el byte leído


                       if (cmdtype==TYPE_NONE) //Si no se sabe el tipo de comando
                       {
                          if (cmdpositiondata==0)  //El comando acaba de comenzar
                          {
                            tempstr[0]=bytereceived;
                            //cmdpos=POS_SND_CMDTYPE;
                            cmdpositiondata=1;
                          }else  //Se está analizando el tipo de comando
                          {
                            if (cmdpositiondata>=3)  // Ya se ha terminado de anañizar el tipo de comando
                            {
                              if (bytereceived=='-')  // Se intenta adivinar que comando es
                              {
                                  if (DEBUG_MSG==1) Serial.print(F("CMD type read successfully: "));
                                  if (DEBUG_MSG==1) Serial.print(tempstr[0]);
                                  if (DEBUG_MSG==1) Serial.print(tempstr[1]);
                                  //tempcmd[2]=tempstr[2]-1;
                                  if (DEBUG_MSG==1) Serial.println((char)(tempstr[2]));

                                  if (choosecmdtype()==true) //Se asigna el tipo de comando Devuelve true si encuentra el comando correcto Usa variables globales
                                  {
                                    if (DEBUG_MSG==1) Serial.print(F("Found command for command type: "));
                                    if (DEBUG_MSG==1) Serial.println(cmdtype);
                                  }else
                                  {
                                    Serial.println(F("!!!Command not found. Exiting command"));
                                    commandmustend=true;
                                  }
                                  
                                  //commandmustend=true;
                              }else  //Si al final del tipo de comando no está el signo correspondiente,da error y sale del comando
                              {
                                  Serial.print(F("Error: Sign - after cmd type not present: "));
                                  Serial.println((char)bytereceived);
                                  commandmustend=true;
                              }
                            }else
                            {
                              tempstr[cmdpositiondata]=bytereceived;
                              //Serial.println(tempstr[cmdpositiondata]);                              
                              cmdpositiondata=cmdpositiondata+1;
                            }
                          }

                          
                       }else if (cmdtype=TYPE_SND)
                       {
                          command_SND();
                       }else
                       {
                          Serial.println(F("A strange command has been assigned but not enough code is present. Ending command..."));
                          commandmustend=true;
                       }
                      
                  



                
                if (commandmustend==true)
                {
                  if (DEBUG_MSG==1) Serial.println("!!! Commando finalizado por funcion");
                  goto endcommand;
                }

                goto leerotrocaracter;  //Se vuelve para leer otro carácter

                                
            }else  //Si no hay carácter disponible se suma cada segundo uno al contador. Si el contador llega al máximo, se espera a una señal de inicio
            {
                if (requesttime>=1000)  //Si ha pasado un segundo sin recibir cadena
                {
                     Serial.println(F("!!! Commando finalizado automaticamente por exceso de tiempo de espera"));
                     goto endcommand;
                }else  //Si aun no ha pasado 1 seg se sumo 1 ms
                {
                     requesttime=requesttime+1;
                    // Serial.print("Sumado 1 seg, ");
                     //Serial.println(requesttime);
                     delay(1);
                     goto leerotrocaractertrasfallar;
                }
            }

            endcommand:;
            if (DEBUG_MSG==1) Serial.println("---Final de commando");
          
        }
      }
     
      
}
 








