enum staats{
  S_BEGIN,
  S_NAAR_HOK,
  S_HOK,
  S_STOPPEN,
  S_RUST,

  S_BEGINNEN_SCHOONMAAK,
  S_SCHOONMAAK_BEWEGEN,
  S_SCHOONMAAK,



  S_BEGINNEN_SPELEN,
  S_NAAR_BEGIN_PLAAT,
  S_BEGIN_PLAAT,
  S_SPELEN,


  S_PAUZE,



  S_VOLGEND_NUMMER,
  S_VORRIG_NUMMER,

  S_DOOR_SPOELEN,
  S_TERUG_SPOELEN,


  S_ERROR
};


enum staats staat = S_STOPPEN;


unsigned long staatInterval;






void printStaat(int s){
  if( s == S_BEGIN                ){ Serial.print("BEGIN");               return;}
  if( s == S_NAAR_HOK             ){ Serial.print("NAAR_HOK");            return;}
  if( s == S_HOK                  ){ Serial.print("HOK");                 return;}
  if( s == S_STOPPEN              ){ Serial.print("STOPPEN");             return;}
  if( s == S_RUST                 ){ Serial.print("RUST");                return;}

  if( s == S_BEGINNEN_SCHOONMAAK  ){ Serial.print("BEGINNEN_SCHOONMAAK"); return;}
  if( s == S_SCHOONMAAK_BEWEGEN   ){ Serial.print("SCHOONMAAK_BEWEGEN");  return;}
  if( s == S_SCHOONMAAK           ){ Serial.print("SCHOONMAAK");          return;}



  if( s == S_BEGINNEN_SPELEN      ){ Serial.print("BEGINNEN_SPELEN");     return;}
  if( s == S_NAAR_BEGIN_PLAAT     ){ Serial.print("NAAR_BEGIN_PLAAT");    return;}
  if( s == S_BEGIN_PLAAT          ){ Serial.print("BEGIN_PLAAT");         return;}
  if( s == S_SPELEN               ){ Serial.print("SPELEN");              return;}


  if( s == S_PAUZE                ){ Serial.print("PAUZE");               return;}



  if( s == S_VOLGEND_NUMMER       ){ Serial.print("VOLGEND_NUMMER");      return;}
  if( s == S_VORRIG_NUMMER        ){ Serial.print("VORRIG_NUMMER");       return;}

  if( s == S_DOOR_SPOELEN         ){ Serial.print("DOOR_SPOELEN");        return;}
  if( s == S_TERUG_SPOELEN        ){ Serial.print("TERUG_SPOELEN");       return;}



  if( s == S_ERROR                ){ Serial.print("ERROR");               return;}
  
  
  Serial.print("??????");

}










void setStaat(enum staats s){
  staatInterval = millis();
  // karRemmen = false;

  printStaat(staat); Serial.print(" > "); printStaat(s); Serial.println();

  staat = s; // set staat
}






