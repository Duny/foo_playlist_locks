#include "stdafx.h"

// {C74F38A8-471B-49ab-9BB8-5C9F87F0C623}
static const GUID my_popup = 
{ 0xc74f38a8, 0x471b, 0x49ab, { 0x9b, 0xb8, 0x5c, 0x9f, 0x87, 0xf0, 0xc6, 0x23 } };
// {754047B8-9503-4747-85A0-E6CCF7D5B3F5}
static const GUID install_lock = 
{ 0x754047b8, 0x9503, 0x4747, { 0x85, 0xa0, 0xe6, 0xcc, 0xf7, 0xd5, 0xb3, 0xf5 } };
// {100A677D-3902-4b6f-A7E8-B96DEA5A7891}
static const GUID uninstall_lock = 
{ 0x100a677d, 0x3902, 0x4b6f, { 0xa7, 0xe8, 0xb9, 0x6d, 0xea, 0x5a, 0x78, 0x91 } };


static mainmenu_commands_factory_t<my_mainmenu_commands> g_mainmenu_commands;
static mainmenu_commands_factory_t<my_mainmenu_edit_popup> g_mainmenu_edit_popup;


GUID my_mainmenu_edit_popup::get_guid()
{
	return my_popup;
}

GUID my_mainmenu_edit_popup::get_parent()
{
	return mainmenu_groups::edit_part3;
}

t_uint32 my_mainmenu_edit_popup::get_sort_priority()
{
	return 0;
}

void my_mainmenu_edit_popup::get_display_string(pfc::string_base & p_out)
{
	p_out.set_string(COMPONENT_NAME);
}


t_uint32 my_mainmenu_commands::get_command_count()
{
	return 2;
}

GUID my_mainmenu_commands::get_command(t_uint32 p_index)
{
	if (p_index == 0)
		return install_lock;
	else if (p_index == 1)
		return uninstall_lock;
	return pfc::guid_null;
}

void my_mainmenu_commands::get_name(t_uint32 p_index, pfc::string_base & p_out)
{
	if (p_index == 0)
		p_out.set_string("Install lock");
	else if (p_index == 1)
		p_out.set_string("Uninstall lock");
}

bool my_mainmenu_commands::get_description(t_uint32 p_index, pfc::string_base & p_out)
{
	return false;
}

GUID my_mainmenu_commands::get_parent()
{
	return my_popup;
}

bool my_mainmenu_commands::get_display(t_uint32 p_index,pfc::string_base & p_text,t_uint32 & p_flags)
{
	t_pm pm;
	t_size active = pm->get_active_playlist();
	if (active == pfc_infinite) return false;
	if (t_apm()->is_client_present(active)) return false;

	bool is_locked = pm->playlist_lock_is_present(active);

    p_flags = 0;
	if (p_index == 0) // install my lock
	{
		if (is_locked)
			p_flags |= mainmenu_commands::flag_disabled;
	}
	else if (p_index == 1) // uninstall lock
	{
		p_flags |= mainmenu_commands::flag_disabled;
		if (is_locked)
		{
			pfc::string8 lock_name;
			pm->playlist_lock_query_name(active, lock_name);
			if (lock_name == pfc::string8(COMPONENT_NAME))
				p_flags = 0;
		}
	}
	get_name(p_index,p_text);
	return true;
}

void my_mainmenu_commands::execute(t_uint32 p_index, service_ptr_t<service_base> p_callback)
{
	t_pm pm;
	t_size active = pm->get_active_playlist();

	if (p_index == 0) // install lock
	{
		pm->playlist_lock_install(active, g_lock);
        t_pm_v2()->playlist_set_property_int(active, my_playlist_property, (int)1);
	}
	else if (p_index == 1) // uninstall lock
	{
		pm->playlist_lock_uninstall(active, g_lock); 
        t_pm_v2()->playlist_remove_property(active, my_playlist_property);
	}
}