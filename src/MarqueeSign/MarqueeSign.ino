#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>
#include <MD_Parola.h>
#include <MD_MAX72xx.h>
#include <SPI.h>

ESP8266WebServer server(80);

char* ssid = "MarqueeSign";
char* passphrase = "password";
String st;
String content;
int statusCode;
char Deneme[8][61];
uint8_t Aktif[8] = { 1, 1, 1, 0, 0, 0, 0, 0 };
uint8_t Speed[8] = { 40, 70, 40, 40, 40, 40, 40, 40 };
uint8_t Efekt[8] = { 1, 1, 1, 1, 1, 1, 1, 3 };
textEffect_t Effect[8];
char nsid[32];
char npas[64];

//////////////////////////////////
//    Tabela Tanimlamasi    //
//////////////////////////////////
#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
#define MAX_DEVICES 5
#define CLK_PIN   D5  //13
#define DATA_PIN  D7  // 11
#define CS_PIN    D8  //10
MD_Parola P = MD_Parola(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);
#define ARRAY_SIZE(x)  (sizeof(x)/sizeof(x[0]))

void setup() {
  Serial.begin(115200);
  EEPROM.begin(512);
  delay(10);
  Serial.println();
  Serial.println();
  Serial.println("Startup");
  // read eeprom for ssid and pass
  Serial.println("Reading EEPROM ssid");
  for (int y = 0; y < 390; y++)
    Serial.print(int(EEPROM.read(y)));
  Serial.println("");
  if (EEPROM.read(400) != 255) {
    Serial.println("Sistem Yukleniyor...");
    for (int g1 = 0; g1 < 8; g1++) {
      for (int g2 = 0; g2 < 45; g2++) {
        Deneme[g1][g2] = char(EEPROM.read(48 * g1 + g2));
      }
      Aktif[g1] = int(EEPROM.read(48 * g1 + 45));
      Speed[g1] = int(EEPROM.read(48 * g1 + 46));
      Efekt[g1] = int(EEPROM.read(48 * g1 + 47));
      Serial.println(Deneme[g1]);
      Serial.println(Aktif[g1]);
      Serial.println(Speed[g1]);
      Serial.println(Efekt[g1]);
    }
    Serial.println("Sistem Basariyla Yuklendi");
    Tazele();
  } else {
    Serial.println("Sistem Ilk defa Acildigindan Kuruluyor...");
    String Meleka = "Wifi: ";
    Meleka += ssid;
    strcpy(Deneme[0], Meleka.c_str());
    Meleka = "Sifre: ";
    Meleka += passphrase;
    strcpy(Deneme[1], Meleka.c_str());
    strcpy(Deneme[2], "IP: 192.168.4.1");
    Tazele();
  }
  P.begin();
  String esid;
  for (int i = 410; i < 442; ++i)
  {
    esid += char(EEPROM.read(i));
  }
  Serial.print("SSID: ");
  Serial.println(esid);
  Serial.println("Reading EEPROM pass");
  String epass = "";
  for (int i = 442; i < 506; ++i)
  {
    epass += char(EEPROM.read(i));
  }
  Serial.print("PASS: ");
  Serial.println(epass);
  if ( esid[1] != 255 ) {
    Serial.println("Son Kullanicisin");
    WiFi.begin(esid.c_str(), epass.c_str());
    if (testWifi()) {
      launchWeb(0);
      return;
    }
  } else
    Serial.println("Ilk Kullanicisin");
  setupAP();
}
bool testWifi(void) {
  int c = 0;
  Serial.println("Waiting for Wifi to connect");
  while ( c < 20 ) {
    if (WiFi.status() == WL_CONNECTED) {
      return false;
    }
    delay(500);
    Serial.print(WiFi.status());
    c++;
  }
  Serial.println("");
  Serial.println("Connect timed out, opening AP");
  return false;
}


void launchWeb(int webtype) {
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("Local IP: ");
  Serial.println(WiFi.localIP());
  Serial.print("SoftAP IP: ");
  Serial.println(WiFi.softAPIP());
  createWebServer(webtype);
  // Start the server
  server.begin();
  Serial.println("Server started");
}

void setupAP(void) {
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
  int n = WiFi.scanNetworks();
  Serial.println("scan done");
  if (n == 0)
    Serial.println("no networks found");
  else
  {
    Serial.print(n);
    Serial.println(" networks found");
    for (int i = 0; i < n; ++i)
    {
      // Print SSID and RSSI for each network found
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.print(WiFi.SSID(i));
      Serial.print(" (");
      Serial.print(WiFi.RSSI(i));
      Serial.print(")");
      Serial.println((WiFi.encryptionType(i) == ENC_TYPE_NONE) ? " " : "*");
      delay(10);
    }
  }
  Serial.println("");
  st = "<ol>";
  for (int i = 0; i < n; ++i)
  {
    // Print SSID and RSSI for each network found
    st += "<li>";
    st += WiFi.SSID(i);
    st += " (";
    st += WiFi.RSSI(i);
    st += ")";
    st += (WiFi.encryptionType(i) == ENC_TYPE_NONE) ? " " : "*";
    st += "</li>";
  }
  st += "</ol>";
  delay(100);
  WiFi.softAP(ssid, passphrase, 6);
  Serial.println("softap");
  launchWeb(1);
  Serial.println("over");
}

void createWebServer(int webtype)
{
  if ( webtype == 1 ) {
    server.on("/", []() {
      IPAddress ip = WiFi.softAPIP();
      String ipStr = String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]);
      content = "<!DOCTYPE HTML>\r\n<html>Hello from ESP8266 at ";
      content += ipStr;
      content += "<p>";
      content += st;
      content += "</p><form method='get' action='setting'><label>SSID: </label><input name='ssid' length=32><input name='pass' length=64><input type='submit'></form><br><form method='get' action='Deneme'> <input type='submit'></form>";
      content += "</html>";
      server.send(200, "text/html", content);
    });
    server.on("/setting", []() {
      String qsid = server.arg("ssid");
      String qpass = server.arg("pass");
      if (qsid.length() > 0 && qpass.length() > 0) {
        Serial.println("clearing eeprom");
        for (int i = 0; i < 512; ++i) {
          EEPROM.write(i, 0);
        }
        Serial.println(qsid);
        Serial.println("");
        Serial.println(qpass);
        Serial.println("");

        Serial.println("writing eeprom ssid:");
        for (int i = 410; i < 410 + qsid.length(); ++i)
        {
          EEPROM.write(i, qsid[i % 410]);
          Serial.print("Wrote: ");
          Serial.println(qsid[i % 410]);
        }
        Serial.println("writing eeprom pass:");
        for (int i = 410; i < 410 + qpass.length(); ++i)
        {
          EEPROM.write(32 + i, qpass[i % 410]);
          Serial.print("Wrote: ");
          Serial.println(qpass[i % 410]);
        }
        EEPROM.commit();
        content = "{\"Success\":\"saved to eeprom... reset to boot into new wifi\"}";
        statusCode = 200;
      } else {
        content = "{\"Error\":\"404 not found\"}";
        statusCode = 404;
        Serial.println("Sending 404");
      }
      server.send(statusCode, "application/json", content);
    });
    server.on("/Deneme", []() {
      content = "<!DOCTYPE HTML><html><head><style> body {background-color:#FFFFFF;} h1   {color: #94D466;} h2   {color: #D4E88B;} #myHeader {background-color: #FFFFAE; color: #26490D; padding: 40px; text-align: center;} </style></head><body>";
      content += "<h1 id='myHeader'>Marquee Sign</h1>";
      content += "<form method='get' action='Kaydet'>  <fieldset>  <legend><h1>Mesaj Listesi</h1></legend> ";
      for (int zz = 0; zz < 8; zz++) {
        content += "<p>Mesaj ";
        content += String(zz + 1);
        content += ":</p>      Aktif:<input name='Aktif";
        content += String(zz);
        content += "' type='checkbox'";
        if (Aktif[zz] == 1) content += " checked";
        content += ">      Yazi: <input type='text' name='Yazi";
        content += String(zz);
        content += "' value='";
        content += Deneme[zz];
        content += "' maxlength=32 >Hizli <input type='Range' name='Hiz";
        content += String(zz);
        content += "' value='";
        content += Speed[zz];
        content += "' min='10' max='100'> Yavas<select name='Efekt";
        content += String(zz);
        content += "'><option value='1' ";
        if (Efekt[zz] == 1) content += "selected";
        content += ">Sola Kayan</option><option value='2'";
        if (Efekt[zz] == 2) content += "selected";
        content += ">Saga Kayan</option><option value='3'";
        if (Efekt[zz] == 3) content += "selected";
        content += ">Pacman</option></select>Efekt <br>";
      }
      content += "<input type='submit' value='Kaydet'></fieldset></form>";
      content += "</form></body></html>";
      server.send(200, "text/html", content);
    });
    server.on("/Kaydet", []() {
      Serial.println("EEPROM Temizleniyor...");
      for (int i = 0; i < 385; ++i) {
        EEPROM.write(i, 0);
      }
      for (int y = 0; y < 8; y++) {
        Yazzi(y);
        Aktiv(y);
        Efect(y);
        Spleed(y);
      }
      EEPROM.write(400, 0);
      EEPROM.commit();
      Serial.println("###################");
      for (int y = 0; y < 8; y++) {
        Serial.println(Deneme[y]);
        Serial.println(Aktif[y]);
        Serial.println(Efekt[y]);
        Serial.println(Speed[y]);
      }
      Serial.println("###################");
      for (int y = 0; y < 390; y++)
        Serial.print(int(EEPROM.read(y)));
      Serial.println("");
      Tazele();
      content = "<!DOCTYPE HTML><html><head><style> body {background-color:#FFFFFF;} h1   {color: #94D466;} h2   {color: #D4E88B;} #myHeader {background-color: #FFFFAE; color: #26490D; padding: 40px; text-align: center;} </style></head><body>";
      content += "<h1 id='myHeader'>Basariyla Kaydedildi</h1>";
      server.send(200, "text/html", content);
    });
  } else if (webtype == 0) {
    server.on("/", []() {
      IPAddress ip = WiFi.localIP();
      String ipStr = String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]);
      server.send(200, "application/json", "{\"IP\":\"" + ipStr + "\"}");
    });
    server.on("/cleareeprom", []() {
      content = "<!DOCTYPE HTML>\r\n<html>";
      content += "<p>Clearing the EEPROM</p></html>";
      server.send(200, "text/html", content);
      Serial.println("clearing eeprom");
      for (int i = 0; i < 96; ++i) {
        EEPROM.write(i, 0);
      }
      EEPROM.commit();
    });
  }
}
void Yazzi(int Ya) {
  String kollal = "Yazi";
  kollal += Ya;
  String kDeneme = server.arg(kollal);
  for (int fz = 0; fz < 45; fz++)
    Deneme[Ya][fz] = char(0);
  for (int ff = 0; ff < kDeneme.length(); ff++) {
    Deneme[Ya][ff] = kDeneme[ff];
    EEPROM.write(Ya * 48 + ff, char(Deneme[Ya][ff]));
  }
}
void Aktiv(int Ya) {
  String kollal = "Aktif";
  kollal += Ya;
  String kDeneme = server.arg(kollal);
  if (kDeneme == "on") {
    Aktif[Ya] = 1;
    EEPROM.write(Ya * 48 + 45, 1);
  }
  else {
    Aktif[Ya] = 0;
    EEPROM.write(Ya * 48 + 45, 0);
  }
}
void Efect(int Ya) {
  String kollal = "Efekt";
  kollal += Ya;
  String kDeneme = server.arg(kollal);
  Efekt[Ya] = kDeneme.toInt();
  EEPROM.write(Ya * 48 + 47, kDeneme.toInt());
}
void Spleed(int Ya) {
  String kollal = "Hiz";
  kollal += Ya;
  String kDeneme = server.arg(kollal);
  Speed[Ya] = kDeneme.toInt();
  EEPROM.write(Ya * 48 + 46, kDeneme.toInt());
}
void Tazele() {
  for (int Tazel = 0; Tazel < 8; Tazel++) {
    if (Efekt[Tazel] == 2)
      Effect[Tazel] = PA_SCROLL_LEFT;
    else
      Effect[Tazel] = PA_SCROLL_LEFT;
  }
}
const uint8_t F_PMAN1 = 6;
const uint8_t W_PMAN1 = 8;
static const uint8_t PROGMEM pacman1[F_PMAN1 * W_PMAN1] =  // gobbling pacman animation
{
  0x00, 0x81, 0xc3, 0xe7, 0xff, 0x7e, 0x7e, 0x3c,
  0x00, 0x42, 0xe7, 0xe7, 0xff, 0xff, 0x7e, 0x3c,
  0x24, 0x66, 0xe7, 0xff, 0xff, 0xff, 0x7e, 0x3c,
  0x3c, 0x7e, 0xff, 0xff, 0xff, 0xff, 0x7e, 0x3c,
  0x24, 0x66, 0xe7, 0xff, 0xff, 0xff, 0x7e, 0x3c,
  0x00, 0x42, 0xe7, 0xe7, 0xff, 0xff, 0x7e, 0x3c,
};

const uint8_t F_PMAN2 = 6;
const uint8_t W_PMAN2 = 18;
static const uint8_t PROGMEM pacman2[F_PMAN2 * W_PMAN2] =  // ghost pursued by a pacman
{
  0x00, 0x81, 0xc3, 0xe7, 0xff, 0x7e, 0x7e, 0x3c, 0x00, 0x00, 0x00, 0xfe, 0x7b, 0xf3, 0x7f, 0xfb, 0x73, 0xfe,
  0x00, 0x42, 0xe7, 0xe7, 0xff, 0xff, 0x7e, 0x3c, 0x00, 0x00, 0x00, 0xfe, 0x7b, 0xf3, 0x7f, 0xfb, 0x73, 0xfe,
  0x24, 0x66, 0xe7, 0xff, 0xff, 0xff, 0x7e, 0x3c, 0x00, 0x00, 0x00, 0xfe, 0x7b, 0xf3, 0x7f, 0xfb, 0x73, 0xfe,
  0x3c, 0x7e, 0xff, 0xff, 0xff, 0xff, 0x7e, 0x3c, 0x00, 0x00, 0x00, 0xfe, 0x73, 0xfb, 0x7f, 0xf3, 0x7b, 0xfe,
  0x24, 0x66, 0xe7, 0xff, 0xff, 0xff, 0x7e, 0x3c, 0x00, 0x00, 0x00, 0xfe, 0x73, 0xfb, 0x7f, 0xf3, 0x7b, 0xfe,
  0x00, 0x42, 0xe7, 0xe7, 0xff, 0xff, 0x7e, 0x3c, 0x00, 0x00, 0x00, 0xfe, 0x73, 0xfb, 0x7f, 0xf3, 0x7b, 0xfe,
};
void loop() {
  server.handleClient();
  static textPosition_t just = PA_LEFT;
  static uint8_t bnmf = 0;
  if (P.displayAnimate()) {
    if (Aktif[bnmf] == 1)
      P.displayText(Deneme[bnmf], just, Speed[bnmf], 0, Effect[bnmf], Effect[bnmf]);
    bnmf++;
    if (bnmf == 8) bnmf = 0;
  }
}
