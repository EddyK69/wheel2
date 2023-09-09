//  ====================================
//    Arm motor class
//  ====================================
class ArmMotor
{
  public:
  #define MIN_GEWICHT 0.5  // in gram
  #define MAX_GEWICHT 4    // in gram
  #define HOK_GEWICHT -10

  float targetGewicht = 2.3;
  float gewicht = HOK_GEWICHT;
  float kracht = 0;

  float snelheidOp = 0.004;
  float snelheidAf = 0.01;

  float krachtLaag = 0.28;  //7de proto
  float krachtHoog = 0.58;

  //op de plaat
  float netUitHokGewicht = -0.8;
  float netOpDePlaatGewicht = 0.5;
  //van de plaat af
  float netVanDePlaatGewicht = 0;
  float netInHokGewicht = -1;


  bool armMotorAan = false;

  Interval armInt {10, MILLIS};
  Interval naaldEropInterval {0, MILLIS};

  void armInit() { setPwm(armMotor); }

  float armGewicht2pwm(float gewicht){
    float pwm = mapF(gewicht,   MIN_GEWICHT,   MAX_GEWICHT,    krachtLaag, krachtHoog);
    return limieteerF(pwm, 0, 1);  
  }

  float pwm2armGewicht(float pwm){
    float gewicht = mapF(pwm,  krachtLaag, krachtHoog,  MIN_GEWICHT,   MAX_GEWICHT);
    return gewicht;
  }

  bool isNaaldErop(){
    // return gewicht == armTargetGewicht  &&  naaldEropInterval.sinds() > 250;
    return gewicht == targetGewicht;
  }

  bool isNaaldEraf(){
	return gewicht == HOK_GEWICHT;
  }

  int isNaaldEropSinds(){
    return naaldEropInterval.sinds();
  }

  bool isNaaldEropVoorZoLang(int ms){
    return isNaaldErop() && naaldEropInterval.sinds() > ms;
  }



  bool naaldErop(){
    armMotorAan = true;
    return isNaaldErop();
  }

  bool naaldEraf(){
    armMotorAan = false;
    return isNaaldEraf();
  }



  bool naaldNoodStop(){
    gewicht = HOK_GEWICHT; // zet de arm dan meteen uit
    armMotorAan = false;
    return true;  
  }

  void armFunc()
  {
    if(armInt.loop()){

      if(staat == S_CALIBREER){
        gewicht = pwm2armGewicht(kracht);
        pwmWriteF(armMotor, kracht);
        return;
      }

      if(staat == S_HOMEN_VOOR_SPELEN  ||  staat == S_HOMEN_VOOR_SCHOONMAAK  ||staat == S_NAAR_HOK){
        if(armMotorAan == true){
          Serial.println("NAALD HAD NOOIT AAN MOGEN STAAN!!!");
          naaldNoodStop();
        }
      }


      

      if(armMotorAan == true){//moet de arm motor aan?

        if(gewicht < netUitHokGewicht){//als de arm net aan staat jump meteen naar nognetInHokGewicht
          gewicht = netUitHokGewicht;
        }

        if(gewicht < targetGewicht){//is de arm al op het target gewicht?
          gewicht += snelheidOp;
        }

        if(gewicht > netOpDePlaatGewicht){//is de arm al op de plaat?
          gewicht = targetGewicht;//zet dan de arm meteen op target gewicht
        }
      }
      

      if(armMotorAan == false){// moet de arm motor uit?
        
        if(gewicht > netVanDePlaatGewicht){ //als de arm net is uitgezet
          gewicht = netVanDePlaatGewicht; // zet haal dan meteen het meeste gewicht van de arm
        }

        if(gewicht > HOK_GEWICHT){ //is de arm nog niet helemaal uit
          gewicht -= snelheidAf; // zet hem dan wat minder hard
        }
        
        if(gewicht < netInHokGewicht){ //is de arm al van de plaat?
          gewicht = HOK_GEWICHT; // zet de arm dan meteen uit
        }
      }



      kracht = armGewicht2pwm(gewicht);

      pwmWriteF(armMotor, kracht);





      if(gewicht != targetGewicht){
        naaldEropInterval.reset();
      }
    }

  }
};



