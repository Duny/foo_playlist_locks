#include "stdafx.h"

namespace playlist_locks
{
    using namespace vk_com_api;

    class working_thread : public threaded_process_callback
    {
    public:
        void start ()
        {
            threaded_process::g_run_modeless (this, 
                threaded_process::flag_show_abort | threaded_process::flag_show_item | threaded_process::flag_no_focus | threaded_process::flag_show_delayed,
                core_api::get_main_window (), "working...");
        }

    private:
        void run (threaded_process_status & p_status, abort_callback & p_abort) override;

        void on_done (HWND p_wnd, bool p_was_aborted) override
        {
        }
    };


    void working_thread::run (threaded_process_status & p_status, abort_callback & p_abort)
    {
        class my_completion_notify : public completion_notify
        {
            const win32_event & complete;
            void on_completion (unsigned p_code) override
            {
                const_cast<win32_event &>(complete).set_state (true);
            }
        public:
            my_completion_notify (const win32_event & p_complete) : complete (p_complete) {}
        };

        class my_process_locations_notify : public process_locations_notify
        {
            const win32_event & complete;
            void on_completion (const pfc::list_base_const_t<metadb_handle_ptr> & p_items) override
            {
                auto count = p_items.get_count ();
                console::formatter () << "Processed " << count << " items\n";
                static_api_ptr_t<playlist_manager>()->activeplaylist_add_items (p_items, bit_array_true ());

                const_cast<win32_event &>(complete).set_state (true);
            }
            virtual void on_aborted ()
            {
                return;
            }
        public:
            my_process_locations_notify (const win32_event & p_complete) : complete (p_complete) {}
        };

        vk_com_api::audio::get_count num_tracks (vk_com_api::get_user_id_as_uint32 ());
        if (!num_tracks.call (p_abort)) {
            if (!num_tracks.aborted ())
                console::formatter() << num_tracks.get_error () << "\n";
            return;
        }
        console::formatter() << "total tracks: " << num_tracks << "\n";

        vk_com_api::audio::get * tracks = new vk_com_api::audio::get;
        if (!tracks->call (p_abort)) {
            if (!tracks->aborted ())
                console::formatter() << tracks->get_error () << "\n";
            return;
        }

        pfc::list_t<const char*> urls;
        metadb_handle_list list;
        static_api_ptr_t<metadb> metadb_api;
        auto n = tracks->get_count ();
        pfc::string_formatter f;
        while (n --> 0) {
            auto s = tracks->get_item (n).get<f_at_info_url>();
            if (s.length () != 0) {
                urls.add_item (s);
                metadb_handle_ptr handle;
                metadb_api->handle_create (handle, make_playable_location (s, 0));
                if (handle.is_valid ())
                    list.add_item (handle);
                else
                    console::formatter () << "handle_create failed.";
            }

        }
    }

    class vk_music : public lock_t
    {
        // lock_t overrides
        const char * get_name () const override
        {
            return "Vk.com music";
        }

        GUID get_guid () const override
        {
            return create_guid (0x42229ae5, 0xf777, 0x4bc, 0x8d, 0xf8, 0xd7, 0x12, 0xe3, 0xa8, 0xc5, 0xb3);
        }

        bool is_exclusive () const override
        {
            return false;
        }

        void on_installed (t_size p_playlist_index)
        {
            service_ptr_t<working_thread> thread = new service_impl_t<working_thread> ();
            thread->start ();
        }
    };

    //namespace { vk_music g_vk_com_music_lock; }
}