#include "stdafx.h"

DECLARE_COMPONENT_VERSION("Playlist Locks", "1.0.1",
"This component provides several playlist locks. Each lock somehow filters/modifies playlist contents. \
Unlike autoplaylist, playlist lock allows you to modify content manually.\
Locks are installed/uninstalled through the \"Edit->Playlist locks\" menu. Locks are stackable.\n\n\
Provided locks:\n\
1.Media Library changes tracker\n\
-  Inserts new medial library items or removes deleted items.\n\
2.Remove played\n\
-  Automatically removes all instances of played song.\n\n\
(c) 2010-2011 Duny <majorquake3@gmail.com>");

VALIDATE_COMPONENT_FILENAME(COMPONENT_NAME".dll");