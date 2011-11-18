#include "stdafx.h"

namespace playlist_locks
{
    struct remove_item_thread : main_thread_callback
    {
        metadb_handle_ptr m_item;
        playlist_lock_special_ptr m_lock;

        void callback_run () override
        {
            remove_track_from_playlists (m_item, m_lock);
        }

        void get_remove_mask (
            const pfc::list_base_const_t<metadb_handle_ptr> &p_playlist_items,
            const metadb_handle_ptr &p_item,
            pfc::bit_array_var_impl &p_out)
        {
            auto total_items = p_playlist_items.get_size ();
            while (total_items --> 0)
                if (p_item == p_playlist_items[total_items])
                    p_out.set (total_items);
        }

        void remove_track_from_playlists (const metadb_handle_ptr &p_item, playlist_lock_special_ptr p_lock)
        {
            pfc::list_t<t_size> playlists;
            static_api_ptr_t<lock_manager>()->get_playlists (p_lock, playlists);

            static_api_ptr_t<playlist_manager> api;
            auto n = playlists.get_size ();
            while (n --> 0) {
                metadb_handle_list playlist_items;
                api->playlist_get_all_items (playlists[n], playlist_items);

                pfc::bit_array_var_impl remove_mask;
                get_remove_mask (playlist_items, p_item, remove_mask);

                api->playlist_remove_items (playlists[n], remove_mask);
            }
        }

        remove_item_thread (const metadb_handle_ptr &p_item, playlist_lock_special_ptr p_lock) : m_item (p_item), m_lock (p_lock) {}
    };

    class remove_played : public play_callback_static_impl_simple, public playlist_lock_special
    {
        metadb_handle_ptr m_previous_track;

        //
        // playlist_lock_special overrides
        //
        void get_lock_name (pfc::string_base &p_out) const override { p_out.set_string ("Remove played"); }
        GUID get_guid () const { return guid_inline<0x62f9cebd, 0x3327, 0x483a, 0xa7, 0xb7, 0x44, 0x7d, 0x35, 0xdc, 0x7f, 0x31>::guid; };


        //
        // play_callback overrides
        //
        unsigned get_flags () override { return flag_on_playback_new_track | flag_on_playback_stop; }

        void on_playback_new_track (metadb_handle_ptr p_track) override
        {
            if (m_previous_track.is_valid () && m_previous_track != p_track)
                static_api_ptr_t<main_thread_callback_manager>()->add_callback (new service_impl_t<remove_item_thread> (m_previous_track, this));
                
            m_previous_track = p_track;
        }

        void on_playback_stop (play_control::t_stop_reason p_reason) 
        {
            // handle case of last track in playlist
            if (p_reason == play_control::stop_reason_eof) {
                static_api_ptr_t<playlist_manager> api;
                t_size playing_playlist = api->get_playing_playlist ();
                if (playing_playlist != pfc_infinite && api->playlist_get_item_count (playing_playlist) == 1)
                    static_api_ptr_t<main_thread_callback_manager>()->add_callback (new service_impl_t<remove_item_thread> (m_previous_track, this));
            }
            else if (p_reason == play_control::stop_reason_user)
                m_previous_track.detach ();
        }

    public:
        ~remove_played () { m_previous_track.detach (); }
    };

    static play_callback_static_factory_t<remove_played> g_remove_played_lock;
}