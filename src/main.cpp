#include <Arduino.h>
#include <AsyncJson.h>
#include <ArduinoJson.h>
#include <FastLED.h>

#define LED_PIN     4
#define NUM_LEDS    186
#define BRIGHTNESS  255
#define LED_TYPE    WS2812B
#define COLOR_ORDER GRB
CRGB leds[NUM_LEDS];


// #define ComLEDS 4
// #define RXD2 16
// #define TXD2 17


//-----Constantes liées à ce programme


unsigned long lastShowTime = 0;   // Temps du dernier appel de FastLED.show()
const unsigned long showDelay = 0 ;  // Délai minimum entre deux appels (en millisecondes) - correspond à environ 30 fps

const int nbPointMax = 16;
const int nbParamametre = 4;
const int limiteParametre[nbParamametre][2]={{0,190},{0,255},{0,255},{0,NUM_LEDS}};

const String nomParametre[nbParamametre]={"hue","saturation","luminosite","position"};
const size_t CAPACITY = JSON_ARRAY_SIZE(nbPointMax*nbParamametre*2);


bool ap = false;
String header;

String output26State = "off";
String output27State = "off";


//-----Variable communication serie 

String mesToESP;
bool emission=false;
int etatEmission=0;
int etatReception=0;
int preEtatEmission=0;
int preEtatReception=0;
int essai =0;



unsigned long long debAttente=0;
unsigned long long actAttente=0;

//-----Variables de parametrage

String nomLumiere = "défaut";
String nomEffet = "défaut";


bool changementParams = true;
bool changementEffets = true;
bool connexion = false;
bool allumee = false;
bool comPower = false;
int nbPoint=2;
int nbEffet=2;
String etatAct="allumage";
String etatPre="eteint";

int listeActPara[nbParamametre][nbPointMax];
int listeNewPara[nbParamametre][nbPointMax]={{5,5},{255,255},{255,255},{0,77}};
int listeEffect[nbParamametre][nbPointMax];
int listeNewEffect[nbParamametre][nbPointMax]={{1,2},{3,4},{6,7},{144,144,}};
String NomEffet[8]={"Aucun","Rot Trigo","Rot Horaire","Respiration","Scintillement","Crepitement","Extinction ETB","Extinction BTE"};


//-----Variables de calcul
float ledNewHue[NUM_LEDS];
float ledActHue[NUM_LEDS];
float vectHue[NUM_LEDS];
int posHuePersonalie[nbPointMax];

float ledNewSat[NUM_LEDS];
float ledActSat[NUM_LEDS];
float vectSat[NUM_LEDS];
int posSatPersonalie[nbPointMax];

float ledNewLum[NUM_LEDS];
float ledActLum[NUM_LEDS];
float vectLum[NUM_LEDS];
int posLumPersonalie[nbPointMax];

//float listeScinti[NUM_LEDS];
int choixPre=0;
int choixPrePre=0;

int resolution=200;
int mode=0;

int longueurRampe=15;




int choixPointPre=0;

bool debug=false;

//-----Gestion du temps

unsigned long long superMillis() 
{
  static unsigned long nbRollover = 0;
  static unsigned long previousMillis = 0;
  unsigned long currentMillis = millis();
  
  if (currentMillis < previousMillis) {
     nbRollover++;
  }
  previousMillis = currentMillis;

  unsigned long long finalMillis = nbRollover;
  finalMillis <<= 32;
  finalMillis +=  currentMillis;
  return finalMillis;
}

void A();
void B();
void C();
void D();

void show()
{
  unsigned long currentTime = millis();  // Obtenir le temps actuel en millisecondes
  
  // Calculer le temps écoulé depuis le dernier appel
  unsigned long elapsedTime = currentTime - lastShowTime; 
  
  // Vérifier si suffisamment de temps s'est écoulé depuis le dernier appel
  if (elapsedTime >= showDelay)
  {
    // // Serial.println("FastLED.show() called");
    FastLED.delay(1/60); 
    FastLED.show();        // Mettre à jour les LEDs
    lastShowTime = currentTime;  // Mettre à jour le temps de l'appel
    // // Serial.print("Time since last show: ");
    // // Serial.print(elapsedTime);  // Afficher le temps écoulé
    // // Serial.println(" ms");
  }
  // else
  // {
  //   // Serial.print("FastLED.show() skipped (too soon), elapsed time: ");
  //   // Serial.print(elapsedTime);  // Afficher le temps écoulé
  //   // Serial.println(" ms");
  // }
}


void ChangementParrametres()//sert uniquement a la comunication
{
  for(int i=0; i<nbParamametre;i++)
  {
    // Serial.println("Changement parametre");
    for(int o=0; o<nbPoint; o++)
    {
      listeActPara[i][o]=listeNewPara[i][o];
    }
  }
  //// Serial.println("changement Couleur termine");
}

void ChangementEffet()//sert uniquement a la comunication
{
  for(int i=0; i<nbParamametre;i++)
  {
    for(int o=0; o<nbEffet; o++)
    {
      listeEffect[i][o]=listeNewEffect[i][o];      
    }
  }
  //// Serial.println("changement Effet termine");
}



bool ReceptionJSON()
{
    bool jsonOk = false;
    String messageRecu = "";
    bool inMessage = false;
    int openBracesCount = 0;
    unsigned long lastReadTime = millis();  // Gérer les délais

    // Serial.println("Début lecture");

    // Tant que des données sont disponibles et que le délai n'est pas dépassé
    if (Serial)
    {
      while ((millis() - lastReadTime < 200) || Serial.available() > 0)  // Timeout après 1 seconde
      {
          // S'il y a des données disponibles sur la liaison série
          if (Serial.available() > 0)
          {
              char carLu = Serial.read();
              lastReadTime = millis();  // Réinitialiser le délai

              // Gestion des accolades ouvrantes et fermantes
              if (carLu == '{') 
              {
                  inMessage = true;
                  openBracesCount++;
                  messageRecu += carLu;
              } 
              else if (carLu == '}' && inMessage) 
              {
                  openBracesCount--;
                  messageRecu += carLu;

                  if (openBracesCount == 0) 
                  {
                      jsonOk = true;
                      break;
                  }
              } 
              else if (inMessage) 
              {
                  messageRecu += carLu;
              }
          }
      }
    }else{
      Serial.begin(115200);
    }
    // Serial.println("Fin de lecture");

  if (jsonOk) 
  {
    // Serial.println("\nMessage JSON reçu :");
    // Serial.println(messageRecu);

    DynamicJsonDocument docRecupration(4096);
    DeserializationError error = deserializeJson(docRecupration, messageRecu);
    
    if (error) 
    {
      // Serial.print("Erreur de désérialisation : ");
      // Serial.println(error.c_str());
      return false;
    }

    // ----- Commande allumage -----
    comPower = docRecupration["Para"]["comPower"].as<bool>();
    // Serial.print("ComPower = ");
    // Serial.println(comPower);

    // ----- nomLumière -----
    nomLumiere = docRecupration["Para"]["nomLumiere"].as<String>();
    // Serial.print("nomLumiere = ");
    // Serial.println(nomLumiere);

    // ----- nomEffet -----
    nomEffet = docRecupration["Para"]["nomEffet"].as<String>();
    // Serial.print("nomEffet = ");
    // Serial.println(nomEffet);

    // ----- NbPoint -----
    nbPoint = docRecupration["Para"]["nbPoint"].as<int>();
    // Serial.print("NbPoint = ");
    // Serial.println(nbPoint);

    // ----- NbEffet -----
    nbEffet = docRecupration["Para"]["nbEffet"].as<int>();
    // Serial.print("NbEffet = ");
    // Serial.println(nbEffet);

    // ----- ListePoint -----
    JsonArray listeNewParaArray = docRecupration["Para"]["listeNewPara"].as<JsonArray>();
    int i = 0;
    for (JsonVariant v : listeNewParaArray) 
    {
      int o = 0;
      JsonArray innerArray = v.as<JsonArray>();
      for (JsonVariant w : innerArray) 
      {
        if (i < 3)
        {
          listeNewPara[i][o] = w.as<int>();
        } 
        else 
        {
          listeNewPara[i][o] = w.as<int>() * NUM_LEDS / 200;
        }
        // Serial.print(listeNewPara[i][o]);
        // Serial.print(" ");
        o++;
      }
      i++;
    }

    // ----- ListeEffet -----
    JsonArray listeNewEffectArray = docRecupration["Para"]["listeNewEffect"].as<JsonArray>();
    i = 0;
    for (JsonVariant v : listeNewEffectArray) 
    {
      int o = 0;
      JsonArray innerArray = v.as<JsonArray>();
      for (JsonVariant w : innerArray) 
      {
        listeNewEffect[i][o] = w.as<int>();
        // Serial.print(listeNewEffect[i][o]);
        // Serial.print(" ");
        o++;
      }
      i++;
    }
  }else{
    // Serial.println("json invalide ISSOU");
  }

  return jsonOk;
}

void receptionSerie()
{
  if(Serial.available() > 0)
    {
      bool b = ReceptionJSON();
      // Serial.print(b);
      if(b)
      {
        changementParams =true;
        changementEffets =true;

        if(comPower != allumee)
        {
          if(comPower == false && allumee == true)
          {
            C();
          }
          if(comPower == true && allumee == false)
          {
            D();
          }
        }else if(allumee == true)
        {
          A();
        }

      }
    }else{
      B();
    }
}

void CommunicationLEDS()
{
  for(int i=0 ; i<NUM_LEDS ; i++)
  {
    if(Serial){if(Serial.available()>0 ){receptionSerie(); break;}}
    

    CHSV spectrumcolor;
    spectrumcolor.hue = ledActHue[i];//couleur
    spectrumcolor.saturation =ledActSat[i];//0 blanc
    spectrumcolor.value =  ledActLum[i];//0noir
    hsv2rgb_rainbow( spectrumcolor, leds[i] ); 
  }
  if(comPower == true || allumee == true)
  {
    show();
  }
}

void ChangementParPoint(bool hue, bool sat, bool lum)
{
  for(int i=0;i<NUM_LEDS;i++)
  {
    if(hue)
    {
      ledActHue[i]=ledActHue[i]+vectHue[i];
    }
    if(sat)
    {
      ledActSat[i]=ledActSat[i]+vectSat[i];
    }
    if(lum)
    {
      ledActLum[i]=ledActLum[i]+vectLum[i];
    }
  }
}

void calculPosPerso(int posAct[nbPointMax], int posPers[nbPointMax], float tabSortie[NUM_LEDS])
{
  int index = nbPoint-1;
  int choixPoint;
  do{
    choixPoint = random(nbPoint);
  }while(choixPointPre==choixPoint);
  choixPointPre=choixPoint;
  int positionPointChoix=posAct[choixPoint];

  if(index == 1)
  {
    int positionPointPre = posAct[(nbPoint+choixPoint - 1 )%nbPoint];
    int longPre = (NUM_LEDS+positionPointPre-positionPointChoix)%NUM_LEDS;

    posPers[(nbPoint+choixPoint - 1 )%nbPoint]=(NUM_LEDS + positionPointPre-longPre/2)%NUM_LEDS;
    posPers[choixPoint]=positionPointChoix;
  }
  if(index > 1)
  {

    int positionPointPre = posAct[(nbPoint+choixPoint - 1 )%nbPoint];
    int longPre = (NUM_LEDS-positionPointPre+positionPointChoix)%NUM_LEDS;
    int positionPointSui = posAct[(nbPoint+choixPoint + 1 )%nbPoint];
    int longSui = (NUM_LEDS+positionPointSui-positionPointChoix)%NUM_LEDS;

    for(int i = 0; i<nbPoint;i++)
    {
      if(i==(nbPoint+choixPoint - 1 )%nbPoint)
      {
        posPers[i]=(NUM_LEDS + positionPointPre+longPre/2)%NUM_LEDS;
      }
      else if(i==(nbPoint+choixPoint + 1 )%nbPoint)
      {
        posPers[i]=(NUM_LEDS + positionPointSui-longSui/2)%NUM_LEDS;
      }
      else
      {
        posPers[i]=posAct[i];
      }
    }
  }
  for(int i=0;i<nbPoint-1;i++)
  {
    if(posPers[i]>posPers[i+1])
    {
      int stock = posPers[i];
      posPers[i]=posPers[i+1];
      posPers[i+1]=stock;
    }
  }
}



void RotationTrigo(float Tab[NUM_LEDS])
{
  int Var_Stock;
  
  for(int i=0;i<NUM_LEDS;i++)
  {
    if(i==0)
    {
      Var_Stock=Tab[i];
      Tab[i]=Tab[i+1];
    }else if(i>0 && i<NUM_LEDS-1)
    {
      Tab[i]=Tab[i+1];
    }else{
    Tab[i]=Var_Stock;
    }
  }
}

void RotationHoraire(float Tab[NUM_LEDS])
{
  int Var_Stock;

  for(int s=NUM_LEDS-1;s>=0;s--)
  {
    if(s==NUM_LEDS-1)
    {
      Var_Stock=Tab[s];
      Tab[s]=Tab[s-1];
    }else if(s>0 && s<NUM_LEDS-1)
    {
      Tab[s]=Tab[s-1];
    }else{
      Tab[s]=Var_Stock;
    }
  }
}

void CalculDegradeParPoint(float tabEntree[NUM_LEDS],float tabSortie[NUM_LEDS],float tabVecteur[NUM_LEDS],int resolution)
{
  for(int i= 0; i< NUM_LEDS; i++)
  {
    tabVecteur[i]=(tabSortie[i]-tabEntree[i])/resolution;
  }
}

void paraToTab(float tabLED[NUM_LEDS], int tabPos[nbPointMax], int tabObj[nbPointMax])
{
  if(nbPoint==1)
  {
    for(int i=0; i< NUM_LEDS;i++)
    {
      tabLED[i]=tabObj[0];
    }
  }else{
    for(int i=0; i<nbPoint; i++)
    {
      int I=(i+1)%nbPoint;
      int deltaPos=(NUM_LEDS-tabPos[i]+tabPos[I])%NUM_LEDS;
      float deltaObj=tabObj[i]-tabObj[I];
      float vecteur = deltaObj/deltaPos;

      for(int o=tabPos[i];o != tabPos[(i+1)%nbPoint] ; o=(o+1)%NUM_LEDS)
      {
        if(o==tabPos[i])
        {
          tabLED[o]=tabObj[i];
        }
        else
        {
          int y;
          if(o==0)
          {
            y=NUM_LEDS-1;
          }else{
            y=(o-1)%NUM_LEDS;
          }
          tabLED[o]=tabLED[y]-vecteur;
        }
      }
    }
  }
}

void calculScintillement(int n,float tabEntree[NUM_LEDS], float tabSortie[NUM_LEDS])
{
  int min=1000;
  int max=0;

  for(int i=0;i<NUM_LEDS;i++)
  {
    tabSortie[i]=tabEntree[i];
  }

  for(int i = 0 ; i<nbPoint; i++)
  {
    if(listeActPara[n][i]>max)
    {
      max=listeActPara[n][i];
    }
    if(listeActPara[n][i]<min)
    {
      min=listeActPara[n][i];
    }
  }

  int nbScinti = int(NUM_LEDS/20);
  for(int i=0;i<nbScinti;i++)
  {
    int choixPoint = i*(NUM_LEDS/nbScinti)+7+random(7);
    float choixVal = tabEntree[choixPoint];

    int choix=random(2);
    if(choix == choixPre && choix == choixPointPre)
    {
      choix=(choix+1)%2;
    }
    if(choix==1){choixVal=min;}
    else{choixVal=max;}

    choixPrePre=choixPre;
    choixPre=choix;

    int prePoint = choixPoint -5;
    int suiPoint = choixPoint +5;

    float preVal = tabEntree[prePoint];
    float suiVal = tabEntree[suiPoint];

    float preVect = (choixVal-preVal)/5.0;
    float suiVect = (suiVal-choixVal)/5.0;

    for(int p=0;p<5;p++)
    {
      tabSortie[prePoint+p]=preVal+p*preVect;
    }
    for(int s=0;s<5;s++)
    {
      tabSortie[choixPoint+s]=choixVal+s*suiVect;
    }
  }
}

void Scintillement(int mode, bool HUE, bool SAT, bool LUM, int resolution)
{
  switch (mode)
  {
  case 0:
    if(HUE){calculScintillement(0,ledActHue,ledNewHue);}
    if(SAT){calculScintillement(1,ledActSat,ledNewSat);}
    if(LUM){calculScintillement(2,ledActLum,ledNewLum);}

    if(HUE){CalculDegradeParPoint(ledActHue,ledNewHue,vectHue,resolution);}
    if(SAT){CalculDegradeParPoint(ledActSat,ledNewSat,vectSat,resolution);}
    if(LUM){CalculDegradeParPoint(ledActLum,ledNewLum,vectLum,resolution);}

    break;

  case 1:
    ChangementParPoint(HUE, SAT, LUM);
    break;

  case 2:

    if(HUE){paraToTab(ledNewHue, listeActPara[3], listeActPara[0]);}
    if(SAT){paraToTab(ledNewSat, listeActPara[3], listeActPara[1]);}
    if(LUM){paraToTab(ledNewLum, listeActPara[3], listeActPara[2]);}

    if(HUE){CalculDegradeParPoint(ledActHue,ledNewHue,vectHue,resolution);}
    if(SAT){CalculDegradeParPoint(ledActSat,ledNewSat,vectSat,resolution);}
    if(LUM){CalculDegradeParPoint(ledActLum,ledNewLum,vectLum,resolution);}

    break;

  case 3:
    ChangementParPoint(HUE, SAT, LUM);
    break;
  
  default:
    break;
  }
}

void respiration(int mode, bool HUE, bool SAT, bool LUM, int resolution)
{
  switch (mode)
  {
  case 0:
    if(HUE)
    {
    calculPosPerso(listeActPara[3],posHuePersonalie,ledNewHue);
    paraToTab(ledNewHue, posHuePersonalie, listeActPara[0]);
    CalculDegradeParPoint(ledActHue,ledNewHue,vectHue,resolution);
    }
    if(SAT)
    {
    calculPosPerso(listeActPara[3],posSatPersonalie,ledNewSat);
    paraToTab(ledNewSat, posSatPersonalie, listeActPara[1]);
    CalculDegradeParPoint(ledActSat,ledNewSat,vectSat,resolution);
    }
    if(LUM)
    {
    calculPosPerso(listeActPara[3],posLumPersonalie,ledNewLum);
    paraToTab(ledNewLum, posLumPersonalie, listeActPara[2]);
    CalculDegradeParPoint(ledActLum,ledNewLum,vectLum,resolution);
    }
    break;
  case 1:
    ChangementParPoint(HUE, SAT, LUM);
    break;
  case 2:
    if(HUE)
    {
    paraToTab(ledNewHue, listeActPara[3], listeActPara[0]);
    CalculDegradeParPoint(ledActHue,ledNewHue,vectHue,resolution);
    }
    if(SAT)
    {
    paraToTab(ledNewSat, listeActPara[3], listeActPara[1]);
    CalculDegradeParPoint(ledActSat,ledNewSat,vectSat,resolution);
    }
    if(LUM)
    {
    paraToTab(ledNewLum, listeActPara[3], listeActPara[2]);
    CalculDegradeParPoint(ledActLum,ledNewLum,vectLum,resolution);
    }
    break;
  case 3:
    ChangementParPoint(HUE, SAT, LUM);
    break;  
  default:
    break;
  }
}

void crepitement(int mode, bool HUE, bool SAT, bool LUM, int resolution, int o)
{
  int n;
  int min;
  int max;
  int delta;

  switch (mode)
  {
  case 1:
    if(HUE){n=0;}
    if(SAT){n=1;}
    if(LUM){n=2;}

    for(int i = 0 ; i<nbPoint; i++)
    {
      if(i==0)
      {
        max=listeActPara[n][i];
        min=listeActPara[n][i];
      }
      if(listeActPara[n][i]>max)
      {
        max=listeActPara[n][i];
      }
      if(listeActPara[n][i]<min)
      {
        min=listeActPara[n][i];
      }
    }
    delta=max-min;
    
    for(int i=0;i<NUM_LEDS;i++)
    {
      if(random(o/10)==0)
      {
        if(HUE){ledActHue[i]=min+random(delta);}
        if(SAT){ledActSat[i]=min+random(delta);}
        if(LUM){ledActLum[i]=min+random(delta);}
      }
    }
    break;

  case 2:
    if(HUE){paraToTab(ledNewHue, listeActPara[3], listeActPara[0]);}
    if(SAT){paraToTab(ledNewSat, listeActPara[3], listeActPara[1]);}
    if(LUM){paraToTab(ledNewLum, listeActPara[3], listeActPara[2]);}
    
    if(HUE){CalculDegradeParPoint(ledActHue,ledNewHue,vectHue,resolution);}
    if(SAT){CalculDegradeParPoint(ledActSat,ledNewSat,vectSat,resolution);}
    if(LUM){CalculDegradeParPoint(ledActLum,ledNewLum,vectLum,resolution);}

    break;

  case 3:
    ChangementParPoint(HUE, SAT, LUM);
    break;
  
  default:
    break;
  }
}

void CalculExtEndToBegin(float tab[NUM_LEDS],int n,int resolution, int o)
{
  float pas = (float(NUM_LEDS+longueurRampe)/float(resolution))*100.0;
  int posTopTri = ((resolution - o) * pas)-longueurRampe*100.0;
  int valTopTri;

  valTopTri=limiteParametre[n][1];
  for(int i=0;i<longueurRampe;i++)
  {
    int posLed=(posTopTri/100)+1+i;

    if(posLed < NUM_LEDS && posLed >= 0)
    {
      float delta = (posLed+i)*100.0-float(posTopTri);
      int newVal = valTopTri-(delta*valTopTri)/(longueurRampe*100);

      if(newVal<tab[posLed] && newVal>=0)
      {
        tab[posLed]=int(newVal);
      }
      if(newVal<tab[posLed] && newVal<0)
      {
        tab[posLed]=0;
      }
      if(i == longueurRampe-1)
      {
        tab[posLed]=0;
      }
    }
  }  
}

void CalculExtBeginToEnd(float tab[NUM_LEDS],int n,int resolution, int o)
{
  float pas = (float(NUM_LEDS+longueurRampe)/float(resolution))*100.0;
  int posTopTri = ((o) * pas)+longueurRampe*100.0;
  int valTopTri;

  valTopTri=limiteParametre[n][1];
  for(int i=0;i<longueurRampe;i++)
  {
    int posLed=(posTopTri/100)-1-i;

    if(posLed < NUM_LEDS && posLed >= 0)
    {
      float delta = float(posTopTri)-(posLed-i)*100.0;// Je crois que c'est bon
      int newVal = valTopTri-(delta*valTopTri)/(longueurRampe*100);//Repris sur extinction ETB

      if(newVal<tab[posLed] && newVal>=0)
      {
        tab[posLed]=int(newVal);
      }
      if(newVal<tab[posLed] && newVal<0)
      {
        tab[posLed]=0;
      }
      if(i == longueurRampe-1)
      {
        tab[posLed]=0;
      }
    }
  }  
}

void ExtinctionEndToBegin(int mode,bool HUE, bool SAT, bool LUM,int resolution, int o)
{
  
  switch (mode)
  {
  case 1:
    if(HUE){CalculExtEndToBegin(ledActHue,0,resolution, o);}
    if(SAT){CalculExtEndToBegin(ledActSat,1,resolution, o);}
    if(LUM){CalculExtEndToBegin(ledActLum,2,resolution, o);}
  break;

  case 2:
    if(HUE){paraToTab(ledNewHue, listeActPara[3], listeActPara[0]);}
    if(SAT){paraToTab(ledNewSat, listeActPara[3], listeActPara[1]);}
    if(LUM){paraToTab(ledNewLum, listeActPara[3], listeActPara[2]);}
    
    if(HUE){CalculDegradeParPoint(ledActHue,ledNewHue,vectHue,resolution);}
    if(SAT){CalculDegradeParPoint(ledActSat,ledNewSat,vectSat,resolution);}
    if(LUM){CalculDegradeParPoint(ledActLum,ledNewLum,vectLum,resolution);}
    break;

  case 3:
    ChangementParPoint(HUE, SAT, LUM);
    break;
  
  default:
    break;
  }
}

void ExtinctionBeginToEnd(int mode,bool HUE, bool SAT, bool LUM,int resolution, int o)
{
  
  switch (mode)
  {
  case 1:
    if(HUE){CalculExtBeginToEnd(ledActHue,0,resolution, o);}
    if(SAT){CalculExtBeginToEnd(ledActSat,1,resolution, o);}
    if(LUM){CalculExtBeginToEnd(ledActLum,2,resolution, o);}
  break;

  case 2:
    if(HUE){paraToTab(ledNewHue, listeActPara[3], listeActPara[0]);}
    if(SAT){paraToTab(ledNewSat, listeActPara[3], listeActPara[1]);}
    if(LUM){paraToTab(ledNewLum, listeActPara[3], listeActPara[2]);}
    
    if(HUE){CalculDegradeParPoint(ledActHue,ledNewHue,vectHue,resolution);}
    if(SAT){CalculDegradeParPoint(ledActSat,ledNewSat,vectSat,resolution);}
    if(LUM){CalculDegradeParPoint(ledActLum,ledNewLum,vectLum,resolution);}
    break;

  case 3:
    ChangementParPoint(HUE, SAT, LUM);
    break;
  
  default:
    break;
  }
}

void AplliPara(int mode,bool HUE, bool SAT, bool LUM,int resolution)
{
  switch (mode)
  {
  case 0:
    if(HUE){paraToTab(ledNewHue, listeActPara[3], listeActPara[0]);}
    if(SAT){paraToTab(ledNewSat, listeActPara[3], listeActPara[1]);}
    if(LUM){paraToTab(ledNewLum, listeActPara[3], listeActPara[2]);}

    if(HUE){CalculDegradeParPoint(ledActHue,ledNewHue,vectHue,resolution);}
    if(SAT){CalculDegradeParPoint(ledActSat,ledNewSat,vectSat,resolution);}
    if(LUM){CalculDegradeParPoint(ledActLum,ledNewLum,vectLum,resolution);}
    break;
  
  case 1:
    if(HUE){ChangementParPoint(true, false, false);}
    if(SAT){ChangementParPoint(false, true, false);}
    if(LUM){ChangementParPoint(false, false, true);}

  break;

  default:
    break;
  }

}

void gestionEffets(int mode, int i)
{
  resolution=listeEffect[3][i];
  switch (mode)
  {
  case 0:// Initialisation des effets
  // Serial.println("Case N°0 - Initialisation des effets");
    if(Serial){if(Serial.available()>0 ){receptionSerie(); break;}}
    


    if(listeEffect[0][i]==0){AplliPara(mode,true,false,false,resolution);}
    if(listeEffect[1][i]==0){AplliPara(mode,false,true,false,resolution);}
    if(listeEffect[2][i]==0){AplliPara(mode,false,false,true,resolution);}

    if(listeEffect[0][i]==3 && nbPoint > 1){respiration(mode,true,false,false,resolution);}
    if(listeEffect[1][i]==3 && nbPoint > 1){respiration(mode,false,true,false,resolution);}
    if(listeEffect[2][i]==3 && nbPoint > 1){respiration(mode,false,false,true,resolution);}

    if(listeEffect[0][i]==4 && nbPoint > 1){Scintillement(mode,true,false,false,resolution);}
    if(listeEffect[1][i]==4 && nbPoint > 1){Scintillement(mode,false,true,false,resolution);}
    if(listeEffect[2][i]==4 && nbPoint > 1){Scintillement(mode,false,false,true,resolution);}
  
    break;

  case 1:// Aller des effets
    // Serial.println("Case N°1 - Aller des effets");
    for(int o = 0; o<resolution;o++)
    {
      if(Serial){if(Serial.available()>0 ){receptionSerie(); break;}}
      


      if(listeEffect[0][i]==0){AplliPara(mode,true,false,false,resolution);}
      if(listeEffect[1][i]==0){AplliPara(mode,false,true,false,resolution);}
      if(listeEffect[2][i]==0){AplliPara(mode,false,false,true,resolution);}  

      if(listeEffect[0][i]==1 && nbPoint > 1){RotationHoraire(ledActHue);}
      if(listeEffect[1][i]==1 && nbPoint > 1){RotationHoraire(ledActSat);}
      if(listeEffect[2][i]==1 && nbPoint > 1){RotationHoraire(ledActLum);}

      if(listeEffect[0][i]==2 && nbPoint > 1){RotationTrigo(ledActHue);}
      if(listeEffect[1][i]==2 && nbPoint > 1){RotationTrigo(ledActSat);}
      if(listeEffect[2][i]==2 && nbPoint > 1){RotationTrigo(ledActLum);}

      if(listeEffect[0][i]==3 && nbPoint > 1){respiration(mode,true,false,false,resolution);}
      if(listeEffect[1][i]==3 && nbPoint > 1){respiration(mode,false,true,false,resolution);}
      if(listeEffect[2][i]==3 && nbPoint > 1){respiration(mode,false,false,true,resolution);}

      if(listeEffect[0][i]==4 && nbPoint > 1){Scintillement(mode,true,false,false,resolution);}
      if(listeEffect[1][i]==4 && nbPoint > 1){Scintillement(mode,false,true,false,resolution);}
      if(listeEffect[2][i]==4 && nbPoint > 1){Scintillement(mode,false,false,true,resolution);}

      if(listeEffect[0][i]==5 && nbPoint > 1){crepitement(mode,true,false,false,resolution,o);}
      if(listeEffect[1][i]==5 && nbPoint > 1){crepitement(mode,false,true,false,resolution,o);}
      if(listeEffect[2][i]==5 && nbPoint > 1){crepitement(mode,false,false,true,resolution,o);}

      if(listeEffect[0][i]==6){ExtinctionEndToBegin(mode,true,false,false,resolution,o);}
      if(listeEffect[1][i]==6){ExtinctionEndToBegin(mode,false,true,false,resolution,o);}
      if(listeEffect[2][i]==6){ExtinctionEndToBegin(mode,false,false,true,resolution,o);}

      if(listeEffect[0][i]==7){ExtinctionBeginToEnd(mode,true,false,false,resolution,o);}
      if(listeEffect[1][i]==7){ExtinctionBeginToEnd(mode,false,true,false,resolution,o);}
      if(listeEffect[2][i]==7){ExtinctionBeginToEnd(mode,false,false,true,resolution,o);}
          
      CommunicationLEDS();
    }

        
    break;

  case 2:// dé-initialisation des effets
    // Serial.println("Case N°2 - dé-initialisation des effets");
    if(Serial){if(Serial.available()>0 ){receptionSerie(); break;}}
    

    if(listeEffect[0][i]==3 && nbPoint > 1){respiration(mode,true,false,false,resolution);}
    if(listeEffect[1][i]==3 && nbPoint > 1){respiration(mode,false,true,false,resolution);}
    if(listeEffect[2][i]==3 && nbPoint > 1){respiration(mode,false,false,true,resolution);}

    if(listeEffect[0][i]==4 && nbPoint > 1){Scintillement(mode,true,false,false,resolution);}
    if(listeEffect[1][i]==4 && nbPoint > 1){Scintillement(mode,false,true,false,resolution);}
    if(listeEffect[2][i]==4 && nbPoint > 1){Scintillement(mode,false,false,true,resolution);}

    if(listeEffect[0][i]==5 && nbPoint > 1){crepitement(mode,true,false,false,resolution,0);}
    if(listeEffect[1][i]==5 && nbPoint > 1){crepitement(mode,false,true,false,resolution,0);}
    if(listeEffect[2][i]==5 && nbPoint > 1){crepitement(mode,false,false,true,resolution,0);}

    if(listeEffect[0][i]==6){ExtinctionEndToBegin(mode,true,false,false,resolution,0);}
    if(listeEffect[1][i]==6){ExtinctionEndToBegin(mode,false,true,false,resolution,0);}
    if(listeEffect[2][i]==6){ExtinctionEndToBegin(mode,false,false,true,resolution,0);}

    if(listeEffect[0][i]==7){ExtinctionBeginToEnd(mode,true,false,false,resolution,0);}
    if(listeEffect[1][i]==7){ExtinctionBeginToEnd(mode,false,true,false,resolution,0);}
    if(listeEffect[2][i]==7){ExtinctionBeginToEnd(mode,false,false,true,resolution,0);}
        
    break;

      case 3:// retour des effets
    // Serial.println("Case N°3 - retour des effets");
    for(int o = 0; o<resolution;o++)
    {
      
      if(Serial){if(Serial.available()>0 ){receptionSerie(); break;}}
      

      if(listeEffect[0][i]==1 && nbPoint > 1){RotationHoraire(ledActHue);}
      if(listeEffect[1][i]==1 && nbPoint > 1){RotationHoraire(ledActSat);}
      if(listeEffect[2][i]==1 && nbPoint > 1){RotationHoraire(ledActLum);}

      if(listeEffect[0][i]==2 && nbPoint > 1){RotationTrigo(ledActHue);}
      if(listeEffect[1][i]==2 && nbPoint > 1){RotationTrigo(ledActSat);}
      if(listeEffect[2][i]==2 && nbPoint > 1){RotationTrigo(ledActLum);}

      if(listeEffect[0][i]==3 && nbPoint > 1){respiration(mode,true,false,false,resolution);}
      if(listeEffect[1][i]==3 && nbPoint > 1){respiration(mode,false,true,false,resolution);}
      if(listeEffect[2][i]==3 && nbPoint > 1){respiration(mode,false,false,true,resolution);}

      if(listeEffect[0][i]==4 && nbPoint > 1){Scintillement(mode,true,false,false,resolution);}
      if(listeEffect[1][i]==4 && nbPoint > 1){Scintillement(mode,false,true,false,resolution);}
      if(listeEffect[2][i]==4 && nbPoint > 1){Scintillement(mode,false,false,true,resolution);}
          
      if(listeEffect[0][i]==5 && nbPoint > 1){crepitement(mode,true,false,false,resolution,o);}
      if(listeEffect[1][i]==5 && nbPoint > 1){crepitement(mode,false,true,false,resolution,o);}
      if(listeEffect[2][i]==5 && nbPoint > 1){crepitement(mode,false,false,true,resolution,o);}

      if(listeEffect[0][i]==6){ExtinctionEndToBegin(mode,true,false,false,resolution,o);}
      if(listeEffect[1][i]==6){ExtinctionEndToBegin(mode,false,true,false,resolution,o);}
      if(listeEffect[2][i]==6){ExtinctionEndToBegin(mode,false,false,true,resolution,o);}

      if(listeEffect[0][i]==7){ExtinctionBeginToEnd(mode,true,false,false,resolution,o);}
      if(listeEffect[1][i]==7){ExtinctionBeginToEnd(mode,false,true,false,resolution,o);}
      if(listeEffect[2][i]==7){ExtinctionBeginToEnd(mode,false,false,true,resolution,o);}
          
      CommunicationLEDS();
    }  
    break;

  default:
    break;
  }
}

void A()
{
  Serial.println("Fonction A");
  etatPre=etatAct;
  etatAct="Parametres";
  ChangementParrametres();

  paraToTab(ledNewHue, listeActPara[3], listeActPara[0]);
  paraToTab(ledNewSat, listeActPara[3], listeActPara[1]);
  paraToTab(ledNewLum, listeActPara[3], listeActPara[2]);

  resolution=80;
      
  CalculDegradeParPoint(ledActHue,ledNewHue,vectHue,resolution);
  CalculDegradeParPoint(ledActSat,ledNewSat,vectSat,resolution);
  CalculDegradeParPoint(ledActLum,ledNewLum,vectLum,resolution);

  for(int i = 0; i< resolution; i++)
  {
    ChangementParPoint(true, true, true);
    CommunicationLEDS();
    if(Serial){if(Serial.available()>0 ){receptionSerie(); break;}}
  }
  // Serial.println("Modifications terminées !");

  changementParams =false;
}

void B()
{
  Serial.println("Fonction B");
  etatPre=etatAct;
  etatAct="Effet";
  ChangementEffet();
  changementEffets = false;
  //// Serial.println("Modifications terminées !");
}

void C()
{
  Serial.println("Fonction C");
  etatPre=etatAct;
  etatAct="Off";
  // Serial.println(etatAct);
  for(int i=0;i<nbPoint;i++)
  {
    listeNewPara[2][i]=0;
  }
  paraToTab(ledNewLum, listeActPara[3], listeNewPara[2]);
  resolution=30;
  CalculDegradeParPoint(ledActLum,ledNewLum,vectLum,resolution);

  for(int i = 0; i< resolution; i++)
  {
    ChangementParPoint(false, false, true);
    CommunicationLEDS();
  }
  allumee =false;
}

void D()
{
  Serial.println("Fonction D");
  etatPre=etatAct;
  etatAct="On";
  // Serial.println(etatAct);
  paraToTab(ledNewLum, listeActPara[3], listeActPara[2]);
  resolution=30;
  CalculDegradeParPoint(ledActLum,ledNewLum,vectLum,resolution);
  for(int i = 0; i< resolution; i++)
  {
    ChangementParPoint(false, false, true);
    CommunicationLEDS();
  }
  allumee =true;
}

void setup()
{
  //----------------------------------------------------Serial
  // Serial.begin(115200);
  // Serial.println("\n");
  // Serial.begin(115200, SERIAL_8N1, RXD2, TXD2);


  // pinMode(ComLEDS, OUTPUT);
  // FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
  // FastLED.setBrightness(  BRIGHTNESS );
  Serial.begin(115200);
  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(BRIGHTNESS);
  FastLED.delay(1/60); 
  


}

void loop() {

  Serial.print("Loop !");
  if(emission==true)
  {

  }else
  {
    if(Serial){receptionSerie();}
  }
  
  if(etatAct!=etatPre)
  {
    etatPre=etatAct;
  }

  if(comPower == allumee)
  {
    if(changementParams == true && connexion == false && allumee == true)
    {
      A();
    }
    if(changementParams == false && changementEffets == true && connexion == false && allumee == true)
    {
      B();
    }
    if(changementParams == false && changementEffets== false && connexion == false && allumee == true)//Resipration pour HUE
    {
      etatPre=etatAct;
      etatAct="Boucle d'éffets";
      for(int i= 0; i<nbEffet;i++)
      {
        resolution=listeEffect[3][i];
        gestionEffets(0, i);
        gestionEffets(1, i);
        gestionEffets(2, i);
        gestionEffets(3, i);
        if(changementParams==true || changementEffets == true ||  comPower!=allumee || Serial.available()>0 || etatReception>0){break;}

      }
    }
  }else{
    if(comPower == false && allumee == true)
    {
      C();
    }
    if(comPower == true && allumee == false)
    {
      D();
    }
  }
  if(!Serial1)
  {
    Serial1.begin(115200);
    if(!Serial1)
    {
      B();
    }

  }
}



