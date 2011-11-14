#ifndef _FOO_PLAYLIST_LOCKS_HELPERS_H_
#define _FOO_PLAYLIST_LOCKS_HELPERS_H_

// helpers for tuple stream i/o
template<class T1>
inline stream_reader_formatter<> & read_tuple (stream_reader_formatter<> &stream, boost::tuples::cons<T1, boost::tuples::null_type> &value) { return stream >> value.head; }

inline stream_reader_formatter<> & read_tuple (stream_reader_formatter<> &stream, boost::tuples::null_type &value) { return stream; }

template<class T1, class T2>
inline stream_reader_formatter<> & read_tuple (stream_reader_formatter<> &stream, boost::tuples::cons<T1, T2> &value) { stream >> value.head; return read_tuple (stream, value.tail); }

template<class T1>
inline stream_writer_formatter<> & write_tuple (stream_writer_formatter<> &stream, const boost::tuples::cons<T1, boost::tuples::null_type> &value) { return stream << value.head; }

inline stream_writer_formatter<> & write_tuple (stream_writer_formatter<> &stream, const boost::tuples::null_type &value) { return stream; }

template<class T1, class T2>
inline stream_writer_formatter<> & write_tuple (stream_writer_formatter<> &stream, const boost::tuples::cons<T1, T2> &value) { stream << value.head; return write_tuple (stream, value.tail); }


// Helper to get "inline" GUID definitions. For example:
// some_func (guid_inline<0xbfeaa7ea, 0x6810, 0x41c6, 0x82, 0x6, 0x12, 0x95, 0x5a, 0x89, 0xdf, 0x49>::guid);
template <t_uint32 d1, t_uint16 d2, t_uint16 d3, t_uint8 d4, t_uint8 d5, t_uint8 d6, t_uint8 d7, t_uint8 d8, t_uint8 d9, t_uint8 d10, t_uint8 d11>
struct guid_inline { static const GUID guid;};

template <t_uint32 d1, t_uint16 d2, t_uint16 d3, t_uint8 d4, t_uint8 d5, t_uint8 d6, t_uint8 d7, t_uint8 d8, t_uint8 d9, t_uint8 d10, t_uint8 d11>
__declspec (selectany) const GUID guid_inline<d1, d2, d3, d4, d5, d6, d7, d8, d9, d10, d11>::guid = { d1, d2, d3, { d4, d5, d6, d7, d8, d9, d10, d11 } };


// Empty playlist_lock implementation. Does not implements get_lock_name () function
class playlist_lock_impl_simple : public playlist_lock
{
    t_uint32 get_filter_mask () override { return 0; }

    bool query_items_add (unsigned, metadb_handle_list_cref, const bit_array&) override { return true; }
    bool query_items_reorder (const unsigned*, unsigned) override { return true; }
    bool query_items_remove (const bit_array&, bool) override { return true; }
    bool query_item_replace (unsigned, const metadb_handle_ptr&, const metadb_handle_ptr&) override { return true; }
    bool query_playlist_rename (const char *, unsigned) override { return true; }
    bool query_playlist_remove () override { return true; }
    bool execute_default_action (unsigned) override { return false; }
    void on_playlist_index_change (unsigned) override {}
    void on_playlist_remove() override {}
    void show_ui () override {}
};


// helper: does not register itself. provides empty implementation of playlist_callback members only 
class playlist_callback_impl_simple : public playlist_callback {
public:
    void on_items_added (t_size, t_size, const pfc::list_base_const_t<metadb_handle_ptr>&, const bit_array&) override {}
    void on_items_reordered (t_size, const t_size*, t_size) override {}
    void on_items_removing (t_size, const bit_array&, t_size, t_size) override {}
    void on_items_removed (t_size, const bit_array&, t_size, t_size) override {}
    void on_items_selection_change (t_size, const bit_array&, const bit_array&) override {}
    void on_item_focus_change (t_size, t_size, t_size) override {}

    void on_items_modified (t_size, const bit_array&) override {}
    void on_items_modified_fromplayback (t_size, const bit_array&, play_control::t_display_level) override {}

    void on_items_replaced (t_size, const bit_array&, const pfc::list_base_const_t<t_on_items_replaced_entry>&) override {}

    void on_item_ensure_visible (t_size, t_size) override {}

    void on_playlist_activate (t_size, t_size) override {}
    void on_playlist_created (t_size, const char*, t_size) override {}
    void on_playlists_reorder (const t_size*, t_size) override {}
    void on_playlists_removing (const bit_array&, t_size, t_size) override {}
    void on_playlists_removed (const bit_array&, t_size, t_size) override {}
    void on_playlist_renamed (t_size, const char*, t_size) override {}

    void on_default_format_changed () override {}
    void on_playback_order_changed (t_size) override {}
    void on_playlist_locked (t_size, bool) override {}
};
#endif