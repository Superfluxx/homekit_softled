#include <FastLED.h>
#include <HomeSpan.h>  // Inclure la bibliothèque HomeSpan pour HomeKit

// ---- Configuration des LEDs ----
#define LED_PIN     4
#define NUM_LEDS    7
#define BRIGHTNESS  255
#define LED_TYPE    WS2812B
#define COLOR_ORDER GRB
CRGB leds[NUM_LEDS];

//----Class





//-----Constantes liées à ce programme


const int nbPointMax = 16;
const int nbParametre = 4;
const int nbPreset = 2;
const int limiteParametre[nbParametre][2]={{0,190},{0,255},{0,255},{0,NUM_LEDS}};

const String nomParametre[nbParametre]={"hue","saturation","luminosite","position"};




//-----Variables de parametrage

String nomLumiere = "défaut";
String nomEffet = "défaut";


bool changementParams = true;
bool changementEffets = true;
bool allumee = false;
bool compower_flag = false;
bool preset_flag = false;
bool comPower = false;
int nbPoint=2;
int nbEffet=1;
String etatAct="allumage";
String etatPre="eteint";

int listeActPara[nbParametre][nbPointMax];
int listeNewPara[nbParametre][nbPointMax]={{1,100},{255,210},{200,255},{1,3}};
int listeEffect[nbParametre][nbPointMax];
int listeNewEffect[nbParametre][nbPointMax]={{0},{0},{0},{250}};
String NomEffet[8]={"Aucun","Rot Trigo","Rot Horaire","Respiration","Scintillement","Crepitement","Extinction ETB","Extinction BTE"};



class Preset {
public:
  int nbPoint;
  int nbEffet;

  int listeNewPara[nbParametre][nbPointMax];
  int listeNewEffect[nbParametre][nbPointMax];

  Preset() {
  nbPoint = 0;
  nbEffet = 0;
  memset(listeNewPara, 0, sizeof(listeNewPara));
  memset(listeNewEffect, 0, sizeof(listeNewEffect));
}


  Preset(int _nbPoint, int _nbEffet,
        int _listeNewPara[nbParametre][nbPointMax],
        int _listeNewEffect[nbParametre][nbPointMax])
      : nbPoint(_nbPoint), nbEffet(_nbEffet) {

    for (int i = 0; i < nbParametre; i++) {
      for (int j = 0; j < nbPointMax; j++) {
        listeNewPara[i][j] = _listeNewPara[i][j];
        listeNewEffect[i][j] = _listeNewEffect[i][j];
      }
    }
  }
};

Preset clonePreset(Preset src) {
  Preset clone;
  clone.nbPoint = src.nbPoint;
  clone.nbEffet = src.nbEffet;

  for (int i = 0; i < nbParametre; i++) {
    for (int j = 0; j < nbPointMax; j++) {
      clone.listeNewPara[i][j] = src.listeNewPara[i][j];
      clone.listeNewEffect[i][j] = src.listeNewEffect[i][j];
    }
  }

  return clone;
}


int datalisteNewPara_1[nbParametre][nbPointMax] = {{100, 120},{255, 210},{200, 255},{1, 3}};
int dataListeNewEffect_1[nbParametre][nbPointMax] = {{1},{2},{3},{250}};
Preset preset_1(2, 1, datalisteNewPara_1, dataListeNewEffect_1);

int datalisteNewPara_2[nbParametre][nbPointMax] = {{1, 20},{200, 255},{100, 100},{1, 3}};
int dataListeNewEffect_2[nbParametre][nbPointMax] = {{0},{0},{0},{250}};
Preset preset_2(2, 1, datalisteNewPara_2, dataListeNewEffect_2);

Preset presets[nbPreset]={preset_1, preset_2};
Preset preset_act = presets[0];
Preset preset_pre = presets[0];

void printLine(const char* name, int lineIndex, int tableau[nbParametre][nbPointMax], int nbPoints) {
  Serial.printf("%s: [", name);
  for (int j = 0; j < nbPoints; j++) {
    Serial.printf("%d", tableau[lineIndex][j]);
    if (j < nbPoints - 1) Serial.print(", ");
  }
  Serial.println("]");
}

void applyPreset(Preset monPreset) {
  preset_flag = false;

  nbPoint=monPreset.nbPoint;
  nbEffet=monPreset.nbEffet;

  for (int i = 0; i < nbParametre; i++) {
    for (int j = 0; j < nbPoint; j++) {
      listeNewPara[i][j] = monPreset.listeNewPara[i][j];
      listeNewEffect[i][j] = monPreset.listeNewEffect[i][j];
    }
  }
  changementParams = true;
  changementEffets = true;
}



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




void A();
void B();
void C();
void D();

void show()
{

    FastLED.delay(1/60); 
    FastLED.show();        // Mettre à jour les LEDs

}


void ChangementParrametres()//sert uniquement a la comunication
{
  for(int i=0; i<nbParametre;i++)
  {
    for(int o=0; o<nbPoint; o++)
    {
      listeActPara[i][o]=listeNewPara[i][o];
    }
  }
}

void ChangementEffet()//sert uniquement a la comunication
{
  for(int i=0; i<nbParametre;i++)
  {
    for(int o=0; o<nbEffet; o++)
    {
      listeEffect[i][o]=listeNewEffect[i][o];      
    }
  }
}


void CommunicationLEDS()
{
  for(int i=0 ; i<NUM_LEDS ; i++)
  {
    if(compower_flag){break;}
    

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
    if(compower_flag){break;}
    


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
    for(int o = 0; o<resolution;o++)
    {
      if(compower_flag){break;}
      


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
    if(compower_flag){break;}
    

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
    for(int o = 0; o<resolution;o++)
    {
      
      if(compower_flag){break;}
      

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
  }

  changementParams =false;
}

void B()
{
  etatPre=etatAct;
  etatAct="Effet";
  ChangementEffet();
  changementEffets = false;
}

void C() {
  etatPre = etatAct;
  etatAct = "Off";

  Preset preset_extinct = clonePreset(preset_act);

  for (int i = 0; i < nbPoint; i++) {
    preset_extinct.listeNewPara[2][i] = 0;
  }
  applyPreset(preset_extinct); 
  A();
  B();
  allumee = false;
}

void D()
{
  etatPre=etatAct;
  etatAct="On";
  applyPreset(preset_act);
  A();
  B();
  allumee =true;
}



// ---- Tâche dédiée aux LEDs ----
void ledTask(void *parameter) {
    // Initialiser les LEDs
    FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
    FastLED.setBrightness(BRIGHTNESS);
    FastLED.delay(1/60); 
    FastLED.clear();
    FastLED.show();


    while (true) {

        if(etatAct!=etatPre)
        {
            etatPre=etatAct;
        }

        if(allumee == true && comPower == true)
        {
            if(changementParams == true )
            {
            A();
            }
            if(changementEffets == true)
            {
            B();
            }
            if(changementParams == false && changementEffets == false)//Resipration pour HUE
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
              if(changementParams==true || changementEffets == true ||  comPower!=allumee || compower_flag==true || preset_flag==true){break;}
            }
            }
        }else{
            if(compower_flag == true){
              compower_flag = false;
            }
            if(comPower == false && allumee == true)
            {
            C();
            }
            if(comPower == true && allumee == false)
            {
            D();
            }
        }
        vTaskDelay(10 / portTICK_PERIOD_MS);  // Pause minimale pour d'autres tâches
    }
}



struct LEDService : Service::LightBulb {
    Characteristic::On *power;  // Caractéristique pour contrôler l'état de la lumière (on/off)

    LEDService() : Service::LightBulb() {
        power = new Characteristic::On(false);  // Initialisation à éteint
    }

    boolean update() override {
        compower_flag = true;
        if (power->getNewVal()) {
            comPower = true;
        } else {
            comPower = false;
        }
        return true;
    }
};


struct PresetSelectorService : Service::Fan {
  Characteristic::RotationSpeed *presetSlider;
  int lastSelectedPreset = -1;  // Mémorise le dernier preset appliqué

  PresetSelectorService() : Service::Fan() {
    presetSlider = new Characteristic::RotationSpeed(0);  // Valeur initiale : preset[0]
    presetSlider->setRange(0, nbPreset - 1, 1);            // Min = 0, Max = nbPreset-1, step = 1
  }

  boolean update() override {
    preset_flag = true;
    int selectedPreset = (int)presetSlider->getVal();

    if (selectedPreset != lastSelectedPreset) {
      lastSelectedPreset = selectedPreset;
      preset_act = presets[selectedPreset];
      applyPreset(preset_act);
    }

    

    return true;
  }
};


// ---- Tâche dédiée à HomeKit et au serveur ----
void homeKitTask(void *parameter) {
    Serial.println("HomeKit Task démarré");
    // Configuration HomeKit
    homeSpan.setPairingCode("11122333");  // Définir le code d'appairage
    homeSpan.setQRID("111-22-333");      // QR ID pour HomeKit

    // Gestion des paramètres Wi-Fi
    homeSpan.setWifiCredentials("Livebox-72CA_EXT_24", "NCqLSErkrjCKFuow5t");
    homeSpan.begin(Category::Lighting, "test_esp32");  // Initialiser HomeKit

    // ---- Configuration de l'accessoire principal ----
    new SpanAccessory();
      new Service::AccessoryInformation();
        new Characteristic::Identify();
        new Characteristic::Name("LED Lights");
      new LEDService();
      new PresetSelectorService();


    Serial.println("Accessoire configuré, boucle HomeKit démarrée...");

    // Démarrer la boucle principale HomeKit
    while (true) {
        homeSpan.poll();  // Gérer les requêtes HomeKit
        vTaskDelay(10 / portTICK_PERIOD_MS);  // Laisser du temps pour éviter une surcharge


    }
}

void setup() {
    Serial.begin(115200);
    Serial.println("Initialisation du système...");


    // Démarrer la tâche LED
    xTaskCreatePinnedToCore(
        ledTask,          // Fonction de la tâche
        "LED Task",       // Nom de la tâche
        4096,             // Taille de la pile
        NULL,             // Paramètre (non utilisé)
        3,                // Priorité élevée
        NULL,             // Handle de tâche
        1                 // Exécuter sur le second core
    );

    // Démarrer la tâche HomeKit
    xTaskCreatePinnedToCore(
        homeKitTask,      // Fonction de la tâche
        "HomeKit Task",   // Nom de la tâche
        8192,             // Taille de la pile (plus grand pour HomeKit)
        NULL,             // Paramètre (non utilisé)
        2,                // Priorité moyenne
        NULL,             // Handle de tâche
        1                 // Exécuter sur le second core
    );
}

void loop() {
    // Laisser le système en mode multitâche ; ne rien bloquer ici
    delay(1000);
}



