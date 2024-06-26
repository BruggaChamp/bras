/**
   * @file bras.cpp
   * @brief Communication entre l'afficheur STONE HMI, Arduino MEGA et action des casiers du B.R.A.S
   * documentation : https://docs.ks-infinite.fr/bras/
   * github : https://github.com/kerogs/bras/
   * @author Lucas W.
   * @author Florian V.
   * @author Jessy K.
   * @version b1.1.36
   * @date 07/02/2024
   * @copyright Copyright - B.R.A.S, Kerogs Infinite, Lycée Condorcet - Stiring-Wendel
   */

#include "Adafruit_Thermal.h"
#include "SoftwareSerial.h"

#define led 2
#define TX_PIN 47
#define RX_PIN 49

SoftwareSerial printSerial(RX_PIN, TX_PIN);
Adafruit_Thermal printer(&printSerial);

char tampon[30];
int tamponPos = 0;
String tamponStr;

char incomingByte;

// Casier Action
int casierActionNumber;
char casierActionName;

int casiersPassword[7];

int PasswordTemp;

int M1dirpin = 7;
int M1steppin = 6;
const int BP1 = 30;
const int BP2 = 31;
const int BPfdc = 32;
const int BPfdc2 = 33;
const int LedR = 40;
const int LedG = 41;

bool casierUtilisation = false;  // true ? casiser utilisé : casier non utilisé
bool modeAdmin = false;

void setup() {
  configset();

  // sendColorHMI("Casier1", "bg_color", "white");
  // Serial3.print("ST<{\"cmd_code\":\"set_buzzer\",\"type\":\"system\",\"time\":10000}>ET");
  // Serial3.print("ST<{\"cmd_code\":\"set_color\",\"type\":\"widget\",\"widget\":\"Casier1\",\"color_object\":\"bg_color\", \"color\":4293602631}>ET");
}



void loop() {
  // éteint la led à la fin d'une trame
  digitalWrite(led, LOW);
  // if (digitalRead(BP1) == HIGH) /*Si le bouton 1 est appuyer*/
  // {
  //   while (digitalRead(BPfdc2) != HIGH) /*Tant que le bouton de find de course est différent de l'état haut*/
  //   {
  //     rotation_inverse();      /*Lance la rotation du sens des aiguilles d'une montres*/
  //     digitalWrite(LedR, LOW); /*Eteint la led Rouge*/
  //   }
  // } else if (digitalRead(BP2) == HIGH) /*Autrement, pendant que le bouton 2 est appyer*/
  // {
  //   while (digitalRead(BPfdc) != HIGH) /*Tant que le bouton 2 de fin de course est différent de l'état haut*/
  //   {
  //     rotation_montre();       /*Lance la rotation du sens inverse des aiguilles d'une montres*/
  //     digitalWrite(LedR, LOW); /*Eteint la led Rouge*/
  //   }
  // }

  // Début de trame
  if (searchArray(tampon, "ST<", 3)) {
    Serial.println("[DEBUT DE TRAME]");

    if (searchArray(tampon, "MA", 2) || modeAdmin) {
      modeAdmin = true;

      if (searchArray(tampon, "AP", 2)) {
        Serial.println("[MODE ADMIN [???]]");
        // if valeur casier == 4
        int tamponLength = sizeof(tampon) / sizeof(tampon[0]);
        PasswordTemp = getValueAdmin(tampon, tamponLength);
        Serial.println(PasswordTemp);

        if (PasswordTemp == "160524") {
          Serial.println("[MODE ADMIN [CONFIRMATION]]");
        }
      }
    }

    // Bouton Casier appuyer
    if (searchArray(tampon, "Casier", 6)) {
      Serial.println("[CASIER]");
      for (int i = 0; i <= 6; i++) {
        String casierNumber = "Casier" + String(i);
        if (searchArray(tampon, casierNumber.c_str(), 7)) {
          Serial.println("[NUMERO " + String(i) + "]");
          casierActionNumber = i;
          break;
        }
      }

      // TODO EVITER LA REPETITION POUR PAS DESACTIVER AUTOMATIQUEMENT

      // Si casier déjà fermer ?
      for (int i = 1; i < 7; i++) {
        if (casierActionNumber == i) {
          if (casiersPassword[i]) {
            Serial3.print("ST<{\"cmd_code\":\"set_enable\",\"type\":\"widget\",\"widget\":\"IC\",\"enable\":false}>ET");
            Serial.println("[CASIER STATUS : DEJA UTILISER]");
            casierUtilisation = true;
          } else {
            Serial3.print("ST<{\"cmd_code\":\"set_enable\",\"type\":\"widget\",\"widget\":\"IC\",\"enable\":true}>ET");
            Serial.println("[CASIER STATUS : PAS UTILISER]");
          }
        }
      }
    }

    // MDP set Casier
    if (casierActionNumber != 0 && searchArray(tampon, "PC", 2)) {
      Serial.println("[CASIER PASSWORD [MDP]");
      // if valeur casier === 4
      int tamponLength = sizeof(tampon) / sizeof(tampon[0]);
      PasswordTemp = getValue(tampon, tamponLength);
      Serial.println(PasswordTemp);

      if (!casierUtilisation) {
        for (int i = 1; i < 7; i++) {
          if (casierActionNumber == i) {
            casiersPassword[i] = PasswordTemp;
          }
        }
      }

      if (numDigits(PasswordTemp) == 4) {
        Serial3.println("ST<{\"cmd_code\":\"set_enable\",\"type\":\"widget\",\"widget\":\"CC\",\"enable\":true}>ET");
        Serial.println("ST<{\"cmd_code\":\"set_enable\",\"type\":\"widget\",\"widget\":\"CC\",\"enable\":true}>ET");
      } else {
        Serial3.println("ST<{\"cmd_code\":\"set_enable\",\"type\":\"widget\",\"widget\":\"CC\",\"enable\":false}>ET");
        Serial.println("ST<{\"cmd_code\":\"set_enable\",\"type\":\"widget\",\"widget\":\"CC\",\"enable\":false}>ET");
      }

      Serial.println(numDigits(PasswordTemp));
    }

    // Confirmer Client (CC)
    if (casierActionNumber != 0 && searchArray(tampon, "CC", 2)) {
      Serial.println("[CONFIRMATION CLIENT]");
      Serial.println(casierActionNumber);

      Serial.println(casierActionNumber);
      Serial.println(PasswordTemp);
      // Serial.println(casier1Password);

      for (int i = 1; i < 7; i++) {
        Serial.print(i);
        Serial.print(" : ");
        Serial.println(casiersPassword[i]);
      }

      Serial3.println("ST<{\"cmd_code\":\"set_enable\",\"type\":\"widget\",\"widget\":\"CC\",\"enable\":false}>ET");
      Serial.println("ST<{\"cmd_code\":\"set_enable\",\"type\":\"widget\",\"widget\":\"CC\",\"enable\":false}>ET");

      // si casier utilisé
      if (casierUtilisation) {
        // Check bon MDP
        Serial.println(casierActionNumber);
        Serial.println(casiersPassword[casierActionNumber]);
        Serial.println(PasswordTemp);
        if (casiersPassword[casierActionNumber] == PasswordTemp) {
          sendColorHMI(casierActionNumber, "bg_color", "green");
          Serial.println("[OUVERTURE PORTE]");
          casiersPassword[casierActionNumber] = 0;

          // Ouvrir la porte.
          while (digitalRead(BPfdc) != HIGH) {
            rotation_montre();
            digitalWrite(LedG, HIGH);
            digitalWrite(LedR, LOW);
          }
        } else {
          Serial.println("[ERREUR : FAUX MDP]");
        }
      } else {
        sendColorHMI(casierActionNumber, "bg_color", "red");
        Serial.println("[FERMETURE PORTE]");
        // Fermer la porte
        while (digitalRead(BPfdc2) != HIGH) {
          rotation_inverse();
          digitalWrite(LedG, LOW);
          digitalWrite(LedR, HIGH);
        }
        imprimante(casierActionNumber, PasswordTemp);

        // Ouvrir la porte.
        // while (digitalRead(BPfdc) != HIGH) {
        //   rotation_montre();
        // }
      }
      casierActionNumber = NULL;
      casierUtilisation = false;
      PasswordTemp = 0;
    }

    // Password popup up close
    if (casierActionNumber != 0 && searchArray(tampon, "FermerPopup", 11)) {
      Serial.println("[PASSWORD POPUP FERME]");

      casierActionNumber = NULL;
      casierUtilisation = false;
    }
  }

  tamponReset();
}




















void serialEvent3() {
  while (Serial3.available() > 0) {
    digitalWrite(led, HIGH);
    incomingByte = Serial3.read();
    tampon[tamponPos] = incomingByte;
    tamponPos++;
  }

  // Afficher la trame en ASCII
  Serial.println("+=======================+");
  Serial.print("|> ASCII : ");
  for (int i = 0; i < tamponPos; i++) {
    Serial.print(tampon[i]);
  }
  Serial.println("");

  // Afficher la trame en HEX
  Serial.print("|> HEX : ");
  for (int i = 0; i < tamponPos; i++) {
    Serial.print(tampon[i], HEX);
    Serial.print(" ");
  }
  Serial.println("");

  Serial.println("+=======================+");

  // En attente d'un autre
  Serial.println("En attente...");
}


// configset
void configset() {
  // PC
  while (!Serial) Serial.print(".");
  Serial.begin(9600);
  Serial.print(".");

  // HMI
  while (!Serial3) Serial.print(".");
  Serial3.begin(9600);
  Serial.print(".");

  // Imprimante
  while (!printSerial) Serial.print(".");
  printSerial.begin(19200);
  Serial.print(".");
  printer.begin();
  Serial.print(".");

  // LED
  pinMode(led, OUTPUT);
  Serial.print(".");

  pinMode(M1dirpin, OUTPUT);
  Serial.print(".");
  pinMode(M1steppin, OUTPUT);
  Serial.print(".");
  pinMode(BP1, INPUT);
  Serial.print(".");
  pinMode(BP2, INPUT);
  Serial.print(".");
  pinMode(BPfdc, INPUT);
  Serial.print(".");
  pinMode(BPfdc2, INPUT);
  Serial.print(".");
  pinMode(LedR, OUTPUT);
  Serial.print(".");
  pinMode(LedG, OUTPUT);
  Serial.print(".");


  // Fin chargement (ne pas supprimer)
  delay(500);
  Serial.println("Fin");
}

// Fonction pour rechercher une chaîne de caractères dans un tableau
bool searchArray(char array[], char arraySearch[], int numberCSearch) {
  bool sequenceFound = false;

  for (int i = 0; i < 30 - numberCSearch + 1; i++) {
    bool match = true;
    for (int j = 0; j < numberCSearch; j++) {
      if (tampon[i + j] != arraySearch[j]) {
        match = false;
        break;
      }
    }
    if (match) {
      sequenceFound = true;
      break;
    }
  }
  return sequenceFound;
}

void tamponReset() {
  // fin de trame
  if (searchArray(tampon, ">ET", 3)) {
    Serial.println("[FIN DE TRAME]");
    for (int i = 0; i < sizeof(tampon); i++) {
      tampon[i] = 0;
    }
    tamponPos = 0;
  }
}

void sendColorHMI(char widget[], char color_object[], char color[]) {
  // Vert : rgba(14, 217, 38, 1) = 4279163174
  // Rouge : rgba(235, 45, 71, 1) = 4293602631
  // Blanc : rgba(255, 255, 255, 1)= 4294967295
  // Serial3.print("ST<{\"cmd_code\":\"set_buzzer\",\"type\":\"system\",\"time\":200}>ET");
  // Serial3.print("ST<{\"cmd_code\":\"set_color\",\"type\":\"widget\",\"widget\":\"Casier1\",\"color_object\":\"bg_color\", \"color\":4293602631}>ET");
  // Serial3.print("ST<{\"cmd_code\":\"set_color\",\"type\":\"widget\",\"widget\":\"Casier1\",\"color_object\":\"text_color\", \"color\":4294967295}>ET");
  // Serial3.print("ST<{\"cmd_code\":\"set_color\",\"type\":\"widget\",\"widget\":\"Casier1\",\"color_object\":\"border_color\", \"color\":4294967295}>ET");

  color == "white" ? color = "4294967295" : color = color;
  color == "black" ? color = "4278190080" : color = color;
  color == "green" ? color = "4279163174" : color = color;
  color == "red" ? color = "4293602631" : color = color;

  widget == 1 ? widget = "Casier1" : widget = widget;
  widget == 2 ? widget = "Casier2" : widget = widget;
  widget == 3 ? widget = "Casier3" : widget = widget;
  widget == 4 ? widget = "Casier4" : widget = widget;
  widget == 5 ? widget = "Casier5" : widget = widget;
  widget == 6 ? widget = "Casier6" : widget = widget;

  String widgetStr = widget;
  String color_objectStr = color_object;
  String colorStr = color;

  Serial3.println("ST<{\"cmd_code\":\"set_color\",\"type\":\"widget\",\"widget\":\"" + widgetStr + "\",\"color_object\":\"" + color_objectStr + "\", \"color\":" + colorStr + "}>ET");
  Serial.println("[ENVOIE HMI] ST<{\"cmd_code\":\"set_color\",\"type\":\"widget\",\"widget\":\"" + widgetStr + "\",\"color_object\":\"" + color_objectStr + "\", \"color\":" + colorStr + "}>ET");
}

// Récupérer password
int getValue(char array[], int length) {
  char arraySearchStart[] = "\"PC\":";
  char arraySearchEnd[] = ">ET";
  int numberCSearchStart = sizeof(arraySearchStart) - 1;  // -1 pour ne pas compter le caractère de fin de chaîne '\0'
  int numberCSearchEnd = sizeof(arraySearchEnd) - 1;

  bool sequenceFound = false;
  int startIndex = -1;

  for (int i = 0; i < length - numberCSearchStart + 1; i++) {
    bool match = true;
    for (int j = 0; j < numberCSearchStart; j++) {
      if (array[i + j] != arraySearchStart[j]) {
        match = false;
        break;
      }
    }
    if (match) {
      sequenceFound = true;
      startIndex = i + numberCSearchStart;
      break;
    }
  }

  if (!sequenceFound) {
    return -1;
  }

  sequenceFound = false;
  int endIndex = -1;

  for (int i = startIndex; i < length - numberCSearchEnd + 1; i++) {
    bool match = true;
    for (int j = 0; j < numberCSearchEnd; j++) {
      if (array[i + j] != arraySearchEnd[j]) {
        match = false;
        break;
      }
    }
    if (match) {
      sequenceFound = true;
      endIndex = i;
      break;
    }
  }

  if (!sequenceFound) {
    return -1;
  }

  String pcValue = "";
  for (int i = startIndex; i < endIndex; i++) {
    pcValue += array[i];
  }

  return pcValue.toInt();
}

// Récupérer password
int getValueAdmin(char array[], int length) {
  char arraySearchStart[] = "\"AP\":";
  char arraySearchEnd[] = ">ET";
  int numberCSearchStart = sizeof(arraySearchStart) - 1;  // -1 pour ne pas compter le caractère de fin de chaîne '\0'
  int numberCSearchEnd = sizeof(arraySearchEnd) - 1;

  bool sequenceFound = false;
  int startIndex = -1;

  for (int i = 0; i < length - numberCSearchStart + 1; i++) {
    bool match = true;
    for (int j = 0; j < numberCSearchStart; j++) {
      if (array[i + j] != arraySearchStart[j]) {
        match = false;
        break;
      }
    }
    if (match) {
      sequenceFound = true;
      startIndex = i + numberCSearchStart;
      break;
    }
  }

  if (!sequenceFound) {
    return -1;
  }

  sequenceFound = false;
  int endIndex = -1;

  for (int i = startIndex; i < length - numberCSearchEnd + 1; i++) {
    bool match = true;
    for (int j = 0; j < numberCSearchEnd; j++) {
      if (array[i + j] != arraySearchEnd[j]) {
        match = false;
        break;
      }
    }
    if (match) {
      sequenceFound = true;
      endIndex = i;
      break;
    }
  }

  if (!sequenceFound) {
    return -1;
  }

  String pcValue = "";
  for (int i = startIndex; i < endIndex; i++) {
    pcValue += array[i];
  }

  return pcValue.toInt();
}

// Nombre de chiffre int
int numDigits(int number) {
  if (number == 0) {
    return 1;
  }

  int digits = 0;
  if (number < 0) digits = 1;  // compte le signe moins pour les nombres négatifs
  while (number) {
    number /= 10;
    digits++;
  }
  return digits;
}

void rotation_montre() {
  digitalWrite(M1dirpin, LOW);
  digitalWrite(M1steppin, LOW);
  delayMicroseconds(2);
  digitalWrite(M1steppin, HIGH);
  delayMicroseconds(750);
}

void rotation_inverse() {
  digitalWrite(M1dirpin, HIGH);
  digitalWrite(M1steppin, LOW);
  delayMicroseconds(2);
  digitalWrite(M1steppin, HIGH);
  delayMicroseconds(750);
}

void imprimante(int codeCasierNumber, int codeCasier) {
  printer.underlineOn();
  printer.justify('C');
  printer.setSize('M');
  printer.println("-------------------------");
  printer.println(F("KEROGS INFINITE - BRAS"));
  printer.println("-------------------------");
  printer.print(F("Casier Numero "));
  printer.print(codeCasierNumber);
  printer.print(F("\n"));
  printer.setSize('L');
  printer.boldOn();
  printer.println(F("CODE"));
  printer.println(codeCasier);
  printer.setSize('M');
  printer.justify('C');
  printer.println("-------------------");
  printer.boldOff();
  printer.print(F("\n"));
  printer.print(F("\n"));
  printer.setSize('S');
  printer.feed(2);
  printer.setDefault();  // Restore printer to defaults
}