static const uint32_t REGEXP_PCRE2_JIT_OPTIONS = PCRE2_JIT_COMPLETE;
static const uint32_t REGEXP_PCRE2_JIT_STACK_START_SIZE = 32 * 1024;
static const uint32_t REGEXP_PCRE2_JIT_STACK_MAX_SIZE = 512 * 1024;

typedef struct {
    pcre2_code *re_code;
    pcre2_match_data *match_data;
    pcre2_match_context *match_context;
    PCRE2_SPTR pattern, subject;
    PCRE2_SIZE subject_length, offset, error_offset;
    int error_code;
} Matcher;

int matcher_init(Matcher *matcher,
                 PCRE2_SPTR pattern,
                 PCRE2_SPTR subject,
                 PCRE2_SIZE subject_length)
{
    pcre2_code *re_code;
    re_code = pcre2_compile(
        pattern,
        PCRE2_ZERO_TERMINATED,
        PCRE2_CASELESS | PCRE2_MULTILINE,
        &matcher->error_code,
        &matcher->error_offset,
        NULL
    );

    if (!re_code)
        return 0;

    pcre2_jit_compile(re_code, REGEXP_PCRE2_JIT_OPTIONS);
    matcher->match_data = pcre2_match_data_create_from_pattern(re_code, STH_NULL);

    pcre2_jit_stack *jit_stack = pcre2_jit_stack_create(REGEXP_PCRE2_JIT_STACK_START_SIZE,
                                                        REGEXP_PCRE2_JIT_STACK_MAX_SIZE,
                                                        STH_NULL);

    matcher->match_context = pcre2_match_context_create(STH_NULL);
    pcre2_jit_stack_assign(matcher->match_context, STH_NULL, jit_stack);

    matcher->re_code = re_code;
    return 1;
}

void matcher_deinit(Matcher *matcher) {
    if (matcher->re_code) {
        pcre2_match_context_free(matcher->match_context);
        pcre2_match_data_free(matcher->match_data);
        pcre2_code_free(matcher->re_code);
    }
}

int matcher_next(Matcher *matcher,
                 PCRE2_SPTR *out_substring_start,
                 PCRE2_SIZE *out_substring_length)
{
    int rc = pcre2_match(
        matcher->re_code,
        matcher->subject,
        matcher->subject_length,
        matcher->offset,
        0,
        matcher->match_data,
        matcher->match_context
    );
    if (rc < 0)
        return 0;

    PCRE2_SIZE *ovector = pcre2_get_ovector_pointer(matcher->match_data);
    *out_substring_start = matcher->subject + ovector[0];
    *out_substring_length = ovector[1] - ovector[0];
    matcher->offset = ovector[1];

    return 1;
}

void matcher_error_info(Matcher *matcher,
                        PCRE2_UCHAR *out_error_buffer,
                        PCRE2_SIZE error_buffer_length)
{
    pcre2_get_error_message(matcher->error_code, out_error_buffer, error_buffer_length);
}
