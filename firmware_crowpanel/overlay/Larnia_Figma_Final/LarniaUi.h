#pragma once
#include <Arduino.h>
#include <lvgl.h>
#include <ArduinoJson.h>
#include "LarniaStorage.h"

class LarniaUi {
public:
    void begin(LarniaStorage* storage);
    void update();

    void showSplash();
    void showProfiles();
    void showTeachers();
    void showTeacherDetail(const String& teacherId);
    void showChat(const String& teacherId = "");
    void showMaterials(const String& teacherId = "");
    void showSettings();
    void showSettingsAI();
    void showSettingsGeneral();
    void showAvatarLibrary();
    void showPanelAnswer(const String& text, const String& avatarId = "");

    void setOnline(bool online);
    void setTalking(bool talking);
    void setStatus(const String& status);
    void handleEvent(const String& type, JsonVariant data);

private:
    enum Page { PAGE_SPLASH, PAGE_PROFILES, PAGE_TEACHERS, PAGE_DETAIL, PAGE_TESTS, PAGE_CHAT, PAGE_MATERIALS, PAGE_SETTINGS, PAGE_SETTINGS_AI, PAGE_SETTINGS_GENERAL, PAGE_AVATARS };

    LarniaStorage* _storage = nullptr;
    lv_obj_t* _root = nullptr;
    lv_obj_t* _answerLabel = nullptr;
    lv_obj_t* _statusLabel = nullptr;
    lv_obj_t* _mouth = nullptr;
    lv_obj_t* _keyboard = nullptr;
    lv_obj_t* _profileNameTa = nullptr;
    lv_obj_t* _profileAgeTa = nullptr;
    lv_obj_t* _teacherNameTa = nullptr;
    lv_obj_t* _urlTa = nullptr;
    lv_obj_t* _chatTa = nullptr;
    lv_obj_t* _aiAssistantTa = nullptr;
    lv_obj_t* _deviceNameTa = nullptr;
    lv_obj_t* _wifiSsidTa = nullptr;
    lv_obj_t* _wifiPasswordTa = nullptr;
    lv_obj_t* _backendHostTa = nullptr;
    lv_obj_t* _backendPortTa = nullptr;
    lv_obj_t* _brightnessSlider = nullptr;
    String _activeTeacherId;
    String _activeAvatarId;
    bool _talking = false;
    bool _online = false;
    uint32_t _lastTouch = 0;
    uint32_t _lastAnim = 0;
    Page _currentPage = PAGE_SPLASH;
    Page _backPages[12];
    String _backTeacherIds[12];
    uint8_t _backCount = 0;

    void clear(lv_color_t bg = lv_color_hex(0xFFFFFF));
    void wake();
    void pushCurrent();
    void goBack();
    void showPage(Page page, const String& teacherId = "");

    lv_obj_t* label(lv_obj_t* parent, const char* text, int x, int y, const lv_font_t* font, lv_color_t color, int w = 0);
    lv_obj_t* button(lv_obj_t* parent, const char* text, int x, int y, int w, int h, lv_color_t color, lv_event_cb_t cb = nullptr);
    lv_obj_t* card(lv_obj_t* parent, int x, int y, int w, int h, int radius = 20, bool clickable = false, lv_event_cb_t cb = nullptr);
    lv_obj_t* textarea(lv_obj_t* parent, int x, int y, int w, int h, const char* placeholder, bool numeric = false);
    lv_obj_t* field(lv_obj_t* parent, int x, int y, int w, const char* placeholder, bool error = false);
    lv_obj_t* modal(int h = 420);
    void img(lv_obj_t* parent, const char* src, int x, int y);
    void header(const char* title, const char* subtitle = nullptr, bool dark = false, bool back = false);
    void logoHeader();
    void addBackButton(bool dark = false);
    void addTabs(const char* active);
    void addToggle(lv_obj_t* parent, int x, int y, bool on);
    void addPill(lv_obj_t* parent, int x, int y, int w, const char* text, bool active);
    void addEmptyState(const char* title, const char* subtitle, lv_event_cb_t cb = nullptr);
    void addProfileCard(const LarniaProfile& profile, int x, int y);
    void addTeacherCard(const LarniaTeacher& teacher, int x, int y);
    void addDetailCard(int x, int y, const char* title, const char* body, const char* iconSrc, lv_event_cb_t cb);
    String teacherName(const String& id) const;

    void showNewProfileModal();
    void showNewTeacherModal();
    void showAddUrlModal();
    void showAddFileModal(const char* errorText = nullptr);
    void showTests(const String& teacherId);
    void makeKeyboard();
    void hideKeyboard();

    static void touchCb(lv_event_t* e);
    static void backCb(lv_event_t* e);
    static void textareaFocusCb(lv_event_t* e);
    static void keyboardCb(lv_event_t* e);
    static void goProfilesCb(lv_event_t* e);
    static void goTeachersCb(lv_event_t* e);
    static void goDetailCb(lv_event_t* e);
    static void goChatCb(lv_event_t* e);
    static void goMaterialsCb(lv_event_t* e);
    static void goSettingsCb(lv_event_t* e);
    static void goSettingsAICb(lv_event_t* e);
    static void goSettingsGeneralCb(lv_event_t* e);
    static void goTestsCb(lv_event_t* e);
    static void goAvatarsCb(lv_event_t* e);
    static void openProfileModalCb(lv_event_t* e);
    static void openTeacherModalCb(lv_event_t* e);
    static void openUrlModalCb(lv_event_t* e);
    static void openFileModalCb(lv_event_t* e);
    static void closeModalCb(lv_event_t* e);
    static void createProfileCb(lv_event_t* e);
    static void createTeacherCb(lv_event_t* e);
    static void addUrlCb(lv_event_t* e);
    static void addFileCb(lv_event_t* e);
    static void sendChatCb(lv_event_t* e);
    static void saveSettingsCb(lv_event_t* e);
    static void deleteProfileCb(lv_event_t* e);
    static void deleteTeacherCb(lv_event_t* e);
    static void deleteMaterialCb(lv_event_t* e);
};
