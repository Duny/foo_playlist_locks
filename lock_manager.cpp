#include "stdafx.h"

//
// for storing plugin configuration
struct playlist_lock_data
{
    t_size            m_playlist; // # of playlist with lock
    pfc::list_t<GUID> m_lock_guid_list; // list of all locks installed on this playlist
};

FB2K_STREAM_READER_OVERLOAD(pfc::list_t<GUID>) { t_size n = 0; stream >> n; GUID g; while (n --> 0) { stream >> g; value.add_item (g); } return stream; }
FB2K_STREAM_WRITER_OVERLOAD(pfc::list_t<GUID>) { t_size n = value.get_size (); stream << n; while (n --> 0) stream << value[n]; return stream; }

FB2K_STREAM_READER_OVERLOAD(playlist_lock_data) { return stream >> value.m_playlist >> value.m_lock_guid_list; }
FB2K_STREAM_WRITER_OVERLOAD(playlist_lock_data) { return stream << value.m_playlist << value.m_lock_guid_list; }


namespace playlist_locks
{
    class lock_simple : public playlist_lock_impl_simple
    {
        // playlist_lock implementation
        void get_lock_name (pfc::string_base &p_out) override { 
            pfc::string_formatter str = LOCK_NAME;
            auto playlist = static_api_ptr_t<playlist_manager>()->get_active_playlist ();
            if (playlist != pfc_infinite) {
                pfc::list_t<playlist_lock_special_ptr> playlist_locks;

                static_api_ptr_t<lock_manager>()->playlist_get_locks (playlist, playlist_locks);
                if (playlist_locks.get_count ()) {
                    str << "(";
                    pfc::string8_fast lock_name;
                    playlist_locks.for_each ([&] (playlist_lock_special_ptr p_lock)
                    {
                        p_lock->get_lock_name (lock_name);
                        str << lock_name << ",";
                    });
                    str.replace_char (',', ')', str.length () - 1);
                }
            }
            p_out.set_string (str); 
        }
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

    class lock_manager_impl : public lock_manager, public playlist_callback_impl_simple
    {
        
        // Member variables
        //
        static cfg_objList<playlist_lock_data> m_data;
        mutable critical_section m_section; // synchronization for accessing m_data

        service_ptr_t<playlist_lock> m_lock; // instance of playlist_lock for install/uninstall on playlists


        // playlist_callback implementation
        void on_playlists_reorder (const t_size * p_order, t_size p_count) override
        {
            insync (m_section);

            for (t_size n = 0, data_size = m_data.get_size (); n < data_size; ++n) {
                for (t_size i = 0; i < p_count; ++i) {
                    if (m_data[n].m_playlist == i) {
                        m_data[n].m_playlist = p_order[i];
                        break;
                    }
                }
            }
        }

        void on_playlists_removed (const bit_array &p_mask, t_size p_old_count, t_size p_new_count)
        {
            insync (m_section);

            // FIXME!!!!
            m_data.sort_t ([] (const playlist_lock_data &one, const playlist_lock_data &two) -> int
            {
                    if (one.m_playlist < two.m_playlist) return -1;
                    else if (one.m_playlist > two.m_playlist) return 1;
                    else return 0;
            });
            console::formatter f;
            m_data.for_each ([&f] (const playlist_lock_data &p_data) { f << p_data.m_playlist << " "; });

            while (p_old_count --> 0) {
                if (p_mask[p_old_count]) {
                    int i = m_data.get_size () - 1;
                    while (i --> 0) {
                        if (m_data[i].m_playlist > p_old_count)
                            m_data[i].m_playlist--;
                        else if (m_data[i].m_playlist == p_old_count) {
                            m_data.remove_by_idx (i);
                            break;
                        }
                        else
                            break;
                    }
                }
            }
        }

        // lock_manager implementation
        //
        const service_ptr_t<playlist_lock>& get_foobar2000_lock () const override { return m_lock; }

        t_size get_lock_type_count () const override { return get_registered_locks_list ().get_count (); }

        // no boundary check
        playlist_lock_special_ptr get_lock_type (t_size p_index) const override { return get_registered_locks_list()[p_index]; }

        bool playlist_has_lock (t_size p_playlist, playlist_lock_special_ptr p_lock) const override
        {
            insync (m_section);

            auto index = find_lock_data_by_playlist_id (p_playlist);
            if (index != pfc_infinite)
                return m_data[index].m_lock_guid_list.find_item (p_lock->get_guid ()) != pfc_infinite;

            return false;
        }

        void playlist_lock_toggle (t_size p_playlist, playlist_lock_special_ptr p_lock) override
        {
            insync (m_section);

            static_api_ptr_t<playlist_manager> api;

            // get guid of the requested lock type
            auto lock_guid = p_lock->get_guid ();
            
            auto index = find_lock_data_by_playlist_id (p_playlist);
            if (index != pfc_infinite) {
                auto guid_index = m_data[index].m_lock_guid_list.find_item (lock_guid);
                if (guid_index == pfc_infinite) // add lock to list
                    m_data[index].m_lock_guid_list.add_item (lock_guid);
                else {
                    m_data[index].m_lock_guid_list.remove_by_idx (guid_index);
                    if (m_data[index].m_lock_guid_list.get_size () == 0) { // all lock types removed => uninstall lock and remove data item from list
                        api->playlist_lock_uninstall (p_playlist, m_lock);
                        m_data.remove_by_idx (index);
                        return;
                    }
                }

                // make foobar2000 call lock_simple->get_lock_name () for statusbar update
                api->playlist_lock_uninstall (p_playlist, m_lock);
                api->playlist_lock_install (p_playlist, m_lock);

            }
            else {
                playlist_lock_data new_data;
                new_data.m_playlist = p_playlist;
                new_data.m_lock_guid_list.add_item (lock_guid); 
                m_data.add_item (new_data);

                api->playlist_lock_install (p_playlist, m_lock);
            }
        }

        bool playlist_get_locks (t_size p_playlist, pfc::list_base_t<playlist_lock_special_ptr> &p_out) const override
        {
            insync (m_section);

            auto data_index = find_lock_data_by_playlist_id (p_playlist);
            if (data_index != pfc_infinite) {
                p_out.remove_all ();
                for (t_size i = 0, n = m_data[data_index].m_lock_guid_list.get_size (); i < n; ++i) {                    ;
                    if (playlist_lock_special_ptr lock_ptr = find_lock_ptr_by_guid (m_data[data_index].m_lock_guid_list[i]))
                        p_out.add_item (lock_ptr);
                }
                return true;
            }
            else
                return false;
        }

        // helper functions
        inline t_size find_lock_data_by_playlist_id (t_size p_playlist) const
        {
            for (t_size i = 0, n = m_data.get_size (); i < n; ++i)
                if (m_data[i].m_playlist == p_playlist) return i;
            return pfc_infinite;
        }

        inline playlist_lock_special * find_lock_ptr_by_guid (const GUID &p_guid) const
        {
            const registered_locks_list& p_list = get_registered_locks_list ();
            for (t_size i = 0, n = p_list.get_size (); i < n; ++i) {
                if (p_list[i]->get_guid () == p_guid)
                    return p_list[i];
            }
            return nullptr;
        }

   public:
       lock_manager_impl () : m_lock (new service_impl_t<lock_simple>()) {}

        void on_init ()
        {
            static_api_ptr_t<playlist_manager>()->register_callback (this, flag_on_playlists_reorder | flag_on_playlists_removed);

            static_api_ptr_t<playlist_manager> api;
            auto playlist_count = api->get_playlist_count ();
            for (t_size i = 0, n = m_data.get_size (); i < n; ++i) {
                auto playlist_index = m_data[i].m_playlist;
                if (m_data[i].m_lock_guid_list.get_size () == 0 || playlist_index >= playlist_count || api->playlist_lock_is_present (playlist_index)) {
                    m_data.remove_by_idx (i);
                    n--;
                }
                else
                    api->playlist_lock_install (playlist_index, m_lock);
            }
        }

        void on_quit ()
        {
            static_api_ptr_t<playlist_manager>()->unregister_callback (this);
        }
    };
    cfg_objList<playlist_lock_data> lock_manager_impl::m_data (guid_inline<0x478572b6, 0x7828, 0x47e6, 0x8f, 0x38, 0xe1, 0xfb, 0x70, 0xa, 0x99, 0x93>::guid);

    static service_factory_single_t<lock_manager_impl> g_locks_manager_factory;

    namespace {
        class playlist_locks_initializer : public initquit
        {
            void on_init () override { g_locks_manager_factory.get_static_instance ().on_init (); }
            void on_quit () override { g_locks_manager_factory.get_static_instance ().on_quit (); }
        };
        static initquit_factory_t<playlist_locks_initializer> g_initializer;
    }
}