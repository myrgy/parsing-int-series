#pragma once

#include <cstdint>
#include <cstring>
#include <stdexcept>

#include "scalar-parse-common.h"

namespace scalar {

    template <typename INSERTER>
    void parse_unsigned(const char* data, size_t size, const char* separators, INSERTER output) {

        uint32_t result = 0;
        size_t digits = 0;

        for (size_t i=0; i < size; i++) {
            const char c = data[i];
            if (c >= '0' && c <= '9') {
                result = 10 * result + (c - '0');
                digits += 1;
            } else if (contains(separators, c)) {
                if (digits > 0) {
                    *output++ = result;
                    result = 0;
                    digits = 0;
                }
            } else {
                throw std::runtime_error("Wrong character (scalar)");
            }
        }

        if (digits > 0) {
            *output++ = result;
        }
    }

} // namespace


