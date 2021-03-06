#include "stdafx.h"

void remove_track_from_my_playlists(metadb_handle_ptr &p_track)
{
    if (!p_track.is_valid()) return;
    
    t_pm_v2 pm;
    int is_my;
    for (t_size i = 0, n = pm->get_playlist_count(); i < n; i++)
    {
        if (pm->playlist_get_property_int(i, my_playlist_property, is_my) 
            && is_my == 1) 
        {
	        metadb_handle_list items;
	        pm->playlist_get_all_items (i, items);

	        pfc::bit_array_var_impl mask;
	        if (get_playlist_remove_mask(items,
		        pfc::list_single_ref_t<metadb_handle_ptr>(p_track), mask))
		        pm->playlist_remove_items(i, mask);
        }
    }
}


class remove_track_process_callback : public threaded_process_callback
{
    metadb_handle_ptr m_handle;

public:
    remove_track_process_callback(metadb_handle_ptr p_handle) : m_handle(p_handle) {}

	virtual void on_init(HWND p_wnd) {}
    virtual void run(threaded_process_status & p_status,abort_callback & p_abort) 
    {
        Sleep (100);
    }
	virtual void on_done(HWND p_wnd,bool p_was_aborted)
    {
        remove_track_from_my_playlists(m_handle);
    }
};


void my_play_callback::on_playback_new_track(metadb_handle_ptr p_track)
{
    if (m_prev_track_played.is_valid())
    {
        service_ptr_t<threaded_process_callback> thread = 
			new service_impl_t<remove_track_process_callback>(m_prev_track_played);
        threaded_process::g_run_modeless( 
			thread,
			threaded_process::flag_show_delayed,
			core_api::get_main_window(),
			"Removing track from my autoplaylist...");
    }
    m_prev_track_played = p_track;
}

void my_play_callback::on_playback_stop(play_control::t_stop_reason p_reason)
{
    if (p_reason == play_control::stop_reason_eof ||
        p_reason == play_control::stop_reason_starting_another)
		remove_track_from_my_playlists(m_prev_track_played);

	m_prev_track_played.detach();
}