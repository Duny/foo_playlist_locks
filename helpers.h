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


// Data structure used by lock_manager for storing information about
// locks installed on specified playlists
// First parameters is playlist id, second is list of guids of
// locks installed on this playlist
typedef boost::tuple<t_size, pfc::list_t<GUID>> playlist_lock_data;

FB2K_STREAM_READER_OVERLOAD(pfc::list_t<GUID>) { t_size n = 0; stream >> n; GUID g; while (n --> 0) { stream >> g; value.add_item (g); } return stream;  }
FB2K_STREAM_WRITER_OVERLOAD(pfc::list_t<GUID>) { t_size n = value.get_size (); while (n --> 0) stream << value[n]; return stream; }

FB2K_STREAM_READER_OVERLOAD(playlist_lock_data) { return read_tuple (stream, value); }
FB2K_STREAM_WRITER_OVERLOAD(playlist_lock_data) { return write_tuple (stream, value); }

#endif