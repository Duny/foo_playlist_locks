#include "stdafx.h"

namespace playlist_locks
{
    class playlist_locks_mainmenu_commands : public mainmenu_commands
    {
        static mainmenu_group_popup_factory g_playlist_locks_group_popup;

        t_uint32 get_command_count () override { return LOCK_COUNT; }

        GUID get_command (t_uint32 p_index) override { return get_lock_manager ()->get_lock_type (p_index)->get_guid (); }

        void get_name (t_uint32 p_index, pfc::string_base &p_out) override { p_out = get_lock_manager ()->get_lock_type (p_index)->get_lock_name (); } 

        bool get_description (t_uint32, pfc::string_base&) override { return false; }

        GUID get_parent () override { return g_playlist_locks_group_popup.get_static_instance ().get_guid (); }
        
        bool get_display (t_uint32 p_index, pfc::string_base &p_text, t_uint32 &p_flags) override
        {
            static_api_ptr_t<playlist_manager> pm_api;
            static_api_ptr_t<lock_manager> lm_api;
            auto active_playlist = pm_api->get_active_playlist ();

            p_flags = active_playlist == pfc_infinite ? flag_disabled : 0;
            p_text  = lm_api->get_lock_type (p_index)->get_lock_name ();

            if (p_flags != flag_disabled && pm_api->playlist_lock_is_present (active_playlist)) {
                pfc::string8_fast lock_name;
                if (!pm_api->playlist_lock_query_name (active_playlist, lock_name) || // error or
                    lock_name.find_first (LOCK_NAME) == pfc_infinite) // not our lock
                    p_flags |= flag_disabled;
            }
                        
            // set menu item check if lock is present on active playlist
            if (p_flags != flag_disabled && lm_api->playlist_has_lock (active_playlist, p_index))
                p_flags |= flag_checked;
            
            return !(p_flags & flag_disabled);
        }

        void execute (t_uint32 p_index, service_ptr_t<service_base>) override
        {
            get_lock_manager ()->activeplaylist_lock_toggle (p_index);
        }
    };

    mainmenu_group_popup_factory playlist_locks_mainmenu_commands::g_playlist_locks_group_popup (
        guid_inline<0x657815ee, 0x1148, 0x48cf, 0x82, 0x40, 0xcb, 0x2b, 0xfa, 0x9c, 0xcc, 0x45>::guid,
        mainmenu_groups::edit, 0, "Playlist locks");

    static mainmenu_commands_factory_t<playlist_locks_mainmenu_commands> g_mainmenu_commands;
}