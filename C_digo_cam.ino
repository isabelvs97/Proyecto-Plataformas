//Code based on Arduino IDE Sketches examples
//Works with ESP32-CAM module
//Uploads photo to static IP and stores photo in SD card. Set cont for indicating how many times

#include "WiFi.h"
#include "ESPAsyncWebServer.h"
#include "esp_camera.h"
#include "FS.h"                // SD Card ESP32
#include "SD_MMC.h"            // SD Card ESP32
#include "soc/soc.h"           // Disable brownour problems
#include "soc/rtc_cntl_reg.h"  // Disable brownour problems
#include "driver/rtc_io.h"
#include <EEPROM.h>            // read and write from flash memory
#include <SD.h>
 
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

#define EEPROM_SIZE 1
 
const char* ssid = "xxxx";
const char* password = "xxxx";

File file;
int cont = 0;
int pictureNumber = 0;
 
AsyncWebServer server(80);
 
void setup() {
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); //disable brownout detector

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
  config.frame_size = FRAMESIZE_SVGA;
  config.jpeg_quality = 12;
  

   if(psramFound()){
    config.frame_size = FRAMESIZE_UXGA; // FRAMESIZE_ + QVGA|CIF|VGA|SVGA|XGA|SXGA|UXGA
    config.jpeg_quality = 12;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }
  
  // Inicializamos la cámara
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }
  
  //Serial.println("Starting SD Card");
  if(!SD_MMC.begin()){
    Serial.println("SD Card Mount Failed");
    return;
  }
  
  uint8_t cardType = SD_MMC.cardType();
  if(cardType == CARD_NONE){
    Serial.println("No SD Card attached");
    return;
  }

  WiFi.begin(ssid, password);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED)
   {
     delay(300);
     Serial.print(".");
     
   }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  //Se inicia el servidor
  server.begin();
 
}

void loop(){
  
  while(cont < 8){
   
    server.on("/photo", HTTP_GET, [](AsyncWebServerRequest * request) {
    camera_fb_t * photo = NULL;
    photo = esp_camera_fb_get();
    request->send_P(200, "image/jpeg", (const uint8_t *)photo->buf, photo->len);
    esp_camera_fb_return(photo);
    });

    delay(1000);
   
   //initialize EEPROM with predefined size
    camera_fb_t * photo = NULL;
    photo = esp_camera_fb_get();
    EEPROM.begin(EEPROM_SIZE);
    pictureNumber = EEPROM.read(0) + 1;
  
    // Nombre de la foto tomada
    String path = "/picture" + String(pictureNumber) +".jpg";

    fs::FS &fs = SD_MMC; 
    
    //Abrimos el archivo para guardar la foto
    File file = fs.open(path.c_str(), FILE_WRITE);
    if(!file){
    Serial.println("Failed to open file in writing mode");
      } 
    else {
    //Se guarda la imagen
    file.write(photo->buf, photo->len); 
    EEPROM.write(0, pictureNumber);
    EEPROM.commit();
     }
     
    file.close();
    esp_camera_fb_return(photo);
    delay(8000);
    cont++;  
  }
  
 }
