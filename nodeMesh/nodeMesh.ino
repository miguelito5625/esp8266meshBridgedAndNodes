 //************************************************************
// this is a simple example that uses the painlessMesh library
//
// 1. sends a silly message to every node on the mesh at a random time between 1 and 5 seconds
// 2. prints anything it receives to Serial.print
//
//
//************************************************************
//#include <Arduino.h>
#include "painlessMesh.h"
#include <ArduinoJson.h>


#define   MESH_PREFIX     "whateverYouLike"
#define   MESH_PASSWORD   "somethingSneaky"
#define   MESH_PORT       5555

Scheduler userScheduler; // to control your personal task
painlessMesh  mesh;

// uncomment only one of the following
#define WHITE
//#define RED
//#define GREEN

#ifdef WHITE
#define ROLE    "white"
#define VERSION "WHITE v1.0.0"
#define MESSAGE "White "
#endif

#ifdef RED
#define ROLE    "red"
#define VERSION "RED - 1.0.0"
#define MESSAGE "Red "
#endif

#ifdef GREEN
#define ROLE    "green"
#define VERSION "GREEN - 1.0.0"
#define MESSAGE "Green "
#endif


bool calc_delay = false;
SimpleList<uint32_t> nodes;
uint32_t nsent=0;
char buff[512];

int temperatura = 0;
int humedad = 0;


// User stub
void sendMessage() ; // Prototype so PlatformIO doesn't complain

Task taskSendMessage( TASK_SECOND * 5 , TASK_FOREVER, &sendMessage );

// Needed for painless library


void receivedCallback( uint32_t from, String &msg ) 
  {
  Serial.printf("Rx from %u <- %s\n", from, msg.c_str());

  if (strcmp(msg.c_str(),"GETRT") == 0)
    {
    mesh.sendBroadcast( mesh.subConnectionJson(true).c_str() );
    }
  else
    {
    sprintf(buff,"Rx:%s",msg.c_str());
    }
  
  }



void newConnectionCallback(uint32_t nodeId) 
  {
  Serial.printf("--> Start: New Connection, nodeId = %u\n", nodeId);
  Serial.printf("--> Start: New Connection, %s\n", mesh.subConnectionJson(true).c_str());
  }



void changedConnectionCallback() 
  {
  Serial.printf("Changed connections\n");

  nodes = mesh.getNodeList();
  Serial.printf("Num nodes: %d\n", nodes.size());
  Serial.printf("Connection list:");
  SimpleList<uint32_t>::iterator node = nodes.begin();
  while (node != nodes.end()) 
    {
    Serial.printf(" %u", *node);
    node++;
    }
  Serial.println();
  calc_delay = true;

  sprintf(buff,"Nodes:%d",nodes.size());
  }



void nodeTimeAdjustedCallback(int32_t offset) 
  {
  Serial.printf("Adjusted time %u Offset = %d\n", mesh.getNodeTime(),offset);
  }



void onNodeDelayReceived(uint32_t nodeId, int32_t delay)
  {
  Serial.printf("Delay from node:%u delay = %d\n", nodeId,delay);
  }


void sendMessage() 
{

  temperatura = random(24,38);
  humedad = random(200,1024);

// Serializing in JSON Format 
  DynamicJsonDocument doc(1024);
  doc["nodeId"] = mesh.getNodeId();
  doc["nodeTag"] = "dispositivo1";
  doc["temperatura"] = temperatura;
  doc["humedad"] = humedad;
  String msg ;
  serializeJson(doc, msg);
  // msg += mesh.getNodeId();
  mesh.sendBroadcast( msg );
  Serial.println(msg);
//  taskSendMessage.setInterval((TASK_SECOND * 1));/
  
}



//void sendMessage(String msg) {
//  if (strcmp(msg.c_str(), "") == 0)
//    {
//    nsent++;
//    msg = MESSAGE;
//    //msg += mesh.getNodeId();
//    msg += nsent;
//    }
//  mesh.sendBroadcast( msg );
//  Serial.printf("Tx-> %s\n", msg.c_str());
//
//  sprintf(buff,"Tx:%s",msg.c_str());
//
//  if (calc_delay) 
//    {
//    SimpleList<uint32_t>::iterator node = nodes.begin();
//    while (node != nodes.end()) 
//      {
//      mesh.startDelayMeas(*node);
//      node++;
//      }
//    calc_delay = false;
//    }
//
//  //taskSendMessage.setInterval( random( TASK_SECOND * 2, TASK_SECOND * 10 ));
//}




void setup() 
  {
  Serial.begin(115200);

  //mesh.setDebugMsgTypes( ERROR | MESH_STATUS | CONNECTION | SYNC | COMMUNICATION | MSG_TYPES | REMOTE ); // all types on except GENERAL
  //mesh.setDebugMsgTypes( ERROR | MESH_STATUS | CONNECTION | SYNC | COMMUNICATION | GENERAL | MSG_TYPES | REMOTE ); // all types on
  mesh.setDebugMsgTypes( ERROR | STARTUP );  // set before init() so that you can see startup messages

  mesh.init( MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT );
  mesh.onReceive(&receivedCallback);
  mesh.onNewConnection(&newConnectionCallback);
  mesh.onChangedConnections(&changedConnectionCallback);
  mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);

  // if you want your node to accept OTA firmware, simply include this line
  // with whatever role you want your hardware to be. For instance, a
  // mesh network may have a thermometer, rain detector, and bridge. Each of
  // those may require different firmware, so different roles are preferrable.
  //
  // MAKE SURE YOUR UPLOADED OTA FIRMWARE INCLUDES OTA SUPPORT OR YOU WILL LOSE
  // THE ABILITY TO UPLOAD MORE FIRMWARE OVER OTA. YOU ALSO WANT TO MAKE SURE
  // THE ROLES ARE CORRECT
  mesh.initOTAReceive(ROLE);

  userScheduler.addTask( taskSendMessage );
  taskSendMessage.enable();
  
  sprintf(buff,"Id:%d",mesh.getNodeId());

  }






void loop() 
  {
  // it will run the user scheduler as well
  mesh.update();
  }
