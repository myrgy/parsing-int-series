#pragma once

#include <cstdint>
#include <cstring>
#include <stdexcept>

#include "scalar-parse-common.h"

namespace scalar {

    template <typename INSERTER>
    void parse_signed(const char* data, size_t size, const char* separators, INSERTER output) {

        enum State {
            Separator,
            Plus,
            Minus,
            Digit
        };

        State state = Separator;
        State prev = Separator;
        bool negative = false;
        int32_t number = 0;

        for (size_t i=0; i < size; i++) {
            const char c = data[i];
            if (c == '+') {
                state = Plus;
            } else if (c == '-') {
                state = Minus;
            } else if (c >= '0' && c <= '9') {
                state = Digit;
            } else if (contains(separators, c)) {
                state = Separator;
            } else {
                throw std::runtime_error("Wrong character (scalar)");
            }

            switch (state) {
                case Plus:
                    if (prev != Separator) {
                        throw std::runtime_error("Invalid syntax ('+' follows a non-separator character)");
                    }
                    number = 0;
                    negative = false;
                    break;

                case Minus:
                    if (prev != Separator) {
                        throw std::runtime_error("Invalid syntax ('-' follows a non-separator character)");
                    }
                    number = 0;
                    negative = true;
                    break;

                case Digit:
                    if (prev == Separator) {
                        number = c - '0';
                        negative = false;
                    } else {
                        number = 10*number + c - '0';
                    }
                    break;

                case Separator:
                    if (prev == Digit) {
                        if (negative) {
                            *output = -number;
                        } else {
                            *output = number;
                        }
                    } else if (prev != Separator) {
                        throw std::runtime_error("Invalid syntax ('-' or '+' not followed by any digit)");
                    }
                    break;
            } // switch

            prev = state;
        } // for

        if (state == Separator) {
            if (prev == Digit) {
                if (negative) {
                    *output = -number;
                } else {
                    *output = number;
                }
            } else if (prev != Separator) {
                throw std::runtime_error("Invalid syntax ('-' or '+' not followed by any digit)");
            }
        }
    }

} // namespace

