//
// Created by root on 31/05/2020.
//

#include "new_tests.hpp"

auto example1() {
    ulong shift = 16u;
    size_t filter_max_capacity = 1u << shift;
    size_t lookup_reps = 1u << (shift + 2u);
    size_t error_power_inv = BITS_PER_ELEMENT_MACRO;
    size_t bench_precision = 20;
    b_all_wrapper<uint64_t, BITS_PER_ELEMENT_MACRO>(filter_max_capacity, lookup_reps, error_power_inv, bench_precision,
                                                    1, 1, 1, 1);
}




int main(int argc, char **argv) {
    validate_example1();


    //Default values
    size_t filter_indicator = -1;
    ulong shift = 30u;
    size_t shift_add_to_lookups = 2u;
    size_t bench_precision = 20;
    size_t remainder_length = BITS_PER_ELEMENT_MACRO;

    size_t reps = 1u << (shift + shift_add_to_lookups), max_distinct_capacity = 1u << shift;

    /**Parsing*/
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    char *end;
    size_t values[4]{filter_indicator, shift, shift_add_to_lookups, bench_precision};
    for (int i = 1; i < argc; ++i) {
        values[i - 1] = strtol(argv[i], &end, 10);
    }

    filter_indicator = values[0];
    shift = values[1];
    shift_add_to_lookups = values[2];
    bench_precision = values[3];

//    return 0;
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    if (filter_indicator == 0)
        filter_indicator = -1;
    reps = 1u << (shift + shift_add_to_lookups), max_distinct_capacity = 1u << shift;
    using itemType = uint64_t ;
    b_all_wrapper<itemType, BITS_PER_ELEMENT_MACRO>(max_distinct_capacity, reps, BITS_PER_ELEMENT_MACRO, bench_precision, filter_indicator & 1,  filter_indicator & 2,  filter_indicator & 4,  filter_indicator & 8);
}