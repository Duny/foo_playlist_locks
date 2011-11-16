#include "stdafx.h"

namespace playlist_locks
{
    class media_library_tracker : public playlist_lock_special
    {
        void get_lock_name (pfc::string_base &p_out) const override { p_out.set_string ("ML changes tracker"); }
        GUID get_guid () const { return guid_inline<0xe33d9a0c, 0xdec1, 0x493f, 0x9c, 0xa7, 0x81, 0x35, 0x73, 0x38, 0x16, 0x78>::guid; };
    };
    static register_playlist_lock_special_t<media_library_tracker> g_media_library_tracker_lock;


    class library_callback_impl : public library_callback_dynamic
    {
        void on_items_added (const pfc::list_base_const_t<metadb_handle_ptr> & p_data) override
        {
            pfc::list_t<t_size> playlists;
            static_api_ptr_t<lock_manager>()->get_playlists (g_media_library_tracker_lock.get_lock (), playlists);

            auto n = playlists.get_size ();
            static_api_ptr_t<playlist_manager> api;
            while (n --> 0)
                api->playlist_add_items (playlists[n], p_data, bit_array_false());
        }

        void on_items_removed (const pfc::list_base_const_t<metadb_handle_ptr> & p_data) override
        {
            pfc::list_t<t_size> playlists;
            static_api_ptr_t<lock_manager>()->get_playlists (g_media_library_tracker_lock.get_lock (), playlists);

            auto n = playlists.get_size ();
            static_api_ptr_t<playlist_manager> api;
            while (n --> 0) {
                metadb_handle_list playlist_items;
                api->playlist_get_all_items (playlists[n], playlist_items);
                bit_array_bittable remove_mask (playlist_items.get_size ());
                get_remove_mask (playlist_items, p_data, remove_mask);
                api->playlist_remove_items (playlists[n], remove_mask);
            }
        }

        // not used
        void on_items_modified (const pfc::list_base_const_t<metadb_handle_ptr> & p_data) override {}

        // helpers
        void get_remove_mask (
            const pfc::list_base_const_t<metadb_handle_ptr> &p_playlist_items,
            const pfc::list_base_const_t<metadb_handle_ptr> &p_items_to_remove,
            bit_array_bittable &p_out)
        {
            // FIXME!!!!!!
            auto n = p_items_to_remove.get_size (), m = p_playlist_items.get_size ();
            while (n --> 0) {
                for (t_size j = 0; j < m; j++) {
                    if (p_playlist_items[j] == p_items_to_remove[n]) {
                        p_out.set (j);
                    }
                }
            }
        }
    };

    namespace
    {
        class library_callback_initializer : public initquit
        {
            library_callback_impl m_callback;
            void on_init () override { static_api_ptr_t<library_manager_v3>()->register_callback (&m_callback); }
            void on_quit () override { static_api_ptr_t<library_manager_v3>()->unregister_callback (&m_callback); }
        };
        static initquit_factory_t<library_callback_initializer> g_initializer;
    }
}