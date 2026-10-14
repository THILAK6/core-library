#if defined(ESP32)

#include "WiFi/WiFiManager.h"

WiFiManager::WiFiManager(Preferences &preferences, uint32_t deviceId)
    : preferences(preferences),
      deviceId(deviceId),
      wifiEnabled(false),
      connected(false),
      onMenuSaveCallback(nullptr),
      localIP(192, 168, 1, 200),
      gateway(192, 168, 1, 1),
      subnet(255, 255, 255, 0),
      dns1(8, 8, 8, 8),
      dns2(8, 8, 4, 4),
      apIP(192, 168, 4, 1),
      server(80),
      wifiTaskHandle(nullptr)
{
}

WiFiManager::~WiFiManager()
{
    stop();
}

void WiFiManager::setDeviceId(uint32_t id)
{
    this->deviceId = id;
}

void WiFiManager::bindMenuManager(MenuManager *manager)
{
    if (manager)
    {
        this->boundViewMenus = manager->getViewMenus();
        this->boundEditMenus = manager->getEditMenus();
        this->onMenuSaveCallback = manager->getSaveCallback();
    }
}

void WiFiManager::bindMenus(const std::vector<Menu *> &viewMenus, const std::vector<Menu *> &editMenus, std::function<void()> onSave)
{
    this->boundViewMenus = viewMenus;
    this->boundEditMenus = editMenus;
    this->onMenuSaveCallback = onSave;
}

AsyncWebServer &WiFiManager::getServer()
{
    return this->server;
}

bool WiFiManager::isConnected() const
{
    return connected && (WiFi.status() == WL_CONNECTED);
}

String WiFiManager::getLocalIP() const
{
    if (isConnected())
    {
        return WiFi.localIP().toString();
    }
    return "Not Connected";
}

String WiFiManager::getAPIP() const
{
    return WiFi.softAPIP().toString();
}

String WiFiManager::getSSID() const
{
    return this->ssid;
}

int WiFiManager::getRSSI() const
{
    if (isConnected())
    {
        return WiFi.RSSI();
    }
    return 0;
}

void WiFiManager::loadCredentials()
{
    ssid = preferences.getString("ssid", "Don't connect_2.4G");
    wifiPassword = preferences.getString("wifipass", "qsxnkp610");
    staticIPStr = preferences.getString("ip", "192.168.1.200");
    if (staticIPStr == "192.168.0.200")
    {
        staticIPStr = "192.168.1.200";
        preferences.putString("ip", staticIPStr);
    }

    if (!localIP.fromString(staticIPStr))
    {
        localIP = IPAddress(192, 168, 1, 200);
    }
}

void WiFiManager::saveCredentials(const String &newSsid, const String &newPass, const String &newIp)
{
    ssid = newSsid;
    wifiPassword = newPass;
    staticIPStr = newIp;

    preferences.putString("ssid", ssid);
    preferences.putString("wifipass", wifiPassword);
    preferences.putString("ip", staticIPStr);

    localIP.fromString(staticIPStr);
    Serial.println("[WiFiManager] Credentials updated in Preferences");
}

void WiFiManager::startSoftAP()
{
    char softAP_ssid[48];
    if (deviceId > 0)
    {
        snprintf(softAP_ssid, sizeof(softAP_ssid), "TK_TECH_DIGITAL_COUNTER_%06X", deviceId);
    }
    else
    {
        snprintf(softAP_ssid, sizeof(softAP_ssid), "TK_TECH_DEVICE_%06X", (uint32_t)(ESP.getEfuseMac() & 0xFFFFFF));
    }

    IPAddress apGateway(192, 168, 4, 1);
    IPAddress apSubnet(255, 255, 255, 0);

    WiFi.softAPConfig(apIP, apGateway, apSubnet);
    WiFi.softAP(softAP_ssid, "12345678");
    Serial.printf("[WiFiManager] SoftAP started: %s (IP: %s)\n", softAP_ssid, WiFi.softAPIP().toString().c_str());
}

void WiFiManager::connectSTA()
{
    if (ssid.length() == 0)
    {
        Serial.println("[WiFiManager] No STA SSID specified.");
        connected = false;
        return;
    }

    Serial.printf("[WiFiManager] Connecting to STA SSID: %s\n", ssid.c_str());
    WiFi.disconnect();

    if (localIP != IPAddress(0, 0, 0, 0) && staticIPStr != "0.0.0.0" && staticIPStr != "dhcp")
    {
        // Only apply static IP if it belongs to the same /24 subnet as the router gateway
        if ((localIP[0] == gateway[0]) && (localIP[1] == gateway[1]) && (localIP[2] == gateway[2]))
        {
            if (!WiFi.config(localIP, gateway, subnet, dns1, dns2))
            {
                Serial.println("[WiFiManager] STA static IP configuration failed, using DHCP");
            }
        }
        else
        {
            Serial.printf("[WiFiManager] Static IP %s not in gateway %s subnet, falling back to DHCP\n",
                          staticIPStr.c_str(), gateway.toString().c_str());
        }
    }

    WiFi.begin(ssid.c_str(), wifiPassword.c_str());

    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 25)
    {
        delay(100);
        attempts++;
    }

    if (WiFi.status() == WL_CONNECTED)
    {
        connected = true;
        Serial.printf("[WiFiManager] Connected to %s! IP: %s (RSSI: %d dBm)\n",
                      ssid.c_str(), WiFi.localIP().toString().c_str(), WiFi.RSSI());
    }
    else
    {
        connected = false;
        Serial.println("[WiFiManager] Station connection timeout. Running in SoftAP mode.");
    }
}

void WiFiManager::begin(bool enableWiFi)
{
    this->wifiEnabled = enableWiFi;
    if (!wifiEnabled)
    {
        Serial.println("[WiFiManager] WiFi disabled by configuration.");
        WiFi.mode(WIFI_OFF);
        return;
    }

    loadCredentials();

    WiFi.mode(WIFI_AP_STA);
    startSoftAP();
    connectSTA();

    setupWebServer();
    setupOTA();

    server.begin();
    Serial.println("[WiFiManager] HTTP Web Server started on port 80");

    if (wifiTaskHandle == nullptr)
    {
        xTaskCreatePinnedToCore(wifiTaskTrampoline, "wifiMgrTask", 8192, this, 1, &wifiTaskHandle, 1);
    }
}

void WiFiManager::stop()
{
    if (wifiTaskHandle != nullptr)
    {
        vTaskDelete(wifiTaskHandle);
        wifiTaskHandle = nullptr;
    }
    server.end();
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
    connected = false;
    wifiEnabled = false;
    Serial.println("[WiFiManager] Stopped.");
}

void WiFiManager::handle()
{
    if (wifiEnabled)
    {
        ArduinoOTA.handle();
    }
}

MenuItem *WiFiManager::findMenuItem(const String &identifier, bool &isEditableOut, int &foundIdOut) const
{
    String needle = identifier;
    needle.trim();
    needle.toLowerCase();

    bool isNumeric = (needle.length() > 0);
    for (size_t c = 0; c < needle.length(); ++c)
    {
        if (!isDigit(needle.charAt(c)))
        {
            isNumeric = false;
            break;
        }
    }

    int targetIndex = isNumeric ? needle.toInt() : -1;
    int currentIndex = 0;

    // Search View Menus
    for (Menu *menu : boundViewMenus)
    {
        if (!menu) continue;
        for (MenuItem *item : menu->getMenuItems())
        {
            if (!item) continue;
            String sn = item->getShortName();
            sn.trim();
            sn.toLowerCase();

            String n = item->getName();
            n.trim();
            n.toLowerCase();

            if ((isNumeric && currentIndex == targetIndex) || (sn.length() > 0 && sn == needle) || (n.length() > 0 && n == needle))
            {
                isEditableOut = false;
                foundIdOut = currentIndex;
                return item;
            }
            currentIndex++;
        }
    }

    // Search Edit Menus
    for (Menu *menu : boundEditMenus)
    {
        if (!menu) continue;
        for (MenuItem *item : menu->getMenuItems())
        {
            if (!item) continue;
            String sn = item->getShortName();
            sn.trim();
            sn.toLowerCase();

            String n = item->getName();
            n.trim();
            n.toLowerCase();

            if ((isNumeric && currentIndex == targetIndex) || (sn.length() > 0 && sn == needle) || (n.length() > 0 && n == needle))
            {
                isEditableOut = true;
                foundIdOut = currentIndex;
                return item;
            }
            currentIndex++;
        }
    }

    foundIdOut = -1;
    return nullptr;
}

static String escapeJSONString(const String &input)
{
    String output = "";
    output.reserve(input.length() + 8);
    for (size_t i = 0; i < input.length(); ++i)
    {
        char c = input.charAt(i);
        if (c == '"') output += "\\\"";
        else if (c == '\\') output += "\\\\";
        else if (c == '\b') output += "\\b";
        else if (c == '\f') output += "\\f";
        else if (c == '\n') output += "\\n";
        else if (c == '\r') output += "\\r";
        else if (c == '\t') output += "\\t";
        else if (c < 32)
        {
            // omit non-printable control characters
        }
        else
        {
            output += c;
        }
    }
    return output;
}

String WiFiManager::generateSingleItemJSON(MenuItem *item, int id, bool editable)
{
    if (!item) return "{}";

    String json = "{";
    json += "\"id\":" + String(id) + ",";
    json += "\"group\":\"" + escapeJSONString(item->getMainName()) + "\",";
    json += "\"name\":\"" + escapeJSONString(item->getName()) + "\",";

    String shortName = item->getShortName();
    shortName.trim();
    json += "\"shortName\":\"" + escapeJSONString(shortName) + "\",";
    json += "\"editable\":" + String(editable ? "true" : "false") + ",";
    json += "\"resettable\":" + String(item->hasReset() ? "true" : "false") + ",";

    if (item->getDatatype() == MenuType::ValueType)
    {
        json += "\"type\":\"value\",";
        float val = item->value();
        if (isnan(val) || isinf(val))
        {
            val = 0.0f;
        }
        json += "\"value\":" + String(val, 3);
    }
    else
    {
        json += "\"type\":\"mode\",";
        std::vector<String> modes = item->modes();
        int curMode = item->currentMode();
        if (curMode < 0 || (!modes.empty() && curMode >= (int)modes.size()))
        {
            curMode = 0;
        }
        json += "\"currentMode\":" + String(curMode) + ",";
        String modeName = item->currentModeValue();
        modeName.trim();
        json += "\"currentModeName\":\"" + escapeJSONString(modeName) + "\",";
        json += "\"modes\":[";
        for (size_t i = 0; i < modes.size(); ++i)
        {
            if (i > 0) json += ",";
            String m = modes[i];
            m.trim();
            json += "\"" + escapeJSONString(m) + "\"";
        }
        json += "]";
    }
    json += "}";
    return json;
}

String WiFiManager::generateMenuJSON()
{
    String json = "[";
    json.reserve(4096);
    int id = 0;

    // View Menus
    for (Menu *menu : boundViewMenus)
    {
        if (!menu) continue;
        for (MenuItem *item : menu->getMenuItems())
        {
            if (!item) continue;
            if (id > 0) json += ",";
            json += generateSingleItemJSON(item, id++, false);
        }
    }

    // Edit Menus
    for (Menu *menu : boundEditMenus)
    {
        if (!menu) continue;
        for (MenuItem *item : menu->getMenuItems())
        {
            if (!item) continue;
            if (id > 0) json += ",";
            json += generateSingleItemJSON(item, id++, true);
        }
    }

    json += "]";
    return json;
}

void WiFiManager::setupOTA()
{
    char hostname[32];
    snprintf(hostname, sizeof(hostname), "tk-counter-%06X", deviceId);
    ArduinoOTA.setHostname(hostname);

    ArduinoOTA.onStart([]() {
        Serial.println("[ArduinoOTA] Start firmware update");
    });
    ArduinoOTA.onEnd([]() {
        Serial.println("\n[ArduinoOTA] Update completed successfully! Rebooting...");
    });
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
        Serial.printf("[ArduinoOTA] Progress: %u%%\r", (progress / (total / 100)));
    });
    ArduinoOTA.onError([](ota_error_t error) {
        Serial.printf("[ArduinoOTA] Error[%u]: ", error);
        if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
        else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
        else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
        else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
        else if (error == OTA_END_ERROR) Serial.println("End Failed");
    });
    ArduinoOTA.begin();

    server.on("/ota", HTTP_GET, [this](AsyncWebServerRequest *request) {
        request->send(200, "text/html", this->generateOTAHTML());
    });

    server.on("/ota", HTTP_POST, [](AsyncWebServerRequest *request) {
        bool shouldReboot = !Update.hasError();
        AsyncWebServerResponse *response = request->beginResponse(200, "text/plain", shouldReboot ? "OK" : "FAIL");
        response->addHeader("Connection", "close");
        request->send(response);
        if (shouldReboot)
        {
            delay(200);
            ESP.restart();
        }
    }, [](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
        if (!index)
        {
            Serial.printf("[WebOTA] Update Start: %s\n", filename.c_str());
            if (!Update.begin(UPDATE_SIZE_UNKNOWN))
            {
                Update.printError(Serial);
            }
        }
        if (!Update.hasError())
        {
            if (Update.write(data, len) != len)
            {
                Update.printError(Serial);
            }
        }
        if (final)
        {
            if (Update.end(true))
            {
                Serial.printf("[WebOTA] Update Success: %u bytes\n", (uint32_t)(index + len));
            }
            else
            {
                Update.printError(Serial);
            }
        }
    });
}

void WiFiManager::setupWebServer()
{
    // Home Configuration & Parameters Portal
    server.on("/", HTTP_GET, [this](AsyncWebServerRequest *request) {
        request->send(200, "text/html", this->generatePortalHTML());
    });

    // WiFi Configuration Update Handler
    server.on("/update", HTTP_POST, [this](AsyncWebServerRequest *request) {
        String new_ssid = request->arg("ssid");
        String new_pass = request->arg("password");
        String new_ip = request->arg("ip");

        IPAddress testIp;
        if (!testIp.fromString(new_ip))
        {
            request->send(400, "text/plain", "Invalid IP Address format");
            return;
        }

        this->saveCredentials(new_ssid, new_pass, new_ip);
        request->send(200, "text/html", "<h3>Settings saved! Reconnecting WiFi...</h3><a href='/'>Return to portal</a>");

        delay(100);
        this->connectSTA();
    });

    // Device Reboot Endpoint
    server.on("/reboot", HTTP_POST, [](AsyncWebServerRequest *request) {
        request->send(200, "text/plain", "Rebooting...");
        delay(200);
        ESP.restart();
    });

    // ----------------------------------------------------
    // REST API Endpoints for MenuItems & Connected Devices
    // ----------------------------------------------------

    // 1. GET /api/items (or /api/menu) - Retrieve all registered parameters
    auto handleGetItems = [this](AsyncWebServerRequest *request) {
        AsyncWebServerResponse *response = request->beginResponse(200, "application/json", this->generateMenuJSON());
        response->addHeader("Access-Control-Allow-Origin", "*");
        response->addHeader("Cache-Control", "no-cache, no-store, must-revalidate");
        request->send(response);
    };
    server.on("/api/items", HTTP_GET, handleGetItems);
    server.on("/api/menu", HTTP_GET, handleGetItems);

    // 2. GET /api/item - Retrieve single parameter by id or name
    server.on("/api/item", HTTP_GET, [this](AsyncWebServerRequest *request) {
        String target = "";
        if (request->hasArg("id")) target = request->arg("id");
        else if (request->hasArg("name")) target = request->arg("name");
        else if (request->hasArg("short")) target = request->arg("short");

        bool isEditable = false;
        int foundId = -1;
        MenuItem *item = this->findMenuItem(target, isEditable, foundId);
        if (!item)
        {
            AsyncWebServerResponse *response = request->beginResponse(404, "application/json", "{\"error\":\"Item not found\"}");
            response->addHeader("Access-Control-Allow-Origin", "*");
            request->send(response);
            return;
        }

        AsyncWebServerResponse *response = request->beginResponse(200, "application/json", this->generateSingleItemJSON(item, foundId, isEditable));
        response->addHeader("Access-Control-Allow-Origin", "*");
        request->send(response);
    });

    // 3. POST /api/item/set - Update value or mode of a parameter
    server.on("/api/item/set", HTTP_POST, [this](AsyncWebServerRequest *request) {
        String target = "";
        if (request->hasArg("id")) target = request->arg("id");
        else if (request->hasArg("name")) target = request->arg("name");
        else if (request->hasArg("short")) target = request->arg("short");

        bool isEditable = false;
        int foundId = -1;
        MenuItem *item = this->findMenuItem(target, isEditable, foundId);
        if (!item)
        {
            request->send(404, "application/json", "{\"error\":\"Item not found\"}");
            return;
        }
        if (!isEditable)
        {
            request->send(403, "application/json", "{\"error\":\"Item is read-only\"}");
            return;
        }

        if (item->getDatatype() == MenuType::ValueType && request->hasArg("value"))
        {
            float newVal = request->arg("value").toFloat();
            item->updateValue(newVal);
            if (this->onMenuSaveCallback)
            {
                this->onMenuSaveCallback();
            }
            request->send(200, "application/json", "{\"success\":true,\"message\":\"Value updated\"}");
        }
        else if (item->getDatatype() == MenuType::ModeType && request->hasArg("mode"))
        {
            int newMode = request->arg("mode").toInt();
            item->updateMode(newMode);
            if (this->onMenuSaveCallback)
            {
                this->onMenuSaveCallback();
            }
            request->send(200, "application/json", "{\"success\":true,\"message\":\"Mode updated\"}");
        }
        else
        {
            request->send(400, "application/json", "{\"error\":\"Missing 'value' or 'mode' argument\"}");
        }
    });

    // 4. POST /api/item/reset - Trigger reset on a resettable parameter
    server.on("/api/item/reset", HTTP_POST, [this](AsyncWebServerRequest *request) {
        String target = "";
        if (request->hasArg("id")) target = request->arg("id");
        else if (request->hasArg("name")) target = request->arg("name");
        else if (request->hasArg("short")) target = request->arg("short");

        bool isEditable = false;
        int foundId = -1;
        MenuItem *item = this->findMenuItem(target, isEditable, foundId);
        if (!item)
        {
            request->send(404, "application/json", "{\"error\":\"Item not found\"}");
            return;
        }
        if (!item->hasReset())
        {
            request->send(400, "application/json", "{\"error\":\"Item is not resettable\"}");
            return;
        }

        item->triggerReset();
        request->send(200, "application/json", "{\"success\":true,\"message\":\"Reset triggered\"}");
    });
}

void WiFiManager::wifiTaskTrampoline(void *param)
{
    WiFiManager *mgr = static_cast<WiFiManager *>(param);
    mgr->runWifiTask();
}

void WiFiManager::runWifiTask()
{
    uint32_t lastReconnectCheck = 0;

    for (;;)
    {
        uint32_t now = millis();

        ArduinoOTA.handle();

        if (now - lastReconnectCheck >= 5000)
        {
            lastReconnectCheck = now;
            connected = (WiFi.status() == WL_CONNECTED);
        }

        vTaskDelay(20 / portTICK_PERIOD_MS);
    }
}

String WiFiManager::generatePortalHTML()
{
    String html = R"rawliteral(<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>TK TECH | Device Portal</title>
<style>
  :root { --bg: #0f172a; --card: #1e293b; --accent: #0284c7; --accent-hover: #0369a1; --text: #f8fafc; --muted: #94a3b8; --border: #334155; --success: #34d399; --danger: #ef4444; }
  * { box-sizing: border-box; margin: 0; padding: 0; font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif; }
  body { background: var(--bg); color: var(--text); padding: 16px; display: flex; justify-content: center; }
  .container { width: 100%; max-width: 600px; }
  .card { background: var(--card); border: 1px solid var(--border); border-radius: 12px; padding: 20px; margin-bottom: 16px; box-shadow: 0 4px 6px -1px rgba(0,0,0,0.2); }
  h1 { font-size: 1.3rem; font-weight: 700; margin-bottom: 4px; color: #38bdf8; }
  h2 { font-size: 1.1rem; font-weight: 600; margin-bottom: 12px; color: #f1f5f9; display: flex; justify-content: space-between; align-items: center; }
  p.sub { font-size: 0.8rem; color: var(--muted); margin-bottom: 16px; }
  .badge { display: inline-block; padding: 3px 8px; border-radius: 9999px; font-size: 0.72rem; font-weight: 600; }
  .badge.online { background: #065f46; color: #34d399; }
  .badge.offline { background: #7f1d1d; color: #f87171; }
  .stat-grid { display: grid; grid-template-columns: 1fr 1fr; gap: 8px; margin-bottom: 16px; }
  .stat-box { background: rgba(15,23,42,0.6); padding: 8px 12px; border-radius: 8px; border: 1px solid var(--border); }
  .stat-label { font-size: 0.68rem; color: var(--muted); text-transform: uppercase; letter-spacing: 0.05em; }
  .stat-val { font-size: 0.9rem; font-weight: 600; margin-top: 2px; }
  label { display: block; font-size: 0.82rem; font-weight: 500; margin-bottom: 4px; color: var(--text); }
  input, select { width: 100%; padding: 8px 10px; margin-bottom: 12px; background: #0f172a; border: 1px solid var(--border); border-radius: 6px; color: #fff; font-size: 0.9rem; }
  input:focus, select:focus { outline: none; border-color: var(--accent); }
  button { padding: 9px 14px; background: var(--accent); color: #fff; border: none; border-radius: 6px; font-size: 0.85rem; font-weight: 600; cursor: pointer; transition: background 0.2s; }
  button:hover { background: var(--accent-hover); }
  .btn-danger { background: var(--danger); }
  .btn-danger:hover { background: #dc2626; }
  .btn-ota { display: block; text-align: center; text-decoration: none; padding: 10px; background: #334155; color: #38bdf8; border-radius: 6px; font-size: 0.9rem; font-weight: 600; margin-top: 8px; }
  .btn-ota:hover { background: #475569; }
  .item-card { background: rgba(15,23,42,0.5); border: 1px solid var(--border); border-radius: 8px; padding: 12px; margin-bottom: 10px; display: flex; flex-direction: column; gap: 6px; }
  .item-header { display: flex; justify-content: space-between; align-items: center; }
  .item-title { font-size: 0.92rem; font-weight: 600; }
  .item-short { font-size: 0.75rem; color: var(--muted); background: #334155; padding: 2px 6px; border-radius: 4px; }
  .item-row { display: flex; align-items: center; gap: 8px; margin-top: 4px; }
  .item-val { font-size: 1.1rem; font-weight: 700; color: #38bdf8; }
  .toast { position: fixed; bottom: 20px; right: 20px; background: #065f46; color: #fff; padding: 10px 16px; border-radius: 6px; font-size: 0.85rem; display: none; z-index: 99; }
</style>
</head>
<body>
<div class="container">
  <!-- Device Overview -->
  <div class="card">
    <h1>TK TECH Controller</h1>
    <p class="sub">Machine #)rawliteral";
    html += String(deviceId);
    html += R"rawliteral( &bull; Firmware Portal</p>
    <div style="display:flex; justify-content:space-between; align-items:center; margin-bottom:12px;">
      <span style="font-size:0.85rem; font-weight:600;">WiFi Status</span>
      <span class="badge )rawliteral";
    html += (isConnected() ? "online\">STA Connected" : "offline\">AP Only");
    html += R"rawliteral(</span>
    </div>
    <div class="stat-grid">
      <div class="stat-box"><div class="stat-label">STA IP</div><div class="stat-val">)rawliteral";
    html += getLocalIP();
    html += R"rawliteral(</div></div>
      <div class="stat-box"><div class="stat-label">SoftAP IP</div><div class="stat-val">)rawliteral";
    html += getAPIP();
    html += R"rawliteral(</div></div>
      <div class="stat-box"><div class="stat-label">Signal (RSSI)</div><div class="stat-val">)rawliteral";
    html += (isConnected() ? String(getRSSI()) + " dBm" : "N/A");
    html += R"rawliteral(</div></div>
      <div class="stat-box"><div class="stat-label">Free Heap</div><div class="stat-val">)rawliteral";
    html += String(ESP.getFreeHeap() / 1024) + " KB";
    html += R"rawliteral(</div></div>
    </div>
  </div>

  <!-- Live Menu Items Dashboard -->
  <div class="card">
    <h2>
      <span>Live Parameters & Remote Control</span>
      <label style="font-size:0.75rem; font-weight:normal; display:flex; align-items:center; gap:4px; margin:0; cursor:pointer;">
        <input type="checkbox" id="chkAuto" style="width:auto; margin:0;" checked> Auto-refresh (2s)
      </label>
    </h2>
    <div id="itemsContainer">Loading parameters...</div>
  </div>

  <!-- Network Configuration -->
  <div class="card">
    <h2>WiFi Settings</h2>
    <form action="/update" method="POST">
      <label>WiFi Network Name (SSID)</label>
      <input type="text" name="ssid" value=")rawliteral";
    html += ssid;
    html += R"rawliteral(" required>
      <label>WiFi Password</label>
      <input type="password" name="password" value=")rawliteral";
    html += wifiPassword;
    html += R"rawliteral(">
      <label>Static IP Address</label>
      <input type="text" name="ip" value=")rawliteral";
    html += staticIPStr;
    html += R"rawliteral(" required>
      <button type="submit" style="width:100%;">Save &amp; Reconnect</button>
    </form>
    <a href="/ota" class="btn-ota">&uarr; Over-The-Air (OTA) Firmware Upgrade</a>
  </div>
</div>

<div id="toast" class="toast"></div>

<script>
  function showToast(msg) {
    const t = document.getElementById('toast');
    t.innerText = msg;
    t.style.display = 'block';
    setTimeout(() => { t.style.display = 'none'; }, 2500);
  }

  function escapeHtml(str) {
    if (str === null || str === undefined) return '';
    return String(str).replace(/&/g, '&amp;').replace(/</g, '&lt;').replace(/>/g, '&gt;').replace(/"/g, '&quot;');
  }

  function setItemValue(id, val) {
    fetch('/api/item/set?id=' + id + '&value=' + encodeURIComponent(val), { method: 'POST' })
      .then(r => r.json())
      .then(d => {
        showToast(d.success ? 'Saved successfully!' : ('Error: ' + d.error));
        fetchItems();
      })
      .catch(err => showToast('Error: ' + err.message));
  }

  function setItemMode(id, modeIndex) {
    fetch('/api/item/set?id=' + id + '&mode=' + modeIndex, { method: 'POST' })
      .then(r => r.json())
      .then(d => {
        showToast(d.success ? 'Mode updated!' : ('Error: ' + d.error));
        fetchItems();
      })
      .catch(err => showToast('Error: ' + err.message));
  }

  function resetItem(id) {
    if (!confirm('Are you sure you want to reset this item?')) return;
    fetch('/api/item/reset?id=' + id, { method: 'POST' })
      .then(r => r.json())
      .then(d => {
        showToast(d.success ? 'Reset triggered!' : ('Error: ' + d.error));
        fetchItems();
      })
      .catch(err => showToast('Error: ' + err.message));
  }

  function fetchItems() {
    fetch('/api/items')
      .then(r => {
        if (!r.ok) {
          throw new Error('HTTP ' + r.status + ' (' + r.statusText + ')');
        }
        return r.json();
      })
      .then(items => {
        const c = document.getElementById('itemsContainer');
        if (!Array.isArray(items)) {
          throw new Error('Expected items array but received: ' + JSON.stringify(items));
        }
        if (items.length === 0) {
          c.innerHTML = '<p style="font-size:0.85rem; color:#94a3b8;">No parameters registered.</p>';
          return;
        }

        let html = '';
        items.forEach(it => {
          html += '<div class="item-card">';
          html += '<div class="item-header">';
          html += '<span class="item-title">' + escapeHtml(it.group) + ' &bull; ' + escapeHtml(it.name) + '</span>';
          html += '<span class="item-short">' + escapeHtml(it.shortName) + (it.editable ? ' [Edit]' : '') + '</span>';
          html += '</div>';

          if (it.type === 'value') {
            const numVal = (it.value !== null && it.value !== undefined && !isNaN(it.value)) ? Number(it.value).toFixed(2) : '0.00';
            html += '<div class="item-row">';
            html += '<span class="item-val">' + numVal + '</span>';
            if (it.editable) {
              const inpVal = (it.value !== null && it.value !== undefined && !isNaN(it.value)) ? it.value : 0;
              html += '<input type="number" step="any" id="inp_' + it.id + '" value="' + inpVal + '" style="max-width:110px; margin:0;">';
              html += '<button onclick="setItemValue(' + it.id + ', document.getElementById(\'inp_' + it.id + '\').value)">Set</button>';
            }
            if (it.resettable) {
              html += '<button class="btn-danger" onclick="resetItem(' + it.id + ')">Reset</button>';
            }
            html += '</div>';
          } else if (it.type === 'mode') {
            html += '<div class="item-row">';
            if (it.editable && it.modes && it.modes.length > 0) {
              html += '<select id="sel_' + it.id + '" style="margin:0; max-width:180px;">';
              it.modes.forEach((m, idx) => {
                html += '<option value="' + idx + '"' + (idx === it.currentMode ? ' selected' : '') + '>' + escapeHtml(m) + '</option>';
              });
              html += '</select>';
              html += '<button onclick="setItemMode(' + it.id + ', document.getElementById(\'sel_' + it.id + '\').value)">Set</button>';
            } else {
              html += '<span class="item-val">' + escapeHtml(it.currentModeName || '') + '</span>';
            }
            html += '</div>';
          }
          html += '</div>';
        });
        c.innerHTML = html;
      })
      .catch(err => {
        console.error(err);
        const c = document.getElementById('itemsContainer');
        c.innerHTML = '<div style="background:#450a0a; color:#fca5a5; padding:12px; border-radius:6px; font-size:0.85rem; border:1px solid #7f1d1d;">' +
                      '<strong>Failed to load parameters:</strong> ' + escapeHtml(err.message || String(err)) + 
                      '<br style="margin-bottom:6px;"><a href="/api/items" target="_blank" style="color:#93c5fd; text-decoration:underline;">Click here to inspect raw /api/items JSON</a>' +
                      '<br><button onclick="fetchItems()" style="margin-top:8px; padding:4px 10px; font-size:0.75rem;">Retry Now</button></div>';
      });
  }

  fetchItems();
  setInterval(() => {
    if (document.getElementById('chkAuto').checked) {
      fetchItems();
    }
  }, 2000);
</script>
</body>
</html>)rawliteral";
    return html;
}

String WiFiManager::generateOTAHTML()
{
    String html = R"rawliteral(<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>TK TECH | Firmware Update</title>
<style>
  :root { --bg: #0f172a; --card: #1e293b; --accent: #0284c7; --text: #f8fafc; --muted: #94a3b8; --border: #334155; }
  * { box-sizing: border-box; margin: 0; padding: 0; font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif; }
  body { background: var(--bg); color: var(--text); padding: 20px; display: flex; justify-content: center; }
  .card { background: var(--card); border: 1px solid var(--border); border-radius: 12px; padding: 24px; width: 100%; max-width: 440px; }
  h1 { font-size: 1.3rem; margin-bottom: 6px; color: #38bdf8; }
  p { font-size: 0.85rem; color: var(--muted); margin-bottom: 20px; }
  input[type=file] { width: 100%; padding: 12px; margin-bottom: 16px; background: #0f172a; border: 1px dashed var(--border); border-radius: 6px; color: #fff; cursor: pointer; }
  button { width: 100%; padding: 12px; background: var(--accent); color: #fff; border: none; border-radius: 6px; font-size: 0.95rem; font-weight: 600; cursor: pointer; }
  .progress-bar { width: 100%; height: 16px; background: #0f172a; border-radius: 8px; margin: 16px 0; overflow: hidden; display: none; }
  .progress-fill { height: 100%; width: 0%; background: #38bdf8; transition: width 0.1s; }
  #status { font-size: 0.85rem; text-align: center; margin-top: 10px; color: #34d399; }
  a { display: block; text-align: center; color: var(--muted); margin-top: 16px; font-size: 0.85rem; text-decoration: none; }
</style>
</head>
<body>
<div class="card">
  <h1>Firmware Update (OTA)</h1>
  <p>Select your compiled <code>firmware.bin</code> file to upload to the controller.</p>
  <form id="uploadForm">
    <input type="file" name="firmware" id="firmware" accept=".bin" required>
    <div class="progress-bar" id="progressBar"><div class="progress-fill" id="progressFill"></div></div>
    <button type="submit" id="btnSubmit">Upload &amp; Flash</button>
    <div id="status"></div>
  </form>
  <a href="/">&larr; Back to Configuration Portal</a>
</div>
<script>
  const form = document.getElementById('uploadForm');
  form.addEventListener('submit', (e) => {
    e.preventDefault();
    const file = document.getElementById('firmware').files[0];
    if (!file) return;
    const xhr = new XMLHttpRequest();
    const pBar = document.getElementById('progressBar');
    const pFill = document.getElementById('progressFill');
    const status = document.getElementById('status');
    const btn = document.getElementById('btnSubmit');
    pBar.style.display = 'block';
    btn.disabled = true;
    btn.innerText = 'Uploading...';
    xhr.upload.addEventListener('progress', (ev) => {
      if (ev.lengthComputable) {
        const pct = Math.round((ev.loaded / ev.total) * 100);
        pFill.style.width = pct + '%';
        status.innerText = 'Uploading: ' + pct + '%';
      }
    });
    xhr.onload = () => {
      if (xhr.status === 200) {
        status.innerText = 'Flash successful! Rebooting device in 3s...';
        setTimeout(() => { window.location.href = '/'; }, 3500);
      } else {
        status.innerText = 'Upload failed! Error: ' + xhr.responseText;
        btn.disabled = false;
        btn.innerText = 'Upload & Flash';
      }
    };
    xhr.open('POST', '/ota');
    const formData = new FormData();
    formData.append('firmware', file);
    xhr.send(formData);
  });
</script>
</body>
</html>)rawliteral";
    return html;
}

#endif // ESP32
