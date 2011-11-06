#include "stdafx.h"

static initquit_factory_t<my_initquit> g_initquit;


void my_initquit::on_init()
{
    t_lm_v3()->register_callback(&m_library_callback_dynamic);

    static_api_ptr_t<play_callback_manager>()->register_callback(&m_play_callback, 
        m_play_callback.get_flags(), false);
}

void my_initquit::on_quit()
{
    t_lm_v3()->unregister_callback(&m_library_callback_dynamic);

    static_api_ptr_t<play_callback_manager>()->unregister_callback(&m_play_callback);
}