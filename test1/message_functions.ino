
void message_STA()
{

  if (bytereceived==',')
  {         
     decompresscolor();  //Coge los datos de tempstr y los pasa descomprimidos a tempcolor

      strip.setPixelColor(count1,tempcolor[0],tempcolor[1],tempcolor[2]);

     if (count1==(LED_LENGHT-1))
     {
        long stoptime=millis();
        Serial.print(F("Color data parsed successfully for "));
        Serial.print(LED_LENGHT);
        Serial.print(F(" LEDs"));
        Serial.print(F(". Took: "));
        Serial.print(stoptime-starttime);
        Serial.println(F(" miliseconds")); 
        strip.show();
     }
     count1=count1+1;
     msgpositiondata=0;
     
  }else
  {
     if (count1==LED_LENGHT)
     {
        Serial.println(F("Exceded number of LEDS present in module. Skipping message..."));
        commandmustend=true;
     }
     if (msgpositiondata>=10) //Si se ha pasado el límite de carácteres
     {

        Serial.println(F("Color data format incorrect (>10 bytes)"));
        Serial.println(count1);
        Serial.println(F("Skipping message..."));
        commandmustend=true;
     }
     if (msgpositiondata==0)
     {
      memset(tempstr,0,sizeof(tempstr));  //Si es el primer byte del color, se reinicia el búfer
     }

     tempstr[msgpositiondata]=bytereceived;    
     msgpositiondata++;     
  }
  //Serial.print((char)bytereceived);

}

void message_STR()
{
  if (count1==0) //Se esta leyendo el led de comienzo y final
  {
      if (bytereceived==',') //Se ha acabado de leer los LEDS
      {
        count1=1;
        tempstr2[msgpositiondata]=',';
        msgpositiondata=0;

      }else
      {
        if (msgpositiondata>10)
        {
            Serial.println(F("LED data format incorrect (>10 bytes)"));
            Serial.println(F("Skipping message..."));
            commandmustend=true;
        }else
        {
            tempstr2[msgpositiondata]=bytereceived;
            msgpositiondata++;
        }
        
      }
  }else //Se esta recibiendo el color
  {
      if (bytereceived==',')  //Se ha acabado de leer el color a poner
      {
        int startLED=0;
        int stopLED=0;
        int endposition;
        decompresscolor();

        for (byte i=0;i<sizeof(tempstr2);i++)
        {
          if(tempstr2[i]==',')
          {
            endposition=i;
          }
        }
        
        for (byte i=0;i<sizeof(tempstr2);i++)
        {
           if(tempstr2[i]=='-')
           {
              for (int j=0;j<i;j++)
              {
                startLED=startLED+((tempstr2[j]-'0')*(int)pow(10,(i-j)-1));               
              }
              for (int j=i+1;j<endposition;j++)
              {
                stopLED=stopLED+((tempstr2[j]-'0')*(int)pow(10,endposition-j-1));
              }
           }
        }
        
        if(startLED>stopLED)
        {
          Serial.println("Start LED must be lower than Stop LED");
          commandmustend=true;
        }else if (stopLED>LED_LENGHT)
        {
          Serial.println("Stop number exceed LED strip lenght");
          commandmustend=true;
        }
        else
        {
          for (int i=startLED;i<=stopLED;i++)
          {
           strip.setPixelColor(i,tempcolor[0],tempcolor[1],tempcolor[2]);
          }
            long stoptime=millis();
            Serial.print(F("Color data set successfully for "));
            Serial.print((stopLED-startLED)+1);
            Serial.print(F(" LEDs"));
            Serial.print(F(". Took: "));
            Serial.print(stoptime-starttime);
            Serial.println(F(" miliseconds")); 
            strip.show();

            commandmustend=true;
        }
                
      }else
      {
        if (msgpositiondata>10)
        {
            Serial.println(F("Color data format incorrect (>10 bytes)"));
            Serial.println(F("Skipping message..."));
            commandmustend=true;
        }else
        {
           tempstr[msgpositiondata]=bytereceived;
           msgpositiondata++;
        }       
      }
  }
}

