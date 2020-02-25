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
SampleTracker t, p, h, g; // doesn't need parens?!

//Sensor setup:
#include <Adafruit_BME680.h>
#define BME_SCK 22
#define BME_SDU 21
//#define SCL   22 //SCK on BME680
//#define SDA   21 //SDI on BME680
Adafruit_BME680 bme; //I2C

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
  static double x;
  if (! bme.performReading()) {
    Serial.println("Failed to perform reading :(");
    return;
  }else{
    message = "";
    x = bme.temperature;
    if(t.isSignificant(x)){
      message += "Temperature = ";
      message += x;
      message += " *C\n                   ";
    }
    x = bme.pressure / 1000.0; //because kPa
    if(p.isSignificant(x)){
      message +=  "Pressure = ";
      message += x;
      message += " kPa\n                   ";
    }
    x = bme.humidity;
    if(h.isSignificant(x)){
      message += "Humidity = ";
      message += x;
      message += "%\n                   ";
    }
    x = bme.gas_resistance / 1000.0; //because kOhms
    if(g.isSignificant(x)){
      message += "Gas = ";
      message += x;
      message += " kOhms\n                   ";
    }
    Serial.println("                   " + message);
    if (message != "")
      mesh.sendBroadcast(message);
  }
}

Task taskSendMessage( TASK_SECOND * 1 , TASK_FOREVER, &sendMessage );
Task taskCheckSensor( TASK_SECOND, TASK_FOREVER, &checkSensor );

void sendMessage() {
  //formatting: "                   "
  String msg = "Hello from node " + shortid;
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
// BME sensor
  if(!bme.begin()){
    Serial.println("failed to initialize device! Please check your wiring.");
  }
  
  // Set up oversampling and filter initialization
  bme.setTemperatureOversampling(BME680_OS_8X);
  bme.setHumidityOversampling(BME680_OS_2X);
  bme.setPressureOversampling(BME680_OS_4X);
  bme.setIIRFilterSize(BME680_FILTER_SIZE_3);
  bme.setGasHeater(320, 150); // 320*C for 150 ms

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