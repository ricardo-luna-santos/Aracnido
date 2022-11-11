/*
ESP32-CAM Remote Control 
*/

const char* ssid = "TI-UTXJ";
const char* password = "TECNOLOGIAS123";

#include "esp_wifi.h"
#include "esp_camera.h"
#include <WiFi.h>
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"

#define CAMERA_MODEL_AI_THINKER

#if defined(CAMERA_MODEL_AI_THINKER)
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27

#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

#else
#error "Camera model not selected"
#endif

void startCameraServer();

WiFiServer server(85);

void setup() 
{
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); // prevent brownouts by silencing them
  
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println();
    pinMode(12, OUTPUT);      
    pinMode(13, OUTPUT);
    pinMode(14, OUTPUT);
    pinMode(15, OUTPUT);
    delay(10);
    Serial.println();
    Serial.println();
    Serial.print("Conectando a ");
    Serial.println(ssid);
 
    WiFi.begin(ssid, password);
 
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
 
    Serial.println("");
    Serial.println("Conectado a red Wifi.");
    Serial.println("Dirección IP ");
    Serial.println(WiFi.localIP());
    
    server.begin();
 
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  //init with high specs to pre-allocate larger buffers
  if(psramFound()){
    config.frame_size = FRAMESIZE_QVGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_QVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }

  // camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Error en la Camara 0x%x", err);
    return;
  }

  //drop down frame size for higher initial frame rate
  sensor_t * s = esp_camera_sensor_get();
  s->set_framesize(s, FRAMESIZE_QVGA);
  s->set_vflip(s, 1);
  s->set_hmirror(s, 1);

  
  ledcSetup(7, 5000, 8);
  ledcAttachPin(4, 7);  //pin4 is LED
  
  Serial.println("ssid: " + (String)ssid);
  Serial.println("password: " + (String)password);
  
  WiFi.begin(ssid, password);
  delay(500);

  long int StartTime=millis();
  while (WiFi.status() != WL_CONNECTED) 
  {
      delay(500);
      if ((StartTime+10000) < millis()) break;
  } 

  /*
  int8_t power;
  esp_wifi_set_max_tx_power(20);
  esp_wifi_get_max_tx_power(&power);
  Serial.printf("wifi power: %d \n",power); 
  */
  
  startCameraServer();

  if (WiFi.status() == WL_CONNECTED) 
  {
    Serial.println("");
    Serial.println("WiFi connected");    
    Serial.print("Camera Ready! Use 'http://");
    Serial.print(WiFi.localIP());
    Serial.println("' to connect");
  } else {
    Serial.println("");
    Serial.println("WiFi disconnected");      
    Serial.print("Camera Ready! Use 'http://");
    Serial.print(WiFi.softAPIP());
    Serial.println("' to connect");
    char* apssid = "TI-UTXJ";
    char* appassword = "TECNOLOGIAS123";         //AP password require at least 8 characters.
    WiFi.softAP((WiFi.softAPIP().toString()+"_"+(String)apssid).c_str(), appassword);    
  }

  for (int i=0;i<5;i++) 
  {
    ledcWrite(7,10);  // flash led
    delay(200);
    ledcWrite(7,0);
    delay(200);    
  }       
}

void loop() {
     WiFiClient client = server.available();   // Escuchando a los clientes entrantes
 
  if (client) {                             // Si hay un cliente,
    Serial.println("Nuevo cliente");        // Imprime un mensaje en el puerto serie
    String currentLine = "";                // String para contener datos entrantes del cliente
    while (client.connected()) {            // Bucle mientras el cliente está conectado
      if (client.available()) {             // Si hay bytes para leer del cliente,
        char c = client.read();             // Lee un caracter
        Serial.write(c);                    // Lo imprimimos en el monitor serial
        if (c == '\n') {                    // Si el byte es un carácter de nueva línea
 
          
          if (currentLine.length() == 0) {
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println();
 
            // Contenido HTML
            client.println("<style>button{margin:5px;padding:0 12px;border:0;line-height:28px;cursor:pointer;color:#fff;background:#B3D353;border-radius:5px;font-size:16px;outline:0}button:hover{background:#0174C5}button:active{background:#0174C5}iframe{border:none;}</style>");
            client.print("<body bgcolor=\"#FFFFFF\" link=\"#FFFFFF\" alink=\"#FFFFFFFF\" vlink=\"#FFFFFF\"><center><iframe src=\"http://");
            client.print(WiFi.localIP());
            client.print("\" width=\"600px\" height=\"500px\" scrolling=\"auto\"></iframe></center>");
            client.print("<center><table><tr><td></td><td><a href=\"/E12\"><button name=\"button\">Avanzar</button></a></td><td></td></tr>");
            client.print("<tr><td><a href=\"/E14\"><button name=\"button\">Izquierda</button></a></td><td><a href=\"/E11\"><button name=\"button\">Parar</button></a></td><td><a href=\"/E15\"><button name=\"button\">Derecha</button></a></td></tr>"); 
            client.print("<tr><td></td><td><a href=\"/E13\"><button name=\"button\">Retroceder</button></a></td><td></td></tr></table></center></body>");
            client.print("<img src=\"https://lh3.googleusercontent.com/proxy/EuVcq_HOaYLmQAMZMK8A_AJmGu9qKAO8VILBrZ8y0E-80Em48POIOLwOpjwUfwuIs0T9j-yNeom0dRAjHU5bduxu9C1dbNZOSIYiZDrdE_-LoELC7-9-mbk5_2I\"/>");    
 
            
            client.println();
            // Salir del ciclo while:
            break;
          } else {    // si tienes una nueva línea, borra currentLine:
            currentLine = "";
          }
        } else if (c != '\r') {  
          currentLine += c;      
        }
 
        // Verifica si la solicitud del cliente fue "GET /E" de encendido o "GET /A de apagado":
       if (currentLine.endsWith("GET /E11")) {
          digitalWrite(14, LOW); 
          digitalWrite(15, LOW); 
          digitalWrite(12, LOW);
          digitalWrite(13, LOW);                 
        }
        if (currentLine.endsWith("GET /E12")) {
          digitalWrite(14, HIGH); 
          digitalWrite(15, LOW); 
          digitalWrite(12, HIGH);
          digitalWrite(13, LOW); 
          
        }
        if (currentLine.endsWith("GET /E13")) {
          digitalWrite(14, LOW); 
          digitalWrite(15, HIGH); 
          digitalWrite(12, LOW);
          digitalWrite(13, HIGH);             
        }
        //-----------------------
        if (currentLine.endsWith("GET /E14")) {
          digitalWrite(14, HIGH); 
          digitalWrite(15, LOW); 
          digitalWrite(12, LOW);
          digitalWrite(13, HIGH);      
        }
        if (currentLine.endsWith("GET /E15")) {
          digitalWrite(14, LOW); 
          digitalWrite(15, HIGH);
          digitalWrite(12, HIGH);
          digitalWrite(13, LOW);                
        }
      }
    }
    // Cierra la conexión
    client.stop();
    Serial.println("Cliente desconectado");
  }
  // put your main code here, to run repeatedly:
  delay(1000);
  //Serial.printf("RSSi: %ld dBm\n",WiFi.RSSI()); 
}
