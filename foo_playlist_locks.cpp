#include "stdafx.h"

DECLARE_COMPONENT_VERSION("Playlist Locks", "1.1",
"This component provides several playlist locks. Each lock somehow filters/modifies playlist contents. \
Unlike autoplaylist, playlist lock allows you to modify content manually.\
Locks are installed/uninstalled through the \"Edit->Playlist locks\" menu. Locks are stackable.\n\n\
Provided locks:\n\
1.Media Library changes tracker\n\
-  Inserts new medial library items or removes deleted items.\n\
2.Remove played\n\
-  Automatically removes all instances of played song.\n\n\
(c) 2010-2012 Duny <majorquake3@gmail.com>");

VALIDATE_COMPONENT_FILENAME(COMPONENT_NAME".dll");


// vk_api guid definitions
const GUID vk_com_api::externals::guid_browser_dlg_cfg =
{ 0xf0da521d, 0x5c9e, 0x4fe5, { 0xa1, 0x5c, 0x3e, 0xa9, 0x67, 0x33, 0x3d, 0xf1 } };

const GUID vk_com_api::externals::guid_auth_manager_cfg =
{ 0x660c4f28, 0x40ac, 0x4d42, { 0xb8, 0x64, 0xb4, 0x53, 0x3e, 0x65, 0x31, 0xed } };

const GUID vk_com_api::externals::auth_manager_class_guid = 
{ 0x6edf8e12, 0x4566, 0x4a42, { 0xb3, 0x8d, 0x1f, 0x09, 0xf3, 0xc3, 0x6f, 0x95 } };

const GUID vk_com_api::externals::api_provider_class_guid = 
{ 0x5f4fbf76, 0xf0ad, 0x424e, { 0xb7, 0x64, 0x94, 0x09, 0x37, 0x00, 0x6b, 0x4c } };


// vk api string defs
const char * vk_com_api::externals::auth_browser_dlg_caption_prefix = "Playlist locks : ";

const char * vk_com_api::externals::vk_api_application_id = "2632594";

char const * vk_com_api::externals::component_name_for_console_log = vk_com_api::externals::auth_browser_dlg_caption_prefix;