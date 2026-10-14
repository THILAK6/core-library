#pragma once

#if defined(ESP32)

#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoOTA.h>
#include <Update.h>
#include <Preferences.h>
#include <functional>
#include <vector>

#include "Menu/MenuItem.h"
#include "Menu/Menu.h"
#include "Menu/MenuManager.h"

/**
 * @brief WiFi, Web Portal, REST API, and OTA Manager for ESP32.
 * 
 * Provides:
 * - Station (STA) and Access Point (SoftAP) management with static IP or DHCP
 * - Dynamic REST API (/api/items, /api/item, /api/item/set, /api/item/reset)
 *   exposing all MenuItem parameters to other connected devices
 * - Responsive Web Portal for viewing and remotely updating parameters
 * - In-browser Web OTA firmware upgrade (/ota) with progress reporting
 * - ArduinoOTA support for LAN updates via PlatformIO / Arduino IDE
 * - Dedicated FreeRTOS background task pinned to Core 1
 */
class WiFiManager
{
public:
    WiFiManager(Preferences &preferences, uint32_t deviceId = 0);
    ~WiFiManager();

    /**
     * @brief Initialize WiFi, Web Server, OTA, and background task.
     * @param enableWiFi True to start WiFi (AP + STA), false to keep WiFi off.
     */
    void begin(bool enableWiFi = true);

    /**
     * @brief Periodic handler. Call in loop() if not using background task for OTA maintenance.
     */
    void handle();

    /**
     * @brief Stop WiFi and all associated networking tasks and servers.
     */
    void stop();

    /**
     * @brief Bind MenuManager so all MenuItem parameters are exposed via Web & REST API.
     */
    void bindMenuManager(MenuManager *manager);

    /**
     * @brief Bind menus directly if not using MenuManager.
     */
    void bindMenus(const std::vector<Menu *> &viewMenus, const std::vector<Menu *> &editMenus, std::function<void()> onSave = nullptr);

    /**
     * @brief Check if connected to an external WiFi Access Point as Station.
     */
    bool isConnected() const;

    /**
     * @brief Get Station IP address as string.
     */
    String getLocalIP() const;

    /**
     * @brief Get SoftAP IP address as string.
     */
    String getAPIP() const;

    /**
     * @brief Get connected or configured SSID.
     */
    String getSSID() const;

    /**
     * @brief Get WiFi signal strength (RSSI) in dBm.
     */
    int getRSSI() const;

    /**
     * @brief Set device / machine ID used in SoftAP SSID and web portal.
     */
    void setDeviceId(uint32_t id);

    /**
     * @brief Get internal AsyncWebServer instance to attach custom routes.
     */
    AsyncWebServer &getServer();

    /**
     * @brief Generate JSON string of all registered MenuItems.
     */
    String generateMenuJSON();

    /**
     * @brief Generate JSON representation of a single MenuItem.
     */
    String generateSingleItemJSON(MenuItem *item, int id, bool editable);

private:
    Preferences &preferences;
    uint32_t deviceId;
    bool wifiEnabled;
    volatile bool connected;

    // Bound Menus for Web / API exposure
    std::vector<Menu *> boundViewMenus;
    std::vector<Menu *> boundEditMenus;
    std::function<void()> onMenuSaveCallback;

    MenuItem *findMenuItem(const String &identifier, bool &isEditableOut, int &foundIdOut) const;

    // Credentials & Network Config
    String ssid;
    String wifiPassword;
    String staticIPStr;
    IPAddress localIP;
    IPAddress gateway;
    IPAddress subnet;
    IPAddress dns1;
    IPAddress dns2;
    IPAddress apIP;

    // Web Server
    AsyncWebServer server;

    // FreeRTOS Task
    TaskHandle_t wifiTaskHandle;
    static void wifiTaskTrampoline(void *param);
    void runWifiTask();

    // Internal Lifecycle
    void loadCredentials();
    void saveCredentials(const String &newSsid, const String &newPass, const String &newIp);
    void startSoftAP();
    void connectSTA();
    void setupWebServer();
    void setupOTA();

    // HTML Generation
    String generatePortalHTML();
    String generateOTAHTML();
};

#endif // ESP32
