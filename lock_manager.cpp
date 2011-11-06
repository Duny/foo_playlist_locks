#include "stdafx.h"

namespace playlist_locks
{
    class playlist_locks_lock : public playlist_lock_impl_simple
    {
        // playlist_lock implementation
        void get_lock_name (pfc::string_base &p_out) override { p_out.set_string (LOCK_NAME); }
    };


    class lock_manager_impl : public lock_manager
    {
        // Member variables
        static cfg_objList<playlist_lock_data> m_data;
        service_ptr_t<playlist_lock> m_lock;
        pfc::list_t<playlist_lock_special*> m_registered_locks_list;
        mutable critical_section m_section;

        const service_ptr_t<playlist_lock>& get_lock () const override { return m_lock; }

        void register_lock_type (playlist_lock_special *p_lock)
        {
            insync (m_section);
            GUID guid = p_lock->get_guid ();
            for (t_size i = 0, n = m_registered_locks_list.get_size (); i < n; ++i) {
                if (m_registered_locks_list[i]->get_guid () == guid)
                    return;
            }
            m_registered_locks_list.add_item (p_lock);
        }

        t_size get_lock_type_count () const { return m_registered_locks_list.get_count (); }
        playlist_lock_special *get_lock_type (t_size p_index) const { return m_registered_locks_list[p_index]; }

        bool playlist_lock_present (t_size p_playlist, t_size p_lock_index)
        {
            insync (m_section);
            auto index = find_lock_data_by_playlist_id (p_playlist);
            if (index != pfc_infinite)
                return m_data[index].get<1> ().find_item (m_registered_locks_list[p_lock_index]->get_guid ()) != pfc_infinite;
            else
                return false;
        }

        void playlist_lock_toggle (t_size p_playlist, t_size p_lock_index)
        {
            auto lock_guid = m_registered_locks_list[p_lock_index]->get_guid ();
            insync (m_section);
            for (t_size i = 0, n = m_data.get_size (); i < n; ++i) {
                if (m_data[i].get<0>() == p_playlist) {
                    auto index = m_data[i].get<1>().find_item (lock_guid);
                    if (index == pfc_infinite) // add lock to list
                        m_data[i].get<1>().add_item (lock_guid);
                    else {
                        m_data[i].get<1>().remove_item (lock_guid);
                        if (m_data[i].get<1>().get_size () == 0) { // all lock types removed => uninstall lock and remove playlist from list
                            static_api_ptr_t<playlist_manager>()->playlist_lock_uninstall (p_playlist, m_lock);
                            m_data.remove_by_idx (i);
                            --n;
                        }
                    }
                    return;
                }
            }

            // playlist not found in data -> install lock on it, add new item to m_data
            static_api_ptr_t<playlist_manager>()->playlist_lock_install (p_playlist, m_lock);

            playlist_lock_data new_data;
            new_data.get<0>() = p_playlist;
            new_data.get<1>().add_item (lock_guid);
            m_data.add_item (new_data);
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
    };
    cfg_objList<playlist_lock_data> lock_manager_impl::m_data (guid_inline<0xc7db2db, 0xab80, 0x4c4b, 0xb5, 0xfb, 0x75, 0xa9, 0x21, 0x54, 0xa2, 0x8>::guid);

    static service_factory_single_t<lock_manager_impl> g_locks_manager_factory;
}