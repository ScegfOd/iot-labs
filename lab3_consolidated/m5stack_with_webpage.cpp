/*************************************************************
Group 1:

Solmaz Hashemzadeh
Jonathan Combs
*************************************************************/
#include "painlessMesh.h"
#include <M5Stack.h>
#include "IPAddress.h"
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

#define   MESH_PREFIX     "SHJC"
#define   MESH_PASSWORD   "such_4_password"
#define   MESH_PORT       5555
#define HOSTNAME "HTTP_BRIDGE"

//wifi
#define   SSID     "SHJCwifi"
#define   PASSWORD "such_4_password"
AsyncWebServer server(80);
//WiFiServer server(80);
//WiFi.softAP(ssid, password);
String IP;



//IPAddress myIP = WiFi.softAPIP();
//IPAddress myIP(0,0,0,0);
//IPAddress myAPIP(0,0,0,0);

String shortid;
#define MSG_DB_SIZE 30
String msg_db[MSG_DB_SIZE];//keep last 30 messages for the webpage

Scheduler userScheduler; // to control your personal task
painlessMesh  mesh;

// User stub
void sendMessage();
void clear();

Task taskSendMessage( TASK_SECOND * 1 , TASK_FOREVER, &sendMessage );
Task taskClearScreen( TASK_SECOND * 30 , TASK_FOREVER, &clear);

void clear(){
  M5.Lcd.clear();
  M5.Lcd.setCursor(0,0);
  M5.Lcd.println("Base (node " + shortid + ") is listening. Addr: " + IP/*myAPIP.toString()/*mesh.getStationIP().toString()*/);
}

void sendMessage() {
  //formatting: "                   "
  String msg = "Hello from base! (node " + shortid + ')';
  mesh.sendBroadcast( msg );
  taskSendMessage.setInterval( random( TASK_SECOND * 5, TASK_SECOND * 20 ));
  Serial.println("Sent '" + msg + '\'');
}


int i;

// Needed for painless library
void receivedCallback( const uint32_t &from, const String &msg ) {
  //formatting: "                   "
  String printable = "Received from ";
  printable += static_cast<String>(from%1000);
  printable += ": ";
  printable += msg;
  printable += '\n';
  M5.Lcd.print(printable);
  Serial.print(printable);
  if(++i >= MSG_DB_SIZE) i = 0;
  for(int j = 0, max = printable.length() - 1; j <= max ; j++){
    if(printable[j] == '\n'){
      /*if(j == max){
        printable[j] = '\0';
      }else{*/
        printable = printable.substring(0,j) + "<br>" + printable.substring(j+1);
        max += 3;
        j += 3;
      //}
    } 
  }
  msg_db[i] = printable;
  if(i + 1 < MSG_DB_SIZE){
    msg_db[i+1] = "^^^ newest message ^^^";
  }
}

void newConnectionCallback(uint32_t nodeId) {
  M5.Lcd.printf("New Connection, nodeId = %u\n", nodeId%1000);
}

void changedConnectionCallback() {
  M5.Lcd.printf("Changed connections\n");
}

void nodeTimeAdjustedCallback(int32_t offset) {
  M5.Lcd.printf("Adjusted time %u. Offset = %d\n", mesh.getNodeTime(),offset);
}
String html;
void setup() {
  M5.begin();
  Serial.begin(115200);
  for(i = 0; i < MSG_DB_SIZE; i++){
    msg_db[i] = "";
  }
  i = 0;


//mesh.setDebugMsgTypes( ERROR | MESH_STATUS | CONNECTION | SYNC | COMMUNICATION | GENERAL | MSG_TYPES | REMOTE ); // all types on
  mesh.setDebugMsgTypes( ERROR | STARTUP | CONNECTION );  // set before init() so that you can see startup messages

  mesh.init( MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT );

  mesh.onReceive(&receivedCallback);
  mesh.onNewConnection(&newConnectionCallback);
  mesh.onChangedConnections(&changedConnectionCallback);
  mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);
  //mesh.stationManual(STATION_SSID, STATION_PASSWORD);
  mesh.setHostname(HOSTNAME);
  // Bridge node, should (in most cases) be a root node. See [the wiki](https://gitlab.com/painlessMesh/painlessMesh/wikis/Possible-challenges-in-mesh-formation) for some background
  mesh.setRoot(true);
  // This node and all other nodes should ideally know the mesh contains a root, so call this on all nodes
  mesh.setContainsRoot(true);

  WiFi.softAP(SSID, MESH_PASSWORD, 7);
  //IP = IPAddress(mesh.getAPIP()).toString();
  IP = WiFi.softAPIP().toString();
  Serial.println("My AP IP is " + IP);

  shortid = mesh.getNodeId() % 1000;
  while(shortid.length() < 3)
    shortid = '0' + shortid;
  M5.Lcd.print(shortid);

  userScheduler.addTask( taskSendMessage );
  userScheduler.addTask( taskClearScreen );
  taskSendMessage.enable();
  taskClearScreen.enable();



  //Async webserver

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    html = "";
    for(auto x : msg_db){
      html += "<p>";
      html += x;
      html += "</p>";
    }
    html += "<form><input type='submit' value='Refresh page'></form>";
//    request->send(200, "text/html", "<form><input type='submit' value='Refresh page'></form>");
    request->send(200, "text/html", html.c_str());
/*
    if (request->hasArg("BROADCAST")){
      String msg = request->arg("BROADCAST");
      mesh.sendBroadcast(msg);
    }
*/
  });
  server.begin();
  M5.Lcd.clear();
  M5.Lcd.setCursor(0,0);
  M5.Lcd.print("                  Setup is now done!");
}




void loop() {
  // it will run the user scheduler as well
  mesh.update();
  /*
  WiFiClient client = server.available();
  if (client) {
    Serial.println("new client");
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        Serial.write(c);
        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply
        if (c == '\n' && currentLineIsBlank) {
          // send a standard http response header
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println("Connection: close");  // the connection will be closed after completion of the response
          client.println("Refresh: 5");  // refresh the page automatically every 5 sec
          client.println();
          client.println("<!DOCTYPE HTML>");
          client.println("<html>");
          client.println("Hello World!");
          client.println("</html>");
          break;
        }
        if (c == '\n') {
          // you're starting a new line
          currentLineIsBlank = true;
        } else if (c != '\r') {
          // you've gotten a character on the current line
          currentLineIsBlank = false;
        }
      }
    }
  }
  */
}

