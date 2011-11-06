#include "stdafx.h"

void my_library_callback_dynamic::on_items_added(const pfc::list_base_const_t<metadb_handle_ptr> &p_data)
{
    t_pm_v2 pm;
    int is_my;
    for (t_size i = 0, n = pm->get_playlist_count(); i < n; i++)
    {
        if (pm->playlist_get_property_int(i, my_playlist_property, is_my) 
            && is_my == 1) 
            pm->playlist_add_items(i, p_data, bit_array_false());
    }
}

void my_library_callback_dynamic::on_items_removed(const pfc::list_base_const_t<metadb_handle_ptr> &p_data)
{
    t_pm_v2 pm;
    int is_my;
    for (t_size i = 0, n = pm->get_playlist_count(); i < n; i++)
    {
        if (pm->playlist_get_property_int(i, my_playlist_property, is_my) 
            && is_my == 1) 
        {
            metadb_handle_list playlist_items;
	        pfc::bit_array_var_impl mask;
            pm->playlist_get_all_items (i, playlist_items);
            if (get_playlist_remove_mask(playlist_items, p_data, mask))
		        pm->playlist_remove_items(i, mask);
        }
    }
}

namespace playlist_locks
{
    class media_library_tracker : public playlist_lock_special
    {
        void get_lock_name (pfc::string_base &p_out) override { p_out.set_string ("ML changes tracker"); }
        GUID get_guid () const { return guid_inline<0xe33d9a0c, 0xdec1, 0x493f, 0x9c, 0xa7, 0x81, 0x35, 0x73, 0x38, 0x16, 0x78>::guid; };
    };

    static register_lock_type_t<media_library_tracker> g_lock_ml_tracker;
}