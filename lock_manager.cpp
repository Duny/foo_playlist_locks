#include "stdafx.h"

namespace playlist_locks
{
    class playlist_locks_lock : public playlist_lock_impl_simple
    {
        // playlist_lock implementation
        void get_lock_name (pfc::string_base &p_out) override { p_out.set_string (LOCK_NAME); }
    };

    typedef pfc::list_t<playlist_lock_special*> registered_locks_list;

    static registered_locks_list& get_registered_locks_list ()
    {
        static registered_locks_list list;
        return list;
    }

    void lock_manager::register_lock_type (playlist_lock_special *p_lock)
    {
        GUID guid = p_lock->get_guid ();
        registered_locks_list &p_list = get_registered_locks_list ();
        for (t_size i = 0, n = p_list.get_size (); i < n; ++i) {
            if (p_list[i]->get_guid () == guid)
                return;
        }

        p_list.add_item (p_lock);
    }

    class lock_manager_impl : public lock_manager
    {
        // Member variables
        //
        static cfg_objList<playlist_lock_data> m_data;
        critical_section m_section; // synchronization for accessing m_data

        service_ptr_t<playlist_lock> m_lock; // instance of playlist_lock for installing/uninstalling to playlists

        
        // lock_manager implementation
        //
        const service_ptr_t<playlist_lock>& get_foobar2000_lock () const override { return m_lock; }

        t_size get_lock_type_count () const { return get_registered_locks_list ().get_count (); }

        // no boundary check
        playlist_lock_special * const get_lock_type (t_size p_index) const { return get_registered_locks_list()[p_index]; }

        bool playlist_has_lock (t_size p_playlist, const playlist_lock_special *p_lock)
        {
            insync (m_section);

            auto index = find_lock_data_by_playlist_id (p_playlist);
            if (index != pfc_infinite)
                return m_data[index].get<1> ().find_item (p_lock->get_guid ()) != pfc_infinite;

            return false;
        }

        void playlist_lock_toggle (t_size p_playlist, const playlist_lock_special *p_lock)
        {
            insync (m_section);

            // get guid of the requested lock type
            auto lock_guid = p_lock->get_guid ();
            
            auto index = find_lock_data_by_playlist_id (p_playlist);
            if (index != pfc_infinite) {
                auto guid_index = m_data[index].get<1>().find_item (lock_guid);
                if (guid_index == pfc_infinite) // add lock to list
                    m_data[index].get<1>().add_item (lock_guid);
                else {
                    m_data[index].get<1>().remove_by_idx (guid_index);
                    if (m_data[index].get<1>().get_size () == 0) { // all lock types removed => uninstall lock and remove data item from list
                        static_api_ptr_t<playlist_manager>()->playlist_lock_uninstall (p_playlist, m_lock);
                        m_data.remove_by_idx (index);
                    }
                }
            }
            else { // playlist not found in data -> install lock on it, add new item to m_data
                static_api_ptr_t<playlist_manager>()->playlist_lock_install (p_playlist, m_lock);

                playlist_lock_data new_data;
                new_data.get<0>() = p_playlist;
                new_data.get<1>().add_item (lock_guid);
                m_data.add_item (new_data);
            }
        }

        // helper functions
        inline t_size find_lock_data_by_playlist_id (t_size p_playlist)
        {
            for (t_size i = 0, n = m_data.get_size (); i < n; ++i)
                if (m_data[i].get<0> () == p_playlist) return i;
            return pfc_infinite;
        }

    public:
        lock_manager_impl () : m_lock (new service_impl_t<playlist_locks_lock>()) {}

        static void init_playlists ()
        {
            for (t_size i = 0, n = m_data.get_size (); i < n; ++i)
                static_api_ptr_t<playlist_manager>()->playlist_lock_install (m_data[i].get<0>(), get_lock_manager ()->get_foobar2000_lock ());
        }
    };
    cfg_objList<playlist_lock_data> lock_manager_impl::m_data (guid_inline<0x478572b6, 0x7828, 0x47e6, 0x8f, 0x38, 0xe1, 0xfb, 0x70, 0xa, 0x99, 0x93>::guid);
    static service_factory_single_t<lock_manager_impl> g_locks_manager_factory;

    namespace {
        class playlist_locks_initializer : public initquit
        {
            void on_init () override { lock_manager_impl::init_playlists (); }
            void on_quit () override {}
        };

        static initquit_factory_t<playlist_locks_initializer> g_initializer;
    }
}