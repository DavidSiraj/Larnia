#pragma once
#include <Arduino.h>
#include <vector>

struct LarniaProfile {
    String id;
    String name;
    int age = 0;
    String avatarId;
};

struct LarniaTeacher {
    String id;
    String name;
    String subject;
    String avatarId;
};

struct LarniaMaterial {
    String id;
    String teacherId;
    String title;
    String type; // url, txt, pdf, docx
    String source;
};

struct LarniaSettings {
    String selectedAvatarId;
    String avatarGender;
    String voice = "nova";
    bool mouthAnimationEnabled = true;
    bool showOnTeacherCard = true;
    String assistantName = "Larnia";
    String model = "gpt-5.5";
    String language = "sk";
    String responseLength = "medium";
    bool useMaterials = true;

    String deviceName = "Larnia";
    uint8_t displayBrightness = 75;
    String wifiSsid;
    String wifiPassword;
    String backendHost;
    uint16_t backendPort = 3000;
    bool bluetoothEnabled = false;
    bool wakeOnTouch = true;
    bool autoReconnect = true;
    uint32_t autoSleepSeconds = 120;
};
