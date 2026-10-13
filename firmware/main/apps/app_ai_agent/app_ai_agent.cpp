/*
 * SPDX-FileCopyrightText: 2026 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
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

bool GeminiClient::begin() {
    mclog::tagInfo("GeminiClient", "Starting Gemini Live connection...");
    // Network and WebSocket setup will go here
    return true;
}

void GeminiClient::update() {
    // Handle real-time audio streams here
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
}

// Called when the App is closed
// You can destroy UI, release resources, etc. here
void AppAiAgent::onClose()
{
    mclog::tagInfo(getAppInfo().name, "on close");
}
