#define CHECK_ID_INBOUND(id, max_id, entity_type) \
    if (id >= max_id) { \
        TraceLog(LOG_ERROR, "Can't get %s with id %d", #entity_type, id); \
        exit(1); \
    }

char* read_cstr_file(
    const char* restrict file_path, const char* mode, long* n_bytes
);
