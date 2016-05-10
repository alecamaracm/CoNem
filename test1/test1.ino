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
///////////////////////////////////////////////////////////////



/////////////MODULE IDENTIFICATION/////////////////////////
#define AB_ADDRESS "LEDTEST01"                  // 10 cifras máximo
#define AB_TYPE 1                                // 1 --> LED (60)
//////////////////////////////////////////////////////////


//MESSAGE     |SND-PC-LED0000001: command~

Adafruit_NeoPixel strip = Adafruit_NeoPixel(60, PIN_DATA, NEO_GRB + NEO_KHZ800);
//SoftwareSerial serial2;


byte bytereceived;   //Ultimo byte recibido
int requesttime=0;   //Tiempo que lleva para que se agote el itempo de espera
bool commandmustend;    //Una función lo puede poner en true para forzar el acabado de comando

byte cmdtype=TYPE_NONE;   //SND,...
byte cmdpos=POS_NONE;       //Que esta leyendo: From  To   msg
byte cmdpositiondata;    //En que parte de lo que esta leyendo esta

char strfrom[10];
char strto[10];
char tempstr[2];   //Buffer donde se almacena cualquier tipo de información temporal :Tipo de comando , from, to...

byte msgtype=TYPE_MSG_NONE;  //Sobre el mensaje dentro del comando SND
byte msgpositiondata;

  
void setup() {
  memset( tempstr, 0 , sizeof(tempstr) ) ;
  
  strip.begin();
  Serial.begin(9600);
  strip.show(); // Initialize all pixels to 'off'

  Serial.print("Initialized module: ");
  Serial.print(AB_ADDRESS);
  Serial.print("     Type: ");
  Serial.println(AB_TYPE);
  Serial.print("Available SRAM memory: ");
  Serial.println(availableMemory());

  Serial.println(tempstr[10]);        //El último carácter es inútil, Dios sabe por qué
}


void loop() 
{
      if (Serial.available()>0)
      {
        bytereceived=Serial.read();
        if (bytereceived=='|')  //Si el primer caracter es un comienzo de comando (|)       
        {
            iniciodecomando:;
            Serial.println("---Inicio de commando");

                         

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
                   Serial.println("Commando finalizado con exito");
                  goto endcommand;
                }

                if (bytereceived=='|') //Si se receibe el carácter de empezar comando se cancela el actual y se empieza otro
                {
                  Serial.println("!!! Comando finalizado por la presencia de caracter de inicio de otro. Cancelando actual y pasando al otro");
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
                                  Serial.print("CMD type read successfully: ");
                                  Serial.print(tempstr[0]);
                                  Serial.print(tempstr[1]);
                                  //tempcmd[2]=tempstr[2]-1;
                                  Serial.println((char)(tempstr[2]));

                                  if (choosecmdtype()==true) //Se asigna el tipo de comando Devuelve true si encuentra el comando correcto Usa variables globales
                                  {
                                    Serial.print("Found command for command type: ");
                                    Serial.println(cmdtype);
                                  }else
                                  {
                                    Serial.println("!!!Command not found. Exiting command");
                                    commandmustend=true;
                                  }
                                  
                                  //commandmustend=true;
                              }else  //Si al final del tipo de comando no está el signo correspondiente,da error y sale del comando
                              {
                                  Serial.print("Error: Sign - after cmd type not present: ");
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
                          Serial.println("A strange command has been assigned but not enough code is present. Ending command...");
                          commandmustend=true;
                       }
                      
                  



                
                if (commandmustend==true)
                {
                  Serial.println("!!! Commando finalizado por funcion");
                  goto endcommand;
                }

                goto leerotrocaracter;  //Se vuelve para leer otro carácter

                                
            }else  //Si no hay carácter disponible se suma cada segundo uno al contador. Si el contador llega al máximo, se espera a una señal de inicio
            {
                if (requesttime>=1000)  //Si ha pasado un segundo sin recibir cadena
                {
                     Serial.println("!!! Commando finalizado automaticamente por exceso de tiempo de espera");
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
            Serial.println("---Final de commando");
          
        }
      }

      
      
}
 


void resetcommand()
{
  cmdtype=TYPE_NONE;   //Se pone en desconocido el tipo de comando
  cmdpos=POS_NONE;
  commandmustend=false;
  cmdpositiondata=0;
  memset(strfrom, 0, sizeof(strfrom));
  memset(strto, 0, sizeof(strto));
  memset(tempstr,0,sizeof(tempstr));
}

bool choosecmdtype()     //Se asigna el tipo de comando. Devuelve true si encuentra el comando correcto. Usa variables globales
{
  if (tempstr[0]=='S' && tempstr[1]=='N' && tempstr[2]=='D') //Comando SND
  {
    cmdtype=TYPE_SND;
    cmdpos=POS_SND_NONE;
    return true;
  }

  return false;
}



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// Funciones de comandos

void command_SND()
{
  //Serial.println("Comannd SND invocado!!!");

  if (cmdpos==POS_SND_NONE) 
  {
      cmdpos=POS_SND_FROM;  // Si es la primera vez que se ejecuta el comando SND, se pone en la primera parte de lectura
      cmdpositiondata=0;

      msgtype=TYPE_MSG_NONE; //Se reinicia el mensaje
      msgpositiondata=0;   //Se reinicia el mensaje

      //memset( strfrom, '~' , sizeof(strfrom) ) ;
      //memset( strto, '~' , sizeof(strto) ) ;
  }

  if (cmdpos==POS_SND_FROM)  //Si se está analizando el remitente
  {
      if (bytereceived=='-')    //Ya se ha alcanzado el final del argumento
      {
        Serial.print("\"FROM\" argument parsed successfully: ");
        Serial.println(strfrom);

        cmdpositiondata=0;
        cmdpos=POS_SND_TO;

      }else
      {
        if (cmdpositiondata>=10) 
        {
          Serial.println("Exceded maximum number of charaters for FROM arg.");
          commandmustend=true;
        }
        strfrom[cmdpositiondata]=bytereceived;
        cmdpositiondata++;
      }
      
  }else if(cmdpos==POS_SND_TO)  //Si se está analizando el destinatario
  {
      if (bytereceived==':') //Ya se ha alcanzado el final del argumento
      {
        Serial.print("\"TO\" argument parsed successfully: ");
        Serial.println(strto);
        
        if (areequal(AB_ADDRESS,strto)==true)
        {
          Serial.println("Received data address match with the local one. Executing message...");
          cmdpos=POS_SND_EXECUTE;
          cmdpositiondata=0;
          msgtype=TYPE_MSG_NONE;
        
        }else
        {
          Serial.println("Received data address does not match with the local one. Resending it...");
          cmdpos=POS_SND_RESEND;
          Serial.write("|SND-");
          Serial.write(strfrom);
          Serial.write("-");
          Serial.write(strto);
          Serial.write(":");
        }

      }else
      {
        if (cmdpositiondata>=10) 
        {
          Serial.println("Exceded maximum number of charaters for TO arg.");
          commandmustend=true;
        }
        strto[cmdpositiondata]=bytereceived;
        cmdpositiondata++;
      }
  }else if(cmdpos==POS_SND_RESEND)
  {
      Serial.write(bytereceived);      
  }else if(cmdpos==POS_SND_EXECUTE)
  {
      if (msgtype==TYPE_MSG_NONE)
      {
          if (bytereceived=='-')
          {
            Serial.print("Message type parsing procedure complete: ");
            Serial.println(tempstr);
          }else
          {
            if(cmdpositiondata>=3)
            {
              Serial.println("Exceded maximum number of charaters when parsing command type.");
              commandmustend=true;
            }else
            {
              if (cmdpositiondata==0) memset(tempstr,0,sizeof(tempstr));
              tempstr[cmdpositiondata]=bytereceived;
              cmdpositiondata++;
            }
          }
      }
  }
}



int availableMemory() {
  int size = 2048; // Use 2048 with ATmega328
  byte *buf;

  while ((buf = (byte *) malloc(--size)) == NULL)
    ;

  free(buf);

  return size;
}

bool areequal(char* str1,char* str2)
{
  return strcmp(str1,str2)==0;
}





