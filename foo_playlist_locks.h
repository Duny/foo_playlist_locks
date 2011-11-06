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

// my_library_callback_dynamic
class my_library_callback_dynamic : public library_callback_dynamic
{
public:
    virtual void on_items_added(const pfc::list_base_const_t<metadb_handle_ptr> & p_data);
	virtual void on_items_removed(const pfc::list_base_const_t<metadb_handle_ptr> & p_data);

    // not used
    virtual void on_items_modified(const pfc::list_base_const_t<metadb_handle_ptr> & p_data) {}
};
 
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

// my_mainmenu_edit_popup
class my_mainmenu_edit_popup : public mainmenu_group_popup
{
public:
	virtual GUID get_guid();
	virtual GUID get_parent();
	virtual t_uint32 get_sort_priority();

	virtual void get_display_string(pfc::string_base & p_out);
};

// my_mainmenu_commands
class my_mainmenu_commands : public mainmenu_commands
{
public:
	virtual t_uint32 get_command_count();
	virtual GUID get_command(t_uint32 p_index);
	virtual void get_name(t_uint32 p_index, pfc::string_base & p_out);
	virtual bool get_description(t_uint32 p_index, pfc::string_base & p_out);
	virtual GUID get_parent();
	virtual bool get_display(t_uint32 p_index,pfc::string_base & p_text,t_uint32 & p_flags);
	virtual void execute(t_uint32 p_index, service_ptr_t<service_base> p_callback);
};

// my_lock
class my_lock : public playlist_lock
{
public:
    virtual t_uint32 get_filter_mask() { return 0; }
    virtual void get_lock_name(pfc::string_base & p_out) { p_out.set_string(COMPONENT_NAME); }

	virtual bool query_items_add(unsigned start, const pfc::list_base_const_t<metadb_handle_ptr> & p_data,const bit_array & p_selection)
    { return true; }
	virtual bool query_items_reorder(const unsigned * order,unsigned count)
    { return true; }
	virtual bool query_items_remove(const bit_array & mask,bool p_force)
    { return true; }
	virtual bool query_item_replace(unsigned idx,const metadb_handle_ptr & p_old,const metadb_handle_ptr & p_new)
    { return true; }
	virtual bool query_playlist_rename(const char * p_new_name,unsigned p_new_name_len)
    { return true; }
	virtual bool query_playlist_remove()
    { return true; }
	virtual bool execute_default_action(unsigned p_item)
    { return false; }
    virtual void on_playlist_index_change(unsigned p_new_index) {}
    virtual void on_playlist_remove() {}
    virtual void show_ui() {}
	
};

namespace playlist_locks
{
    class playlist_lock_simple : public playlist_lock
    {
        t_uint32 get_filter_mask () override { return 0; }

        bool query_items_add (unsigned, metadb_handle_list_cref, const bit_array&) override { return true; }
        bool query_items_reorder (const unsigned*, unsigned) override { return true; }
        bool query_items_remove (const bit_array&, bool) override { return true; }
        bool query_item_replace (unsigned, const metadb_handle_ptr&, const metadb_handle_ptr&) override { return true; }
        bool query_playlist_rename (const char *, unsigned) override { return true; }
        bool query_playlist_remove () override { return true; }
        bool execute_default_action (unsigned) override { return false; }
        void on_playlist_index_change (unsigned) override {}
        void on_playlist_remove() override {}
        void show_ui () override {}
    };


    class playlist_lock_special : public playlist_lock_simple
    {
    public:
        virtual GUID get_guid () const = 0;
    };
    typedef service_ptr_t<playlist_lock_special> playlist_lock_special_ptr;


    class lock_manager : public playlist_lock_simple
    {
        friend class service_impl_t<lock_manager>;
        pfc::list_t<playlist_lock_special_ptr> m_registered_locks_list;


        void get_lock_name (pfc::string_base &p_out) { p_out.set_string (COMPONENT_NAME); }

        lock_manager () {}
    public:
        static const service_ptr_t<lock_manager> &get_instance ();

        void register_lock_type (const playlist_lock_special_ptr &p_lock);
    };

    // helper class
    template <class playlist_lock_special_t>
    class register_lock_type_t
    {
        service_ptr_t<playlist_lock_special> m_lock;
    public:
        register_lock_type_t () : m_lock (new service_impl_t<playlist_lock_special_t> ()) { lock_manager::get_instance ()->register_lock_type (m_lock); }
    };
}

// my_playlist_callback_static
class my_playlist_callback_static : public playlist_callback_static
{
public:
	virtual unsigned get_flags() { return flag_on_playlist_created; }
	virtual void on_playlist_created(t_size p_index,const char * p_name,t_size p_name_len);

	virtual void on_items_added(t_size p_playlist,t_size p_start, const pfc::list_base_const_t<metadb_handle_ptr> & p_data,const bit_array & p_selection) {}
	virtual void on_items_reordered(t_size p_playlist,const t_size * p_order,t_size p_count) {}
	virtual void on_items_removing(t_size p_playlist,const bit_array & p_mask,t_size p_old_count,t_size p_new_count) {}
	virtual void on_items_removed(t_size p_playlist,const bit_array & p_mask,t_size p_old_count,t_size p_new_count) {}
	virtual void on_items_selection_change(t_size p_playlist,const bit_array & p_affected,const bit_array & p_state) {}
	virtual void on_item_focus_change(t_size p_playlist,t_size p_from,t_size p_to) {}
	virtual void on_items_modified(t_size p_playlist,const bit_array & p_mask) {}
	virtual void on_items_modified_fromplayback(t_size p_playlist,const bit_array & p_mask,play_control::t_display_level p_level) {}
	virtual void on_items_replaced(t_size p_playlist,const bit_array & p_mask,const pfc::list_base_const_t<t_on_items_replaced_entry> & p_data) {}
	virtual void on_item_ensure_visible(t_size p_playlist,t_size p_idx) {}
	virtual void on_playlist_activate(t_size p_old,t_size p_new) {}
    virtual void on_playlists_reorder(const t_size * p_order,t_size p_count) {}
	virtual void on_playlists_removing(const bit_array & p_mask,t_size p_old_count,t_size p_new_count) {}
	virtual void on_playlists_removed(const bit_array & p_mask,t_size p_old_count,t_size p_new_count);
	virtual void on_playlist_renamed(t_size p_index,const char * p_new_name,t_size p_new_name_len) {}
	virtual void on_default_format_changed() {}
	virtual void on_playback_order_changed(t_size p_new_index) {}
	virtual void on_playlist_locked(t_size p_playlist,bool p_locked) {}
};

// my_initquit
class my_initquit : public initquit
{
    my_library_callback_dynamic m_library_callback_dynamic;
    my_play_callback m_play_callback;
public:
    void on_init();
    void on_quit();
};