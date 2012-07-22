#include "stdafx.h"

namespace playlist_locks
{
    namespace
    {
        mainmenu_group_popup_factory mainmenu_group_popup_playlist_locks (
            create_guid (0x657815ee, 0x1148, 0x48cf, 0x82, 0x40, 0xcb, 0x2b, 0xfa, 0x9c, 0xcc, 0x45),
            mainmenu_groups::edit,
            0,
            "Playlist locks");
    }

    class playlist_locks_mainmenu_commands : public mainmenu_commands
    {
        t_uint32 get_command_count () override { return get_lock_manager ()->get_lock_count (); }

        GUID get_command (t_uint32 p_index) override { return get_lock_manager ()->get_lock_guid (p_index); }

        void get_name (t_uint32 p_index, pfc::string_base & p_out) override { p_out = get_lock_manager ()->get_lock_name (p_index); } 

        bool get_description (t_uint32, pfc::string_base &) override { return false; }

        GUID get_parent () override { return mainmenu_group_popup_playlist_locks.get_static_instance ().get_guid (); }
        
        bool get_display (t_uint32 p_index, pfc::string_base & p_text, t_uint32 & p_flags) override
        {
            static_api_ptr_t<lock_manager> lock_manager_api;
            p_text  = lock_manager_api->get_lock_name (p_index); // menu item text is lock name by its index
            p_flags = 0;

            static_api_ptr_t<playlist_manager> playlist_manager_api;
            t_size active_playlist = playlist_manager_api->get_active_playlist ();

            if (active_playlist == pfc_infinite)
            {
                p_flags = flag_disabled;
                return true; // Nothing to do here, but item will be drawn
            }


            if (!playlist_manager_api->playlist_lock_is_present (active_playlist))
            {
                p_flags = 0; // Enable if playlist has 0 locks
                return true;
            }

            // Если лок, да не наш
            if (!is_our_lock_name (string_utf8_from_playlist_index (active_playlist)))
            {
                p_flags = flag_disabled;
                return true;
            }

            p_flags = lock_manager_api->get_menuitem_flags (p_index, active_playlist);
            return true;
        }

        void execute (t_uint32 p_index, service_ptr_t<service_base>) override
        {
            static_api_ptr_t<playlist_manager> playlist_manager_api;
            t_size active_playlist = playlist_manager_api->get_active_playlist ();
            if (active_playlist == pfc_infinite) 
                return;

            static_api_ptr_t<lock_manager>()->playlist_lock_toggle (active_playlist, p_index);
        }
    };

    namespace
    {
        mainmenu_commands_factory_t<playlist_locks_mainmenu_commands> g_mainmenu_commands_factory;
    }
}