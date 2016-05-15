void resetcommand()
{
  starttime=millis();
  cmdtype=TYPE_NONE;   //Se pone en desconocido el tipo de comando
  cmdpos=POS_NONE;
  commandmustend=false;
  cmdpositiondata=0;
  memset(strfrom, 0, sizeof(strfrom));
  memset(strto, 0, sizeof(strto));
  memset(tempstr,0,sizeof(tempstr));
  memset(tempstr2,0,sizeof(tempstr2));
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


void decompresscolor()  //Convierte los caracteres esceciales a colores  De tempstr a tempcolor
{

  byte readingstep1=0; //posicion en la cadena entrante
  byte readingstep2=0; //En que color está R,G,B...


  while (readingstep2<=2)
  {
      if (tempstr[readingstep1]>=48 && tempstr[readingstep1]<=57)  //Es un caracter normal
      {
     
        tempcolor[readingstep2]=(tempstr[readingstep1]-'0')*100+(tempstr[readingstep1]-'0')*10+(tempstr[readingstep1]-'0');
        readingstep1=readingstep1+3;
        readingstep2++;
      }else  //Si es un carácter especial de compresión de colores
      {
          switch(tempstr[readingstep1]) 
          {
            case 'F'  : //255
            tempcolor[readingstep2]=255;
            break;
  
          case 'E'  : //000
            tempcolor[readingstep2]=0;
            break; 
  
          default : 
            Serial.print(F("Could not decompress colour character: "));
            Serial.println((char)tempstr[readingstep1]);
            tempcolor[readingstep2]=0;        
            break;
         }
         
         readingstep1++;
         readingstep2++;
    }

  }

  //Serial.print(F("Decompressed color for led: "));
 // Serial.print(count1);
  //Serial.print(" Color:");
  //Serial.println(tempstr);

}

