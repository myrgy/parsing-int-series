.PHONY: all clean

FLAGS:=-std=c++11 -Wall -Wextra -pedantic -march=native -O3 $(CXXFLAGS)
FLAGS:=$(FLAGS) -Iinclude -Iinclude/test

PARSER_COMMON_HEADERS= \
    include/block_info.h \
    include/scalar/scalar-parse-common.h\
    include/sse/sse-utils.h \
    include/sse/sse-convert.h \
    include/sse/sse-matcher.h \
    include/sse/sse-matcher-stni.h \
    include/sse/sse-parser-common.h

PARSER_UNSIGNED_HEADERS= \
    include/scalar/scalar-parse-unsigned.h \
    include/sse/sse-parser-unsigned.h \
    include/sse/sse-block-parser-unsigned.h

PARSER_SIGNED_HEADERS= \
    include/scalar/scalar-parse-signed.h \
    include/scalar/std-parser-signed.h \
    include/sse/sse-parser-signed.h \
    include/sse/sse-block-parser-signed.h \
    include/sse/sse-simplified-parser-signed.h

PARSER_HYBRID_DEPS= \
    include/hybrid-parser.h \
    include/hybrid-parser-unsigned.inl \
    include/hybrid-parser-signed.h \
    include/hybrid-parser-signed.inl \
    include/hybrid-shift-back.inl

PARSER_AVX512_HEADERS= \
    include/scalar/scalar-parse-signed.h \
    include/sse/sse-parser-signed.h \
    include/avx512/avx512-parser-signed.h

PARSER_OBJ= \
    obj/block_info.o

PARSER_UNSIGNED_DEPS=$(PARSER_COMMON_HEADERS) $(PARSER_UNSIGNED_HEADERS) $(PARSER_OBJ)
PARSER_SIGNED_DEPS=$(PARSER_COMMON_HEADERS) $(PARSER_SIGNED_HEADERS) $(PARSER_OBJ)
PARSER_DEPS=$(PARSER_COMMON_HEADERS) $(PARSER_UNSIGNED_HEADERS) $(PARSER_SIGNED_HEADERS) $(PARSER_OBJ)
PARSER_AVX512_DEPS=$(PARSER_COMMON_HEADERS) $(PARSER_AVX512_HEADERS) $(PARSER_OBJ)

CMDLINE_OBJ= \
    obj/application.o \
    obj/command_line.o \
    obj/discrete_distribution.o \
    obj/input_generator.o \
    $(PARSER_OBJ)

CMDLINE_DEPS=include/test/*.h test/utils/*cpp $(CMDLINE_OBJ)


UNITTESTS= \
    bin/test-stni-matcher \
    bin/verify_sse_signed_parser \
    bin/verify_sse_signed_parser_validation \
    bin/verify_sse_unsigned_conversion \
    bin/verify_sse_unsigned_parser \

BENCHMARK= \
    bin/benchmark \
    bin/benchmark-all \
    bin/benchmark-cpuclocks \
    bin/benchmark-hwevents

TEST= \
    bin/compare-signed \
    bin/compare-unsigned \
    bin/compare-avx512 \
    bin/statistics \
    bin/spanmaskhistogram


ALL=$(UNITTESTS) $(BENCHMARK) $(TEST)

all: $(ALL)

clean:
	$(RM) $(ALL) obj/*.o

run-unittests: $(UNITTESTS)
	./bin/test-stni-matcher
	./bin/verify_sse_signed_parser
	./bin/verify_sse_signed_parser_validation
	./bin/verify_sse_unsigned_conversion
	./bin/verify_sse_unsigned_parser

# --------------------------------------------------------------------------------

obj/block_info.o: src/block_info.cpp src/block_info.inl include/block_info.h
	$(CXX) $(FLAGS) -c $< -o $@

src/block_info.inl: scripts/generator.py scripts/writer.py
	python $< $@


# unit tests
# --------------------------------------------------------------------------------
bin/test-stni-matcher: test/unittest/test-stni-matcher.cpp include/sse/sse-matcher-stni.h
	$(CXX) $(FLAGS) $< -o $@

bin/verify_sse_signed_parser: test/unittest/verify_sse_signed_parser.cpp $(PARSER_SIGNED_DEPS)
	$(CXX) $(FLAGS) $(PARSER_OBJ) $< -o $@

bin/verify_sse_signed_parser_validation: test/unittest/verify_sse_signed_parser_validation.cpp $(PARSER_SIGNED_DEPS)
	$(CXX) $(FLAGS) $(PARSER_OBJ) $< -o $@

bin/verify_sse_unsigned_conversion: test/unittest/verify_sse_unsigned_conversion.cpp $(PARSER_UNSIGNED_DEPS)
	$(CXX) $(FLAGS) $(PARSER_OBJ) $< -o $@

bin/verify_sse_unsigned_parser: test/unittest/verify_sse_unsigned_parser.cpp $(PARSER_UNSIGNED_DEPS)
	$(CXX) $(FLAGS) $(PARSER_OBJ) $< -o $@


# test programs
# --------------------------------------------------------------------------------

bin/benchmark: test/benchmark.cpp $(PARSER_DEPS) $(CMDLINE_DEPS)
	$(CXX) $(FLAGS) $(CMDLINE_OBJ) $< -o $@

bin/benchmark-all: test/benchmark-all.cpp $(PARSER_DEPS) $(CMDLINE_DEPS) $(PARSER_HYBRID_DEPS)
	$(CXX) $(FLAGS) $(CMDLINE_OBJ) $< -o $@

bin/benchmark-hwevents: test/benchmark-hwevents.cpp $(PARSER_SIGNED_DEPS) $(CMDLINE_DEPS)
	$(CXX) $(FLAGS) $(CMDLINE_OBJ) $< -o $@

bin/benchmark-cpuclocks: test/benchmark-cpuclocks.cpp $(PARSER_SIGNED_DEPS) $(CMDLINE_DEPS) $(PARSER_HYBRID_DEPS)
	$(CXX) $(FLAGS) $(CMDLINE_OBJ) $< -o $@

bin/compare-signed: test/compare-signed.cpp $(PARSER_SIGNED_DEPS) $(CMDLINE_DEPS)
	$(CXX) $(FLAGS) $(CMDLINE_OBJ) $< -o $@

bin/compare-unsigned: test/compare-unsigned.cpp $(PARSER_SIGNED_DEPS) $(CMDLINE_DEPS)
	$(CXX) $(FLAGS) $(CMDLINE_OBJ) $< -o $@

bin/compare-avx512: test/compare-avx512.cpp $(PARSER_AVX512_DEPS) $(CMDLINE_DEPS)
	$(CXX) $(FLAGS) -mavx512vbmi $(CMDLINE_OBJ) $< -o $@

bin/statistics: test/statistics.cpp $(PARSER_DEPS) $(CMDLINE_DEPS) obj/sse-parser-statistics.o
	$(CXX) $(FLAGS) $(CMDLINE_OBJ) -DUSE_STATISTICS obj/sse-parser-statistics.o $< -o $@

bin/spanmaskhistogram: test/spanmaskhistogram.cpp $(PARSER_DEPS) $(CMDLINE_DEPS) obj/sse-parser-statistics.o
	$(CXX) $(FLAGS) $(CMDLINE_OBJ) -DUSE_STATISTICS obj/sse-parser-statistics.o $< -o $@

# cmdline utilites
# --------------------------------------------------------------------------------

obj/application.o: test/utils/application.cpp include/test/*.h obj/command_line.o obj/discrete_distribution.o obj/input_generator.o
	$(CXX) $(FLAGS) -c $< -o $@

obj/command_line.o: test/utils/command_line.cpp include/test/command_line.h
	$(CXX) $(FLAGS) -c $< -o $@

obj/discrete_distribution.o: test/utils/discrete_distribution.cpp include/test/discrete_distribution.h
	$(CXX) $(FLAGS) -c $< -o $@

obj/input_generator.o: test/utils/input_generator.cpp include/test/input_generator.h
	$(CXX) $(FLAGS) -c $< -o $@

obj/sse-parser-statistics.o: src/sse-parser-statistics.cpp include/sse/sse-parser-statistics.h
	$(CXX) $(FLAGS) -c $< -o $@


# hybrid parser
# --------------------------------------------------------------------------------

include/hybrid-parser-unsigned.inl: scripts/hybrid-unsigned.py scripts/hybrid.py
	python $< > $@

include/hybrid-parser-signed.inl: scripts/hybrid-signed.py scripts/hybrid.py
	python $< > $@

include/hybrid-shift-back.inl: scripts/hybrid-shift-back.py scripts/hybrid.py
	python $< > $@


# overall experiments
# --------------------------------------------------------------------------------

overall.txt: bin/benchmark experiments/overalltests/experiment.py experiments/overalltests/testcases.py
	# this is a long-running procedure, it'd be better to see if the program really works
	python experiments/overalltests/experiment.py | tee /tmp/$@
	mv /tmp/$@ $@

report-overall.rst: overall.txt experiments/overalltests/report.py experiments/overalltests/report_writer.py
	python experiments/overalltests/report.py $< "^#*" > /tmp/$@
	mv /tmp/$@ $@

report-overall-short.rst: overall.txt experiments/overalltests/average.py experiments/overalltests/average_writer.py
	python experiments/overalltests/average.py $< "^#*" > /tmp/$@
	mv /tmp/$@ $@

# microbenchmarks
# --------------------------------------------------------------------------------

microbenchmarks.txt: bin/benchmark-cpuclocks experiments/microbenchmarks/experiment.py experiments/microbenchmarks/testcases.py
	# this is a long-running procedure, it'd be better to see if the program really works
	python experiments/microbenchmarks/experiment.py | tee /tmp/$@
	mv /tmp/$@ $@

microbenchmarks.rst: microbenchmarks.txt experiments/microbenchmarks/report.py experiments/microbenchmarks/writer.py
	python experiments/microbenchmarks/report.py $< "^#" > /tmp/$@
	mv /tmp/$@ $@

# span_pattern histogram

hwevents.txt: bin/benchmark-hwevents experiments/hwevents/experiment.py experiments/hwevents/runner.py experiments/hwevents/testcases.py
	python experiments/hwevents/experiment.py > /tmp/$@
	mv /tmp/$@ $@

spanmaskhistogram.txt: bin/spanmaskhistogram experiments/spanmaskhistogram/experiment.py experiments/spanmaskhistogram/testcases.py
	python experiments/spanmaskhistogram/experiment.py > /tmp/$@
	mv /tmp/$@ $@

spanmaskhistogram.rst: spanmaskhistogram.txt hwevents.txt microbenchmarks.txt experiments/spanmaskhistogram/report.py experiments/spanmaskhistogram/report_writer.py
	python experiments/spanmaskhistogram/report.py spanmaskhistogram.txt hwevents.txt microbenchmarks.txt /tmp/$@ "^"
	mv /tmp/$@ $@

