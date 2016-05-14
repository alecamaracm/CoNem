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
        Serial.print(F("\"FROM\" argument parsed successfully: "));
        Serial.println(strfrom);

        cmdpositiondata=0;
        cmdpos=POS_SND_TO;

      }else
      {
        if (cmdpositiondata>=10) 
        {
          Serial.println(F("Exceded maximum number of charaters for FROM arg."));
          commandmustend=true;
        }
        strfrom[cmdpositiondata]=bytereceived;
        cmdpositiondata++;
      }
      
  }else if(cmdpos==POS_SND_TO)  //Si se está analizando el destinatario
  {
      if (bytereceived==':') //Ya se ha alcanzado el final del argumento
      {
        Serial.print(F("\"TO\" argument parsed successfully: "));
        Serial.println(strto);
        
        if (areequal(AB_ADDRESS,strto)==true)
        {
          Serial.println(F("Received data address match with the local one. Executing message..."));
          cmdpos=POS_SND_EXECUTE;
          cmdpositiondata=0;
          msgtype=TYPE_MSG_NONE;
        
        }else
        {
          Serial.println(F("Received data address does not match with the local one. Resending it..."));
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
          Serial.println(F("Exceded maximum number of charaters for TO arg."));
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
            
            if (areequal("STA",tempstr)==true){
              Serial.println(F("Message type STA identified successfully."));
               msgtype=TYPE_MSG_STA;              
            }else
            {
              Serial.print(F("Failed to detect the msg type: Not found"));
              commandmustend=true;
            }
           
            
          }else
          {
            if(cmdpositiondata>=10)
            {
              Serial.println(F("Exceded maximum number of charaters when parsing command type."));
              commandmustend=true;
            }else
            {
              if (cmdpositiondata==0) memset(tempstr,0,sizeof(tempstr));
              tempstr[cmdpositiondata]=bytereceived;
              cmdpositiondata++;
            }
          }
      }else if(msgtype==TYPE_MSG_STA)
      {
          Serial.print("holiholi: ");
          Serial.println((char)bytereceived);
      }
  }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// Final de funciones de comandos
