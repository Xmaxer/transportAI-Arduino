#define CUSTOM_SETTINGS
#define INCLUDE_TERMINAL_SHIELD
#define INCLUDE_GPS_SHIELD
#define INCLUDE_INTERNET_SHIELD
#define INCLUDE_CLOCK_SHIELD


#include <OneSheeld.h>

//Every 5 seconds: update location
//Every 2 seconds: check for order

const char* licence_plate = "00-XX-00000";

unsigned long lastUpdate = 0;
unsigned long lastCheck = 0;
int status = 0;

HttpRequest request("http://www.transport-ai.com/requests/ardra");

void setup() {
  OneSheeld.begin();
  Clock.queryDateAndTime();

  //request.setContentType("application/json");

}

void loop() {
 unsigned long currentTime = millis();

 if(currentTime - lastUpdate >= 5000)
 {
  updateLocation();
  lastUpdate = currentTime;
 }

 if(currentTime - lastCheck >= 2000)
 {
  checkForOrder();
  lastCheck = currentTime;
 }
}

void updateLocation()
{
  float gpsLong = GPS.getLongitude();
  float gpsLat = GPS.getLatitude();
  String year = (String)Clock.getYear();
    String month = ((long)Clock.getMonth() <= 9) ? (String)"0" + Clock.getMonth() : (String)Clock.getMonth();
    String day = ((long)Clock.getDay() <= 9) ? (String)"0" + Clock.getDay() : (String)Clock.getDay();
    String hour = ((long)Clock.getHours() <= 9) ? (String)"0" + Clock.getHours() : (String)Clock.getHours();
    String minute = ((long)Clock.getMinutes() <= 9) ? (String)"0" + Clock.getMinutes() : (String)Clock.getMinutes();
    String second = ((long)Clock.getSeconds() <= 9) ? (String)"0" + Clock.getSeconds() : (String)Clock.getSeconds();
  String time = (String)year + "-" + month + "-" + day+ "T" + hour + ":" + minute + ":" + second + "Z";
  String lat = String(gpsLat,7);
  String lng = String(gpsLong,7);
  int latLength = lat.length() + 1;
  char latChars[latLength];
  lat.toCharArray(latChars, latLength);

    int longLength = lng.length() + 1;
  char longChars[longLength];
  lng.toCharArray(longChars, longLength);

  request.getResponse().setOnJsonResponse(&onJsonReplyOrigin);
  request.setOnSuccess(&onSuccessOrigin);
  
  request.addParameter("latitude", latChars);
  request.addParameter("longitude", longChars);
  request.addParameter("code", "1");
  request.addParameter("car", licence_plate);
  Internet.performGet(request);
  request.deleteParameters();

  request.addParameter("latitude", latChars);
  request.addParameter("longitude", longChars);
  request.addParameter("code", "2");
  request.addParameter("car", licence_plate);
  Internet.performPost(request);
  request.deleteParameters();
}

void checkForOrder()
{
  request.getResponse().setOnJsonResponse(&onJsonReply);
  request.setOnSuccess(&onSuccess);
  request.addParameter("code", "4");
  request.addParameter("car", licence_plate);
  Internet.performGet(request);
  request.deleteParameters();
}

void confirmOrder()
{ 
  request.addParameter("code", "3");
  request.addParameter("car", licence_plate);
  Internet.performPost(request);
  request.deleteParameters();
}

void cancelOrder()
{
        request.addParameter("code", "5");
  request.addParameter("car", licence_plate);
  Internet.performPost(request);
  request.deleteParameters();
}

void onJsonReply(JsonKeyChain & hell,char * output)
{
  status = atoi(output);

  Terminal.println("Status: " + status);

  if(status == 1)
  {
    confirmOrder();
  }
  else if(status == 10)
  {
    cancelOrder();
  }
}

void onSuccess(HttpResponse & response)
{
  response["grpc"]["fields"]["status"]["integer_value"].query();
}

void sendNotification()
{
  request.addParameter("code", "6");
  request.addParameter("car", licence_plate);
  Internet.performPost(request);
  request.deleteParameters();

}

void onJsonReplyOrigin(JsonKeyChain & hell,char * output)
{

  int distance = atoi(output);

  Terminal.println("Distance: " + distance);
  
  if(distance == -1)
  {
    return;
  }
  if(distance <= 30)
  {
    if(status == 2)
    {
      sendNotification();
    }
    else if(status == 3)
    {
      completeOrder();
    }
    
  }

}

void completeOrder()
{
  request.addParameter("code", "7");
  request.addParameter("car", licence_plate);
  Internet.performPost(request);
  request.deleteParameters(); 
}

void onSuccessOrigin(HttpResponse & response)
{
  Terminal.println("updated");
  response["distance"].query();
}

void checkDestination()
{
  request.addParameter("code", "6");
  request.addParameter("car", licence_plate);
  Internet.performGet(request);
  request.deleteParameters(); 
}
