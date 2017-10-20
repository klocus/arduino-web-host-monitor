/*----------------------------------------------------------------
  Name:         Web Host Monitor

  Desc:         Status monitor of hosts that can be entered using
                the web interface.
  
  Date:         13.06.2016
 
  Author:       Paweł Klockiewicz @ CDV
----------------------------------------------------------------*/

#include <SPI.h>
#include <Ethernet.h>
#include <LiquidCrystal.h>
#include <SdFat.h>
#include <NettigoKeypad.h>
#include <aWOT.h>

// Disable warnings before converting strings to 'char *'
#pragma GCC diagnostic ignored "-Wwrite-strings"

// MAC Address and IP of Arduino
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
// IPAddress ip(192, 168, 2, 8);
IPAddress ip(192, 168, 0, 2);
// Server library initialization
EthernetServer server(80);
// Network Client initialization
EthernetClient client;

// SdFat object initialization
SdFat SD;

// Host list variables
char hostsNames[4][20];
int hostsCount;
int maxHosts = 4;
int currentHost = -1;

// LCD library initialization
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);

// Object for button reading
NG_Keypad keypad;

// Object for the web interface
WebApp app;

/**
 * Display Intro
 * Displays a welcome message
 */
void displayIntro()
{
  Serial.println(F("WEB HOST MONITOR"));
  Serial.print(F("You have "));  
  Serial.print(hostsCount);
  Serial.println(F(" hosts"));
  lcd.clear();
  lcd.print("WEB HOST MONITOR");
  lcd.setCursor(0,1); 
  lcd.print("You have ");  
  lcd.print(hostsCount);
  lcd.print(" hosts");
}

/**
 * LCD Printer
 * Displays text on LCD
 */
void lcdPrint(char a[], char b[] = 0)
{
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print(a);
  lcd.setCursor(0,1); 
  lcd.print(b);  
}

/**
 * Save Hosts
 * Writes a list of hosts to the `hosts.txt` file
 */
void saveHosts(char data[])
{
  SdFile wrfile;
  if(!wrfile.open("hosts.txt", O_RDWR | O_CREAT | O_TRUNC))
    Serial.println(F("Opening hosts.txt failed!"));
  else
  {
    Serial.println(F("Writing data to file..."));
    wrfile.println(data);
    wrfile.close();
    Serial.println(F("Writing done."));
  }
}

/**
 * Get Hosts
 * Gets the list of hosts from the `hosts.txt` file
 */
void getHosts()
{
  char line[20];
  int n, i(0);

  SdFile rdfile("hosts.txt", O_READ);

  if (!rdfile.isOpen())
    Serial.println(F("Failed to open hosts.txt"));

  // read lines from the file
  while (((n = rdfile.fgets(line, sizeof(line))) > 0) && (i < maxHosts))
  {
    if (line[0] != '\n')
    { 
      strtok(line, "\r\n");
      strcpy(hostsNames[i], line);
      i++;
    }
  }
  hostsCount = i;
}

/**
 * Next Host
 * Starts the monitor for the next host
 */
void nextHost()
{
  currentHost++;
  if(currentHost > (hostsCount - 1))
    currentHost = 0;
    
  runMonitor(hostsNames[currentHost]);
}

/**
 * Previous Host
 * Starts the monitor for the previous host
 */
void prevHost()
{
  currentHost--;
  if(currentHost < 0)
    currentHost = (hostsCount - 1);
    
  runMonitor(hostsNames[currentHost]);
}

/**
 * Host Monitor
 * Check if host is available online
 */
void runMonitor(char* host)
{
  if(client.connect(host, 80))
  {
    if(client.connected() || client.available())
    {
      Serial.print(host);
      Serial.println(F(" is UP!"));
      lcdPrint(host, "is UP!");
    }
    else
    {
      Serial.print(host);
      Serial.println(F(" is DOWN!"));
      lcdPrint(host, "is DOWN!");
    }
  } 
  else
  {
    Serial.println(F("Connection failed"));
    lcdPrint("Connection", "failed");
  }
  client.stop();
}

/**
 * Index CMD
 * Supports server side requests
 */
void indexCmd(Request &req, Response &res)
{
  char *queryHosts = req.query("hosts");
  int i;

  if(strlen(queryHosts) > 4)
  {
    saveHosts(queryHosts);
    getHosts();
    displayIntro();
  }
  
  P(part_one) =
    "<html>\n"
    "<head>\n"
    "<title>Web Host Monitor</title>\n"
    "<meta name='viewport' content='width=device-width, initial-scale=1'>\n"
    "</head>\n"
    "<body>\n"
    "<h1>Web Host Monitor</h1>\n"
    "<h3>Your hosts (max 4):</h3>\n"
    "<form method='get'>\n"
    "<textarea name='hosts' rows='4' cols='24'>";

  P(part_two) =
    "</textarea><br/>\n"
    "<button type='submit'>Save</button>\n"
    "</form>\n"
    "</body>\n"
    "</html>";

  res.success("text/html");
  res.printP(part_one);

  // hosts list
  for(i = 0; i < hostsCount; i++)
  {
    if((i + 1) == hostsCount)
      res.print(hostsNames[i]);
    else
      res.println(hostsNames[i]);  
  }
    
  res.printP(part_two);
}

/**
 * Arduino Setup
 * Settings - one-time instruction block
 */
void setup()
{
  // Initialize communication on serial port
  Serial.begin(9600); 

  // Initialization of the SD card reader
  if (!SD.begin(4))
  {
    Serial.println(F("SD Card Reader initialization failed!"));
    return;
  }

  // Get a list of hosts
  getHosts();
    
  // Initialization of network settings
  Ethernet.begin(mac, ip);
  Serial.println(Ethernet.localIP());
  
  // Routing to handle URL requests
  app.get("/", &indexCmd);

  // Operation of buttons
  keypad.register_handler(NG_Keypad::UP, prevHost);
  keypad.register_handler(NG_Keypad::DOWN, nextHost);
  keypad.register_handler(NG_Keypad::RIGHT, nextHost);
  keypad.register_handler(NG_Keypad::LEFT, prevHost);
  
  // Initialization of LCD display
  lcd.begin(16, 2);

  // Intro message
  displayIntro();
}

/**
 * Arduino Loop
 * Infinite block of instructions
 */
void loop()
{  
  int rd = analogRead(0);
  keypad.check_handlers(rd);
  
  EthernetClient client = server.available();
  if (client)
  {
      app.process(&client);
      client.stop();
  }
  
  delay(100);
}
