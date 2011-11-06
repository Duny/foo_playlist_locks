#include "stdafx.h"

// {BB87A66D-1020-4baf-8E08-364D67B72823}
const GUID my_playlist_property = 
{ 0xbb87a66d, 0x1020, 0x4baf, { 0x8e, 0x8, 0x36, 0x4d, 0x67, 0xb7, 0x28, 0x23 } };

static service_factory_single_transparent_t<my_playlist_callback_static> g_playlist_callback;


class my_check_playlist_callback : public main_thread_callback
{
public:
    my_check_playlist_callback(t_size playlist) : m_index(playlist) {}

    void callback_run()
    {
        /*int is_my;
        if (t_pm_v2()->playlist_get_property_int(m_index, my_playlist_property, is_my) 
            && is_my == 1)
            t_pm()->playlist_lock_install(m_index, g_lock);*/
    }

private:
    t_size m_index;
};


void my_playlist_callback_static::on_playlist_created(t_size p_index,const char * p_name,t_size p_name_len)
{
    static_api_ptr_t<main_thread_callback_manager>()->add_callback (
		new service_impl_t<my_check_playlist_callback>(p_index));
} 

void my_playlist_callback_static::on_playlists_removed(const bit_array & p_mask,t_size p_old_count,t_size p_new_count)
{
    console::formatter f;
    
    for (t_size i = 0; i < p_old_count; i++)
        f << p_mask[i] << " ";
}