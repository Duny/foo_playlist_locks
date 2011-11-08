#ifndef _FOO_PLAYLIST_LOCKS_LOCK_MANAGER_H_
#define _FOO_PLAYLIST_LOCKS_LOCK_MANAGER_H_

namespace playlist_locks
{
    class NOVTABLE playlist_lock_special
    {
    public:
        virtual GUID get_guid () const = 0;
        virtual void get_lock_name (pfc::string_base &p_out) = 0;
    };
    
    
    class NOVTABLE lock_manager : public service_base
    {
        FB2K_MAKE_SERVICE_INTERFACE_ENTRYPOINT(lock_manager)
    public:
        virtual const service_ptr_t<playlist_lock>& get_foobar2000_lock () const = 0;

        virtual t_size get_lock_type_count () const = 0;
        virtual playlist_lock_special * const get_lock_type (t_size p_index) const = 0;

        virtual bool playlist_has_lock (t_size p_playlist, const playlist_lock_special *p_lock) const = 0;
        inline bool playlist_has_lock (t_size p_playlist, t_size p_index) const { return playlist_has_lock (p_playlist, get_lock_type (p_index)); }

        // adds or removes lock p_index to the list of locks installed on playlist p_playlist
        virtual void playlist_lock_toggle (t_size p_playlist, const playlist_lock_special *p_lock) = 0;
        inline void playlist_lock_toggle (t_size p_playlist, t_size p_index) { return playlist_lock_toggle (p_playlist, get_lock_type (p_index)); }

        virtual bool playlist_get_locks (t_size p_playlist, pfc::list_base_t<playlist_lock_special*> &p_out) const = 0;
        
        static void register_lock_type (playlist_lock_special *p_lock);
    };
    typedef static_api_ptr_t<lock_manager> get_lock_manager;


    // helper
    template <class playlist_lock_special_impl>
    class register_playlist_lock_special_t
    {
        playlist_lock_special_impl m_lock;
    public:
        register_playlist_lock_special_t () { lock_manager::register_lock_type (&m_lock); }
    };


    // {E2B169EC-196D-4FBC-A1BB-6FFD5BA6DDAC}
    __declspec(selectany) const GUID lock_manager::class_guid = 
    { 0xe2b169ec, 0x196d, 0x4fbc, { 0xa1, 0xbb, 0x6f, 0xfd, 0x5b, 0xa6, 0xdd, 0xac } };
}
#endif