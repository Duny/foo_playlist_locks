#include "stdafx.h"

FB2K_STREAM_READER_OVERLOAD(pfc::list_t<GUID>) { t_size n = 0; stream >> n; GUID g; while (n --> 0) { stream >> g; value.add_item (g); } return stream; }
FB2K_STREAM_WRITER_OVERLOAD(pfc::list_t<GUID>) { t_size n = value.get_size (); stream << n; while (n --> 0) stream << value[n]; return stream; }

namespace playlist_locks
{
    // lock_manager class guid
    const GUID lock_manager::class_guid = { 0xe2b169ec, 0x196d, 0x4fbc, { 0xa1, 0xbb, 0x6f, 0xfd, 0x5b, 0xa6, 0xdd, 0xac } };

    class lock_type_manager
    {
        pfc::array_t<lock_ptr> m_locks;

        lock_type_manager () {}
        lock_type_manager (const lock_type_manager &) {}

    public:
        static lock_type_manager & get_manager ()
        {
            static lock_type_manager g_manager;
            return g_manager;
        }

        void register_lock_type (lock_ptr p_lock)
        {
            m_locks.append_single (p_lock);
        }

        t_size get_count () const
        {
            return m_locks.get_size ();
        }

        t_size get_exclusive_count () const
        {
            t_size count = 0;
            auto n = get_count ();
            while (n --> 0)
                if (m_locks[n]->is_exclusive ())
                    count++;
            return count;
        }
            
        const char * get_lock_name (t_size p_index) const
        {
            return p_index < m_locks.get_size () ? m_locks[p_index]->get_name () : "";
        }

        GUID get_lock_guid (t_size p_index) const
        {
            return p_index < m_locks.get_size () ? m_locks[p_index]->get_guid () : pfc::guid_null;
        }

        bool is_lock_exclusive (t_size p_index) const
        {
            return p_index < m_locks.get_size () ? m_locks[p_index]->is_exclusive () : false;
        }

        lock_ptr find_by_guid (const GUID & p_guid)
        {
            if (pfc::guid_null != p_guid)
            {
                auto n = get_count ();
                while (n --> 0)
                    if (m_locks[n]->get_guid () == p_guid)
                        return m_locks[n];
            }

            return nullptr;
        }

        lock_cptr find_by_guid (const GUID & p_guid) const
        {
            return find_by_guid (p_guid);
        }
    };


    //
    // lock service implementation for internal use by lock_manager
    //
    class lock_simple : public playlist_lock_impl_simple
    {
        // playlist_lock implementation
        void get_lock_name (pfc::string_base & p_out) override { 
            p_out = g_foobar2000_lock_name_prefix;

            pfc::string8_fast locks;
            if (static_api_ptr_t<lock_manager>()->activeplaylist_get_lock_names (locks))
                p_out << ": " << locks;
        }

        void show_ui () override
        {
           
        }
    };


    class lock_manager_impl : public lock_manager, public playlist_callback_impl_simple
    {
        // Member variables
        
        // We store list of lock guids installed on each playlist
        static cfg_objList<pfc::list_t<GUID>> m_data;

        service_ptr_t<playlist_lock> m_lock; // instance of playlist_lock for install/uninstall on playlists


        // playlist_callback implementation
        //
        void on_playlist_created (t_size p_index, const char *p_name, t_size p_name_len) override
        {
            m_data.add_item (pfc::list_t<GUID> ());
        }

        void on_playlists_reorder (const t_size *p_order, t_size p_count) override
        {
            m_data.reorder (p_order);
        }

        void on_playlists_removed (const bit_array &p_mask, t_size p_old_count, t_size p_new_count) override
        {
            m_data.remove_mask (p_mask);
        }


        //
        // lock_manager implementation
        //
        t_size get_lock_count () const override
        {
            return lock_type_manager::get_manager ().get_count ();
        }

        const char * get_lock_name (t_size p_index) const override
        {
           return lock_type_manager::get_manager ().get_lock_name (p_index);
        }

        GUID get_lock_guid (t_size p_index) const override
        {
            return lock_type_manager::get_manager ().get_lock_guid (p_index);
        }

        void playlist_lock_toggle (t_size p_playlist_index, t_size p_lock_index) override
        {
            lock_cptr p_lock = lock_type_manager::get_manager ().find_by_guid (get_lock_guid (p_lock_index));
            if (nullptr == p_lock)
                return;
            const GUID  p_lock_guid = p_lock->get_guid ();
            const bool  p_index_is_exclusive_lock = p_lock->is_exclusive ();
            const GUID  p_playlist_exclusive_lock_guid = playlist_get_exclusive_lock_guid (p_playlist_index);
            const bool  p_playlist_has_exclusive_lock = !(p_playlist_exclusive_lock_guid == pfc::guid_null);
            if (playlist_has_lock (p_playlist_index, p_lock_index) || !p_index_is_exclusive_lock || !p_playlist_has_exclusive_lock)
            {
                playlist_lock_toggle_internal (p_playlist_index, p_lock_guid);
                return;
            }
            else // p_playlist_exclusive_lock_guid != p_lock_guid && p_index_is_exclusive_lock
            {
                playlist_lock_toggle_internal (p_playlist_index, p_playlist_exclusive_lock_guid);
                playlist_lock_toggle_internal (p_playlist_index, p_lock_guid);
            }
        }
        
        t_uint32 get_menuitem_flags (t_size p_lock_index, t_size p_playlist_index) const override
        {
            t_uint32 p_flags = 0;
            // Если лок, соответствующий индексу выбраного пункта меню, установлен на плейлисте
            if (playlist_has_lock (p_playlist_index, p_lock_index))
            {
                // Если этот лок является эксклюзивным (взаимоисключающим) 
                // и при этом зарегистрировано более одного эксклюзивного лока (чтобы было из чего выбирать)
                if (lock_type_manager::get_manager ().is_lock_exclusive (p_lock_index) && lock_type_manager::get_manager ().get_exclusive_count () > 1)
                    p_flags |= mainmenu_commands::flag_radiochecked; // Ставим радиофлажок
                else
                    p_flags |= mainmenu_commands::flag_checked; // Иначе галочку
            }
            else // Плейлист еще не имеет лока p_index => он может быть установлен
            {    
            }
            
            return p_flags;
        }

        bool activeplaylist_get_lock_names (pfc::string_base & p_out) const override
        {
            p_out.reset ();

            t_size playlist = static_api_ptr_t<playlist_manager>()->get_active_playlist ();
            if (pfc_infinite != playlist && playlist < m_data.get_size ()) {
                m_data[playlist].for_each ([&] (const GUID & guid)
                {
                    auto p_lock = lock_type_manager::get_manager ().find_by_guid (guid);
                    if (nullptr != p_lock)
                        p_out << p_lock->get_name () << ", "; 
                });
                if (!p_out.is_empty ()) {
                    p_out.truncate (p_out.length () - 2);
                    return true;
                }
            }
            return false;
        }

        

        void get_playlists (const GUID & p_guid_lock, pfc::list_base_t<t_size> & p_out) const override
        {
            auto n = m_data.get_size ();
            while (n --> 0)
                if (m_data[n].have_item (p_guid_lock))
                    p_out.add_item (n);
        }


        // helper functions

        GUID playlist_get_exclusive_lock_guid (t_size p_playlist_index) const
        {
            if (p_playlist_index < m_data.get_size ())
            {
                for (t_size n = m_data[p_playlist_index].get_size (), i = 0; i < n; ++i)
                    if (auto p_lock = lock_type_manager::get_manager ().find_by_guid (m_data[p_playlist_index][i]))
                        if (p_lock->is_exclusive ())
                            return p_lock->get_guid ();
            }
            return pfc::guid_null;
        }

        bool playlist_has_lock (t_size p_playlist_index, t_size p_lock_index) const
        {
            return p_playlist_index < m_data.get_size () ? m_data[p_playlist_index].have_item (get_lock_guid (p_lock_index)) : false;
        }

        void playlist_lock_toggle_internal (t_size p_playlist_index, const GUID & p_lock_guid)
        {
            if (p_lock_guid == pfc::guid_null || p_playlist_index >= m_data.get_size ())
                return;
            
            if (!static_api_ptr_t<autoplaylist_manager>()->is_client_present (p_playlist_index)) {
                static_api_ptr_t<playlist_manager> playlist_manager_api;

                auto guid_index = m_data[p_playlist_index].find_item (p_lock_guid);

                if (guid_index == pfc_infinite) // add lock
                {
                    m_data[p_playlist_index].add_item (p_lock_guid);

                    if (auto p_lock = lock_type_manager::get_manager ().find_by_guid (p_lock_guid))
                        p_lock->on_installed (p_playlist_index);
                }
                else
                {
                    // remove lock 
                    m_data[p_playlist_index].remove_by_idx (guid_index);

                    if (auto p_lock = lock_type_manager::get_manager ().find_by_guid (p_lock_guid))
                        p_lock->on_uninstalled (p_playlist_index);

                    if (m_data[p_playlist_index].get_size () == 0)
                    {
                        // all lock types were removed => uninstall lock from playlist
                        playlist_manager_api->playlist_lock_uninstall (p_playlist_index, m_lock);
                        return;
                    }
                }

                // make foobar2000 update statusbar
                playlist_manager_api->playlist_lock_uninstall (p_playlist_index, m_lock);
                playlist_manager_api->playlist_lock_install (p_playlist_index, m_lock);
            }
        }

    public:
        lock_manager_impl () : m_lock (new service_impl_t<lock_simple>()) {}

        void on_init ()
        {
            static_api_ptr_t<playlist_manager> api;
            api->register_callback (this, flag_on_playlists_reorder | flag_on_playlists_removed | flag_on_playlist_created);
                        
            auto n = m_data.get_size (), playlist_count = api->get_playlist_count ();;
            if (n != playlist_count) {
                m_data.set_size (playlist_count);
                n = playlist_count;
            }

            while (n --> 0)
            {
                if (m_data[n].get_size () && !api->playlist_lock_install (n, m_lock))
                {
                    m_data[n].remove_all ();
                    continue;
                }

                auto m = m_data[n].get_size ();
                while (m --> 0)
                    if (auto p_lock = lock_type_manager::get_manager ().find_by_guid (m_data[n][m]))
                        p_lock->on_installed (n);
            }
        }

        void on_quit ()
        {
            static_api_ptr_t<playlist_manager>()->unregister_callback (this);

            auto n = m_data.get_size ();

            while (n --> 0)
            {
                auto m = m_data[n].get_size ();
                while (m --> 0)
                    if (auto p_lock = lock_type_manager::get_manager ().find_by_guid (m_data[n][m]))
                        p_lock->on_uninstalled (n);
            }
        }
    };

    cfg_objList<pfc::list_t<GUID>> lock_manager_impl::m_data (create_guid (0x478572b6, 0x7828, 0x47e6, 0x8f, 0x38, 0xe1, 0xfb, 0x70, 0xa, 0x99, 0x93));

    //
    // lock_t implementation
    //
    lock_t::lock_t ()
    {
        lock_type_manager::get_manager ().register_lock_type (this);
    }

    lock_t::~lock_t ()
    {}


    namespace {
        service_factory_single_t<lock_manager_impl> g_factory;
        class lock_manager_initializer : public initquit
        {
            void on_init () override { g_factory.get_static_instance ().on_init (); }
            void on_quit () override { g_factory.get_static_instance ().on_quit (); }
        };
        static initquit_factory_t<lock_manager_initializer> g_initializer;
    }
}