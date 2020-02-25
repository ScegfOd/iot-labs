/*************************************************************
Group 1:

Solmaz Hashemzadeh
Jonathan Combs
*************************************************************/
#include "painlessMesh.h"

//mesh:
#define   MESH_PREFIX     "SHJC"
#define   MESH_PASSWORD   "such_4_password"
#define   MESH_PORT       5555
String shortid;

//PIR:
#define PIR_A 35
#define PIR_D 16

Scheduler userScheduler; // to control your personal task
painlessMesh  mesh;

// User stub
void sendMessage();
bool checkPIR;
void resetPIR(){
  checkPIR = true;
}

Task taskSendMessage( TASK_SECOND * 1 , TASK_FOREVER, &sendMessage );
Task taskResetPIR( TASK_SECOND * 1 , TASK_FOREVER, &resetPIR );
void sendMessage() {
  //formatting: "                   "
  String msg = "Hello from node " + shortid + " & the PIR!";
  mesh.sendBroadcast( msg );
  taskSendMessage.setInterval( random( TASK_SECOND * 5, TASK_SECOND * 20 ));
}

// Needed for painless library
void receivedCallback( uint32_t from, String &msg ) {
  Serial.printf("Received from %u: %s\n", from%1000, msg.c_str());
}

void newConnectionCallback(uint32_t nodeId) {
  Serial.printf("New Connection, nodeId = %u\n", nodeId%1000);
}

void changedConnectionCallback() {
  Serial.printf("Changed connections\n");
}

void nodeTimeAdjustedCallback(int32_t offset) {
  Serial.printf("Adjusted time %u. Offset = %d\n", mesh.getNodeTime(),offset);
}

void setup() {
  Serial.begin(115200);
  pinMode(PIR_A, INPUT);
  pinMode(PIR_D, INPUT);

//mesh.setDebugMsgTypes( ERROR | MESH_STATUS | CONNECTION | SYNC | COMMUNICATION | GENERAL | MSG_TYPES | REMOTE ); // all types on
  mesh.setDebugMsgTypes( ERROR | STARTUP );  // set before init() so that you can see startup messages

  mesh.init( MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT );
  mesh.onReceive(&receivedCallback);
  mesh.onNewConnection(&newConnectionCallback);
  mesh.onChangedConnections(&changedConnectionCallback);
  mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);
  // This node and all other nodes should ideally know the mesh contains a root, so call this on all nodes
  mesh.setContainsRoot(true);
  
  shortid = mesh.getNodeId() % 1000;
  while(shortid.length() < 3)
    shortid = '0' + shortid;

  userScheduler.addTask( taskSendMessage );
  userScheduler.addTask( taskResetPIR );
  taskSendMessage.enable();
  taskResetPIR.enable();
  resetPIR();
}

int motionStatus;
String PIR_ping;

void loop() {
  // it will run the user scheduler as well
  mesh.update();
  // If motion is detected, send a message:
  if (checkPIR && digitalRead(PIR_D)){
    checkPIR = false; // don't check more often than once/second
    PIR_ping = "PIR measurement: " + String(analogRead(PIR_A));
    mesh.sendBroadcast( PIR_ping );
    Serial.println( PIR_ping );
  }
}