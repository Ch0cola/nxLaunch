#include <ui/ui_PowerMenu.hpp>
#include <ui/ui_MenuApplication.hpp>
#include <cfg/cfg_Config.hpp>
#include <am/am_DaemonMessages.hpp>
#include <os/os_HomeMenu.hpp>

extern cfg::Config g_Config;
extern cfg::Theme g_Theme;
extern ui::MenuApplication::Ref g_MenuApplication;

namespace ui {
    static Mutex g_power_menu_home_lock = EmptyMutex;
    static bool g_power_menu_home_pressed = false;

    PowerMenu::PowerMenu(const std::string &main_icon) {
        this->on = false;
        this->bgalpha = 0;

        auto textclr = pu::ui::Color::FromHex(g_MenuApplication->GetUIConfigValue<std::string>("text_color", "#e1e1e1ff"));
        auto menufocusclr = pu::ui::Color::FromHex(g_MenuApplication->GetUIConfigValue<std::string>("menu_focus_color", "#555555ff"));
        auto menubgclr = pu::ui::Color::FromHex(g_MenuApplication->GetUIConfigValue<std::string>("menu_bg_color", "#ffff00ff"));

        this->options_menu = pu::ui::elm::Menu::New(200, 115, 880, menubgclr, 60, 8);
        this->options_menu->SetOnFocusColor(menufocusclr);
        g_MenuApplication->ApplyConfigForElement("power_menu", "power_menu_item", this->options_menu);
        
        auto opt_item = pu::ui::elm::MenuItem::New(cfg::GetLanguageString(g_Config.main_lang, g_Config.default_lang, "power_sleep"));
        opt_item->SetIcon(cfg::GetAssetByTheme(g_Theme, "ui/PowerSleepIcon.png"));
        opt_item->AddOnClick(&actions::PowerSleep);
        opt_item->SetColor(textclr);
        this->options_menu->AddItem(opt_item);

        opt_item = pu::ui::elm::MenuItem::New(cfg::GetLanguageString(g_Config.main_lang, g_Config.default_lang, "power_power_off"));
        opt_item->SetIcon(cfg::GetAssetByTheme(g_Theme, "ui/PowerShutdownIcon.png"));
        opt_item->AddOnClick(&actions::PowerShutdown);
        opt_item->SetColor(textclr);
        this->options_menu->AddItem(opt_item);

        opt_item = pu::ui::elm::MenuItem::New(cfg::GetLanguageString(g_Config.main_lang, g_Config.default_lang, "power_reboot"));
        opt_item->SetIcon(cfg::GetAssetByTheme(g_Theme, "ui/PowerRebootIcon.png"));
        opt_item->AddOnClick(&actions::PowerReboot);
        opt_item->SetColor(textclr);
        this->options_menu->AddItem(opt_item);

        opt_item = pu::ui::elm::MenuItem::New(cfg::GetLanguageString(g_Config.main_lang, g_Config.default_lang, "power_reboot_to_payload"));
        opt_item->SetIcon(cfg::GetAssetByTheme(g_Theme, "ui/PowerRebootPayloadIcon.png"));
        opt_item->AddOnClick(&actions::PowerRebootPayload);
        opt_item->SetColor(textclr);
        this->options_menu->AddItem(opt_item);
    }

    s32 PowerMenu::GetX() {
        return 0;
    }

    s32 PowerMenu::GetY() {
        return 0;
    }

    s32 PowerMenu::GetWidth() {
        return 1280;
    }

    s32 PowerMenu::GetHeight() {
        return 720;
    }

    void PowerMenu::Toggle() {
        this->on = !this->on;
    }
    
    bool PowerMenu::IsOn() {
        return this->on && (this->bgalpha > 0);
    }

    void PowerMenu::OnRender(pu::ui::render::Renderer::Ref &drawer, s32 x, s32 y) {
        if (!this->on) {
            if (this->bgalpha > 0) {
                this->bgalpha -= 20;

                if (this->bgalpha < 0) {
                    this->bgalpha = 0;
                }
            }
        }
        else {
            if (this->bgalpha < 220) {
                this->bgalpha += 20;

                if (this->bgalpha > 220) {
                    this->bgalpha = 220;
                }
            }
        }

        this->options_menu->SetVisible(this->bgalpha != 0);

        auto bgalphau8 = static_cast<u8>(this->bgalpha);

        drawer->RenderRectangleFill({ 50, 50, 50, bgalphau8 }, 0, 0, 1280, 720);

        if (this->bgalpha > 0) {
            if (this->bgalpha < 220) {
                drawer->SetBaseRenderAlpha(bgalphau8);
            }

            this->options_menu->OnRender(drawer, this->options_menu->GetProcessedX(), this->options_menu->GetProcessedY());

            if (this->bgalpha < 220) {
                drawer->UnsetBaseRenderAlpha();
            }
        }
    }

    void PowerMenu::OnInput(u64 down, u64 up, u64 held, pu::ui::Touch touch_pos) {
        if (this->on) {
            this->options_menu->OnInput(down, up, held, touch_pos);
        }

        if ((down & KEY_B) || (down & KEY_A)) {
            // B only valid for toggling off
            // A = something selected in the menu
            if (this->on) {
                this->Toggle();
            }
        }
        else {
            if (this->on) {
                mutexLock(&g_power_menu_home_lock);
                auto home_pressed = g_power_menu_home_pressed;
                g_power_menu_home_pressed = false;
                mutexUnlock(&g_power_menu_home_lock);

                if (home_pressed) {
                    this->Toggle();
                }
            }
        }
    }

    void PowerMenuOnHomeButtonDetection() {
        mutexLock(&g_power_menu_home_lock);
        g_power_menu_home_pressed = true;
        mutexUnlock(&g_power_menu_home_lock);
    }
}
