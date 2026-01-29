static const size_t ERROR_BUFFER_SIZE = 256;

void usage(const char *program_name);

int main(int argc, char *argv[]) {
    struct bloom bloom;
    char *subject = NULL;
    size_t subject_length;
    PCRE2_SPTR substring_start;
    PCRE2_SIZE substring_length;
    PCRE2_UCHAR error_buffer[ERROR_BUFFER_SIZE];

    if (argc != 3) {
        usage(argv[0]);
        return 1;
    }

    const PCRE2_SPTR pattern = (PCRE2_SPTR)argv[1];
    if (! (subject = sth_io_file_read_all(argv[2], &subject_length))) {
        fprintf(stderr, "failed to read \'%s\' file\n", argv[2]);
        return 1;
    }

    Matcher matcher = { 0 };
    if (!matcher_init(&matcher, pattern, subject, subject_length)) {
        matcher_error_info(&matcher, error_buffer, sizeof(error_buffer));
        fprintf(stderr, "failed to initialize matcher (%d): error at offset %zu: %s\n",
                matcher.error_code, matcher.error_offset, error_buffer);
        return 1;
    }

    if (bloom_init(&bloom, (1<<20), 0.01f)) {
        fprintf(stderr, "failed to initialize bloom filter\n");
        return 1;
    }

    while (matcher_next(&matcher, &substring_start, &substring_length)) {
        // filter unique matches with bloom filter
        if (!bloom_check(&bloom, substring_start, (int)substring_length)) {
            bloom_add(&bloom, substring_start, (int)substring_length);
            printf("%.*s\n", (int)substring_length, (char*)substring_start);
        }
    }

    matcher_deinit(&matcher);
    return 0;
}

void usage(const char *program_name) {
    fprintf(stderr, "Usage: %s <pattern> <file>\n", program_name);
}
