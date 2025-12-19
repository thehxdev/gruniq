namespace gruniq::regexp {

static const uint32_t JIT_OPTIONS = PCRE2_JIT_COMPLETE;
static const uint32_t JIT_STACK_START_SIZE = 32 * 1024;
static const uint32_t JIT_STACK_MAX_SIZE = 512 * 1024;


class Matcher final {
public:
    int error_code;
    size_t error_offset;

    Matcher(PCRE2_SPTR pattern, PCRE2_SPTR subject, PCRE2_SIZE subject_length);

    ~Matcher();

    int next(PCRE2_SPTR *out_substring_start, PCRE2_SIZE *out_substring_length);

    int has_error();

    void error_info(PCRE2_UCHAR *out_error_buffer,
                    PCRE2_SIZE error_buffer_length);

private:
    pcre2_code *re_code = nullptr;
    pcre2_match_data *match_data = nullptr;
    pcre2_match_context *match_context = nullptr;
    PCRE2_SPTR pattern, subject;
    PCRE2_SIZE subject_length, offset = 0;
};


Matcher::Matcher(PCRE2_SPTR pattern, PCRE2_SPTR subject, PCRE2_SIZE subject_length)
    : pattern{pattern}, subject{subject}, subject_length{subject_length}
{
    re_code = pcre2_compile(
        pattern,
        PCRE2_ZERO_TERMINATED,
        PCRE2_CASELESS | PCRE2_MULTILINE,
        &error_code,
        &error_offset,
        NULL
    );

    if (re_code) {
        pcre2_jit_compile(re_code, JIT_OPTIONS);
        match_data = pcre2_match_data_create_from_pattern(re_code, NULL);
        pcre2_jit_stack *jit_stack = pcre2_jit_stack_create(JIT_STACK_START_SIZE,
                                                            JIT_STACK_MAX_SIZE,
                                                            nullptr);
        match_context = pcre2_match_context_create(nullptr);
        pcre2_jit_stack_assign(match_context, nullptr, jit_stack);
    }
}

Matcher::~Matcher() {
    if (re_code) {
        pcre2_match_context_free(match_context);
        pcre2_match_data_free(match_data);
        pcre2_code_free(re_code);
    }
}

int Matcher::next(PCRE2_SPTR *out_substring_start, PCRE2_SIZE *out_substring_length) {
    int rc = pcre2_match(
        re_code,
        subject,
        subject_length,
        offset,
        0,
        match_data,
        match_context
    );
    if (rc < 0)
        return 0;

    PCRE2_SIZE *ovector = pcre2_get_ovector_pointer(match_data);
    *out_substring_start = subject + ovector[0];
    *out_substring_length = ovector[1] - ovector[0];
    offset = ovector[1];

    return 1;
}

int Matcher::has_error() {
    return (re_code == nullptr);
}

void Matcher::error_info(PCRE2_UCHAR *out_error_buffer,
                         PCRE2_SIZE error_buffer_length)
{
    pcre2_get_error_message(error_code, out_error_buffer, error_buffer_length);
}

} // end namespace gruniq::regexp
