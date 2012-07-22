#ifndef _FOO_PLAYLIST_LOCKS_LOCK_MANAGER_H_
#define _FOO_PLAYLIST_LOCKS_LOCK_MANAGER_H_

namespace playlist_locks
{
    // Custom lock interface
    class NOVTABLE lock_t
    {
    public:
        lock_t (); // autoregisters itself with lock_manager on creation
        virtual ~lock_t () = 0;

        virtual const char * get_name () const = 0;
        virtual GUID get_guid () const = 0;

        // several nonexclusive locks can be installed on the same playlist,
        // no more then one exclusive lock is allowed per playlist
        virtual bool is_exclusive () const = 0;

        virtual void on_installed (t_size p_playlist_index) {}
        virtual void on_uninstalled (t_size p_playlist_index) {}
    };

    typedef lock_t * lock_ptr;
    typedef const lock_t * lock_cptr;
    

    // For displaying our own text in special foobar2000 area on status bar (with yellow lock icon)
    __declspec(selectany) extern const pfc::string8 g_foobar2000_lock_name_prefix = "Playlist Locks";

    class NOVTABLE lock_manager : public service_base
    {
        FB2K_MAKE_SERVICE_INTERFACE_ENTRYPOINT(lock_manager)
    public:
        //
        // Lock types metadata functions
        //

        // returns number of locks known by this manager 
        virtual t_size get_lock_count () const = 0;

        // returns lock name by index (empty string if out of bound)
        virtual const char * get_lock_name (t_size p_index) const = 0;

        // returns lock GUID by index (pfc::guid_null if out of bound)
        virtual GUID get_lock_guid (t_size p_index) const = 0;

        //
        // Playlist functions
        //

        // adds/removes lock to the list of locks installed on playlist
        // p_lock_index from 0 to get_count () - 1
        virtual void playlist_lock_toggle (t_size p_playlist_index, t_size p_lock_index) = 0;
        
        // builds comma separated list of names of locks installed on active playlist
        virtual bool activeplaylist_get_lock_names (pfc::string_base & p_out) const = 0;

        // returns list of playlists having p_guid_lock
        virtual void get_playlists (const GUID & p_guid_lock, pfc::list_base_t<t_size> & p_out) const = 0;

        //
        // Menu functions
        //
        virtual t_uint32 get_menuitem_flags (t_size p_menuitem_index /*=lock index*/, t_size p_playlist_index) const = 0;
    };
    typedef static_api_ptr_t<lock_manager> get_lock_manager;
}

// Helper: executes p_func for each playlist having p_guid_lock
template <typename t_func>
inline void for_each_playlist (const GUID & p_guid_lock, const t_func & p_func) 
{
    pfc::list_t<t_size> playlists;
    playlist_locks::get_lock_manager ()->get_playlists (p_guid_lock, playlists);
    playlists.for_each (p_func);
}

inline bool is_our_lock_name (const pfc::string_base & p_lock_name)
{
    return p_lock_name.length () > 0 && p_lock_name.find_first (playlist_locks::g_foobar2000_lock_name_prefix) == 0;
}
#endif