/*
 * SPDX-FileCopyrightText: 2026 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
#include <esp_websocket_client.h>
#include <driver/i2s.h>
#include "app_ai_agent.h"
#include <hal/hal.h>
#include <mooncake.h>
#include <mooncake_log.h>
#include <assets/assets.h>
#include <smooth_lvgl.hpp>
#include <stackchan/stackchan.h>
#include <apps/common/common.h>

using namespace mooncake;
using namespace smooth_ui_toolkit::lvgl_cpp;
GeminiClient::GeminiClient() {
    // Initialize Gemini client variables here
}
static void websocket_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
    esp_websocket_event_data_t *data = (esp_websocket_event_data_t *)event_data;
    GeminiClient *client = (GeminiClient *)handler_args;
    switch (event_id) {
        case WEBSOCKET_EVENT_CONNECTED:
            mclog::tagInfo("GeminiClient", "WebSocket connected");
            break;
        case WEBSOCKET_EVENT_DATA:
            if (data->op_code == WS_OPCODE_BINARY) {
                // Play audio from Gemini
                i2s_write(I2S_NUM_0, data->data_ptr, data->data_len, NULL, portMAX_DELAY);
            }
            break;
        case WEBSOCKET_EVENT_DISCONNECTED:
            mclog::tagInfo("GeminiClient", "WebSocket disconnected");
            break;
    }
}
bool GeminiClient::begin() {
    mclog::tagInfo("GeminiClient", "Starting Gemini Live connection...");
    i2s_config_t i2s_config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX | I2S_MODE_RX),
    .sample_rate = 16000,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
    .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
    .communication_format = I2S_COMM_FORMAT_STAND_I2S,
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 8,
    .dma_buf_len = 512,
    .use_apll = false,
};
i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
    i2s_pin_config_t pin_config = {
    .bck_io_num = 34,
    .ws_io_num = 33,
    .data_out_num = 13,
    .data_in_num = 14
};
i2s_set_pin(I2S_NUM_0, &pin_config);
    FILE *f = fopen("/sdcard/api_key.txt", "r");
if (f == NULL) {
    mclog::tagError("GeminiClient", "Failed to open API key file");
} else {
    char key[128];
    fgets(key, sizeof(key), f);
    key[strcspn(key, "\r\n")] = 0;
    
    char uri[512];
    snprintf(uri, sizeof(uri), "wss://generativelanguage.googleapis.com/ws/google.ai.generativelanguage.v1beta.GenerativeService.BidiGenerateContent?key=%s", key);
    
    esp_websocket_client_config_t websocket_cfg = {};
    websocket_cfg.uri = uri;
    
    _client = esp_websocket_client_init(&websocket_cfg);
    esp_websocket_register_events(_client, WEBSOCKET_EVENT_ANY, websocket_event_handler, (void *)this);
    esp_websocket_client_start(_client);
    fclose(f);
    return true;
}
// Add any required headers or authentication tokens here

void GeminiClient::update()
{
    // Read audio from I2S microphone
    int8_t mic_buffer[512];
    size_t bytes_read = 0;
    i2s_read(I2S_NUM_0, mic_buffer, sizeof(mic_buffer), &bytes_read, pdMS_TO_TICKS(10));
    
    if (bytes_read > 0) {
        // Send the recorded audio over the WebSocket
        esp_websocket_client_send_bin(_client, (const char*)mic_buffer, bytes_read);
    }

    // Note: To play audio from Gemini, you would write 
    // the received audio data from the websocket event 
    // handler to I2S using i2s_write().
}
}
AppAiAgent::AppAiAgent()
{
    // Configure App name
    setAppInfo().name = "AI.AGENT";
    // Configure App icon
    static auto icon  = assets::get_image("icon_ai_agent.bin");
    setAppInfo().icon = (void*)&icon;
    // Configure App theme color
    static uint32_t theme_color = 0x33CC99;
    setAppInfo().userData       = (void*)&theme_color;
}

// Called when the App is installed
void AppAiAgent::onCreate()
{
    mclog::tagInfo(getAppInfo().name, "on create");
}

// Called when the App is opened
// You can construct UI, initialize operations, etc. here
void AppAiAgent::onOpen()
{
    mclog::tagInfo(getAppInfo().name, "on open");

    // Request to start Xiaozhi service
    // All apps will be uninstall in next mooncake update
    // GetHAL().requestXiaozhiStart();
    wifi_ap_record_t ap_info;
if (esp_wifi_sta_get_ap_info(&ap_info) == ESP_OK) {
    mclog::tagInfo(getAppInfo().name, "Wi-Fi is connected.");
    _gemini_client.begin();
} else {
    mclog::tagError(getAppInfo().name, "Wi-Fi is not connected!");
}
}

// Called repeatedly while the App is running
void AppAiAgent::onRunning()
{
    _gemini_client.update();
}

// Called when the App is closed
// You can destroy UI, release resources, etc. here
void AppAiAgent::onClose()
{
    mclog::tagInfo(getAppInfo().name, "on close");
}
