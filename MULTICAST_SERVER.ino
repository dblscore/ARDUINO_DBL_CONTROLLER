#include <SD.h>
#include <STRING.h>
#include <Ethernet3.h>
#include <EthernetUdp3.h>

// Configuration réseau
byte mac[] = { 0xA8, 0x61, 0x0A, 0xAE, 0x26, 0x35 };
IPAddress ip(192, 168, 100, 10); // Adresse IP du récepteur
IPAddress multicastIP(239, 0, 0, 255); // Adresse multicast
unsigned int multicastPort = 5555;   // Port multicast

EthernetUDP udp;


const int chipSelect = 4;
File root;

int scale;
int chord;
String dblFileName;

String linesDBL[100];
int nLignes;
int ligneActive=0;
/////////////////////////////////////////////////////////////////////////
///////// RECUPERATION DE SCALE ET CHORD EN INT 
/////////////////////////////////////////////////////////////////////////

void getLigneInt(String fileName, int nLigne) {
  File file = SD.open(fileName); // Ouvre le fichier en lecture
  if (!file) {
    Serial.println("Erreur d'ouverture du fichier !");
    return;
  }

  int currentLine = 0; // Compteur de lignes
  String lineContent;

  while (file.available()) {
    lineContent = file.readStringUntil('\n'); // Lire jusqu'à la fin de la ligne

    if (currentLine == nLigne) { 
      // Extraire les données de la ligne
      int separatorIndex = lineContent.indexOf(';');
      if (separatorIndex != -1) {
        scale = lineContent.substring(0, separatorIndex).toInt(); // Convertir en entier
        chord = lineContent.substring(separatorIndex + 1).toInt(); // Enlever les espaces
        Serial.println("dans la fonction");
        Serial.println(scale);
        Serial.println(chord);
      } else {
        Serial.println("Erreur de format dans la ligne.");
      }
      break;
    }
    currentLine++;
  }

  if (currentLine != nLigne) {
    Serial.println("Ligne demandée non trouvée !");
  }

  file.close(); // Fermer le fichier
}

/////////////////////////////////////////////////////////////////////////
///////// RECUPERATION nombre de lignes
/////////////////////////////////////////////////////////////////////////

int getLignesNumber(String fileName) {
  File file = SD.open(fileName); // Ouvre le fichier en lecture
  if (!file) {
    Serial.println("Erreur d'ouverture du fichier !");
    return;
  }

  int currentLine = 0; // Compteur de lignes
  String lineContent;

  while (file.available()) {
    lineContent = file.readStringUntil('\n');
    currentLine++;

  }
return currentLine;
}

/////////////////////////////////////////////////////////////////////////
///////// RECUPERATION D'UNE LIGNE ENTIERE
/////////////////////////////////////////////////////////////////////////

String getLigneString(String fileName, int nLigne) {
  if (nLigne < 0) {
    Serial.println("Numéro de ligne invalide !");
    return "";
  }

  File file = SD.open(fileName); // Ouvre le fichier en lecture
  if (!file) {
    Serial.println("Erreur d'ouverture du fichier !");
    return "";
  }

  int currentLine = 0; // Compteur de lignes
  String lineContent = "";

  while (file.available()) {
    lineContent = file.readStringUntil('\n'); // Lire jusqu'à la fin de la ligne
    if (currentLine == nLigne) { 
      file.close();
      return lineContent; // Retourne directement si la ligne est trouvée
    }
    currentLine++;
  }

  // Si la ligne demandée n'est pas trouvée
  Serial.println("Ligne demandée non trouvée !");
  file.close(); // Fermer le fichier
  return "";
}


void setup() {
 // Open serial communications and wait for port to open:
  Serial.begin(9600);
  // wait for Serial Monitor to connect. Needed for native USB port boards only:
  while (!Serial);

  Serial.print("Initializing SD card...");

  if (!SD.begin(chipSelect)) {
    Serial.println("initialization failed. Things to check:");
    Serial.println("1. is a card inserted?");
    Serial.println("2. is your wiring correct?");
    Serial.println("3. did you change the chipSelect pin to match your shield or module?");
    Serial.println("Note: press reset button on the board and reopen this Serial Monitor after fixing your issue!");
    while (true);
  }

  Serial.println("initialization done.");

  root = SD.open("/");

  printDirectory(root, 0);

  Serial.println("done!");

  dblFileName = "dbl.txt";
  File file = SD.open(dblFileName);

  if (!file) {
    Serial.println("Erreur d'ouverture du fichier !");
    return;
  } else {
    Serial.println("fichier ouvert !");
  }

Serial.println(millis());


nLignes = getLignesNumber(dblFileName);

Serial.println(millis());

for(int i=0;i<nLignes;i++){
  String linetemp = getLigneString(dblFileName, i);
  linetemp.trim();
  linesDBL[i] = linetemp;

  Serial.println(linetemp);
 
}

  Ethernet.begin(mac, ip);
  Serial.begin(9600);
  // Initialisation UDP
  udp.beginMulticast(multicastIP, multicastPort);
  Serial.println("Émetteur prêt.");


  
}

void loop() {
  String payload = getLigneString(dblFileName, ligneActive);//linesDBL[ligneActive];
  Serial.println(payload);
  udp.beginPacket(multicastIP, multicastPort);

  Serial.println(payload.length());
  udp.write(payload.c_str(), payload.length());//print(payload);
  udp.endPacket();

  delay(1000);
  ligneActive++;

  if (ligneActive>=8){
    ligneActive =0;
  }
}

void printDirectory(File dir, int numTabs) {
  while (true) {

    File entry =  dir.openNextFile();
    if (! entry) {
      // no more files
      break;
    }
    for (uint8_t i = 0; i < numTabs; i++) {
      Serial.print('\t');
    }
    Serial.print(entry.name());
    if (entry.isDirectory()) {
      Serial.println("/");
      printDirectory(entry, numTabs + 1);
    } else {
      // files have sizes, directories do not
      Serial.print("\t\t");
      Serial.println(entry.size(), DEC);
    }
    entry.close();
  }
}
