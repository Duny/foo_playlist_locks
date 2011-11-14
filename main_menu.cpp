#include "stdafx.h"

namespace playlist_locks
{
    class playlist_locks_mainmenu_commands : public mainmenu_commands
    {
        static mainmenu_group_popup_factory g_playlist_locks_group_popup;

        t_uint32 get_command_count () override { return get_lock_manager ()->get_lock_type_count (); }

        GUID get_command (t_uint32 p_index) override
        {
            if (p_index < get_lock_manager ()->get_lock_type_count ())
                return get_lock_manager ()->get_lock_type (p_index)->get_guid ();
            else
                return pfc::guid_null;
        }

        void get_name (t_uint32 p_index, pfc::string_base &p_out) override
        {
            if (p_index < get_lock_manager ()->get_lock_type_count ())
                get_lock_manager ()->get_lock_type (p_index)->get_lock_name (p_out);
        } 

        bool get_description (t_uint32, pfc::string_base&) override { return false; }

        GUID get_parent () override { return g_playlist_locks_group_popup.get_static_instance ().get_guid (); }
        
        bool get_display (t_uint32 p_index, pfc::string_base &p_text, t_uint32 &p_flags) override
        {
            p_flags = 0;

            static_api_ptr_t<playlist_manager> pm_api;
            static_api_ptr_t<lock_manager> lm_api;
            t_size active_playlist;

            // menu items available when active playlist is not autoplaylist
            // and active playlist is not locked by some other lock
            if (p_index >= lm_api->get_lock_type_count () ||
                ((active_playlist = pm_api->get_active_playlist ()) == pfc_infinite) ||
                static_api_ptr_t<autoplaylist_manager>()->is_client_present (active_playlist)) {
                p_flags |= flag_disabled;
                return true;
            }
            if (pm_api->playlist_lock_is_present (active_playlist)) {
                pfc::string8_fast playlist_lock_name;
                pm_api->playlist_lock_query_name (active_playlist, playlist_lock_name); 
                if (playlist_lock_name.find_first (LOCK_NAME) == pfc_infinite) {
                    p_flags |= flag_disabled;
                    return true;
                }
            }

            // menu item text is taken from lock implementation class
            lm_api->get_lock_type (p_index)->get_lock_name (p_text);
            
            // check menu item if lock is present on active playlist
            if (lm_api->playlist_has_lock (active_playlist, p_index))
                p_flags |= flag_checked;
            
            return true;
        }

        void execute (t_uint32 p_index, service_ptr_t<service_base>) override
        {
            static_api_ptr_t<playlist_manager> pm_api;

            t_size active_playlist;
            pfc::string8_fast lock_name;

            // menu items available when active playlist is not autoplaylist
            // and active playlist is not locked by some other lock
            if (p_index >= get_lock_manager ()->get_lock_type_count () ||
                ((active_playlist = pm_api->get_active_playlist ()) == pfc_infinite) ||
                static_api_ptr_t<autoplaylist_manager>()->is_client_present (active_playlist))
                    return;
            if (pm_api->playlist_lock_is_present (active_playlist)) {
                pfc::string8_fast playlist_lock_name;
                pm_api->playlist_lock_query_name (active_playlist, playlist_lock_name); 
                if (playlist_lock_name.find_first (LOCK_NAME) == pfc_infinite)
                    return;
            }

            get_lock_manager ()->playlist_lock_toggle (active_playlist, p_index);
        }
    };

    mainmenu_group_popup_factory playlist_locks_mainmenu_commands::g_playlist_locks_group_popup (
        guid_inline<0x657815ee, 0x1148, 0x48cf, 0x82, 0x40, 0xcb, 0x2b, 0xfa, 0x9c, 0xcc, 0x45>::guid,
        mainmenu_groups::edit, 0, "Playlist locks");

    static mainmenu_commands_factory_t<playlist_locks_mainmenu_commands> g_mainmenu_commands;
}