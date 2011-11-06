#include "stdafx.h"

service_ptr_t<playlist_lock> g_lock(new service_impl_t<my_lock>);