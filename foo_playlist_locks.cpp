#include "stdafx.h"

DECLARE_COMPONENT_VERSION(COMPONENT_NAME, "0.2",
"A component, that manages locked playlist. It feels playlist with music, \
coming into Media Library, also it removes music, removed from Media Library.\n\
And also it removes played tracks from the playlist.\n\n\
(c) 2010 Duny (majorquake3@gmail.com)");

VALIDATE_COMPONENT_FILENAME(COMPONENT_NAME".dll");

bool get_playlist_remove_mask(
	const pfc::list_base_const_t<metadb_handle_ptr> &p_playlist_items,
	const pfc::list_base_const_t<metadb_handle_ptr> &p_items_to_remove,
	pfc::bit_array_var_impl &p_out)
{
	bool result = false;
	t_size n = p_items_to_remove.get_count(), 
		   m = p_playlist_items.get_count();
    for (t_size i = 0; i < n; i++)
    {
        for (t_size j = 0; j < m; j++)
        {
			if (p_playlist_items[j] == p_items_to_remove[i])
			{
                p_out.set(j);
				result = true;
			}
        }
    }
	return result;
}