#include <DHTesp.h>
#include <Ticker.h>
#include <WiFi.h>
#include <WiFiMulti.h>

#define COUNT_LOW 0
#define COUNT_HIGH 8888
#define Water 33
#define DHT_PIN 32
#define DHTTYPE DHT11
#define ARR_CNT 7
#define INFO_CNT 8
#define CMD_SIZE 100
#define CDS_SENSOR_PIN 35

//DHT
DHTesp dht;
void tempTask(void *pvParameters);
bool getTemperature();
void triggerGetTemp();
bool initTemp();
TaskHandle_t tempTaskHandle = NULL;
Ticker tempTicker;
ComfortState cf;
bool tasksEnabled = false;
int dhtPin = 32;
float heatIndex;
float dewPoint;
float cr;

//timer
volatile int intCnt;
volatile int srvCnt;
hw_timer_t *timer = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;
void Timer_init();


//wifi
WiFiMulti WiFiMulti;
WiFiClient client;
int status;
int i = 0;
const uint16_t port = 5000;
const char* host = "10.10.141.69";  // ip or dns
void Wifi_init();
void socketEvent();

void IRAM_ATTR onTimer(){
  portENTER_CRITICAL_ISR(&timerMux);
  intCnt++;
  srvCnt++;
  portEXIT_CRITICAL_ISR(&timerMux);
}


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Wifi_init();
  initTemp();
  Timer_init();
  tasksEnabled = true;
  pinMode(SERVO_PIN,OUTPUT);
  ledcSetup(0,50,16);
  ledcAttachPin(SERVO_PIN,0);
}

void loop() {

  if(client.available()){
    socketEvent();
  }      
  int CDS = (analogRead(CDS_SENSOR_PIN));
  int wVal=analogRead(Water);

  if(intCnt>200)
  {
    portENTER_CRITICAL(&timerMux);
    intCnt=0;
    portEXIT_CRITICAL(&timerMux); 
    Serial.printf("Water : %d Temp : %.2f, Hum : %.2f CDS : %d\r\n", wVal, heatIndex, dewPoint, CDS);
    client.printf("[SQL]SENSOR@%d@%.0f@%.0f@%d\r\n", wVal, heatIndex, dewPoint,CDS);
    client.flush();
  }  

  if (!tasksEnabled) {
    tasksEnabled = true;
    if (tempTaskHandle != NULL) {
			vTaskResume(tempTaskHandle);
		}
  }
  yield();
}

void Wifi_init(){
  WiFiMulti.addAP("iot0", "iot00000");
  Serial.println();
  Serial.print("Waiting for WiFi... ");
  while (WiFiMulti.run() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  client.connect(host, port);
  client.print("[ARD:PASSWD]\r\n");
  delay(500);
}

void socketEvent()
{
  int i = 0;
  char * pToken;
  char * pArray[ARR_CNT] = {0};
  char recvBuf[CMD_SIZE] = {0};
  int len;

  len = client.readBytesUntil('\n', recvBuf, CMD_SIZE);
  client.flush();
  Serial.print("recv : ");
  Serial.println(recvBuf);

  pToken = strtok(recvBuf, "[@]");
  while (pToken != NULL)
  {
    pArray[i] =  pToken;
    if (++i >= ARR_CNT)
      break;
    pToken = strtok(NULL, "[@]");
  } 
  if(!strncmp(pArray[1],"Ser",3))
  {
    sendFlag=true;
  }
}

bool initTemp() {
  byte resultValue = 0;
  // Initialize temperature sensor
	dht.setup(dhtPin, DHTesp::DHT11);
	Serial.println("DHT initiated");

  // Start task to get temperature
	xTaskCreatePinnedToCore(
			tempTask,                       /* Function to implement the task */
			"tempTask ",                    /* Name of the task */
			4000,                           /* Stack size in words */
			NULL,                           /* Task input parameter */
			5,                              /* Priority of the task */
			&tempTaskHandle,                /* Task handle. */
			1);                             /* Core where the task should run */

  if (tempTaskHandle == NULL) {
    Serial.println("Failed to start task for temperature update");
    return false;
  } else {
    // Start update of environment data every 20 seconds
    tempTicker.attach(10, triggerGetTemp);
  }
  return true;
}

void triggerGetTemp() {
  if (tempTaskHandle != NULL) {
	   xTaskResumeFromISR(tempTaskHandle);
  }
}

void tempTask(void *pvParameters) {
	Serial.println("tempTask loop started");
	while (1) // tempTask loop
  {
    if (tasksEnabled) {
      // Get temperature values
			getTemperature();
		}
    // Got sleep again
		vTaskSuspend(NULL);
	}
}

bool getTemperature() {
	// Reading temperature for humidity takes about 250 milliseconds!
	// Sensor readings may also be up to 2 seconds 'old' (it's a very slow sensor)
  TempAndHumidity newValues = dht.getTempAndHumidity();
	// Check if any reads failed and exit early (to try again).
	if (dht.getStatus() != 0) {
		Serial.println("DHT11 error status: " + String(dht.getStatusString()));
		return false;
	}

	heatIndex = dht.computeHeatIndex(newValues.temperature, newValues.humidity);
  dewPoint = dht.computeDewPoint(newValues.temperature, newValues.humidity);
  cr = dht.getComfortRatio(cf, newValues.temperature, newValues.humidity);

  String comfortStatus;
  switch(cf) {
    case Comfort_OK:
      comfortStatus = "Comfort_OK";
      break;
    case Comfort_TooHot:
      comfortStatus = "Comfort_TooHot";
      break;
    case Comfort_TooCold:
      comfortStatus = "Comfort_TooCold";
      break;
    case Comfort_TooDry:
      comfortStatus = "Comfort_TooDry";
      break;
    case Comfort_TooHumid:
      comfortStatus = "Comfort_TooHumid";
      break;
    case Comfort_HotAndHumid:
      comfortStatus = "Comfort_HotAndHumid";
      break;
    case Comfort_HotAndDry:
      comfortStatus = "Comfort_HotAndDry";
      break;
    case Comfort_ColdAndHumid:
      comfortStatus = "Comfort_ColdAndHumid";
      break;
    case Comfort_ColdAndDry:
      comfortStatus = "Comfort_ColdAndDry";
      break;
    default:
      comfortStatus = "Unknown:";
      break;
  };

  //Serial.println(" T:" + String(newValues.temperature) + " H:" + String(newValues.humidity) + " I:" + String(heatIndex) + " D:" + String(dewPoint) + " " + comfortStatus);
	return true;
}

void Timer_init(){
  timer = timerBegin(0,80,true);
  timerAttachInterrupt(timer, &onTimer, true);
  timerAlarmWrite(timer, 50000, true);
  timerAlarmEnable(timer);
}