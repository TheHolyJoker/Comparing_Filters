#include "minimal_tests.hpp"

bool print_pass() {
    std::cout << "Passed tests!" << std::endl;
    return true;
}


void get_fp_ratios() {

    using itemType = uint64_t;
    using spare_item = uint64_t;
    using temp_hash = hashTable_Aligned<spare_item, 4>;

    using Table_TC320 = twoChoicer320<itemType>;
    using Table_Dict320 = Dict320<temp_hash, spare_item, itemType>;
    using Table_Dict512 = Dict512<temp_hash, spare_item, itemType>;
    using Table_Dict512_Ver2 = Dict512_Ver2<temp_hash, spare_item, itemType>;
    using Table_CF = cuckoofilter::CuckooFilter<uint64_t, BITS_PER_ELEMENT_MACRO, cuckoofilter::SingleTable>;
    using Table_Dict_CF = Dict512_With_CF<itemType>;
    using Table_Dict512_SS = Dict512_SparseSpare<itemType>;
    using Table_Dict512_Ver3 = Dict512_Ver3<temp_hash, spare_item, itemType>;

    assert((default_validation_test_single<Table_Dict512, itemType>()));
    assert((default_validation_test_single<Table_Dict512_SS, itemType>()));
    assert((default_validation_test_single<Table_Dict_CF, itemType>()));
    assert(print_pass());

    const size_t fp_capacity = std::ceil((1 << 23u) * 0.88);
    const size_t fp_lookups = (1 << 25u);
    const size_t bits_per_element = 8;

    vector<itemType> fp_v_add, fp_v_find;                         //, v_delete;
    vector<vector<itemType> *> fp_elements{&fp_v_add, &fp_v_find};//, &v_delete};
    init_elements(fp_capacity, fp_lookups, &fp_elements, false);
    // return single_fp_rates_probabilistic<Table, itemType>(filter_max_capacity, lookup_reps, bits_per_item, &fp_elements);
    single_fp_rates_probabilistic<Table_Dict512_Ver3, itemType>(fp_capacity, fp_lookups, 8, &fp_elements);
    single_fp_rates_probabilistic<Table_Dict512, itemType>(fp_capacity, fp_lookups, 8, &fp_elements);
    single_fp_rates_probabilistic<Table_Dict512_SS, itemType>(fp_capacity, fp_lookups, 8, &fp_elements);
    single_fp_rates_probabilistic<Table_CF, itemType>(fp_capacity, fp_lookups, 8, &fp_elements);
}

int main(int argc, char **argv) {
    // std::cout << "Here!" << std::endl;
    // assert(0);
    using itemType = uint64_t;
    using spare_item = uint64_t;
    using temp_hash = hashTable_Aligned<spare_item, 4>;

    using Table_TC = twoChoicer<itemType>;
    using Table_TC320 = twoChoicer320<itemType>;
    using Table_Dict320 = Dict320<temp_hash, spare_item, itemType>;
    using Table_Dict512 = Dict512<temp_hash, spare_item, itemType>;
    using Table_Dict512_Ver2 = Dict512_Ver2<temp_hash, spare_item, itemType>;
    using Table_CF = cuckoofilter::CuckooFilter<uint64_t, BITS_PER_ELEMENT_MACRO, cuckoofilter::SingleTable>;
    using Table_Dict_CF = Dict512_With_CF<itemType>;
    using Table_Dict512_SS = Dict512_SparseSpare<itemType>;
    using Table_Dict512_Ver3 = Dict512_Ver3<temp_hash, spare_item, itemType>;


    assert((default_validation_test_single<Table_Dict512, itemType>()));
    assert((default_validation_test_single<Table_Dict512_Ver3, itemType>()));
    // assert((validation_test_single<Table_Dict512_Ver3, itemType>(1<<22,1<<22)));
    // assert((default_validation_test_single<Table_Dict512_SS, itemType>()));
    // assert((default_validation_test_single<Table_Dict_CF, itemType>()));
    // assert((default_validation_test_single<Table_TC, itemType>()));
    assert(print_pass());

    // get_fp_ratios();

    const size_t max_filter_capacity = 62411242;
    const size_t lookup_reps = 124822484;
    const size_t bits_per_element = 8;
    const size_t bench_precision = 16;
    vector<itemType> v_add, v_find, v_delete;
    vector<vector<itemType> *> elements{&v_add, &v_find, &v_delete};
    fill_vec(elements.at(0), max_filter_capacity);
    fill_vec(elements.at(1), lookup_reps);
    size_t del_size = 0;
    // size_t del_size = 1.0 * max_filter_capacity / (double) bench_precision;
    fill_vec(elements.at(2), del_size);

    /** Single benching  */
    // single_bench<Table_Dict512_SS, itemType>(max_filter_capacity, bench_precision, false, &elements);
    // return 0;
    while (true)
    {single_bench<Table_Dict512_Ver3, itemType>(max_filter_capacity, bench_precision, false, &elements);}
    // single_bench<Table_CF, itemType>(max_filter_capacity, bench_precision, false, &elements);
    return 0;
    // while (true) {
    //     // single_bench<Table_Dict512_SS, itemType>(max_filter_capacity, bench_precision, false, &elements);
    //     single_bench<Table_Dict512_Ver3, itemType>(max_filter_capacity, bench_precision, false, &elements);
    //     single_bench<Table_Dict512, itemType>(max_filter_capacity, bench_precision, false, &elements);
    // }

    // // single_bench<Table_TC, itemType>(max_filter_capacity, bench_precision, false, &elements);
    // single_bench<Table_Dict512_SS, itemType>(max_filter_capacity, bench_precision, false, &elements);
    // // single_bench<Table_Dict_CF, itemType>(max_filter_capacity, bench_precision, false, &elements);
    // // single_bench<Table_Dict512_Ver2, itemType>(max_filter_capacity, bench_precision, false, &elements);
    // // single_bench<Table_Dict320, itemType>(max_filter_capacity, bench_precision, false, &elements);
    // // single_bench<Table_CF, itemType>(max_filter_capacity, bench_precision, false, &elements);
    // // }


    // return 0;
}