#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"

#include "esp_http_client.h"
#include "connect_wifi.h"
 #include "esp_crt_bundle.h"

static const char *TAG = "HTTP_CLIENT";

char api_key[] = "RQZXY61HSJURBPZL";

char message[] = "Hello This is a test message";
//https://wokwi.com/projects/381723926056620033



#define OK_BUTTON_PIN 25
#define ERROR_BUTTON_PIN 26


void doSend(){
    char twilio_url[200];
    snprintf(twilio_url,
             sizeof(twilio_url),
             "%s%s%s",
             "https://api.thingspeak.com/update?api_key=",
             api_key,
             "&field1=22&field2=66");

    esp_http_client_config_t config = {
        .url = twilio_url,
        .method = HTTP_METHOD_GET,  
        .transport_type = HTTP_TRANSPORT_OVER_SSL,  //Specify transport type
        .crt_bundle_attach = esp_crt_bundle_attach };

    esp_http_client_handle_t client = esp_http_client_init(&config);
     esp_http_client_set_header(client, "Content-Type", "application/x-www-form-urlencoded");



    esp_err_t err = esp_http_client_perform(client);

    if (err == ESP_OK)
    {
        int status_code = esp_http_client_get_status_code(client);
        if (status_code == 200)
        {
            ESP_LOGI(TAG, "Message sent Successfully");
        }
        else
        {
            ESP_LOGI(TAG, "Message sent Failed");
        }
    }
    else
    {
        ESP_LOGI(TAG, "Message sent Failed");
    }
    esp_http_client_cleanup(client);

}

void sendDataTask(void *pvParameters)
{
    connect_wifi();
    if(wifi_connect_status){
        while(1){
            doSend();
            vTaskDelay(1000*30 / portTICK_PERIOD_MS);
            ESP_LOGI(TAG, "Next send");
        }
    }
}


void errorTriggerTask(){
    gpio_reset_pin(ERROR_BUTTON_PIN);
    gpio_set_pull_mode(ERROR_BUTTON_PIN, GPIO_PULLUP_ONLY);
    int currentValue = gpio_get_level(ERROR_BUTTON_PIN);
    int lastValue = 0;

     while(1){
        lastValue = currentValue;
        //Check button
        currentValue = gpio_get_level(ERROR_BUTTON_PIN);
        //ESP_LOGE("main","Press %d", currentValue);
        if(lastValue == 1 && currentValue == 0){
            vTaskDelay(20 / portTICK_PERIOD_MS);
            currentValue = gpio_get_level(ERROR_BUTTON_PIN);
            if(currentValue == 0){
                //
                esp_wifi_stop();
            }
        }
     }
}



void app_main(void)
{
	esp_err_t ret = nvs_flash_init();
	if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
	{
		ESP_ERROR_CHECK(nvs_flash_erase());
		ret = nvs_flash_init();
	}
	ESP_ERROR_CHECK(ret);
    xTaskCreate(sendDataTask, "sendDataTask", 8192, NULL, 6, NULL);
    xTaskCreate(errorTriggerTask, "errorTriggerTask", 8192, NULL, 6, NULL);
    while(1){
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
