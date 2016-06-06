#include <VirtualWire.h> // inclusion de la librairie VirtualWire
//Message: 
//CODE_APPAREIL sur 2 digits
//CODE_ACTION sur 4 digits
//Timestamp sur 14 digits

const char *CODE_APPAREIL = "D01";
const char *CODE_ACTION_ouverture = "A0001";
const char *CODE_ACTION_fermeture = "A0002";
const char *CODE_ACTION_stop = "A0003";
const char *SEPARATEUR = "|";

int pinEmetteurRC = 12; //Emetteur d'ondes radio
int pinRecepteurRC = 11; //Récepteur d'ondes radio
int pinRelaisCourantR1 = 4; // Envoie le jus vers le relais signal
int pinRelaisSignalR2 = 6; // Envoie le jus vers le volet, en fonction du signal ou de son absence
int pinBoutonHautInter = 8; //Entrée, 5V si le bouton montée est en bas 
int pinBoutonBasInter = 9; //Entrée, 5V si le bouton descente est en bas

char *sDecomposition;

char *codeAction;
char *codeAppareil;
char *timeStamp;
char *dernierTimeStampTraite;

int etatPrecedent; //0:neutre 1:montee 2:descente
int etatEnCours; //0:neutre 1:montee 2:descente

int etatInterrupteurPrecedent; //0:neutre 1:montee 2:descente
int etatInterrupteurEnCours; //0:neutre 1:montee 2:descente

char message[100];

int lireInterrupteur() 
{
  int retour=0; //0:neutre - 1:haut - 2:bas
  boolean bBas = digitalRead(pinBoutonBasInter)==LOW;
  boolean bHaut = digitalRead(pinBoutonHautInter)==LOW;
  
  if(bHaut)
  {
      retour = 1;
  }
  else if(bBas)
  {
      retour = 2;
  }
  return retour;
}

void setup() // Fonction setup()
{
    Serial.begin(9600); // Initialisation du port série pour avoir un retour sur le serial monitor

    //Entrées
    pinMode(pinRecepteurRC, INPUT);
    pinMode(pinBoutonHautInter, INPUT_PULLUP);
    pinMode(pinBoutonBasInter, INPUT_PULLUP);
    //Sorties
    pinMode(pinEmetteurRC, OUTPUT);
    pinMode(pinRelaisCourantR1, OUTPUT);
    pinMode(pinRelaisSignalR2, OUTPUT);

    //Configuration de virtualWire
    vw_set_tx_pin(pinEmetteurRC); //pin transmetteur
    vw_set_rx_pin(pinRecepteurRC); //Pin receveur
    vw_setup(2000); // initialisation de la librairie VirtualWire à 2000 bauds (note: je n'utilise pas la broche PTT)
    vw_rx_start();  // Activation de la partie réception de la librairie VirtualWire
    
    Serial.println("Initialisation terminee");

   //Pour tester les relais
   //delay(1000);
   //digitalWrite(pinRelaisCourantR1, LOW);
   //delay(1000);
   //digitalWrite(pinRelaisSignalR2, LOW);

   digitalWrite(pinRelaisCourantR1, HIGH);
   digitalWrite(pinRelaisSignalR2, HIGH); 

   etatInterrupteurPrecedent = lireInterrupteur(); // pour forcer la mise en état au premier cycle
   etatInterrupteurEnCours = etatInterrupteurPrecedent;
}
 
void loop() // Fonction loop()
{
    uint8_t buf[VW_MAX_MESSAGE_LEN]; // Tableau qui va contenir le message reçu (de taille maximum VW_MAX_MESSAGE_LEN)
    uint8_t buflen = VW_MAX_MESSAGE_LEN; // Taille maximum de notre tableau
    int etatInterrupteurEnCours = lireInterrupteur();
    
      //Vérification de l'interrupteur
      if(etatInterrupteurEnCours == 1)
      {
        etatEnCours = 1;
        //Serial.println("Etat en cours: haut");
      }
      else if(etatInterrupteurEnCours == 2)
      {
        etatEnCours = 2;
        //Serial.println("Etat en cours: bas");
      }
      else
      {
        etatEnCours = 0;
        //Serial.println("Etat en cours: neutre");
      }
  
      if(etatInterrupteurPrecedent!=etatInterrupteurEnCours)
      {
        etatInterrupteurPrecedent = etatInterrupteurEnCours;
        switch(etatEnCours)
        {
          case 0:
           //Serial.println("Courant OFF");
           digitalWrite(pinRelaisCourantR1, HIGH);
           digitalWrite(pinRelaisSignalR2, HIGH); 
          break;
          case 1:
            //Serial.println("Courant OFF");
            digitalWrite(pinRelaisSignalR2, HIGH);
            //Serial.println("Ouverture");
            digitalWrite(pinRelaisCourantR1, LOW);
            //Serial.println("Courant ON");
            digitalWrite(pinRelaisSignalR2, LOW);
            break;
          case 2:
            //Serial.println("Courant OFF");
            digitalWrite(pinRelaisSignalR2, HIGH);
            //Serial.println("Fermeture");
            digitalWrite(pinRelaisCourantR1, HIGH);
            //Serial.println("Courant ON");
            digitalWrite(pinRelaisSignalR2, LOW);
          break;
          default: 
            Serial.println("L'état en cours de l'interrupteur est inconnu");
          break;
        }
      }
    
    
    
    if (vw_wait_rx_max(100)) // Si un message est reçu dans les 200ms qui viennent
    {
        if (vw_get_message(buf, &buflen)) // On copie le message, qu'il soit corrompu ou non
        {
            Serial.println("Message recu");
            Serial.println((char*)buf);

            //Décomposition du message en code_appareil, code_action et timestamp
            int i=0;
            codeAppareil = "";
            codeAction = "";
            timeStamp = "";
            sDecomposition = strtok((char*)buf, SEPARATEUR);
            while(sDecomposition!=NULL)
            {
              if(i==0)
              {
                strcpy(codeAppareil,sDecomposition);
              }
              if(i==1)
              {
                strcpy(codeAction,sDecomposition);
              }
              else if(i==2)
              {
                strcpy(timeStamp,sDecomposition);
              }
              sDecomposition = strtok(NULL, SEPARATEUR);
              i++;
            }

            //Si le message n'a pas déjà été traité, c'est à dire que 
            //le timestamp est inconnu, on traite
            if(strcmp(dernierTimeStampTraite, timeStamp) != 0)
            {
              //Si le message n'est pas destiné à ce récepteur, on le répète
              if(strncmp(CODE_APPAREIL, codeAppareil, strlen(CODE_APPAREIL) != 0))
              {
                 Serial.println("Répétition du message"); // On signale le début de l'envoi
                 memset (message, 0, sizeof (message));
                 strcpy(message, (char*)buf);
                 vw_send((uint8_t *)message, strlen(message)); // On envoie le message 
                 vw_wait_tx(); // On attend la fin de l'envoi
                 Serial.println("Répétition effectuée"); // On signal la fin de l'envoi
                 delay(10);
              }
              //Sinon, ouverture
              else if(strncmp(CODE_ACTION_ouverture, codeAction, strlen(CODE_ACTION_ouverture)) == 0)
              {
                Serial.println("Courant OFF");
                digitalWrite(pinRelaisSignalR2, LOW);
                Serial.println("Ouverture");
                digitalWrite(pinRelaisCourantR1, HIGH);
                Serial.println("Courant ON");
                digitalWrite(pinRelaisSignalR2, HIGH);
              }
              //... ou fermeture
              else if(strncmp(CODE_ACTION_fermeture, codeAction, strlen(CODE_ACTION_fermeture)) == 0)
              {
                Serial.println("Courant OFF");
                digitalWrite(pinRelaisSignalR2, LOW);
                Serial.println("Fermeture");
                digitalWrite(pinRelaisCourantR1, LOW);
                Serial.println("Courant ON");
                digitalWrite(pinRelaisSignalR2, HIGH);
              }
              //... ou neutre
              else if(strncmp(CODE_ACTION_stop, codeAction, strlen(CODE_ACTION_stop)) == 0)
              {
                Serial.println("Courant OFF");
                digitalWrite(pinRelaisSignalR2, LOW);
              }
              //... ou j'ai codé comme une merde
              else
              {
                Serial.print("Cet appareil ne gère pas le code ");
                Serial.println(codeAction);
              }
            }
        }
    }
}
