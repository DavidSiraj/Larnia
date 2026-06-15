#pragma once
#include <Arduino.h>
#include <vector>
#include "LarniaModels.h"

class LarniaStorage {
public:
    bool begin();
    bool loadAll();
    bool saveAll();
    bool saveSettingsOnly();

    std::vector<LarniaProfile>& profiles() { return _profiles; }
    std::vector<LarniaTeacher>& teachers() { return _teachers; }
    std::vector<LarniaMaterial>& materials() { return _materials; }
    LarniaSettings& settings() { return _settings; }

    bool addProfile(const String& name, int age);
    bool addTeacher(const String& name, const String& subject, const String& avatarId = "");
    bool addMaterial(const String& teacherId, const String& title, const String& type, const String& source);
    bool setTeacherAvatar(const String& teacherId, const String& avatarId);
    bool removeLastProfile();
    bool removeLastTeacher();
    bool removeLastMaterial();
    void clearDemoData();

private:
    std::vector<LarniaProfile> _profiles;
    std::vector<LarniaTeacher> _teachers;
    std::vector<LarniaMaterial> _materials;
    LarniaSettings _settings;

    String makeId(const String& prefix);
    bool loadProfiles();
    bool loadTeachers();
    bool loadMaterials();
    bool loadSettings();
    bool writeFile(const char* path, const String& text);
    String readFile(const char* path);
};
