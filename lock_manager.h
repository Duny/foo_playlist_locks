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
        virtual const service_ptr_t<playlist_lock>& get_lock () const = 0;

        virtual void register_lock_type (playlist_lock_special *p_lock) = 0;
        virtual t_size get_lock_type_count () const = 0;
        virtual playlist_lock_special *get_lock_type (t_size p_index) const = 0;
        virtual bool playlist_lock_present (t_size p_playlist, t_size p_lock_index) = 0;
        // adds of removes lock p_lock_index to the list of locks installed on playlist p_playlist
        virtual void playlist_lock_toggle (t_size p_playlist, t_size p_lock_index) = 0;
    };
    typedef static_api_ptr_t<lock_manager> get_lock_manager;

    // helper class
    template <class playlist_lock_special_t>
    class register_lock_type_t : public playlist_lock_special_t
    {
    public:
        register_lock_type_t () { get_lock_manager ()->register_lock_type (this); }
    };


    // {E2B169EC-196D-4FBC-A1BB-6FFD5BA6DDAC}
    __declspec(selectany) const GUID lock_manager::class_guid = 
    { 0xe2b169ec, 0x196d, 0x4fbc, { 0xa1, 0xbb, 0x6f, 0xfd, 0x5b, 0xa6, 0xdd, 0xac } };
}
#endif