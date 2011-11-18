#ifndef _FOO_PLAYLIST_LOCKS_LOCK_MANAGER_H_
#define _FOO_PLAYLIST_LOCKS_LOCK_MANAGER_H_

namespace playlist_locks
{
    class lock_t;
    typedef const lock_t * lock_ñptr;
    
    class NOVTABLE lock_manager : public service_base
    {
        FB2K_MAKE_SERVICE_INTERFACE_ENTRYPOINT(lock_manager)
    public:
        virtual lock_ñptr get_lock_type (t_size p_index) const = 0;

        // checks if lock is installed on p_playlist
        // p_lock_index from 0 to LOCK_COUNT-1
        virtual bool playlist_has_lock (t_size p_playlist, t_size p_lock_index) const = 0 ;

        // adds or removes lock to the list of locks installed on p_playlist
        // p_lock_index from 0 to LOCK_COUNT-1
        virtual void activeplaylist_lock_toggle (t_size p_lock_index) = 0;
        
        // returns comma separated list of names of locks installed on active playlist
        virtual bool activeplaylist_get_lock_names (pfc::string_base &p_out) const = 0;

        // returns list of playlists having p_lock
        virtual void get_playlists (lock_ñptr p_lock, pfc::list_base_t<t_size> &p_out) const = 0;
        inline void for_each_playlist (lock_ñptr p_lock, const boost::function<void (t_size)> &p_func) 
        {
            pfc::list_t<t_size> playlists;
            get_playlists (p_lock, playlists);
            playlists.for_each (p_func);
        }

        static void register_lock_type (lock_ñptr p_lock);
    };
    typedef static_api_ptr_t<lock_manager> get_lock_manager;


    class NOVTABLE lock_t
    {
    public:
        lock_t () { lock_manager::register_lock_type (this); }
        virtual ~lock_t () {}

        virtual GUID get_guid () const = 0;
        virtual const char* get_lock_name () const = 0;
    };
}
#endif