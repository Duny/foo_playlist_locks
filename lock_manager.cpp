#include "stdafx.h"

FB2K_STREAM_READER_OVERLOAD(pfc::list_t<GUID>) { t_size n = 0; stream >> n; GUID g; while (n --> 0) { stream >> g; value.add_item (g); } return stream; }
FB2K_STREAM_WRITER_OVERLOAD(pfc::list_t<GUID>) { t_size n = value.get_size (); stream << n; while (n --> 0) stream << value[n]; return stream; }

namespace playlist_locks
{
    // lock_manager class guid
    const GUID lock_manager::class_guid = { 0xe2b169ec, 0x196d, 0x4fbc, { 0xa1, 0xbb, 0x6f, 0xfd, 0x5b, 0xa6, 0xdd, 0xac } };

    // array of all registered lock types
    namespace { lock_ñptr g_lock_types[LOCK_COUNT]; static t_uint32 g_num_locks = 0; }
    
    //
    // lock service implementation for internal use by lock_manager
    //
    class lock_simple : public playlist_lock_impl_simple
    {
        // playlist_lock implementation
        void get_lock_name (pfc::string_base &p_out) override { 
            p_out = LOCK_NAME;

            pfc::string8_fast locks;
            if (static_api_ptr_t<lock_manager>()->activeplaylist_get_lock_names (locks))
                p_out << ": " << locks;
        }
    };


    void lock_manager::register_lock_type (lock_ñptr p_lock)
    {
        g_lock_types[g_num_locks++] = p_lock;
    }

    class lock_manager_impl : public lock_manager, public playlist_callback_impl_simple
    {
        // Member variables
        
        // We store array of lists of locks guids installed on each playlist
        static cfg_objList<pfc::list_t<GUID>> m_data;
        mutable critical_section m_section; // synchronization for accessing m_data

        service_ptr_t<playlist_lock> m_lock; // instance of playlist_lock for install/uninstall on playlists


        // playlist_callback implementation
        //
        void on_playlist_created (t_size p_index, const char *p_name, t_size p_name_len) override
        {
            insync (m_section);
            m_data.add_item (pfc::list_t<GUID> ());
        }

        void on_playlists_reorder (const t_size *p_order, t_size p_count) override
        {
            insync (m_section);
            m_data.reorder (p_order);
        }

        void on_playlists_removed (const bit_array &p_mask, t_size p_old_count, t_size p_new_count) override
        {
            insync (m_section);
            m_data.remove_mask (p_mask);
        }

        // lock_manager implementation
        //

        // no boundary check
        lock_ñptr get_lock_type (t_size p_index) const override { return g_lock_types[p_index]; }

        bool playlist_has_lock (t_size p_playlist, t_size p_lock) const override
        {
            insync (m_section);
            return m_data[p_playlist].have_item (get_lock_type (p_lock)->get_guid ());
        }

        void activeplaylist_lock_toggle (t_size p_lock) override
        {
            insync (m_section);

            static_api_ptr_t<playlist_manager> api;
            auto guid = get_lock_type (p_lock)->get_guid ();
            auto playlist = api->get_active_playlist ();
            
            if (!static_api_ptr_t<autoplaylist_manager>()->is_client_present (playlist)) {
                auto guid_index = m_data[playlist].find_item (guid);
                if (guid_index == pfc_infinite) // add lock 
                    m_data[playlist].add_item (guid);
                else { // remove lock
                    m_data[playlist].remove_by_idx (guid_index);
                    if (m_data[playlist].get_size () == 0) { // all lock types were removed => uninstall lock from playlist
                        api->playlist_lock_uninstall (playlist, m_lock);
                        return;
                    }
                }

                // make foobar2000 update statusbar
                api->playlist_lock_uninstall (playlist, m_lock);
                api->playlist_lock_install (playlist, m_lock);
            }
        }
        
        bool activeplaylist_get_lock_names (pfc::string_base &p_out) const override
        {
            p_out.reset ();

            t_size playlist = static_api_ptr_t<playlist_manager>()->get_active_playlist ();
            if (playlist != pfc_infinite) {
                m_data[playlist].for_each ([&] (const GUID &guid) { p_out << find_lock_by_guid (guid)->get_lock_name () << ", "; });
                if (!p_out.is_empty ()) {
                    p_out.truncate (p_out.length () - 2);
                    return true;
                }
            }
            return false;
        }


        void get_playlists (lock_ñptr p_lock, pfc::list_base_t<t_size> &p_out) const override
        {
            GUID guid = p_lock->get_guid ();
            p_out.remove_all ();

            insync (m_section);

            auto n = m_data.get_size ();
            while (n --> 0)
                if (m_data[n].have_item (guid))
                    p_out.add_item (n);
        }


        // helper functions
        inline lock_ñptr find_lock_by_guid (const GUID &p_guid) const
        {
            auto n = LOCK_COUNT;
            while (n --> 0)
                if (g_lock_types[n]->get_guid () == p_guid)
                    return g_lock_types[n];

            // shouldn't get here..
            return nullptr;
        }

    public:
        lock_manager_impl () : m_lock (new service_impl_t<lock_simple>()) {}

        void on_init ()
        {
            PFC_ASSERT (g_num_locks == LOCK_COUNT);

            static_api_ptr_t<playlist_manager> api;
            api->register_callback (this, flag_on_playlists_reorder | flag_on_playlists_removed | flag_on_playlist_created);

            insync (m_section);
                        
            auto n = m_data.get_size (), playlist_count = api->get_playlist_count ();;
            if (n != playlist_count) {
                m_data.set_size (playlist_count);
                n = playlist_count;
            }

            while (n --> 0)
                if (m_data[n].get_size () && !api->playlist_lock_install (n, m_lock))
                    m_data[n].remove_all ();
        }

        void on_quit () { static_api_ptr_t<playlist_manager>()->unregister_callback (this); }
    };

    cfg_objList<pfc::list_t<GUID>> lock_manager_impl::m_data (guid_inline<0x478572b6, 0x7828, 0x47e6, 0x8f, 0x38, 0xe1, 0xfb, 0x70, 0xa, 0x99, 0x93>::guid);
        

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