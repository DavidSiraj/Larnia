#include "LarniaUi.h"
#include "LarniaTheme.h"
#include "LarniaConfig.h"
#include <ArduinoJson.h>
#include <stdlib.h>


// Bridge functions are implemented in Larnia_Figma_Final.ino.
// LarniaUi.cpp needs declarations because it is compiled as a separate C++ file.
extern void larniaSendChatFromUi(const String& teacherId, const String& question);
extern void larniaReconnectNetworkFromUi();

#if defined(LVGL_VERSION_MAJOR) && LVGL_VERSION_MAJOR >= 9
#define lv_img_create lv_image_create
#define lv_img_set_src lv_image_set_src
#endif

static lv_obj_t* active_screen()
{
#if defined(LVGL_VERSION_MAJOR) && LVGL_VERSION_MAJOR >= 9
    return lv_screen_active();
#else
    return lv_scr_act();
#endif
}

// Doplnková lišta nad LVGL klávesnicou.
// LVGL základná klávesnica nemá slovenskú diakritiku ani podtržník,
// preto tieto znaky vkladáme priamo do aktívneho textarea.
static lv_obj_t* g_larnia_active_ta = nullptr;
static lv_obj_t* g_larnia_quickbar = nullptr;

static void larniaQuickKeyCb(lv_event_t* e)
{
    const char* txt = static_cast<const char*>(lv_event_get_user_data(e));
    if (!txt || !g_larnia_active_ta) return;
    lv_textarea_add_text(g_larnia_active_ta, txt);
}

static void larniaMakeQuickbar(lv_obj_t* root)
{
    if (!root) return;

    if (g_larnia_quickbar) {
        lv_obj_clear_flag(g_larnia_quickbar, LV_OBJ_FLAG_HIDDEN);
        lv_obj_move_foreground(g_larnia_quickbar);
        return;
    }

    static const char* keys[] = {
        "á","ä","č","ď","é","í","ĺ","ľ","ň","ó","ô","ŕ","š","ť","ú","ý","ž","_","-",".","@","/"
    };
    const int keyCount = sizeof(keys) / sizeof(keys[0]);

    g_larnia_quickbar = lv_obj_create(root);
    lv_obj_set_size(g_larnia_quickbar, 1024, 46);
    lv_obj_align(g_larnia_quickbar, LV_ALIGN_BOTTOM_MID, 0, -185);
    lv_obj_set_style_bg_color(g_larnia_quickbar, lv_color_hex(0xEBF9FA), 0);
    lv_obj_set_style_bg_opa(g_larnia_quickbar, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(g_larnia_quickbar, 1, 0);
    lv_obj_set_style_border_color(g_larnia_quickbar, lv_color_hex(0x82CEDC), 0);
    lv_obj_set_style_radius(g_larnia_quickbar, 0, 0);
    lv_obj_set_style_pad_all(g_larnia_quickbar, 4, 0);
    lv_obj_clear_flag(g_larnia_quickbar, LV_OBJ_FLAG_SCROLLABLE);

    int x = 6;
    int y = 5;
    int w = 42;
    int h = 34;

    for (int i = 0; i < keyCount; i++) {
        lv_obj_t* b = lv_btn_create(g_larnia_quickbar);
        lv_obj_set_size(b, w, h);
        lv_obj_set_pos(b, x, y);
        lv_obj_set_style_radius(b, 9, 0);
        lv_obj_set_style_bg_color(b, lv_color_hex(0xFFFFFF), 0);
        lv_obj_set_style_bg_opa(b, LV_OPA_COVER, 0);
        lv_obj_set_style_border_width(b, 1, 0);
        lv_obj_set_style_border_color(b, lv_color_hex(0xDDEEF2), 0);
        lv_obj_set_style_shadow_width(b, 0, 0);
        lv_obj_add_event_cb(b, larniaQuickKeyCb, LV_EVENT_CLICKED, (void*)keys[i]);

        lv_obj_t* l = lv_label_create(b);
        lv_label_set_text(l, keys[i]);
        lv_obj_set_style_text_color(l, lv_color_hex(0x125B90), 0);
        lv_obj_set_style_text_font(l, FONT_BODY_BOLD, 0);
        lv_obj_center(l);

        x += w + 4;
    }

    lv_obj_move_foreground(g_larnia_quickbar);
}

static const char* avatarSrc(const String& id)
{
    if (id == "avatar_01") return "S:/avatars/avatar_01/1.png";
    if (id == "avatar_02") return "S:/avatars/avatar_02/1.png";
    if (id == "avatar_03") return "S:/avatars/avatar_03/1.png";
    if (id == "avatar_04") return "S:/avatars/avatar_04/1.png";
    if (id == "avatar_05") return "S:/avatars/avatar_05/1.png";
    if (id == "avatar_06") return "S:/avatars/avatar_06/1.png";
    if (id == "avatar_07") return "S:/avatars/avatar_07/1.png";
    if (id == "avatar_08") return "S:/avatars/avatar_08/1.png";
    return "S:/avatars/avatar_01/1.png";
}
static const char* avatarSrcByIndex(int i)
{
    switch(i) {
        case 0: return "S:/avatars/avatar_01/1.png";
        case 1: return "S:/avatars/avatar_02/1.png";
        case 2: return "S:/avatars/avatar_03/1.png";
        case 3: return "S:/avatars/avatar_04/1.png";
        case 4: return "S:/avatars/avatar_05/1.png";
        case 5: return "S:/avatars/avatar_06/1.png";
        case 6: return "S:/avatars/avatar_07/1.png";
        default: return "S:/avatars/avatar_08/1.png";
    }
}

void LarniaUi::begin(LarniaStorage* storage)
{
    _storage = storage;
    _root = active_screen();
    _lastTouch = millis();
    showSplash();
}
void LarniaUi::wake() { _lastTouch = millis(); }

void LarniaUi::clear(lv_color_t bg)
{
    _root = active_screen();
    lv_obj_clean(_root);
    lv_obj_set_style_bg_color(_root, bg, 0);
    lv_obj_set_style_bg_opa(_root, LV_OPA_COVER, 0);
    lv_obj_clear_flag(_root, LV_OBJ_FLAG_SCROLLABLE);
    _answerLabel = nullptr; _statusLabel = nullptr; _mouth = nullptr; _keyboard = nullptr;
    _aiAssistantTa = nullptr; _deviceNameTa = nullptr; _wifiSsidTa = nullptr; _wifiPasswordTa = nullptr; _backendHostTa = nullptr; _backendPortTa = nullptr; _brightnessSlider = nullptr;
    g_larnia_quickbar = nullptr; g_larnia_active_ta = nullptr;
    _profileNameTa = nullptr; _profileAgeTa = nullptr; _teacherNameTa = nullptr; _urlTa = nullptr; _chatTa = nullptr;
}

void LarniaUi::touchCb(lv_event_t* e)
{
    auto* self = static_cast<LarniaUi*>(lv_event_get_user_data(e));
    if (self) self->wake();
}

void LarniaUi::pushCurrent()
{
    if (_backCount >= 12) return;
    _backPages[_backCount] = _currentPage;
    _backTeacherIds[_backCount] = _activeTeacherId;
    _backCount++;
}
void LarniaUi::goBack()
{
    hideKeyboard();
    if (_backCount == 0) { showProfiles(); return; }
    _backCount--;
    showPage(_backPages[_backCount], _backTeacherIds[_backCount]);
}
void LarniaUi::showPage(Page page, const String& teacherId)
{
    switch (page) {
        case PAGE_SPLASH: showSplash(); break;
        case PAGE_PROFILES: showProfiles(); break;
        case PAGE_TEACHERS: showTeachers(); break;
        case PAGE_DETAIL: showTeacherDetail(teacherId); break;
        case PAGE_TESTS: showTests(teacherId); break;
        case PAGE_CHAT: showChat(teacherId); break;
        case PAGE_MATERIALS: showMaterials(teacherId); break;
        case PAGE_SETTINGS: showSettings(); break;
        case PAGE_SETTINGS_AI: showSettingsAI(); break;
        case PAGE_SETTINGS_GENERAL: showSettingsGeneral(); break;
        case PAGE_AVATARS: showAvatarLibrary(); break;
    }
}

lv_obj_t* LarniaUi::label(lv_obj_t* parent, const char* text, int x, int y, const lv_font_t* font, lv_color_t color, int w)
{
    lv_obj_t* l = lv_label_create(parent);
    lv_label_set_text(l, text ? text : "");
    lv_obj_set_style_text_color(l, color, 0);
    lv_obj_set_style_text_font(l, font, 0);
    lv_obj_set_style_text_line_space(l, 2, 0);
    if (w > 0) { lv_obj_set_width(l, w); lv_label_set_long_mode(l, LV_LABEL_LONG_WRAP); }
    lv_obj_set_pos(l, x, y);
    lv_obj_clear_flag(l, LV_OBJ_FLAG_CLICKABLE);
    return l;
}

lv_obj_t* LarniaUi::button(lv_obj_t* parent, const char* text, int x, int y, int w, int h, lv_color_t color, lv_event_cb_t cb)
{
    lv_obj_t* b = lv_btn_create(parent);
    lv_obj_set_size(b, w, h); lv_obj_set_pos(b, x, y);
    lv_obj_set_style_radius(b, 12, 0);
    lv_obj_set_style_bg_color(b, color, 0);
    lv_obj_set_style_bg_opa(b, LV_OPA_COVER, 0);
    lv_obj_set_style_shadow_width(b, 0, 0);
    lv_obj_set_style_border_width(b, 0, 0);
    lv_obj_add_flag(b, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(b, cb ? cb : touchCb, LV_EVENT_CLICKED, this);
    lv_obj_t* t = lv_label_create(b);
    lv_label_set_text(t, text ? text : "");
    lv_obj_set_style_text_color(t, C_WHITE(), 0);
    lv_obj_set_style_text_font(t, FONT_BODY_BOLD, 0);
    lv_obj_center(t);
    return b;
}

lv_obj_t* LarniaUi::card(lv_obj_t* parent, int x, int y, int w, int h, int radius, bool clickable, lv_event_cb_t cb)
{
    lv_obj_t* c = lv_obj_create(parent);
    lv_obj_set_size(c, w, h); lv_obj_set_pos(c, x, y);
    lv_obj_set_style_bg_color(c, C_WHITE(), 0);
    lv_obj_set_style_bg_opa(c, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(c, radius, 0);
    lv_obj_set_style_border_width(c, 0, 0);
    lv_obj_set_style_shadow_width(c, 20, 0);
    lv_obj_set_style_shadow_opa(c, LV_OPA_20, 0);
    lv_obj_set_style_shadow_ofs_x(c, 5, 0); lv_obj_set_style_shadow_ofs_y(c, 12, 0);
    lv_obj_clear_flag(c, LV_OBJ_FLAG_SCROLLABLE);
    if (clickable) { lv_obj_add_flag(c, LV_OBJ_FLAG_CLICKABLE); if (cb) lv_obj_add_event_cb(c, cb, LV_EVENT_CLICKED, this); }
    return c;
}

void LarniaUi::img(lv_obj_t* parent, const char* src, int x, int y)
{
    lv_obj_t* im = lv_img_create(parent);
    lv_img_set_src(im, src);
    lv_obj_set_pos(im, x, y);
}

void LarniaUi::makeKeyboard()
{
    if (_keyboard) {
        lv_obj_clear_flag(_keyboard, LV_OBJ_FLAG_HIDDEN);
        larniaMakeQuickbar(_root);
        lv_obj_move_foreground(_keyboard);
        return;
    }

    _keyboard = lv_keyboard_create(_root);
    lv_obj_set_size(_keyboard, 1024, 185);
    lv_obj_align(_keyboard, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_style_bg_color(_keyboard, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_bg_opa(_keyboard, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(_keyboard, 1, 0);
    lv_obj_set_style_border_color(_keyboard, lv_color_hex(0xDDEEF2), 0);
    lv_obj_set_style_shadow_width(_keyboard, 18, 0);
    lv_obj_set_style_shadow_opa(_keyboard, LV_OPA_20, 0);
    lv_obj_set_style_shadow_ofs_y(_keyboard, -6, 0);
    lv_obj_add_event_cb(_keyboard, keyboardCb, LV_EVENT_READY, this);
    lv_obj_add_event_cb(_keyboard, keyboardCb, LV_EVENT_CANCEL, this);

    larniaMakeQuickbar(_root);
    lv_obj_move_foreground(_keyboard);
}
void LarniaUi::hideKeyboard()
{
    if (g_larnia_quickbar) { lv_obj_del(g_larnia_quickbar); g_larnia_quickbar = nullptr; }
    if (_keyboard) { lv_obj_del(_keyboard); _keyboard = nullptr; }
    g_larnia_active_ta = nullptr;
}

lv_obj_t* LarniaUi::textarea(lv_obj_t* parent, int x, int y, int w, int h, const char* placeholder, bool numeric)
{
    lv_obj_t* ta = lv_textarea_create(parent);
    lv_obj_set_size(ta, w, h); lv_obj_set_pos(ta, x, y);
    lv_textarea_set_one_line(ta, true);
    lv_textarea_set_placeholder_text(ta, placeholder ? placeholder : "");
    lv_obj_set_style_text_font(ta, FONT_BODY, 0);
    lv_obj_set_style_text_color(ta, C_BLUE(), 0);
    lv_obj_set_style_radius(ta, 12, 0);
    lv_obj_set_style_border_width(ta, 0, 0);
    lv_obj_set_style_bg_color(ta, C_WHITE(), 0);
    lv_obj_set_style_shadow_width(ta, 18, 0);
    lv_obj_set_style_shadow_opa(ta, LV_OPA_20, 0);
    lv_obj_set_style_shadow_ofs_x(ta, 5, 0); lv_obj_set_style_shadow_ofs_y(ta, 12, 0);

    // Na CrowPaneli dotyk nie vždy vyvolá LV_EVENT_FOCUSED spoľahlivo.
    // Preto otvárame klávesnicu aj na CLICKED/PRESSED.
    lv_obj_add_flag(ta, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(ta, textareaFocusCb, LV_EVENT_FOCUSED, this);
    lv_obj_add_event_cb(ta, textareaFocusCb, LV_EVENT_CLICKED, this);
    lv_obj_add_event_cb(ta, textareaFocusCb, LV_EVENT_PRESSED, this);

    if (numeric) lv_textarea_set_accepted_chars(ta, "0123456789");
    return ta;
}

lv_obj_t* LarniaUi::field(lv_obj_t* parent, int x, int y, int w, const char* placeholder, bool error)
{
    lv_obj_t* f = lv_obj_create(parent);
    lv_obj_set_size(f, w, 56); lv_obj_set_pos(f, x, y);
    lv_obj_set_style_radius(f, 12, 0); lv_obj_set_style_bg_color(f, C_WHITE(), 0);
    lv_obj_set_style_border_width(f, error ? 3 : 0, 0); lv_obj_set_style_border_color(f, error ? C_ERROR() : C_WHITE(), 0);
    lv_obj_set_style_shadow_width(f, 18, 0); lv_obj_set_style_shadow_opa(f, LV_OPA_20, 0);
    lv_obj_set_style_shadow_ofs_x(f, 5, 0); lv_obj_set_style_shadow_ofs_y(f, 12, 0);
    lv_obj_clear_flag(f, LV_OBJ_FLAG_SCROLLABLE);
    if (error) { img(f, "S:/ui/icon_error_35.png", 10, 10); label(f, placeholder, 56, 18, FONT_BODY, C_ERROR(), w - 70); }
    else label(f, placeholder, 16, 18, FONT_BODY, lv_color_hex(0x777777), w - 30);
    return f;
}

lv_obj_t* LarniaUi::modal(int h)
{
    lv_obj_t* shade = lv_obj_create(_root);
    lv_obj_set_size(shade, 1024, 600); lv_obj_set_pos(shade, 0, 0);
    lv_obj_set_style_bg_color(shade, lv_color_hex(0x000000), 0); lv_obj_set_style_bg_opa(shade, LV_OPA_30, 0);
    lv_obj_set_style_border_width(shade, 0, 0); lv_obj_clear_flag(shade, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(shade, LV_OBJ_FLAG_CLICKABLE); lv_obj_add_event_cb(shade, closeModalCb, LV_EVENT_CLICKED, this);
    lv_obj_t* m = card(_root, 262, (600 - h) / 2, 500, h, 24, false, nullptr);
    lv_obj_set_style_shadow_width(m, 24, 0);
    return m;
}

void LarniaUi::logoHeader() { img(_root, "S:/ui/logo_header.png", 760, 70); }
void LarniaUi::addBackButton(bool dark)
{
    lv_color_t col = dark ? C_CYAN2() : C_BLUE();
    lv_obj_t* b = lv_btn_create(_root);
    lv_obj_set_size(b, 42, 42); lv_obj_set_pos(b, 20, 25);
    lv_obj_set_style_radius(b, 12, 0); lv_obj_set_style_bg_color(b, dark ? lv_color_hex(0x18375F) : C_WHITE(), 0);
    lv_obj_set_style_border_width(b, 2, 0); lv_obj_set_style_border_color(b, col, 0); lv_obj_set_style_shadow_width(b, 0, 0);
    lv_obj_add_event_cb(b, backCb, LV_EVENT_CLICKED, this);
    label(b, "<", 13, 4, FONT_H2, col, 22);
}
void LarniaUi::header(const char* title, const char* subtitle, bool dark, bool back)
{
    lv_color_t color = dark ? C_CYAN2() : C_BLUE();
    if (back) addBackButton(dark);
    int x = back ? 75 : 45;
    label(_root, title, x, dark ? 70 : 80, dark ? FONT_BIG : FONT_TITLE, color, 610);
    if (subtitle) label(_root, subtitle, x, dark ? 124 : 137, FONT_SMALL, dark ? C_CYAN2() : C_BLUE(), 620);
    logoHeader();
}

void LarniaUi::addTabs(const char* active)
{
    const char* labels[3] = {"Avatar", "AI", "Generálne"};
    lv_event_cb_t cbs[3] = {goSettingsCb, goSettingsAICb, goSettingsGeneralCb};
    int x = 75;
    for (int i=0;i<3;i++) {
        bool is = strcmp(active, labels[i]) == 0;
        lv_color_t bg = is ? C_CYAN() : C_WHITE();
        lv_obj_t* b = button(_root, labels[i], x, 160, i==2?150:130, 40, bg, cbs[i]);
        if (!is) {
            lv_obj_set_style_border_width(b, 1, 0); lv_obj_set_style_border_color(b, lv_color_hex(0xDDEEF2), 0);
            lv_obj_set_style_text_color(lv_obj_get_child(b,0), C_BLUE(), 0);
        }
        x += (i==2?150:130) + 16;
    }
}

void LarniaUi::addToggle(lv_obj_t* parent, int x, int y, bool on)
{
    lv_obj_t* sw = lv_switch_create(parent);
    lv_obj_set_pos(sw, x, y); lv_obj_set_size(sw, 58, 30);
    if (on) lv_obj_add_state(sw, LV_STATE_CHECKED);
}
void LarniaUi::addPill(lv_obj_t* parent, int x, int y, int w, const char* text, bool active)
{
    lv_obj_t* p = card(parent, x, y, w, 36, 18, false, nullptr);
    lv_obj_set_style_shadow_width(p, 0, 0);
    lv_obj_set_style_bg_color(p, active ? C_CYAN() : lv_color_hex(0xEBF9FA), 0);
    label(p, text, 14, 9, FONT_SMALL, active ? C_WHITE() : C_BLUE(), w-20);
}
void LarniaUi::addEmptyState(const char* title, const char* subtitle, lv_event_cb_t cb)
{
    lv_obj_t* e = card(_root, 45, 275, 932, 245, 20, cb != nullptr, cb);
    lv_obj_set_style_bg_color(e, C_EMPTY(), 0); lv_obj_set_style_border_width(e, 4, 0); lv_obj_set_style_border_color(e, C_CYAN(), 0);
    img(e, "S:/ui/profile_icon_110.png", 410, 30);
    label(e, title, 280, 125, FONT_H1, C_BLUE(), 520);
    label(e, subtitle, 270, 170, FONT_H2, C_BLUE(), 560);
}
String LarniaUi::teacherName(const String& id) const
{
    if (_storage) for (auto& t : _storage->teachers()) if (t.id == id) return t.name;
    return "AI učiteľ";
}
void LarniaUi::addProfileCard(const LarniaProfile& profile, int x, int y)
{
    lv_obj_t* c = card(_root, x, y, 216, 216, 20, true, goTeachersCb);
    img(c, profile.avatarId.length()?avatarSrc(profile.avatarId):"S:/ui/profile_icon_110.png", 53, 35);
    label(c, profile.name.c_str(), 20, 162, FONT_H2, C_BLUE(), 176);
}
void LarniaUi::addTeacherCard(const LarniaTeacher& teacher, int x, int y)
{
    lv_obj_t* c = card(_root, x, y, 280, 280, 20, true, goDetailCb);
    img(c, teacher.avatarId.length()?avatarSrc(teacher.avatarId):"S:/ui/teacher_placeholder_125.png", 78, 45);
    label(c, teacher.name.c_str(), 24, 218, FONT_H2, C_BLUE(), 232);
}
void LarniaUi::addDetailCard(int x, int y, const char* title, const char* body, const char* iconSrc, lv_event_cb_t cb)
{
    lv_obj_t* c = card(_root, x, y, 288, 116, 20, true, cb);
    lv_obj_set_style_bg_opa(c, LV_OPA_90, 0); lv_obj_set_style_shadow_color(c, C_CYAN(), 0); lv_obj_set_style_shadow_width(c, 24, 0);
    img(c, iconSrc, 22, 30);
    label(c, title, 92, 18, FONT_H2, C_BLUE(), 170);
    label(c, body, 92, 55, FONT_SMALL, C_BLUE(), 175);
}

// Screens
void LarniaUi::showSplash()
{
    _currentPage = PAGE_SPLASH; _backCount = 0; clear(C_WHITE());
    img(_root, "S:/ui/splash_logo.png", 364, 145);
    button(_root, "Pokračovať", 412, 510, 200, 44, C_CYAN(), goProfilesCb);
}
void LarniaUi::showProfiles()
{
    _currentPage = PAGE_PROFILES; clear(C_WHITE());
    header("Moje profily", "Spravujte existujúce profily alebo pridajte nového člena.", false, false);
    button(_root, "Odstrániť", 45, 170, 162, 40, C_BLUE2(), deleteProfileCb);
    button(_root, "+ Vytvoriť profil", 224, 170, 267, 40, C_CYAN(), openProfileModalCb);
    if (!_storage || _storage->profiles().empty()) {
        lv_obj_t* empty = card(_root, 45, 275, 420, 245, 20, true, openProfileModalCb);
        lv_obj_set_style_bg_color(empty, C_EMPTY(), 0); lv_obj_set_style_border_width(empty, 4, 0); lv_obj_set_style_border_color(empty, C_CYAN(), 0);
        img(empty, "S:/ui/profile_icon_110.png", 155, 35);
        label(empty, "Zatiaľ žiadny profil.", 80, 150, FONT_H2, C_BLUE(), 300);
        label(empty, "Začni tlačidlom + Vytvoriť profil.", 45, 185, FONT_BODY, C_BLUE(), 335);
        return;
    }
    int i=0; for (auto& p : _storage->profiles()) { addProfileCard(p, 45 + (i%4)*239, 275 + (i/4)*230); if (++i>=4) break; }
}
void LarniaUi::showTeachers()
{
    _currentPage = PAGE_TEACHERS; clear(C_WHITE());
    header("Môj smart učiteľ", "Váš prístup k interaktívnemu vzdelávaniu začína tu.", false, true);
    button(_root, "Odstrániť", 75, 170, 162, 40, C_BLUE2(), deleteTeacherCb);
    button(_root, "+ Vytvoriť učiteľa", 254, 170, 245, 40, C_CYAN(), openTeacherModalCb);
    if (!_storage || _storage->teachers().empty()) { addEmptyState("Zatiaľ žiadny učiteľ.", "Vytvor si vlastného AI učiteľa.", openTeacherModalCb); return; }
    int i=0; for (auto& t : _storage->teachers()) { addTeacherCard(t, 69 + (i%3)*314, 245 + (i/3)*300); if (++i>=3) break; }
}
void LarniaUi::showTeacherDetail(const String& teacherId)
{
    _currentPage = PAGE_DETAIL; _activeTeacherId = teacherId; clear(lv_color_hex(0x08233A));
    img(_root, "S:/ui/bg_detail_1024.png", 0, 0);
    String title = teacherName(teacherId); header(title.c_str(), nullptr, true, true);
    img(_root, "S:/ui/ai_core_270.png", 377, 190);
    addDetailCard(45, 300, "Testy", "Prever svoje vedomosti a sleduj doterajší progres v učení.", "S:/ui/icon_tests_55.png", goTestsCb);
    addDetailCard(690, 300, "Chat", "Analyzuj učivo a objavuj nové súvislosti cez správy.", "S:/ui/icon_chat_55.png", goChatCb);
    addDetailCard(45, 430, "Materiály", "Všetky potrebné študijné podklady na jednom mieste.", "S:/ui/icon_materials_55.png", goMaterialsCb);
    addDetailCard(690, 430, "Nastavenia", "Prispôsob si prostredie a spôsob učenia.", "S:/ui/icon_settings_55.png", goSettingsCb);
}
void LarniaUi::showTests(const String& teacherId)
{
    _currentPage = PAGE_TESTS; _activeTeacherId = teacherId; clear(C_WHITE());
    header("Testy", "Testy budú dostupné po pridaní študijných materiálov.", false, true);
    lv_obj_t* p = card(_root, 45, 240, 934, 220, 20, false, nullptr);
    lv_obj_set_style_bg_color(p, C_EMPTY(), 0); lv_obj_set_style_border_color(p, C_CYAN(), 0); lv_obj_set_style_border_width(p, 4, 0);
    label(p, "Zatiaľ žiadny test.", 330, 70, FONT_H1, C_BLUE(), 360);
    label(p, "Po pridaní materiálov ich backend môže použiť na tvorbu testov.", 230, 125, FONT_H2, C_BLUE(), 520);
}
void LarniaUi::showChat(const String& teacherId)
{
    _currentPage = PAGE_CHAT; _activeTeacherId = teacherId; clear(C_WHITE());
    header("Chat", "Napíš otázku pre AI učiteľa.", false, true);
    lv_obj_t* p = card(_root, 45, 220, 934, 250, 20, false, nullptr);
    label(p, "AI učiteľ", 30, 28, FONT_H2, C_BLUE(), 250);
    _answerLabel = label(p, "Tu sa zobrazí odpoveď po odoslaní otázky.", 30, 80, FONT_BODY, C_BLUE(), 850);
    _chatTa = textarea(_root, 45, 505, 820, 56, "Napíš otázku...", false);
    button(_root, ">", 888, 505, 70, 56, C_CYAN(), sendChatCb);
}
void LarniaUi::showMaterials(const String& teacherId)
{
    _currentPage = PAGE_MATERIALS; _activeTeacherId = teacherId; clear(C_WHITE());
    header("Knižnica materiálov", "Správa dokumentov a zdrojov pre AI učiteľa.", false, true);
    button(_root, "Pridať URL", 75, 170, 215, 40, C_BLUE2(), openUrlModalCb);
    button(_root, "Nahrať súbor", 306, 170, 215, 40, C_CYAN(), openFileModalCb);
    int count=0; if (_storage) for (auto& m : _storage->materials()) if (!_activeTeacherId.length() || m.teacherId == _activeTeacherId) count++;
    if (!count) {
        lv_obj_t* empty = card(_root, 45, 275, 932, 245, 20, false, nullptr);
        lv_obj_set_style_bg_color(empty, C_EMPTY(), 0); lv_obj_set_style_border_color(empty, C_CYAN(), 0); lv_obj_set_style_border_width(empty, 4, 0);
        img(empty, "S:/ui/icon_doc_60.png", 436, 36);
        label(empty, "Zatiaľ žiadny dokument.", 290, 110, FONT_H1, C_BLUE(), 520);
        label(empty, "Podporované: PDF, DOCX, TXT (max. 10 MB)", 220, 160, FONT_H2, C_BLUE(), 620);
        return;
    }
    lv_obj_t* box = card(_root, 45, 275, 935, 285, 20, false, nullptr);
    int y=18, shown=0; for (auto& m : _storage->materials()) {
        if (_activeTeacherId.length() && m.teacherId != _activeTeacherId) continue;
        const char* ic = (m.type=="url") ? "S:/ui/icon_url_35.png" : (m.type=="pdf" ? "S:/ui/icon_pdf_35.png" : "S:/ui/icon_file_35.png");
        img(box, ic, 24, y); label(box, m.title.c_str(), 72, y+8, FONT_BODY, lv_color_hex(0x000000), 720);
        lv_obj_t* del = card(box, 850, y-6, 46, 46, 10, true, deleteMaterialCb); lv_obj_set_style_shadow_width(del,0,0); img(del,"S:/ui/icon_delete_32.png",7,7);
        y += 47; if (++shown>=5) break;
    }
}

// Settings
void LarniaUi::showSettings()
{
    _currentPage = PAGE_SETTINGS; clear(C_WHITE());
    header("Nastavenia", "Avatar, AI a všeobecné nastavenia zariadenia.", false, true);
    addTabs("Avatar");
    lv_obj_t* p = card(_root, 45, 230, 934, 330, 20, false, nullptr);
    label(p, "Avatar", 30, 25, FONT_H2, C_BLUE(), 220);
    label(p, "Výber avatara zo SD karty", 30, 78, FONT_BODY, C_BLUE(), 300);
    label(p, "Pohlavie: žena / muž", 30, 118, FONT_BODY, C_BLUE(), 300);
    label(p, "Hlas AI učiteľa", 30, 158, FONT_BODY, C_BLUE(), 300);
    label(p, "Animácia úst", 30, 198, FONT_BODY, C_BLUE(), 300);
    addToggle(p, 350, 190, true);
    img(p, "S:/ui/profile_icon_120.png", 665, 55);
    button(p, "Otvoriť knižnicu avatarov", 610, 205, 250, 44, C_CYAN(), goAvatarsCb);
}
void LarniaUi::showSettingsAI()
{
    _currentPage = PAGE_SETTINGS_AI; clear(C_WHITE());
    header("Nastavenia", "Prispôsob správanie AI učiteľa a spôsob odpovedania.", false, true);
    addTabs("AI");

    LarniaSettings* st = _storage ? &_storage->settings() : nullptr;

    lv_obj_t* left = card(_root,45,236,455,309,20,false,nullptr);
    label(left,"Identita AI učiteľa",30,26,FONT_H2,C_BLUE(),320);
    label(left,"Názov asistenta",30,82,FONT_SMALL,C_BLUE(),160);
    _aiAssistantTa = textarea(left,30,105,210,45,"Larnia",false);
    if (st && st->assistantName.length()) lv_textarea_set_text(_aiAssistantTa, st->assistantName.c_str());

    label(left,"Jazyk odpovedí",260,82,FONT_SMALL,C_BLUE(),160);
    field(left,260,105,165,"Slovenčina",false);
    label(left,"Štýl odpovedí",30,178,FONT_SMALL,C_BLUE(),160);
    field(left,30,202,395,"Priateľský a odborný",false);

    lv_obj_t* learn = card(_root,524,236,455,144,20,false,nullptr);
    label(learn,"Spôsob učenia",30,26,FONT_H2,C_BLUE(),300);
    label(learn,"Používať materiály predmetu",30,72,FONT_BODY,C_BLUE(),300); addToggle(learn,335,68,st ? st->useMaterials : true);
    label(learn,"Q&A režim",30,108,FONT_BODY,C_BLUE(),300); addToggle(learn,335,104,true);

    lv_obj_t* form = card(_root,524,405,455,140,20,false,nullptr);
    label(form,"Dĺžka a forma odpovede",30,26,FONT_H2,C_BLUE(),330);
    String len = st ? st->responseLength : "medium";
    addPill(form,30,78,95,"Krátka",len=="short");
    addPill(form,140,78,110,"Stredná",len!="short" && len!="long");
    addPill(form,265,78,120,"Podrobná",len=="long");

    button(_root,"Uložiť nastavenia",781,552,198,34,C_CYAN(),saveSettingsCb);
}

void LarniaUi::showSettingsGeneral()
{
    _currentPage = PAGE_SETTINGS_GENERAL; clear(C_WHITE());
    header("Nastavenia", "Všeobecné nastavenia zariadenia Larnia.", false, true);
    addTabs("Generálne");

    LarniaSettings* st = _storage ? &_storage->settings() : nullptr;

    lv_obj_t* dev = card(_root,45,236,455,144,20,false,nullptr);
    label(dev,"Zariadenie",30,26,FONT_H2,C_BLUE(),300);
    label(dev,"Názov zariadenia",30,78,FONT_SMALL,C_BLUE(),160);
    _deviceNameTa = textarea(dev,30,100,210,42,"Larnia",false);
    if (st && st->deviceName.length()) lv_textarea_set_text(_deviceNameTa, st->deviceName.c_str());

    label(dev,"Jas displeja",260,78,FONT_SMALL,C_BLUE(),160);
    _brightnessSlider = lv_slider_create(dev);
    lv_obj_set_pos(_brightnessSlider,260,112);
    lv_obj_set_size(_brightnessSlider,160,14);
    lv_slider_set_range(_brightnessSlider,10,100);
    lv_slider_set_value(_brightnessSlider,st ? st->displayBrightness : 75,LV_ANIM_OFF);

    lv_obj_t* wifi = card(_root,524,236,455,144,20,false,nullptr);
    label(wifi,"Wi‑Fi",30,26,FONT_H2,C_BLUE(),300);
    label(wifi,"Wi‑Fi SSID",30,78,FONT_SMALL,C_BLUE(),160);
    _wifiSsidTa = textarea(wifi,30,100,190,42,"Názov siete",false);
    if (st && st->wifiSsid.length()) lv_textarea_set_text(_wifiSsidTa, st->wifiSsid.c_str());

    label(wifi,"Heslo",245,78,FONT_SMALL,C_BLUE(),160);
    _wifiPasswordTa = textarea(wifi,245,100,180,42,"Heslo",false);
    lv_textarea_set_password_mode(_wifiPasswordTa, true);
    if (st && st->wifiPassword.length()) lv_textarea_set_text(_wifiPasswordTa, st->wifiPassword.c_str());

    lv_obj_t* back = card(_root,45,405,455,140,20,false,nullptr);
    label(back,"Backend",30,26,FONT_H2,C_BLUE(),300);
    label(back,"Backend IP",30,78,FONT_SMALL,C_BLUE(),160);
    _backendHostTa = textarea(back,30,100,195,42,"192.168.1.100",false);
    if (st && st->backendHost.length()) lv_textarea_set_text(_backendHostTa, st->backendHost.c_str());

    label(back,"Port",245,78,FONT_SMALL,C_BLUE(),80);
    _backendPortTa = textarea(back,245,100,90,42,"3000",true);
    if (st) { String port = String(st->backendPort); lv_textarea_set_text(_backendPortTa, port.c_str()); }
    label(back,"Auto reconnect",350,105,FONT_SMALL,C_BLUE(),120);
    addToggle(back,350,70,st ? st->autoReconnect : true);

    lv_obj_t* other = card(_root,524,405,455,140,20,false,nullptr);
    label(other,"Ostatné",30,26,FONT_H2,C_BLUE(),300);
    label(other,"Bluetooth audio výstup",30,78,FONT_BODY,C_BLUE(),250); addToggle(other,330,72,st ? st->bluetoothEnabled : true);
    label(other,"Aktivácia dotykom",30,113,FONT_BODY,C_BLUE(),250); addToggle(other,330,107,st ? st->wakeOnTouch : true);

    _statusLabel = label(_root, "Po uložení sa panel pokúsi pripojiť na Wi‑Fi a backend.", 45, 565, FONT_SMALL, C_BLUE(), 660);
    button(_root,"Uložiť a pripojiť",781,552,198,34,C_CYAN(),saveSettingsCb);
}

void LarniaUi::showAvatarLibrary()
{
    _currentPage = PAGE_AVATARS; clear(C_WHITE());
    header("Knižnica avatarov", "Avatary sú uložené na SD karte a používateľ ich môže priradiť.", false, true);
    button(_root, "+ Pridať avatara", 75, 150, 200, 40, C_CYAN(), nullptr);
    for (int i=0;i<8;i++) { int x=60+(i%4)*235, y=230+(i/4)*155; lv_obj_t* c=card(_root,x,y,180,130,20,true,nullptr); img(c,avatarSrcByIndex(i),49,12); String text=String("Avatar ")+String(i+1); label(c,text.c_str(),45,100,FONT_SMALL,C_BLUE(),100); }
}
void LarniaUi::showPanelAnswer(const String& text, const String& avatarId)
{
    clear(lv_color_hex(0x08233A)); img(_root,"S:/ui/bg_empty_1024.png",0,0);
    label(_root,"Larnia",45,70,FONT_BIG,C_CYAN2(),300); label(_root,_online?"Online":"Offline",45,120,FONT_BODY_BOLD,C_CYAN2(),160);
    lv_obj_t* avatarBox = card(_root,370,145,291,330,30,false,nullptr); lv_obj_set_style_bg_color(avatarBox,C_CYAN2(),0);
    img(avatarBox,avatarId.length()?avatarSrc(avatarId):"S:/ui/ai_core_270.png",11,12);
    _mouth = lv_obj_create(avatarBox); lv_obj_set_size(_mouth,80,10); lv_obj_set_pos(_mouth,105,260); lv_obj_set_style_radius(_mouth,8,0); lv_obj_set_style_bg_color(_mouth,C_BLUE(),0); lv_obj_set_style_border_width(_mouth,0,0); lv_obj_add_flag(_mouth,LV_OBJ_FLAG_HIDDEN);
    lv_obj_t* ans=card(_root,150,500,725,70,20,false,nullptr); _answerLabel=label(ans,text.c_str(),25,18,FONT_BODY,C_BLUE(),675);
}

// Modals
void LarniaUi::showNewProfileModal()
{
    lv_obj_t* m=modal(462); img(m,"S:/ui/profile_icon_110.png",35,30); label(m,"Nový profil",114,46,FONT_H2,C_BLUE(),350);
    label(m,"Meno",40,105,FONT_H2,C_BLUE(),200); _profileNameTa=textarea(m,40,139,420,56,"napr. Mária",false);
    label(m,"Vek",40,225,FONT_H2,C_BLUE(),200); _profileAgeTa=textarea(m,40,259,420,56,"napr. 13",true);
    button(m,"Zrušiť",40,370,136,52,C_BLUE2(),closeModalCb); button(m,"+ Vytvoriť profil",188,370,272,52,C_CYAN(),createProfileCb);
}
void LarniaUi::showNewTeacherModal()
{
    lv_obj_t* m=modal(455); img(m,"S:/ui/profile_icon_110.png",35,30); label(m,"Nový učiteľ",114,46,FONT_H2,C_BLUE(),350);
    label(m,"Názov učiteľa / predmetu",40,105,FONT_H2,C_BLUE(),360); _teacherNameTa=textarea(m,40,139,420,56,"napr. Anglický jazyk",false);
    label(m,"Avatar",40,225,FONT_H2,C_BLUE(),200); field(m,40,259,420,"bez avatara alebo výber zo SD karty",false);
    button(m,"Zrušiť",40,372,136,52,C_BLUE2(),closeModalCb); button(m,"+ Vytvoriť učiteľa",188,372,272,52,C_CYAN(),createTeacherCb);
}
void LarniaUi::showAddUrlModal()
{
    lv_obj_t* m=modal(360); img(m,"S:/ui/icon_url_35.png",40,40); label(m,"Pridať webový odkaz",88,43,FONT_H2,C_BLUE(),360);
    label(m,"AI učiteľ analyzuje obsah odkazu a zahrnie ho do tvojich študijných materiálov.",40,103,FONT_BODY,C_BLUE(),407);
    _urlTa=textarea(m,40,180,420,56,"https://priklad.sk/clanok-na-studium",false);
    button(m,"Zrušiť",40,282,136,52,C_BLUE2(),closeModalCb); button(m,"+ Pridať URL",188,282,272,52,C_CYAN(),addUrlCb);
}
void LarniaUi::showAddFileModal(const char* errorText)
{
    lv_obj_t* m=modal(errorText?420:360); img(m,"S:/ui/icon_file_35.png",40,40); label(m,"Pridať súbor",88,43,FONT_H2,C_BLUE(),360);
    label(m,"Nahraj študijné podklady a rozšír vedomosti svojho AI učiteľa.",40,103,FONT_BODY,C_BLUE(),407);
    int y=180; if(errorText){label(m,errorText,40,152,FONT_BODY_BOLD,C_ERROR(),407); y=226;}
    field(m,40,y,420,"Vybrať súbor zo zariadenia (PDF, DOCX, TXT)",errorText!=nullptr);
    button(m,"Zrušiť",40,errorText?322:282,136,52,C_BLUE2(),closeModalCb); button(m,"+ Nahrať súbor",188,errorText?322:282,272,52,C_CYAN(),addFileCb);
}

// Callbacks
void LarniaUi::backCb(lv_event_t* e){auto*self=static_cast<LarniaUi*>(lv_event_get_user_data(e)); if(self) self->goBack();}
void LarniaUi::textareaFocusCb(lv_event_t* e)
{
    auto* self = static_cast<LarniaUi*>(lv_event_get_user_data(e));
    if (!self) return;

    lv_obj_t* ta = lv_event_get_target(e);
    self->makeKeyboard();
    if (!self->_keyboard) return;

    g_larnia_active_ta = ta;
    lv_keyboard_set_textarea(self->_keyboard, ta);

#if defined(LV_KEYBOARD_MODE_NUMBER)
    if (ta == self->_profileAgeTa) lv_keyboard_set_mode(self->_keyboard, LV_KEYBOARD_MODE_NUMBER);
    else lv_keyboard_set_mode(self->_keyboard, LV_KEYBOARD_MODE_TEXT_LOWER);
#endif

    lv_obj_clear_flag(self->_keyboard, LV_OBJ_FLAG_HIDDEN);
    larniaMakeQuickbar(self->_root);
    lv_obj_move_foreground(self->_keyboard);
    self->wake();
}
void LarniaUi::keyboardCb(lv_event_t* e)
{
    auto* self = static_cast<LarniaUi*>(lv_event_get_user_data(e));
    if (!self) return;
    self->hideKeyboard();
}
void LarniaUi::goProfilesCb(lv_event_t* e){auto*self=static_cast<LarniaUi*>(lv_event_get_user_data(e)); if(!self)return; self->pushCurrent(); self->showProfiles();}
void LarniaUi::goTeachersCb(lv_event_t* e){auto*self=static_cast<LarniaUi*>(lv_event_get_user_data(e)); if(!self)return; self->pushCurrent(); self->showTeachers();}
void LarniaUi::goDetailCb(lv_event_t* e){auto*self=static_cast<LarniaUi*>(lv_event_get_user_data(e)); if(!self||!self->_storage||self->_storage->teachers().empty())return; self->pushCurrent(); self->showTeacherDetail(self->_storage->teachers().front().id);}
void LarniaUi::goChatCb(lv_event_t* e){auto*self=static_cast<LarniaUi*>(lv_event_get_user_data(e)); if(!self)return; self->pushCurrent(); self->showChat(self->_activeTeacherId);}
void LarniaUi::goMaterialsCb(lv_event_t* e){auto*self=static_cast<LarniaUi*>(lv_event_get_user_data(e)); if(!self)return; self->pushCurrent(); self->showMaterials(self->_activeTeacherId);}
void LarniaUi::goSettingsCb(lv_event_t* e){auto*self=static_cast<LarniaUi*>(lv_event_get_user_data(e)); if(!self)return; self->pushCurrent(); self->showSettings();}
void LarniaUi::goSettingsAICb(lv_event_t* e){auto*self=static_cast<LarniaUi*>(lv_event_get_user_data(e)); if(self) self->showSettingsAI();}
void LarniaUi::goSettingsGeneralCb(lv_event_t* e){auto*self=static_cast<LarniaUi*>(lv_event_get_user_data(e)); if(self) self->showSettingsGeneral();}
void LarniaUi::goTestsCb(lv_event_t* e){auto*self=static_cast<LarniaUi*>(lv_event_get_user_data(e)); if(!self)return; self->pushCurrent(); self->showTests(self->_activeTeacherId);}
void LarniaUi::goAvatarsCb(lv_event_t* e){auto*self=static_cast<LarniaUi*>(lv_event_get_user_data(e)); if(!self)return; self->pushCurrent(); self->showAvatarLibrary();}
void LarniaUi::openProfileModalCb(lv_event_t* e){auto*self=static_cast<LarniaUi*>(lv_event_get_user_data(e)); if(self) self->showNewProfileModal();}
void LarniaUi::openTeacherModalCb(lv_event_t* e){auto*self=static_cast<LarniaUi*>(lv_event_get_user_data(e)); if(self) self->showNewTeacherModal();}
void LarniaUi::openUrlModalCb(lv_event_t* e){auto*self=static_cast<LarniaUi*>(lv_event_get_user_data(e)); if(self) self->showAddUrlModal();}
void LarniaUi::openFileModalCb(lv_event_t* e){auto*self=static_cast<LarniaUi*>(lv_event_get_user_data(e)); if(self) self->showAddFileModal(nullptr);}
void LarniaUi::closeModalCb(lv_event_t* e){auto*self=static_cast<LarniaUi*>(lv_event_get_user_data(e)); if(!self)return; self->hideKeyboard(); self->showPage(self->_currentPage,self->_activeTeacherId);}
void LarniaUi::createProfileCb(lv_event_t* e){auto*self=static_cast<LarniaUi*>(lv_event_get_user_data(e)); if(!self||!self->_storage)return; const char* nm=self->_profileNameTa?lv_textarea_get_text(self->_profileNameTa):""; const char* ag=self->_profileAgeTa?lv_textarea_get_text(self->_profileAgeTa):""; String name=String(nm); name.trim(); if(!name.length()) name=String("Nový profil ")+String(self->_storage->profiles().size()+1); self->_storage->addProfile(name, atoi(ag)); self->hideKeyboard(); self->showProfiles();}
void LarniaUi::createTeacherCb(lv_event_t* e){auto*self=static_cast<LarniaUi*>(lv_event_get_user_data(e)); if(!self||!self->_storage)return; const char* nm=self->_teacherNameTa?lv_textarea_get_text(self->_teacherNameTa):""; String name=String(nm); name.trim(); if(!name.length()) name=String("Nový učiteľ ")+String(self->_storage->teachers().size()+1); self->_storage->addTeacher(name,"",""); self->hideKeyboard(); self->showTeachers();}
void LarniaUi::addUrlCb(lv_event_t* e){auto*self=static_cast<LarniaUi*>(lv_event_get_user_data(e)); if(!self||!self->_storage)return; const char* url=self->_urlTa?lv_textarea_get_text(self->_urlTa):""; String u=String(url); u.trim(); if(!u.length()) u="https://"; self->_storage->addMaterial(self->_activeTeacherId,u,"url",u); self->hideKeyboard(); self->showMaterials(self->_activeTeacherId);}
void LarniaUi::addFileCb(lv_event_t* e){auto*self=static_cast<LarniaUi*>(lv_event_get_user_data(e)); if(!self||!self->_storage)return; String title=String("Študijný materiál ")+String(self->_storage->materials().size()+1)+".txt"; self->_storage->addMaterial(self->_activeTeacherId,title,"txt","/materials/"); self->showMaterials(self->_activeTeacherId);}
void LarniaUi::sendChatCb(lv_event_t* e)
{
    auto* self = static_cast<LarniaUi*>(lv_event_get_user_data(e));
    if (!self || !self->_answerLabel) return;

    const char* txt = self->_chatTa ? lv_textarea_get_text(self->_chatTa) : "";
    String q = String(txt);
    q.trim();

    if (!q.length()) {
        lv_label_set_text(self->_answerLabel, "Napíš otázku do poľa dole a potom ju odošli.");
        return;
    }

    String teacherId = self->_activeTeacherId;
    if (!teacherId.length() && self->_storage && !self->_storage->teachers().empty()) {
        teacherId = self->_storage->teachers().front().id;
    }

    if (!teacherId.length()) {
        lv_label_set_text(self->_answerLabel, "Najprv vytvor učiteľa alebo predmet.");
        return;
    }

    lv_label_set_text(self->_answerLabel, "Odosielam otázku backendu...");
    larniaSendChatFromUi(teacherId, q);
}

void LarniaUi::saveSettingsCb(lv_event_t* e)
{
    auto* self = static_cast<LarniaUi*>(lv_event_get_user_data(e));
    if (!self || !self->_storage) return;

    LarniaSettings& st = self->_storage->settings();

    if (self->_aiAssistantTa) {
        String name = String(lv_textarea_get_text(self->_aiAssistantTa));
        name.trim();
        if (name.length()) st.assistantName = name;
        st.language = "sk";
        st.responseLength = "medium";
        st.useMaterials = true;
    }

    if (self->_deviceNameTa) {
        String device = String(lv_textarea_get_text(self->_deviceNameTa));
        device.trim();
        if (device.length()) st.deviceName = device;
    }

    if (self->_brightnessSlider) {
        st.displayBrightness = (uint8_t)lv_slider_get_value(self->_brightnessSlider);
    }

    if (self->_wifiSsidTa) {
        String ssid = String(lv_textarea_get_text(self->_wifiSsidTa));
        ssid.trim();
        st.wifiSsid = ssid;
    }

    if (self->_wifiPasswordTa) {
        st.wifiPassword = String(lv_textarea_get_text(self->_wifiPasswordTa));
    }

    if (self->_backendHostTa) {
        String host = String(lv_textarea_get_text(self->_backendHostTa));
        host.trim();
        st.backendHost = host;
    }

    if (self->_backendPortTa) {
        int port = atoi(lv_textarea_get_text(self->_backendPortTa));
        if (port > 0) st.backendPort = (uint16_t)port;
    }

    self->_storage->saveSettingsOnly();

    if (self->_statusLabel) lv_label_set_text(self->_statusLabel, "Uložené. Pripájam Wi‑Fi/backend...");
    larniaReconnectNetworkFromUi();
}
void LarniaUi::deleteProfileCb(lv_event_t* e){auto*self=static_cast<LarniaUi*>(lv_event_get_user_data(e)); if(!self||!self->_storage)return; self->_storage->removeLastProfile(); self->showProfiles();}
void LarniaUi::deleteTeacherCb(lv_event_t* e){auto*self=static_cast<LarniaUi*>(lv_event_get_user_data(e)); if(!self||!self->_storage)return; self->_storage->removeLastTeacher(); self->showTeachers();}
void LarniaUi::deleteMaterialCb(lv_event_t* e){auto*self=static_cast<LarniaUi*>(lv_event_get_user_data(e)); if(!self||!self->_storage)return; self->_storage->removeLastMaterial(); self->showMaterials(self->_activeTeacherId);}

// State + backend events
void LarniaUi::setOnline(bool online){_online=online;}
void LarniaUi::setStatus(const String& status){if(_statusLabel) lv_label_set_text(_statusLabel,status.c_str());}
void LarniaUi::setTalking(bool talking){_talking=talking; if(_mouth){ if(talking) lv_obj_clear_flag(_mouth,LV_OBJ_FLAG_HIDDEN); else lv_obj_add_flag(_mouth,LV_OBJ_FLAG_HIDDEN);}}
void LarniaUi::update(){ if(_talking && _mouth && millis()-_lastAnim>120){_lastAnim=millis(); lv_obj_set_height(_mouth, random(8,32));}}
void LarniaUi::handleEvent(const String& type, JsonVariant data)
{
    if(type=="answer") showPanelAnswer(String(data["text"]|""), String(data["avatarId"]|_activeAvatarId));
    else if(type=="talk") setTalking(data["state"]|false);
    else if(type=="online") setOnline(data["state"]|true);
    else if(type=="show") { String page=data["page"]|""; if(page=="profiles")showProfiles(); else if(page=="teachers")showTeachers(); else if(page=="chat")showChat(String(data["teacherId"]|_activeTeacherId)); else if(page=="materials")showMaterials(String(data["teacherId"]|_activeTeacherId)); else if(page=="settings")showSettings(); else if(page=="avatars")showAvatarLibrary(); else showSplash(); }
    else if(type=="sync") { if(_storage){ _storage->loadAll(); showTeachers(); } }
}
