#include "application.h"

#include "input_generator.h"
#include "time_utils.h"

#include <set>
#include <cassert>

namespace {

    std::vector<long> parse_array(const std::string& str) {
        char* c;
        const char* s = str.c_str();

        std::vector<long> result;
        while (true) {
            const long tmp = strtol(s, &c, 10);
            if (*c == ',') {
                result.push_back(tmp);
                s = c + 1;
            } else if (*c == '\0') {
                if (c != s) {
                    result.push_back(tmp);
                }
                break;
            } else {
                throw std::logic_error("Invalid character '" + std::string(1, *c) + "' in string \"" + str + "\"");
            }
        }

        if (result.empty()) {
            throw std::logic_error("Expected at least one number");
        }

        return result;
    }

    std::string parse_separators(const std::string& s) {
        std::set<char> set;
        static const std::string reserved_chars{"0123456789+-"};

        for (char c: s) {
            set.insert(c);
        }

        const bool empty     = set.empty();
        const bool too_large = set.size() > 16;
        bool invalid_chars = false;
        for (char c: reserved_chars) {
            if (set.count(c)) {
                invalid_chars = true;
                break;
            }
        }

        if (empty || too_large || invalid_chars) {
            throw Application::ArgumentError
                    ("Separators must be a non empty, up to 16 chars set; "
                     "forbidden chars are: '0'..'9', '+' and '-'.");
        }

        return std::string{set.begin(), set.end()};
    }

} // namespace unnamed


Application::Application(int argc, char* argv[])
    : cmdline(argc, argv)
    , quiet(false)
    , rd()
    , random(rd()) {}


bool Application::run() {
    init();
    custom_init();
    return custom_run();
}


void Application::init() {

    if (cmdline.empty() || cmdline.has_flag("-h") || cmdline.has_flag("--help")) {
        print_help();
        throw Application::Exit();
    }

    auto to_int = [](const std::string& val) {
        return std::stol(val);
    };

    size            = cmdline.parse_value<size_t>("--size", to_int);
    debug_size      = cmdline.parse_value<size_t>("--debug", to_int, 0);
    loop_count      = cmdline.parse_value<size_t>("--loops", to_int, 1);
    separators_set  = cmdline.parse_value<std::string>("--separators", parse_separators, ",; ");

    const auto seed = cmdline.parse_value("--seed", to_int, 0);
    random.seed(seed);

    {
        const auto arr = cmdline.parse_value<std::vector<long>>("--num", parse_array);
        distribution.numbers = discrete_distribution(arr);
    }
    {
        const auto arr = cmdline.parse_value<std::vector<long>>("--sep", parse_array, {1});
        distribution.separators = discrete_distribution(arr);
    }

    if (cmdline.has_value("--sign")) {
        const auto arr = cmdline.parse_value<std::vector<long>>("--sign", parse_array, {});
        if (arr.size() != 3) {
            throw std::logic_error("--sign expects exactly three-item distribution, like --sign=5,2,1");
        }
        distribution.sign = discrete_distribution(arr);
        sign_nonnull = true;
    } else {
        sign_nonnull = false;
    }
}

std::string Application::generate_unsigned() {

    std::string tmp;

    const std::string msg = (quiet) ? "" : "generating random unsigned numbers ";
    measure_time(msg, [&tmp, this]{
        tmp = ::generate_unsigned(
                    size,
                    get_separators_set(),
                    random,
                    distribution.numbers.get_distribution(),
                    distribution.separators.get_distribution());
    });
    assert(tmp.size() == size);

    if (!quiet && debug_size > 0) {
        printf("first %lu bytes of the data:\n", debug_size);
        fwrite(tmp.data(), debug_size, 1, stdout);
        putchar('\n');
    }

    return tmp;
}

std::string Application::generate_signed() {

    std::string tmp;

    const std::string msg = (quiet) ? "" : "generating random signed numbers ";
    measure_time(msg, [&tmp, this]{
        tmp = ::generate_signed(
                    size,
                    get_separators_set(),
                    random,
                    distribution.numbers.get_distribution(),
                    distribution.separators.get_distribution(),
                    distribution.sign.get_distribution());
    });
    assert(tmp.size() == size);

    if (!quiet && debug_size > 0) {
        printf("first %lu bytes of the data:\n", debug_size);
        fwrite(tmp.data(), debug_size, 1, stdout);
        putchar('\n');
    }

    return tmp;
}

void Application::print_help() const {
    printf("Usage: %s [OPTIONS]\n", cmdline.get_program_name().c_str());
    puts("");
    puts("options are");
    puts("");
    puts("--size=NUMBER         input size (in bytes)");
    puts("--loops=NUMBER        how many times a test must be repeated [default: 1]");
    puts("--seed=NUMBER         seed for random number generator [default: 0]");
    puts("--num=DISTRIBUTION    distribution of lengths of numbers");
    puts("--sep=DISTRIBUTION    distribution of lengths of gaps between numbers [default: '1']");
    puts("--separators=string   list of separator characters [default: \",; \"]");
    puts("--sign=DISTRIBUTION   distribution of sign in front of number [default: '1']");
    puts("--debug=K             prints K first bytes of generated input [default: 0]");
    puts("");
    puts("Distribution is given as a list of comma-separated values.");
    puts("For --num and --sep the list length is unbound, for --sign it");
    puts("must have exactly three items.");

    puts("");
    print_custom_help();
}


void Application::custom_init() {
    // do nothing
}


void Application::print_custom_help() const {
    // do nothing
}
