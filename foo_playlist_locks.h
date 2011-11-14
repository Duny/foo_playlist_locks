#pragma once
 
typedef static_api_ptr_t<playlist_manager> t_pm;
typedef static_api_ptr_t<playlist_manager_v2> t_pm_v2;
typedef static_api_ptr_t<autoplaylist_manager> t_apm;
typedef static_api_ptr_t<library_manager_v3> t_lm_v3;

extern const GUID my_playlist_property;
extern service_ptr_t<playlist_lock> g_lock;

bool get_playlist_remove_mask(
	const pfc::list_base_const_t<metadb_handle_ptr> &p_playlist_items,
	const pfc::list_base_const_t<metadb_handle_ptr> &p_items_to_remove,
	pfc::bit_array_var_impl &p_out);


//
// callbacks
//


 
// my_play_callback
class my_play_callback : public play_callback
{
    metadb_handle_ptr m_prev_track_played;

public:
	~my_play_callback() { m_prev_track_played.detach();	}

    virtual unsigned get_flags() { return flag_on_playback_new_track | flag_on_playback_stop; }
    virtual void on_playback_new_track(metadb_handle_ptr p_track);
	virtual void on_playback_stop(play_control::t_stop_reason p_reason);
	
    // not used
    virtual void on_playback_starting(play_control::t_track_command p_command,bool p_paused) {}
    virtual void on_playback_seek(double p_time) {}
    virtual void on_playback_pause(bool p_state) {}
    virtual void on_playback_edited(metadb_handle_ptr p_track) {}
    virtual void on_playback_dynamic_info(const file_info & p_info) {}
    virtual void on_playback_dynamic_info_track(const file_info & p_info) {}
    virtual void on_playback_time(double p_time) {}
    virtual void on_volume_change(float p_new_val) {}
};