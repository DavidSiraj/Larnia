#include "LarniaStorage.h"
#include <ArduinoJson.h>
#include <SD_MMC.h>

static const char* PROFILES_PATH = "/data/profiles.json";
static const char* TEACHERS_PATH = "/data/teachers.json";
static const char* MATERIALS_PATH = "/data/materials.json";
static const char* SETTINGS_PATH = "/config/settings.json";

bool LarniaStorage::begin()
{
    // Many Elecrow examples use SD_MMC. If your board package exposes SD instead,
    // change this one place.
    if (!SD_MMC.begin("/sdcard", true, false, SDMMC_FREQ_DEFAULT, 5)) {
        Serial.println("[LarniaStorage] SD_MMC mount failed. UI still works, but avatars/settings are unavailable.");
        return false;
    }
    SD_MMC.mkdir("/data");
    SD_MMC.mkdir("/config");
    SD_MMC.mkdir("/avatars");
    return true;
}

String LarniaStorage::readFile(const char* path)
{
    File f = SD_MMC.open(path, FILE_READ);
    if (!f) return "";
    String out;
    while (f.available()) out += char(f.read());
    f.close();
    return out;
}

bool LarniaStorage::writeFile(const char* path, const String& text)
{
    File f = SD_MMC.open(path, FILE_WRITE);
    if (!f) return false;
    f.print(text);
    f.close();
    return true;
}

bool LarniaStorage::loadAll()
{
    loadSettings();
    loadProfiles();
    loadTeachers();
    loadMaterials();
    return true;
}


bool LarniaStorage::saveSettingsOnly()
{
    DynamicJsonDocument doc(4096);
    doc["avatar"]["selectedAvatarId"] = _settings.selectedAvatarId;
    doc["avatar"]["gender"] = _settings.avatarGender;
    doc["avatar"]["voice"] = _settings.voice;
    doc["avatar"]["mouthAnimationEnabled"] = _settings.mouthAnimationEnabled;
    doc["avatar"]["showOnTeacherCard"] = _settings.showOnTeacherCard;

    doc["ai"]["assistantName"] = _settings.assistantName;
    doc["ai"]["model"] = _settings.model;
    doc["ai"]["language"] = _settings.language;
    doc["ai"]["responseLength"] = _settings.responseLength;
    doc["ai"]["useMaterials"] = _settings.useMaterials;
    doc["ai"]["customInstructions"] = "Odpovedaj ako trpezlivý školský učiteľ.";

    doc["general"]["deviceName"] = _settings.deviceName;
    doc["general"]["displayBrightness"] = _settings.displayBrightness;
    doc["general"]["wifiSsid"] = _settings.wifiSsid;
    doc["general"]["wifiPassword"] = _settings.wifiPassword;
    doc["general"]["backendHost"] = _settings.backendHost;
    doc["general"]["backendPort"] = _settings.backendPort;
    doc["general"]["bluetoothEnabled"] = _settings.bluetoothEnabled;
    doc["general"]["wakeOnTouch"] = _settings.wakeOnTouch;
    doc["general"]["autoReconnect"] = _settings.autoReconnect;
    doc["general"]["autoSleepSeconds"] = _settings.autoSleepSeconds;

    String out;
    serializeJsonPretty(doc, out);
    return writeFile(SETTINGS_PATH, out);
}

bool LarniaStorage::saveAll()
{
    DynamicJsonDocument doc(8192);
    JsonArray arr = doc.to<JsonArray>();
    for (auto& p : _profiles) {
        JsonObject o = arr.createNestedObject();
        o["id"] = p.id; o["name"] = p.name; o["age"] = p.age; o["avatarId"] = p.avatarId;
    }
    String out; serializeJsonPretty(doc, out); writeFile(PROFILES_PATH, out);

    doc.clear(); arr = doc.to<JsonArray>();
    for (auto& t : _teachers) {
        JsonObject o = arr.createNestedObject();
        o["id"] = t.id; o["name"] = t.name; o["subject"] = t.subject; o["avatarId"] = t.avatarId;
    }
    out = ""; serializeJsonPretty(doc, out); writeFile(TEACHERS_PATH, out);

    doc.clear(); arr = doc.to<JsonArray>();
    for (auto& m : _materials) {
        JsonObject o = arr.createNestedObject();
        o["id"] = m.id; o["teacherId"] = m.teacherId; o["title"] = m.title; o["type"] = m.type; o["source"] = m.source;
    }
    out = ""; serializeJsonPretty(doc, out); writeFile(MATERIALS_PATH, out);

    saveSettingsOnly();
    return true;
}

bool LarniaStorage::loadProfiles()
{
    _profiles.clear();
    String text = readFile(PROFILES_PATH);
    if (text.isEmpty()) return true;
    DynamicJsonDocument doc(8192);
    if (deserializeJson(doc, text)) return false;
    for (JsonObject o : doc.as<JsonArray>()) {
        LarniaProfile p;
        p.id = String(o["id"] | ""); p.name = String(o["name"] | ""); p.age = o["age"] | 0; p.avatarId = String(o["avatarId"] | "");
        if (p.id.length()) _profiles.push_back(p);
    }
    return true;
}

bool LarniaStorage::loadTeachers()
{
    _teachers.clear();
    String text = readFile(TEACHERS_PATH);
    if (text.isEmpty()) return true;
    DynamicJsonDocument doc(8192);
    if (deserializeJson(doc, text)) return false;
    for (JsonObject o : doc.as<JsonArray>()) {
        LarniaTeacher t;
        t.id = String(o["id"] | ""); t.name = String(o["name"] | ""); t.subject = String(o["subject"] | ""); t.avatarId = String(o["avatarId"] | "");
        if (t.id.length()) _teachers.push_back(t);
    }
    return true;
}

bool LarniaStorage::loadMaterials()
{
    _materials.clear();
    String text = readFile(MATERIALS_PATH);
    if (text.isEmpty()) return true;
    DynamicJsonDocument doc(12000);
    if (deserializeJson(doc, text)) return false;
    for (JsonObject o : doc.as<JsonArray>()) {
        LarniaMaterial m;
        m.id = String(o["id"] | ""); m.teacherId = String(o["teacherId"] | ""); m.title = String(o["title"] | ""); m.type = String(o["type"] | ""); m.source = String(o["source"] | "");
        if (m.id.length()) _materials.push_back(m);
    }
    return true;
}

bool LarniaStorage::loadSettings()
{
    String text = readFile(SETTINGS_PATH);
    if (text.isEmpty()) return true;
    DynamicJsonDocument doc(4096);
    if (deserializeJson(doc, text)) return false;
    _settings.selectedAvatarId = String(doc["avatar"]["selectedAvatarId"] | "");
    _settings.avatarGender = String(doc["avatar"]["gender"] | "");
    _settings.voice = String(doc["avatar"]["voice"] | "nova");
    _settings.mouthAnimationEnabled = doc["avatar"]["mouthAnimationEnabled"] | true;
    _settings.showOnTeacherCard = doc["avatar"]["showOnTeacherCard"] | true;
    _settings.assistantName = String(doc["ai"]["assistantName"] | "Larnia");
    _settings.model = String(doc["ai"]["model"] | "gpt-5.5");
    _settings.language = String(doc["ai"]["language"] | "sk");
    _settings.responseLength = String(doc["ai"]["responseLength"] | "medium");
    _settings.useMaterials = doc["ai"]["useMaterials"] | true;
    _settings.deviceName = String(doc["general"]["deviceName"] | "Larnia");
    _settings.displayBrightness = doc["general"]["displayBrightness"] | 75;
    _settings.wifiSsid = String(doc["general"]["wifiSsid"] | "");
    _settings.wifiPassword = String(doc["general"]["wifiPassword"] | "");
    _settings.backendHost = String(doc["general"]["backendHost"] | "");
    _settings.backendPort = doc["general"]["backendPort"] | 3000;
    _settings.bluetoothEnabled = doc["general"]["bluetoothEnabled"] | false;
    _settings.wakeOnTouch = doc["general"]["wakeOnTouch"] | true;
    _settings.autoReconnect = doc["general"]["autoReconnect"] | true;
    _settings.autoSleepSeconds = doc["general"]["autoSleepSeconds"] | 120;
    return true;
}

String LarniaStorage::makeId(const String& prefix)
{
    return prefix + String("_") + String(millis()) + String("_") + String(random(1000, 9999));
}

bool LarniaStorage::addProfile(const String& name, int age)
{
    LarniaProfile p; p.id = makeId("profile"); p.name = name; p.age = age;
    _profiles.push_back(p); return saveAll();
}

bool LarniaStorage::addTeacher(const String& name, const String& subject, const String& avatarId)
{
    LarniaTeacher t; t.id = makeId("teacher"); t.name = name; t.subject = subject; t.avatarId = avatarId;
    _teachers.push_back(t); return saveAll();
}

bool LarniaStorage::addMaterial(const String& teacherId, const String& title, const String& type, const String& source)
{
    LarniaMaterial m; m.id = makeId("material"); m.teacherId = teacherId; m.title = title; m.type = type; m.source = source;
    _materials.push_back(m); return saveAll();
}

bool LarniaStorage::setTeacherAvatar(const String& teacherId, const String& avatarId)
{
    for (auto& t : _teachers) {
        if (t.id == teacherId) { t.avatarId = avatarId; return saveAll(); }
    }
    return false;
}


bool LarniaStorage::removeLastProfile()
{
    if (_profiles.empty()) return false;
    _profiles.pop_back();
    return saveAll();
}

bool LarniaStorage::removeLastTeacher()
{
    if (_teachers.empty()) return false;
    String removedId = _teachers.back().id;
    _teachers.pop_back();

    // Remove materials bound to the removed teacher.
    for (int i = (int)_materials.size() - 1; i >= 0; --i) {
        if (_materials[i].teacherId == removedId) _materials.erase(_materials.begin() + i);
    }
    return saveAll();
}

bool LarniaStorage::removeLastMaterial()
{
    if (_materials.empty()) return false;
    _materials.pop_back();
    return saveAll();
}


void LarniaStorage::clearDemoData()
{
    _profiles.clear(); _teachers.clear(); _materials.clear(); saveAll();
}
