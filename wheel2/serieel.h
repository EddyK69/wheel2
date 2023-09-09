
bool golven = false;
bool karPIDveranderen = true;


String zin = "";
String vorrigeCommando = "";


void printCommandoMetPadding(String vergelijking, String beschrijving, bool padding){
  Serial.print(vergelijking);

  if(padding){
    int padding = 18 - vergelijking.length(); // padding
    for(int i = 0; i < padding; i++){
      Serial.print(" ");      
    }
  }  


  Serial.print(" " + beschrijving);
}



bool checkZin(String vergelijking, String beschrijving, bool infoPrinten){
  
  if(infoPrinten){
    printCommandoMetPadding(vergelijking, beschrijving, true);
    Serial.println();
    return false;
  }
  
  vergelijking.toLowerCase();
  if( ! zin.startsWith(vergelijking) ) return false;

  vorrigeCommando = vergelijking;
  zin.replace(vergelijking, "");
  zin.trim();
  
  printCommandoMetPadding(vergelijking, beschrijving, false);
  return true;
}



bool checkZinInt(String vergelijking, String beschrijving, bool infoPrinten, int& waarde){
  if(!checkZin(vergelijking, beschrijving, infoPrinten)) return false;

  if( zin.indexOf('?') == -1  &&  zin.length() != 0){ // als er een '?' geen waarde zoeken en als er geen 0 letters meer inzitten
    waarde = zin.toInt();
    Serial.print(" gezet: ");    
  }
  else{
    Serial.print(" opgevraagd: ");
  }
    
  Serial.println(String(waarde));
  return true;
}



bool checkZinFloat(String vergelijking, String beschrijving, bool infoPrinten,  float& waarde){
  if(!checkZin(vergelijking, beschrijving, infoPrinten)) return false;

  if( zin.indexOf('?') == -1  &&  zin.length() != 0){ // als er een '?' geen waarde zoeken en als er geen 0 letters meer inzitten
    waarde = zin.toFloat();
    Serial.print(" gezet: ");    
  }
  else{
    Serial.print(" opgevraagd: ");
  }
    
  Serial.println(String(waarde, 5));
  return true;
}




bool checkZinBool(String vergelijking, String beschrijving, bool infoPrinten, bool& waarde){
  if(!checkZin(vergelijking, beschrijving, infoPrinten)) return false;

  if(zin.indexOf('?') != -1){// als er een '?' is
    Serial.print(" opgevraagd: ");
  }
  else if( isDigit( zin.charAt(0) ) ){ // als er een '?' geen waarde zoeken en als er geen 0 letters meer inzitten
    waarde = zin.toInt();
    Serial.print(" gezet: ");    
  }
  // if(zin.indexOf('t') != -1 ){ // togle
  else{
    waarde = !waarde;
    Serial.print(" geflipt: ");
  }

    
  Serial.println(String(waarde));
  return true;
}




void infoPrintln(bool info){
  if(info){
    Serial.println();
  }
}





void checkenVoorCommando(bool info){

  infoPrintln(info);

  if(checkZinBool("g", "golven", info, golven)){return;}
  if(checkZinBool("PLG", "plaatLeesGolven", info, plaatLeesGolven)){return;}
  if(checkZinBool("KG", "karGolven", info, karGolven)){return;}
  if(checkZinBool("SG", "strobo.golven", info, strobo.golven)){return;}



  //-------------------------------------------------STAAT   
  infoPrintln(info);     
  if(checkZin(">>", "naarVolgendNummer()", info)){ naarVolgendNummer(); return;}
  if(checkZin("<<", "naarVorrigNummer()", info)){ naarVorrigNummer(); return;}
  if(checkZin("hok", "S_HOK", info)){ setStaat(S_HOK); return;}
  if(checkZin("stop", "stoppen()", info)){ stoppen(); return;}
  if(checkZin("spelen", "spelen()", info)){ spelen(); return;}
  if(checkZin("pauze", "pauze()", info)){ pauze(); return;}
  if(checkZin("schoonmaak", "S_SCHOONMAAK", info)){ setStaat(S_SCHOONMAAK); return;}
  if(checkZin("cal", "S_CALIBREER", info)){ setStaat(S_CALIBREER); return;}
  if(checkZinBool("rep", "herhaalDeHelePlaat", info, herhaalDeHelePlaat)){return;}




  //----------------------------------------------------ARM
  infoPrintln(info);
  if(checkZin("NE", "naaldErop()", info)){  naaldErop(); return;}
  if(checkZin("NA", "naaldEraf()", info)){  naaldEraf(); return;}
  if(checkZinFloat("ATG", "armTargetGewicht", info, armTargetGewicht)){return;}
  if(checkZin("AKL", "armKracht500mg calibreer", info)){    armKracht500mg = armKracht;   Serial.println("armKracht500mg: "  + String(armKracht500mg, 5));  return;}
  if(checkZin("AKH", "armKracht500mg calibreer", info)){  armKracht4000mg = armKracht; Serial.println("armKracht4000mg: " + String(armKracht4000mg, 5));   return;}


  //-------------------------------------------------------KAR SENSORS / TRACK SHIT
  infoPrintln(info);
  if(checkZinFloat("PLS", "plaatLeesStroom", info, plaatLeesStroom)){return;}
  if(checkZinInt("VOL", "volume", info, volume)){ volumeOverRide = true; return;}

  if(checkZinFloat("TO", "trackOffset", info, trackOffset)){return;}

  if(checkZin("AHCent", "armHoekCentreer()", info)){  armHoekCentreer(); return;}
  if(checkZin("AHCal", "armHoekCalibreer()", info)){ armHoekCalibreer(); return;}


  //--------------------------------------------------------KAR
  infoPrintln(info);
  if(checkZinFloat("KP", "karP", info, karP)){return;}
  if(checkZinFloat("KI", "karI", info, karI)){return;}
  if(checkZinFloat("KD", "karD", info, karD)){return;}

  // if(checkZinFloat("CNul", "antiCoggNul", info, antiCoggNul)){return;}
  // if(checkZinFloat("CMacht", "antiCoggMacht", info, antiCoggMacht)){return;}
  // if(checkZinFloat("CVerschuiving", "antiCoggVerschuiving", info, antiCoggVerschuiving)){return;}
  // if(checkZinBool("CAan", "antiCoggAan", info, antiCoggAan)){return;}
  // if(checkZinBool("CType", "antiCoggType", info, antiCoggType)){return;}

  //----------------------------------------------------PLATEAU
  infoPrintln(info);
  if(checkZinFloat("PP", "plateauP", info, plateauP)){return;}
  if(checkZinFloat("PI", "plateauI", info, plateauI)){return;}
  if(checkZinFloat("PD", "plateauD", info, plateauD)){return;}

  if(checkZinFloat("TR", "targetRpm", info, targetRpm)){return;}

  if(checkZin("PA", "plateauDraaien()", info)){ plateauDraaien(); return;}
  if(checkZin("PS", "plateauStoppen()", info)){ plateauStoppen(); return;}
  if(checkZinBool("PL", "plateauLogica", info, plateauLogica)){return;}
  if(checkZinBool("PC", "plateauComp", info, plateauComp)){return;}


  //----------------------------------------------------------------------STROBO
  infoPrintln(info);
  if(checkZinInt("SSN", "strobo.sampleNum", info, strobo.sampleNum)){return;}
  if(checkZinBool("SCM", "strobo.onbalansCompenseren", info, strobo.onbalansCompenseren)){return;}
  if(checkZin( "SCC", "strobo.clearCompSamples()", info)){   strobo.clearCompSamples(); Serial.println("clearComp");  return;}
  if(checkZinInt("SCFV", "strobo.faseVerschuiving", info, strobo.faseVerschuiving)){return;}
  if(checkZinFloat("SCV", "strobo.compVermenigvuldiging", info, strobo.compVermenigvuldiging)){return;}
  if(checkZinFloat("SCD", "strobo.compVerval", info, strobo.compVerval)){return;}
  if(checkZinBool("SKC", "strobo.plaatUitMiddenComp", info, strobo.plaatUitMiddenComp)){return;}
  if(checkZinBool("KC", "karUitMiddenCompAan", info, karUitMiddenCompAan)){return;}

  //------------------------------------------------------OPSLAG
  infoPrintln(info);
  if(checkZinFloat("EV", "eepromVersie", info, eepromVersie)){return;}
  if(checkZin("EO", "eepromOpslaan()", info)){   eepromOpslaan();  eepromShit = 1; return;}
  if(checkZin("EL", "eepromUitlezen()", info)){ eepromUitlezen();  return;}
  
  if(checkZin("OC", "orientatie.calibreerOrientatie()", info)){ orientatie.calibreerOrientatie(); return;}



  //------------------------------------------------------HELP
  infoPrintln(info);
  if(checkZin("C?", "commandos", info)){ checkenVoorCommando(true);return;}

  if(checkZin("?", "help", info)){
    Serial.println("\n\n\nhelp---------------------------------------------");   
    Serial.println("plateau P: " + String(plateauP, 5));
    Serial.println("plateau I: " + String(plateauI, 5));
    Serial.println("plateau D: " + String(plateauD, 5));
    Serial.println();

    Serial.println("kar P: " + String(karP, 5));
    Serial.println("kar I: " + String(karI, 5));
    Serial.println("kar D: " + String(karD, 5));
    Serial.println();
    
    Serial.println("staat: " + printStaat(staat));
    Serial.println("volume: " + String(volume));
    Serial.println();
    
    Serial.println("armKracht: " + String(armKracht));
    Serial.println("armGewicht: " + String(armGewicht));
    Serial.println("isNaaldErop(): " + String(isNaaldErop()));
    Serial.println();

    Serial.println("trackOffset: " + String(trackOffset));
    Serial.println();


    eepromPrint();
    Serial.println();
    
    
    printKnoppen();
    orientatie.print();
    Serial.println();


    Serial.println("-------------------------------------------------\n\n\n");
    return;
  }


  Serial.println("fout commando: \"" + zin + "\"");
}
















Interval serieelInt(10000, MICROS);
// Interval serieelInt(5000, MICROS);

void serieelFunc(){
  if(serieelInt.loop()){
    if(golven){
      
      
      
      Serial.print(strobo.vaart - targetRpm, 3);
      
      // Serial.print(", ");
      // Serial.print(strobo.glad, 3);
      // Serial.print(", ");
      // Serial.print(targetRpm, 2);

      Serial.print(", ");
      Serial.print(strobo.glad - targetRpm, 3);

      Serial.print(", ");
      Serial.print(strobo.glad - centerCompTargetRpm, 3);

      // Serial.print(", ");
      // Serial.print(strobo.gladglad - targetRpm, 3);

      Serial.print(", ");
      Serial.print(centerCompTargetRpm - targetRpm, 3);



      Serial.print(", ");
      Serial.print(strobo.teller);

      // Serial.print(", ");
      // Serial.print(strobo.preComp, 4);

      // Serial.print(", ");
      // Serial.print(strobo.plateauComp, 4);

      Serial.print(", ");
      Serial.print(uitBuff, 2);

      

      // Serial.print(", ");
      // Serial.print(armHoekRuw);//1696);
      Serial.print(", ");
      Serial.print(armHoekCall, 4);//1696);
      // Serial.print(", ");
      // Serial.print(armHoekSlow, 5);//1696);
      // Serial.print(", ");
      // Serial.print(armHoekOffset, 5);//1696);






      Serial.print(", ");
      Serial.print(karPos, 3);  
      Serial.print(", ");
      Serial.print(egteKarPos, 3);

      // Serial.print(", ");
      // Serial.print(karPosMidden, 3);

      // // Serial.print(", ");
      // // Serial.print(karPosMidden + strobo.karFourier, 3);  

      // Serial.print(", ");
      // Serial.print(karPosMidden + strobo.karFourierFilt, 3);  




      // Serial.print(", ");
      // Serial.print(karPosMinimaal, 3);
      
      // Serial.print(", ");
      // Serial.print(karPosFilter, 3);
      
      // Serial.print(", ");
      // Serial.print(armKracht);
      // Serial.print(", ");
      // Serial.print(armGewicht);


      // Serial.print(", ");
      // Serial.print(orientatie.x);
      // Serial.print(", ");
      // Serial.print(orientatie.y);
      // Serial.print(", ");
      // Serial.print(orientatie.z);


      Serial.print(", ");
      Serial.print(strobo.karSinFilt * strobo.karSinFilt  +  strobo.karCosFilt * strobo.karCosFilt);





      Serial.println();
    }





    while(Serial.available() > 0){
    
      char letter = Serial.read();
      
      if( ( letter == '\n' || letter == '\r' )&& zin != ""){
        zin.trim();
        zin.toLowerCase();

        if(zin.startsWith("l")){zin.replace("l", vorrigeCommando);} // 'L' is laatste commando voeg laatste commando toe aan zin

        checkenVoorCommando(false);
        

        zin = "";

      }else{
        zin += letter;
      }
    }





  }
}










//             bytes   cycles                
// LD (HL),d8      2   12
// INC L           1   4
//                 3   16

// LD (HL),d8      2   12
// INC HL          1   8
//                 3   20

// LD (HL+),A      1   8
// LD A,d8         2   8
//                 3   16                

// LD A, [DE]      1   8
// LD (HL+),A      1   8
//                 2   16

// LD SP,d16       3   12
// LD (a16),SP     3   20
//                 6   32



