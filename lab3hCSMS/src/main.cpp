/*************************************************************
Group 1:

Solmaz Hashemzadeh
Jonathan Combs
*************************************************************/
#include "painlessMesh.h"
//sample tracker:
class SampleTracker{
private:
  double *data;
  unsigned int limit;
  unsigned int i;
public:
  SampleTracker(unsigned int data_points_remembered = 60){
    limit = data_points_remembered;
    data = new double[limit];
    while(data_points_remembered > 0){
      data[--data_points_remembered] = 0.0;
    }
    i = limit;
  }

  ~SampleTracker(){
    delete[] data;
  }

  bool isSignificant(double x){
    double mean = 0.0;
    for(double *end = data + limit, *x = data; x < end; x++){
      mean += *x;
    }
    mean /= limit;
    double diff;
    double square;
    double square_sum = 0.0;
    for(double *end = data + limit, *x = data; x < end; x++){
      diff = mean - *x;
      square = diff * diff;
      square_sum += square;
    }
    double variance = square_sum / limit;
    double standard_deviation = sqrt(variance);
    
    // add data as well!
    if( ++i >= limit ){
      i = 0;
    }
    data[i] = x;

    return (x >= mean) ? (x >= (mean + standard_deviation)) : (x <= (mean - standard_deviation));
  }

  String toString(){
    String output = "[ ";
    for(double *end = data + limit, *x = data; x < end - 1; x++){
      output += *x;
      output += ", ";
    }
    output += data[limit-1];
    output += " ]";
    return output;
  }
};
SampleTracker checker; // doesn't need parens?!

//Sensor setup:
#define SPEED 115200
#define CSMS 36
#define DRY 3700
//seems to stay above 3760 when dry, but 3700 seems safer when the values swing so widely

//Mesh setup:
#define   MESH_PREFIX     "SHJC"
#define   MESH_PASSWORD   "such_4_password"
#define   MESH_PORT       5555
String shortid;

Scheduler userScheduler; // to control your personal task
painlessMesh  mesh;

// User stub
void sendMessage() ; // Prototype so PlatformIO doesn't complain
void checkSensor(){
  static String message;
  static unsigned int x;
  x = analogRead(CSMS);
  if(checker.isSignificant(x)){
    message = "Dryness: ";
    message += x;
    message += "; sensor is ";
    if (x > DRY)
      message += "dry!";
    else // eventually maybe test for wet soil?
      message += "wet!";
    mesh.sendBroadcast(message);
    Serial.println(message);
  }
}

Task taskSendMessage( TASK_SECOND * 1 , TASK_FOREVER, &sendMessage );
Task taskCheckSensor( TASK_SECOND, TASK_FOREVER, &checkSensor );

void sendMessage() {
  //formatting: "                   "
  String msg = "Hello from node " + shortid;
  mesh.sendBroadcast( msg );
  taskSendMessage.setInterval( random( TASK_SECOND * 5, TASK_SECOND * 20 ));
  Serial.println(checker.toString());
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
  Serial.begin(SPEED);

  //sensor
  pinMode(CSMS, INPUT);


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
  taskSendMessage.enable();
  userScheduler.addTask( taskCheckSensor );
  taskCheckSensor.enable();
}

void loop() {
  // it will run the user scheduler as well
  mesh.update();
}
