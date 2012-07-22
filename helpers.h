#ifndef _FOO_PLAYLIST_LOCKS_HELPERS_H_
#define _FOO_PLAYLIST_LOCKS_HELPERS_H_

// For inplace GUID declaration
struct create_guid : public GUID
{
    create_guid (t_uint32 d1, t_uint16 d2, t_uint16 d3, t_uint8 d4, t_uint8 d5, t_uint8 d6, t_uint8 d7, t_uint8 d8, t_uint8 d9, t_uint8 d10, t_uint8 d11) 
    {
        Data1 = d1;
        Data2 = d2;
        Data3 = d3;
        Data4[0] = d4;  Data4[1] = d5;
        Data4[2] = d6;  Data4[3] = d7;
        Data4[4] = d8;  Data4[5] = d9;
        Data4[6] = d10; Data4[7] = d11;
    }
};

// Empty playlist_lock implementation. Does not implements get_lock_name () function
class playlist_lock_impl_simple : public playlist_lock
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


// helper: does not register itself. provides empty implementation of playlist_callback members only 
class playlist_callback_impl_simple : public playlist_callback {
    void on_items_added (t_size, t_size, const pfc::list_base_const_t<metadb_handle_ptr>&, const bit_array&) override {}
    void on_items_reordered (t_size, const t_size*, t_size) override {}
    void on_items_removing (t_size, const bit_array&, t_size, t_size) override {}
    void on_items_removed (t_size, const bit_array&, t_size, t_size) override {}
    void on_items_selection_change (t_size, const bit_array&, const bit_array&) override {}
    void on_item_focus_change (t_size, t_size, t_size) override {}
    void on_items_modified (t_size, const bit_array&) override {}
    void on_items_modified_fromplayback (t_size, const bit_array&, play_control::t_display_level) override {}
    void on_items_replaced (t_size, const bit_array&, const pfc::list_base_const_t<t_on_items_replaced_entry>&) override {}
    void on_item_ensure_visible (t_size, t_size) override {}
    void on_playlist_activate (t_size, t_size) override {}
    void on_playlist_created (t_size, const char*, t_size) override {}
    void on_playlists_reorder (const t_size*, t_size) override {}
    void on_playlists_removing (const bit_array&, t_size, t_size) override {}
    void on_playlists_removed (const bit_array&, t_size, t_size) override {}
    void on_playlist_renamed (t_size, const char*, t_size) override {}
    void on_default_format_changed () override {}
    void on_playback_order_changed (t_size) override {}
    void on_playlist_locked (t_size, bool) override {}
};


// Empty play_callback_static implementation
class play_callback_static_impl_simple : public play_callback_static
{
    unsigned get_flags () override { return 0; }
    void on_playback_new_track (metadb_handle_ptr p_track) override {}
    void on_playback_stop (play_control::t_stop_reason p_reason) {}
    void on_playback_starting (play_control::t_track_command p_command, bool p_paused) override {}
    void on_playback_seek (double p_time) override {}
    void on_playback_pause (bool p_state) override {}
    void on_playback_edited (metadb_handle_ptr p_track) override {}
    void on_playback_dynamic_info (const file_info & p_info) override {}
    void on_playback_dynamic_info_track (const file_info & p_info) override {}
    void on_playback_time (double p_time) override {}
    void on_volume_change (float p_new_val) override {}
};

// helper: wraps main_thread_callback
template <typename Func>
void run_from_main_thread (Func f)
{
    struct from_main_thread : main_thread_callback
    {
        void callback_run () override { f (); }
        from_main_thread (Func f) : f (f) {}
        Func f;
    };

    static_api_ptr_t<main_thread_callback_manager>()->add_callback (new service_impl_t<from_main_thread> (f));
}

template<class T>
inline void run_in_separate_thread (const T & func)
{
    class thread_dynamic {
    public:
        PFC_DECLARE_EXCEPTION (exception_creation, pfc::exception, "Could not create thread");

        thread_dynamic (const T & func, int priority) : func (func), thread (INVALID_HANDLE_VALUE)
        {
            thread = CreateThread (NULL, 0, g_entry, reinterpret_cast<void*>(this), CREATE_SUSPENDED, NULL);
            if (thread == NULL) throw exception_creation ();
            SetThreadPriority (thread, priority);
            ResumeThread (thread);
        }

    private:
        // Must be instantiated with operator new
        virtual ~thread_dynamic () { CloseHandle (thread); }

        void threadProc () { func (); delete this; }

        static DWORD CALLBACK g_entry (void* p_instance) { return reinterpret_cast<thread_dynamic*>(p_instance)->entry (); }
        unsigned entry () {
            try { threadProc (); }
            catch (...) {}
            return 0;
        }

        T func;
        HANDLE thread;

        PFC_CLASS_NOT_COPYABLE_EX (thread_dynamic)
    };

    new thread_dynamic (func, THREAD_PRIORITY_BELOW_NORMAL);
}

// Helper: playlist lock name from playlist index
class string_utf8_from_playlist_index
{
    pfc::string8 m_data;
public:
    explicit string_utf8_from_playlist_index (t_size p_playlist_index)
    {
        static_api_ptr_t<playlist_manager>()->playlist_lock_query_name (p_playlist_index, m_data);
    }

    operator const char * () const { return m_data.get_ptr (); }
    operator pfc::string8 const & () const { return m_data; }
};
#endif