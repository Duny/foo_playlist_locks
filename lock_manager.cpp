#include "stdafx.h"

namespace playlist_locks
{
    const service_ptr_t<lock_manager> &lock_manager::get_instance ()
    {
        static service_ptr_t<lock_manager> manager (new service_impl_t<lock_manager>);
        return manager;
    }

    void lock_manager::register_lock_type (const playlist_lock_special_ptr &p_lock)
    {
        GUID guid = p_lock->get_guid ();
        for (t_size i = 0, n = m_registered_locks_list.get_size (); i < n; ++i) {
            if (m_registered_locks_list[i]->get_guid () == guid)
                return;
        }
        m_registered_locks_list.add_item (p_lock);
    }
}