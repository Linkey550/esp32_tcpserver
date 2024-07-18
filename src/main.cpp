#include <Arduino.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <WiFiClient.h>
#include <WiFiServer.h>
#include <ArduinoOTA.h>
#include<WebServer.h>




const char *ssid = "PLA·Studio🐣";
const char *password = "woaiwuxie1";


#define MAX_SRV_CLIENTS 4
uint16_t serverPort = 12222;         //服务器端口号


WiFiServer server; //声明一个客户端对象，用于与服务器进行连接


// put function declarations here:
//void Setup_OTA(void);

//task list:
void Task_OTA(void *pvParameters);
void Task_Receive(void *pvParameters);


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println("Booting");//串口开启
  WiFi.mode(WIFI_STA);//从模式
  WiFi.begin(ssid, password);//WiFi连接
  while (WiFi.waitForConnectResult() != WL_CONNECTED)
  {
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
  }//连接状况检测
   ArduinoOTA
      .onStart
      ([](){
      String type;
      if (ArduinoOTA.getCommand() == U_FLASH)
        type = "sketch";
      else // U_SPIFFS
        type = "filesystem";

      // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
      Serial.println("Start updating " + type); 
      })
      .onEnd
      ([](){ 
        Serial.println("\nEnd");
      })
      .onProgress
      ([](unsigned int progress, unsigned int total){ 
        Serial.printf("Progress: %u%%\r", (progress / (total / 100))); 
      })
      .onError([](ota_error_t error){
      Serial.printf("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
      else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
      else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
      else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
      else if (error == OTA_END_ERROR) Serial.println("End Failed"); 
      });

  ArduinoOTA.begin();//OTA服务开启

  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());//输出本地ip

  

  server.begin(serverPort); //服务器启动监听端口号12222
  server.setNoDelay(true);


  xTaskCreatePinnedToCore(Task_OTA, "Task_OTA", 10000, NULL, 1, NULL,  0);
  xTaskCreatePinnedToCore(Task_Receive, "Task_Receive", 10000, NULL, 1, NULL,  1);
}

void loop() {
  // put your main code here, to run repeatedly:
 // Serial.printf("hello world");
 //esp32_server.handleClient();

}


void Task_OTA(void *pvParameters) {
  
  while(1)
  {
    ArduinoOTA.handle();
    // loop to blink without delay
    vTaskDelay(1);
  }
}

void Task_Receive(void *pvParameters)
{
   while(1)
   {

          uint8_t i;
          //检测服务器端是否有活动的客户端连接
          if (server.hasClient())
          {
            for(i = 0; i < MAX_SRV_CLIENTS; i++)
            {
              //查找空闲或者断开连接的客户端，并置为可用
              if (!serverClients[i] || !serverClients[i].connected())
              {
                if(serverClients[i]) serverClients[i].stop();
                serverClients[i] = server.available();
                Serial.print("New client: "); Serial.println(i);
                continue;
              }
              vTaskDelay(1);
            }
            //若没有可用客户端，则停止连接
            WiFiClient serverClient = server.available();
            serverClient.stop();
          }
          //检查客户端的数据
          for(i = 0; i < MAX_SRV_CLIENTS; i++)
          {
            if (serverClients[i] && serverClients[i].connected())
            {
              if(serverClients[i].available())
              {
                //从Telnet客户端获取数据，并推送到URAT端口
                while(serverClients[i].available()) {
                  Serial.println(serverClients[i].read());
                  vTaskDelay(1);
                }
                //vTaskDelay(1);
              }
            }
            vTaskDelay(1);
          }
          vTaskDelay(1);
  }
}


