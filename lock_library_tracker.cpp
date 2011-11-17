#include "stdafx.h"

namespace playlist_locks
{
    class media_library_tracker : public library_callback_dynamic, public playlist_lock_special
    {
        //
        // playlist_lock_special overrides
        //
        void get_lock_name (pfc::string_base &p_out) const override { p_out.set_string ("ML changes tracker"); }
        GUID get_guid () const { return guid_inline<0xe33d9a0c, 0xdec1, 0x493f, 0x9c, 0xa7, 0x81, 0x35, 0x73, 0x38, 0x16, 0x78>::guid; };


        //
        // library_callback_dynamic overrides
        //
        void on_items_added (const pfc::list_base_const_t<metadb_handle_ptr> & p_data) override
        {
            pfc::list_t<t_size> playlists;
            static_api_ptr_t<lock_manager>()->get_playlists (this, playlists);

            static_api_ptr_t<playlist_manager> api;
            auto n = playlists.get_size ();
            while (n --> 0)
                api->playlist_add_items (playlists[n], p_data, bit_array_false());
        }

        void on_items_removed (const pfc::list_base_const_t<metadb_handle_ptr> & p_data) override
        {
            pfc::list_t<t_size> playlists;
            static_api_ptr_t<lock_manager>()->get_playlists (this, playlists);

            static_api_ptr_t<playlist_manager> api;
            auto n = playlists.get_size ();
            while (n --> 0) {
                metadb_handle_list playlist_items;
                api->playlist_get_all_items (playlists[n], playlist_items);

                pfc::bit_array_var_impl remove_mask;
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
            pfc::bit_array_var_impl &p_out)
        {
            auto total_items = p_playlist_items.get_size ();
            p_items_to_remove.enumerate ([&] (const metadb_handle_ptr &p_item)
            {
                for (t_size i = 0; i < total_items; i++)
                    if (p_item == p_playlist_items[i])
                        p_out.set (i);
            });
        }
    };

    namespace
    {
        class library_callback_initializer : public initquit
        {
            media_library_tracker m_callback;
            void on_init () override { static_api_ptr_t<library_manager_v3>()->register_callback (&m_callback); }
            void on_quit () override { static_api_ptr_t<library_manager_v3>()->unregister_callback (&m_callback); }
        };
        static initquit_factory_t<library_callback_initializer> g_initializer;
    }
}