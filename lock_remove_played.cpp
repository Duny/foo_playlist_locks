#include "stdafx.h"

namespace playlist_locks
{
    class remove_item_thread : public main_thread_callback
    {
        metadb_handle_ptr m_item;
        lock_ñptr m_lock;

        void callback_run () override
        {
            static_api_ptr_t<playlist_manager> api;
            static_api_ptr_t<lock_manager>()->for_each_playlist (m_lock, [&] (t_size playlist) 
            {
                metadb_handle_list playlist_items;
                api->playlist_get_all_items (playlist, playlist_items);

                pfc::bit_array_var_impl remove_mask;
                get_remove_mask (playlist_items, m_item, remove_mask);

                api->playlist_remove_items (playlist, remove_mask);
            });
        }

        inline void get_remove_mask (metadb_handle_list_cref p_playlist_items, const metadb_handle_ptr &p_item, pfc::bit_array_var_impl &p_out)
        {
            auto n = p_playlist_items.get_size ();
            while (n --> 0)
                if (p_item == p_playlist_items[n])
                    p_out.set (n);
        }

    public:
        remove_item_thread (const metadb_handle_ptr &p_item, lock_ñptr p_lock) : m_item (p_item), m_lock (p_lock) {}
    };

    class remove_played : public play_callback_static_impl_simple, public lock_t
    {
        // playlist_lock_special overrides
        //
        const char* get_lock_name () const override { return "Remove played"; }

        GUID get_guid () const override { return guid_inline<0x62f9cebd, 0x3327, 0x483a, 0xa7, 0xb7, 0x44, 0x7d, 0x35, 0xdc, 0x7f, 0x31>::guid; };


        //
        // play_callback overrides
        //
        unsigned get_flags () override { return flag_on_playback_new_track | flag_on_playback_stop; }

        void on_playback_new_track (metadb_handle_ptr p_track) override
        {
            if (m_previous_track.is_valid ()) {
                if (m_previous_track != p_track)
                    remove_previous ();
                // Handle case of repeating last item in playlist
                // (on_playback_stop is not called in this case)
                else if (m_previous_track == p_track && playing_pls_item_count () == 1) {
                    remove_previous ();
                    run_from_main_thread ([] () { static_api_ptr_t<playback_control>()->stop (); });
                }
            }

            m_previous_track = p_track;
        }

        void on_playback_stop (play_control::t_stop_reason p_reason) override
        {
            // handle case of last track in playlist
            if (p_reason == play_control::stop_reason_eof && playing_pls_item_count () == 1)
                remove_previous ();
            else if (p_reason == play_control::stop_reason_user)
                m_previous_track.detach ();
        }

        // Helpers
        inline t_size playing_pls_item_count () const
        {
            static_api_ptr_t<playlist_manager> api;
            auto playing_playlist = api->get_playing_playlist ();
            return api->playlist_get_item_count (playing_playlist);
        }

        inline void remove_previous () { main_thread_callback_spawn<remove_item_thread> (m_previous_track, this); }

        // Member variables
        metadb_handle_ptr m_previous_track;

    public:
        ~remove_played () { m_previous_track.detach (); }
    };

    namespace { play_callback_static_factory_t<remove_played> g_factory; }
}