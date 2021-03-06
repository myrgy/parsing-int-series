#include <vector>
#include <iterator>
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cctype>

#include "block_info.h"
#include "scalar/scalar-parse-unsigned.h"
#include "scalar/scalar-parse-signed.h"
#include "sse/sse-convert.h"
#include "sse/sse-matcher.h"

class Verify {

    char buffer[17];
    __m128i input;

    std::vector<uint32_t> result;
    std::vector<uint32_t> reference;

public:
    Verify() {
        memset(buffer, 0, sizeof(buffer));
        result.resize(16);
    }

    bool run() {

        unsigned unsupported = 0;

        for (int x=0; x < 65536; x++) {
            generate_input(x);

            const BlockInfo& b = blocks[x];
            const __m128i shuffle_digits = _mm_loadu_si128((const __m128i*)b.shuffle_digits);
            const __m128i shuffled = _mm_shuffle_epi8(input, shuffle_digits);

            using namespace sse;

            if (b.conversion_routine == Conversion::SSE1Digit) {
                convert_1digit(shuffled, b.element_count, &result[0]);
            } else if (b.conversion_routine == Conversion::SSE2Digits) {
                convert_2digits(shuffled, b.element_count, &result[0]);
            } else if (b.conversion_routine == Conversion::SSE3Digits) {
                convert_3digits(shuffled, b.element_count, &result[0]);
            } else if (b.conversion_routine == Conversion::SSE4Digits) {
                convert_4digits(shuffled, b.element_count, &result[0]);
            } else if (b.conversion_routine == Conversion::SSE8Digits) {
                convert_8digits(shuffled, b.element_count, &result[0]);
            } else {
                unsupported += 1;
                continue;
            }

            if (!compare(b.element_count)) {
                return false;
            }
        } // for

        printf("All OK (%d cases will never be supported by SIMD code)\n", unsupported);
        return true;
    }

private:
    void generate_input(uint16_t x) {

        int k = 0;
        for (int i=0; i < 16; i++) {
            if (x & (1 << i)) {
                buffer[i] = (k % 10) + '0';
                k += 1;
            } else {
                buffer[i] = '_';
            }
        }

        input = _mm_loadu_si128((const __m128i*)buffer);

        std::fill(result.begin(), result.end(), -1);

        reference.clear();
        scalar::parse_unsigned(buffer, 16, "_", std::back_inserter(reference));
    }

    bool compare(size_t n) const {
        for (size_t i=0; i < n; i++) {
            if (result[i] != reference[i]) {
                printf("mismatch at %lu: expected=%u, result=%u\n", i, reference[i], result[i]);
                printf("reference = "); dump(reference, n);
                printf("result =    "); dump(result, n);
                return false;
            }
        }

        return true;
    }

    void dump(const std::vector<uint32_t>& vec, size_t n) const {
        for (size_t i=0; i < n; i++) {
            if (i > 0) printf(", ");
            printf("%u", vec[i]);
        }
        putchar('\n');
    }

};


int main() {

    puts("Verify SSE unsigned converters for valid inputs");
    Verify verify;
    if (!verify.run()) {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

