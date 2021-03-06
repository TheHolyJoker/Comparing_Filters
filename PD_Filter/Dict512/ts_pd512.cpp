#include "ts_pd512.hpp"


namespace v_ts_pd512 {
    size_t popcount_up_to_index(uint64_t word, size_t index) {
        assert(index <= sizeof(word) * CHAR_BIT);
        if (index == sizeof(word) * CHAR_BIT)
            return _mm_popcnt_u64(word);

        uint64_t mask = BZ_MSK(index);
        return _mm_popcnt_u64(word & mask);
    }

    size_t popcount_up_to_index(uint64_t *arr, size_t a_size, size_t index);

    bool v_write_header_cond12(uint64_t h0, size_t index, uint64_t quot) {
        size_t res1 = popcount_up_to_index(h0, index);
        bool cond1 = res1 == (quot - 1);

        size_t res2 = popcount_up_to_index(h0, index + 1);
        bool cond2 = res2 == quot;

        assert(cond1 && cond2);
        return cond1 && cond2;
    }

    bool validate_write_header(int64_t quot, uint64_t index, __m512i *pd) {
        static int counter = 0;
        counter++;
        const __m512i old_pd = *pd;
        __m512i temp_pd0 = old_pd;
        __m512i temp_pd1 = old_pd;
        __m512i temp_pd2 = old_pd;
        ts_pd512::write_header_naive(42, index, &temp_pd1);
        ts_pd512::write_header6(index, &temp_pd2);

        if (!compare_headers(&temp_pd1, &temp_pd2)) {
            // std::cout << "Headers differ" << std::endl;
            size_t temp0_select_quot = pd512_plus::count_zeros_up_to_the_kth_one(&temp_pd0, quot) + quot;
            size_t temp1_select_quot = pd512_plus::count_zeros_up_to_the_kth_one(&temp_pd1, quot) + quot;
            size_t temp2_select_quot = pd512_plus::count_zeros_up_to_the_kth_one(&temp_pd2, quot) + quot;

            size_t temp0_select_quot_m1 = pd512_plus::count_zeros_up_to_the_kth_one(&temp_pd0, quot - 1) + quot - 1;
            size_t temp1_select_quot_m1 = pd512_plus::count_zeros_up_to_the_kth_one(&temp_pd1, quot - 1) + quot - 1;
            size_t temp2_select_quot_m1 = pd512_plus::count_zeros_up_to_the_kth_one(&temp_pd2, quot - 1) + quot - 1;
            std::cout << "temp0_select_quot: " << temp0_select_quot << std::endl;
            std::cout << "temp1_select_quot: " << temp1_select_quot << std::endl;
            std::cout << "temp2_select_quot: " << temp2_select_quot << std::endl;
            std::cout << "temp0_select_quot_m1: " << temp0_select_quot_m1 << std::endl;
            std::cout << "temp1_select_quot_m1: " << temp1_select_quot_m1 << std::endl;
            std::cout << "temp2_select_quot_m1: " << temp2_select_quot_m1 << std::endl;

            v_pd512_plus::print_headers_extended(&temp_pd0);
            v_pd512_plus::print_headers_extended(&temp_pd1);
            v_pd512_plus::print_headers_extended(&temp_pd2);

            assert(0);
        }

        return true;
    }
    size_t v_get_element_body_index(int64_t quot, uint8_t rem, const __m512i *pd) {
        bool cond = ts_pd512::find(quot, rem, pd);
        if (!cond)
            return 128;

        uint64_t begin = pd512_plus::count_ones_up_to_the_kth_zero(pd, quot - 1);
        uint64_t end = pd512_plus::count_ones_up_to_the_kth_zero(pd, quot);

        const uint64_t begin_fingerprint = begin - quot;
        const uint64_t end_fingerprint = end - quot;
        assert(begin_fingerprint <= end_fingerprint);
        assert(end_fingerprint <= ts_pd512::MAX_CAPACITY);

        uint64_t i = begin_fingerprint;
        for (; i < end_fingerprint; ++i) {
            if (rem == ((const uint8_t *) pd)[ts_pd512::kBytes2copy + i])
                return i;
        }
        assert(false);
        return 128;
    }

    size_t v_get_element_header_index(int64_t quot, uint8_t rem, const __m512i *pd) {
        bool cond = ts_pd512::find(quot, rem, pd);
        if (!cond)
            return 128;

        uint64_t begin = pd512_plus::count_ones_up_to_the_kth_zero(pd, quot - 1);
        uint64_t end = pd512_plus::count_ones_up_to_the_kth_zero(pd, quot);

        const uint64_t begin_fingerprint = begin - quot;
        const uint64_t end_fingerprint = end - quot;
        assert(begin_fingerprint <= end_fingerprint);
        assert(end_fingerprint <= ts_pd512::MAX_CAPACITY);

        uint64_t i = begin_fingerprint;
        for (; i < end_fingerprint; ++i) {
            if (rem == ((const uint8_t *) pd)[ts_pd512::kBytes2copy + i])
                return begin + i;
        }
        assert(false);
        return 128;
    }

    bool validate_header_remove(int64_t quot, uint8_t rem, __m512i *pd) {
        const __m512i old_pd = *pd;
        __m512i temp_pd0 = old_pd;
        __m512i temp_pd1 = old_pd;
        __m512i temp_pd2 = old_pd;

        size_t index = v_get_element_header_index(quot, rem, pd);
        if (index == 128) {
            return true;
        }
        ts_pd512::header_remove_naive(index, &temp_pd1);
        ts_pd512::header_remove(index, &temp_pd2);

        if (!compare_headers(&temp_pd1, &temp_pd2)) {
            // std::cout << "Headers differ" << std::endl;
            size_t temp0_select_quot = pd512_plus::count_zeros_up_to_the_kth_one(&temp_pd0, quot) + quot;
            size_t temp1_select_quot = pd512_plus::count_zeros_up_to_the_kth_one(&temp_pd1, quot) + quot;
            size_t temp2_select_quot = pd512_plus::count_zeros_up_to_the_kth_one(&temp_pd2, quot) + quot;

            size_t temp0_select_quot_m1 = pd512_plus::count_zeros_up_to_the_kth_one(&temp_pd0, quot - 1) + quot - 1;
            size_t temp1_select_quot_m1 = pd512_plus::count_zeros_up_to_the_kth_one(&temp_pd1, quot - 1) + quot - 1;
            size_t temp2_select_quot_m1 = pd512_plus::count_zeros_up_to_the_kth_one(&temp_pd2, quot - 1) + quot - 1;
            std::cout << "index:                " << index << std::endl;
            std::cout << "temp0_select_quot:    " << temp0_select_quot << std::endl;
            std::cout << "temp1_select_quot:    " << temp1_select_quot << std::endl;
            std::cout << "temp2_select_quot:    " << temp2_select_quot << std::endl;
            std::cout << "temp0_select_quot_m1: " << temp0_select_quot_m1 << std::endl;
            std::cout << "temp1_select_quot_m1: " << temp1_select_quot_m1 << std::endl;
            std::cout << "temp2_select_quot_m1: " << temp2_select_quot_m1 << std::endl;

            v_pd512_plus::print_headers_extended(&temp_pd0);
            v_pd512_plus::print_headers_extended(&temp_pd1);
            v_pd512_plus::print_headers_extended(&temp_pd2);

            assert(0);
        }

        return true;
    }


    void print_body(const __m512i *pd) {
        std::cout << std::string(80, '=') << std::endl;
        uint8_t temp_arr[ts_pd512::MAX_CAPACITY];
        memcpy(temp_arr, &((uint8_t *) pd)[ts_pd512::kBytes2copy], ts_pd512::MAX_CAPACITY);
        for (size_t i = 0; i < 6; i++) {
            for (size_t j = 0; j < 8; j++) {
                size_t temp_index = i * 8 + j;
                std::cout << std::setw(3) << (uint16_t) temp_arr[temp_index] << " | ";
            }
            std::cout << std::endl;
        }
        for (size_t i = 0; i < 3; i++) {
            size_t temp_index = 48 + i;
            std::cout << std::setw(3) << (uint16_t) temp_arr[temp_index] << " | ";
        }
        std::cout << std::endl;
        std::cout << std::string(80, '=') << std::endl;
    }

    bool validate_body_add(size_t end_fingerprint, uint8_t rem, __m512i *pd) {
        const __m512i old_pd = *pd;
        __m512i temp_pd0 = old_pd;
        __m512i temp_pd1 = old_pd;
        __m512i temp_pd2 = old_pd;

        ts_pd512::body_add_naive(end_fingerprint, rem, &temp_pd0);
        ts_pd512::body_add(end_fingerprint, rem, &temp_pd1);
        bool cond1 = my_equal(temp_pd0, temp_pd1);
        ts_pd512::body_add2(end_fingerprint, rem, &temp_pd2);

        // bool cond1 = my_equal(temp_pd0,temp_pd1);
        bool cond2 = my_equal(temp_pd0, temp_pd2);

        assert(cond1);
        assert(cond2);
        return true;
    }

    size_t get_last_occupied_quot_naive(const __m512i *pd) {
        if (pd512::get_capacity_naive(pd) == 0) {
            std::cout << "the PD is empty!" << std::endl;
            assert(0);
        }
        for (size_t i = ts_pd512::QUOTS - 1; i >= 0; i--) {
            if (ts_pd512::get_specific_quot_capacity_naive2(i, pd))
                return i;
        }
        assert(0);
    }

    bool compare_headers(const __m512i *pd1, const __m512i *pd2) {
        auto temp_res = memcmp((uint8_t *) pd1, (uint8_t *) pd2, 12) == 0;
        if (temp_res) {
            auto res = memcmp((uint8_t *) pd1, (uint8_t *) pd2, 13) == 0;
            if (!res) {
                std::cout << "Header comparison failed in the last byte." << std::endl;
            }
            return res;
        }
        return false;
    }

    bool compare_bodies(const __m512i *pd1, const __m512i *pd2) {
        return memcmp(&((uint8_t *) pd1)[ts_pd512::kBytes2copy], &((uint8_t *) pd2)[ts_pd512::kBytes2copy], ts_pd512::MAX_CAPACITY) == 0;
    }

    void diff_bodies(const __m512i *pd1, const __m512i *pd2, size_t *diff_arr) {
        uint8_t *pointer1 = &((uint8_t *) pd1)[ts_pd512::kBytes2copy];
        uint8_t *pointer2 = &((uint8_t *) pd2)[ts_pd512::kBytes2copy];
        for (size_t i = 0; i < ts_pd512::MAX_CAPACITY; i++) {
            diff_arr[i] = (pointer1[i] == pointer2[i]) ? 0 : 1;
        }

        std::cout << "PD differ in the following body indices:" << std::endl;
        for (size_t i = 0; i < ts_pd512::MAX_CAPACITY; i++) {
            if (diff_arr[i])
                std::cout << i << ", ";
        }
        std::cout << std::endl;
    }

    void pd_to_array(const __m512i *pd, uint64_t *arr, size_t a_size) {
        assert(a_size == pd512::get_capacity(pd) + 2);
        size_t capacity = pd512::get_capacity(pd);
        uint8_t rem_arr[a_size];
        zero_array(rem_arr, a_size);

        size_t quots_caps_arr[ts_pd512::QUOTS];
        zero_array(quots_caps_arr, ts_pd512::QUOTS);

        // std::cout << "capacity: " << capacity << std::endl;
        // quots_caps_arr[0] = ts_pd512::get_specific_quot_capacity_naive2(0, pd);
        // std::cout << quots_caps_arr[0];
        for (size_t i = 0; i < ts_pd512::QUOTS; i++) {
            quots_caps_arr[i] = ts_pd512::get_specific_quot_capacity_naive2(i, pd);
            // std::cout << ", " << quots_caps_arr[i];
        }
        // std::cout << std::endl;

        memcpy(rem_arr, &((const uint8_t *) pd)[ts_pd512::kBytes2copy], capacity);
        size_t index = 0;
        while (index < capacity) {
            // std::cout << "here!" << std::endl;
            for (size_t i = 0; i < ts_pd512::QUOTS; i++) {
                for (size_t j = 0; j < quots_caps_arr[i]; j++) {
                    uint16_t val = ((uint16_t) rem_arr[index]) | (i << 8u);
                    arr[index] = (uint64_t) val;
                    index++;
                }
            }
            if (index < capacity) {
                assert(0);
            }
        }
    }

    size_t array_sym_diff_single(uint64_t *a, size_t a_size, uint64_t *b, size_t b_size) {
        size_t counter = 0;
        for (size_t i = 0; i < a_size; i++) {
            for (size_t j = 0; j < b_size; j++) {
                if (a[i] == b[j]) {
                    counter++;
                    break;
                }
            }
        }
        return a_size - counter;
    }

    size_t array_sym_diff(uint64_t *a, size_t a_size, uint64_t *b, size_t b_size) {
        size_t a_minus_b = array_sym_diff_single(a, a_size, b, b_size);
        size_t b_minus_a = array_sym_diff_single(b, b_size, a, a_size);
        return a_minus_b + b_minus_a;
    }


    size_t sorted_sym_diff(uint64_t *a, size_t a_size, uint64_t *b, size_t b_size) {
        std::sort(a, a + a_size);
        std::sort(b, b + b_size);

        size_t a_index = 0;
        size_t b_index = 0;
        size_t counter = 0;

        while ((a_index < a_size) && (b_index < b_size)) {
            if (a[a_index] == b[b_index]) {
                a_index++;
                b_index++;
            } else if (a[a_index] < b[b_index]) {
                counter++;
                a_index++;
            } else {
                counter++;
                b_index++;
            }
        }
        size_t a_left = a_size - a_index;
        size_t b_left = b_size - b_index;
        size_t res = counter + a_left + b_left;
        assert(res <= a_size + b_size);
        return res;

        // size_t counter = 0;
        // for (size_t i = 0; i < a_size; i++) {
        //     for (size_t j = 0; j < b_size; j++) {
        //         if (a[i] == b[j]) {
        //             counter++;
        //             break;
        //         }
        //     }
        // }
        // size_t a_minus_b = array_sym_diff_single(a, a_size, b, b_size);
        // size_t b_minus_a = array_sym_diff_single(b, b_size, a, a_size);
        // return a_minus_b + b_minus_a;
    }

    size_t pd_distance(const __m512i *pd1, const __m512i *pd2) {
        size_t capacity1 = pd512::get_capacity(pd1);
        size_t capacity2 = pd512::get_capacity(pd2);

        //maybe capacity is too small (< 2).
        size_t a1_size = capacity1 + 2;
        size_t a2_size = capacity2 + 2;
        uint64_t arr1[a1_size];
        zero_array(arr1, a1_size);
        pd_to_array(pd1, arr1, a1_size);

        uint64_t arr2[a2_size];
        zero_array(arr2, a2_size);

        pd_to_array(pd2, arr2, a2_size);

        // size_t sanity_check1 = sorted_sym_diff(arr1, capacity1, arr1, capacity1);
        // assert(sanity_check1 == 0);
        // size_t sanity_check2 = sorted_sym_diff(arr2, capacity2, arr2, capacity2);
        // assert(sanity_check2 == 0);

        return sorted_sym_diff(arr1, capacity1, arr2, capacity2);
        // return array_sym_diff(arr1, capacity1, arr2, capacity2);
    }

    size_t val_pd_distance(int64_t quot, uint8_t rem, const __m512i *old_pd, const __m512i *new_pd) {
        auto res = v_ts_pd512::pd_distance(new_pd, old_pd);
        if (res != 1) {
            std::cout << "old capacity: " << pd512::get_capacity_naive(old_pd) << std::endl;
            std::cout << "new capacity: " << pd512::get_capacity_naive(new_pd) << std::endl;
            std::cout << "quot: " << quot << std::endl;
            std::cout << "rem: " << ((uint64_t) rem) << std::endl;
            std::cout << "res:  " << res << std::endl;
            v_pd512_plus::print_headers_extended(old_pd);
            v_ts_pd512::print_body(old_pd);
            v_pd512_plus::print_headers_extended(new_pd);
            v_ts_pd512::print_body(new_pd);
            v_ts_pd512::pd_distance(new_pd, old_pd);
        }
        return res;
    }

    bool val_AWF(int64_t quot, uint8_t rem, const __m512i *pd) {
        const __m512i old_pd = *pd;
        __m512i temp_pd0 = old_pd;
        __m512i temp_pd1 = old_pd;
        __m512i temp_pd2 = old_pd;
        ts_pd512::add_pd_full_naive_split_qlru(quot, rem, &temp_pd1);
        ts_pd512::add_when_full(quot, rem, &temp_pd2);

        bool cond = my_equal(temp_pd1, temp_pd2);
        if (!cond) {
            std::cout << "quot:              " << quot << std::endl;

            if (!compare_headers(&temp_pd1, &temp_pd2)) {
                std::cout << "Headers differ" << std::endl;
                size_t temp0_select_quot = pd512_plus::count_zeros_up_to_the_kth_one(&temp_pd0, quot) + quot;
                size_t temp1_select_quot = pd512_plus::count_zeros_up_to_the_kth_one(&temp_pd1, quot) + quot;
                size_t temp2_select_quot = pd512_plus::count_zeros_up_to_the_kth_one(&temp_pd2, quot) + quot;

                size_t temp0_select_quot_m1 = pd512_plus::count_zeros_up_to_the_kth_one(&temp_pd0, quot - 1) + quot - 1;
                size_t temp1_select_quot_m1 = pd512_plus::count_zeros_up_to_the_kth_one(&temp_pd1, quot - 1) + quot - 1;
                size_t temp2_select_quot_m1 = pd512_plus::count_zeros_up_to_the_kth_one(&temp_pd2, quot - 1) + quot - 1;
                std::cout << "temp0_select_quot: " << temp0_select_quot << std::endl;
                std::cout << "temp1_select_quot: " << temp1_select_quot << std::endl;
                std::cout << "temp2_select_quot: " << temp2_select_quot << std::endl;
                std::cout << "temp0_select_quot_m1: " << temp0_select_quot_m1 << std::endl;
                std::cout << "temp1_select_quot_m1: " << temp1_select_quot_m1 << std::endl;
                std::cout << "temp2_select_quot_m1: " << temp2_select_quot_m1 << std::endl;
            }
            v_pd512_plus::print_headers_extended(&temp_pd0);
            v_pd512_plus::print_headers_extended(&temp_pd1);
            v_pd512_plus::print_headers_extended(&temp_pd2);

            std::cout << "temp0 cap: " << pd512::get_capacity_naive(&temp_pd0) << std::endl;
            std::cout << "temp1 cap: " << pd512::get_capacity_naive(&temp_pd1) << std::endl;
            std::cout << "temp2 cap: " << pd512::get_capacity_naive(&temp_pd2) << std::endl;
            if (!compare_bodies(&temp_pd1, &temp_pd2)) {
                size_t arr[ts_pd512::MAX_CAPACITY];
                diff_bodies(&temp_pd1, &temp_pd2, arr);
                v_ts_pd512::print_body(&temp_pd0);
                v_ts_pd512::print_body(&temp_pd1);
                v_ts_pd512::print_body(&temp_pd2);
            }


            assert(0);
        }
        assert(cond);

        return true;
    }

    auto equal_other_than_last_byte(__m512i x, __m512i y) -> bool {
        assert(memcmp(&x, &x, 64) == 0);
        assert(memcmp(&y, &y, 64) == 0);
        return (memcmp(&x, &y, 63) == 0);
    }

    bool val_body_remove(size_t index, const __m512i *pd) {
        const __m512i old_pd = *pd;
        __m512i temp_pd0 = old_pd;
        __m512i temp_pd1 = old_pd;
        __m512i temp_pd2 = old_pd;
        ts_pd512::body_remove_naive(index, &temp_pd1);
        ts_pd512::body_remove_att(index, &temp_pd2);
        // ts_pd512::add_when_full(, &temp_pd2);

        bool cond = equal_other_than_last_byte(temp_pd1, temp_pd2);
        if (!cond) {
            // std::cout << "quot:              " << quot << std::endl;

            // if (!compare_headers(&temp_pd1, &temp_pd2)) {
            //     std::cout << "Headers differ" << std::endl;
            //     size_t temp0_select_quot = pd512_plus::count_zeros_up_to_the_kth_one(&temp_pd0, quot) + quot;
            //     size_t temp1_select_quot = pd512_plus::count_zeros_up_to_the_kth_one(&temp_pd1, quot) + quot;
            //     size_t temp2_select_quot = pd512_plus::count_zeros_up_to_the_kth_one(&temp_pd2, quot) + quot;

            //     size_t temp0_select_quot_m1 = pd512_plus::count_zeros_up_to_the_kth_one(&temp_pd0, quot - 1) + quot - 1;
            //     size_t temp1_select_quot_m1 = pd512_plus::count_zeros_up_to_the_kth_one(&temp_pd1, quot - 1) + quot - 1;
            //     size_t temp2_select_quot_m1 = pd512_plus::count_zeros_up_to_the_kth_one(&temp_pd2, quot - 1) + quot - 1;
            //     std::cout << "temp0_select_quot: " << temp0_select_quot << std::endl;
            //     std::cout << "temp1_select_quot: " << temp1_select_quot << std::endl;
            //     std::cout << "temp2_select_quot: " << temp2_select_quot << std::endl;
            //     std::cout << "temp0_select_quot_m1: " << temp0_select_quot_m1 << std::endl;
            //     std::cout << "temp1_select_quot_m1: " << temp1_select_quot_m1 << std::endl;
            //     std::cout << "temp2_select_quot_m1: " << temp2_select_quot_m1 << std::endl;
            // }
            v_pd512_plus::print_headers_extended(&temp_pd0);
            v_pd512_plus::print_headers_extended(&temp_pd1);
            v_pd512_plus::print_headers_extended(&temp_pd2);

            std::cout << "temp0 cap: " << pd512::get_capacity_naive(&temp_pd0) << std::endl;
            std::cout << "temp1 cap: " << pd512::get_capacity_naive(&temp_pd1) << std::endl;
            std::cout << "temp2 cap: " << pd512::get_capacity_naive(&temp_pd2) << std::endl;
            if (!compare_bodies(&temp_pd1, &temp_pd2)) {
                size_t arr[ts_pd512::MAX_CAPACITY];
                diff_bodies(&temp_pd1, &temp_pd2, arr);
                v_ts_pd512::print_body(&temp_pd0);
                v_ts_pd512::print_body(&temp_pd1);
                v_ts_pd512::print_body(&temp_pd2);
            }
            assert(0);
        }
        assert(cond);

        return true;
    }
    bool val_add_when_full(int64_t quot, uint8_t rem, const __m512i *pd) {
        const __m512i old_pd = *pd;
        __m512i temp_pd0 = old_pd;
        __m512i temp_pd1 = old_pd;
        __m512i temp_pd2 = old_pd;

        // ts_pd512::add_when_full_split_naive(quot, rem, &temp_pd0);
        ts_pd512::add_when_full(quot, rem, &temp_pd1);
        // ts_pd512::add_when_full(quot, rem, &temp_pd2);

        bool cond0 = my_equal(temp_pd0, temp_pd0);
        bool cond1 = my_equal(temp_pd0, temp_pd1);
        bool cond2 = my_equal(temp_pd0, temp_pd2);
        bool cond3 = my_equal(temp_pd1, temp_pd2);

        assert(cond1);
        assert(cond2);

        return true;
    }

    bool validate_evicted_qr(int64_t quot, uint8_t rem, int64_t evict_quot, uint8_t evict_rem, __m512i *pd) {
        uint64_t evict_q_val = 64;
        uint64_t evict_r_val = 64;

        ts_pd512::get_evict_qr(quot, rem, pd, &evict_q_val, &evict_r_val);

        assert(evict_quot == evict_q_val);
        assert(evict_rem == evict_r_val);
        return true;
    }


    bool safe_validate_tombstoning_methods(__m512i *pd, size_t reps) {
        const __m512i target = _mm512_set1_epi8(ts_pd512::Tombstone_FP);
        uint64_t v = _mm512_cmpeq_epu8_mask(target, *pd) >> 13ul;
        if (v) {
            return true;
        }
        const __m512i pd0 = *pd;
        size_t capacity = pd512::get_capacity(pd);
        if (capacity < 2)
            return true;
        size_t a_size = capacity + 2;// +2 to deal with the case that capacity < 2, and then an error occur (not an array).
        uint64_t pd_arr[capacity + 2];
        pd_to_array(pd, pd_arr, a_size);

        size_t reps0 = reps;
        for (size_t i = 0; i < reps; i++) {
            const size_t rand_rem_index = rand() % capacity;
            uint8_t rand_rem = ((uint8_t *) pd)[ts_pd512::kBytes2copy + rand_rem_index];
            int64_t rand_quot = pd512_plus::count_ones_up_to_the_kth_zero(pd, rand_rem_index);
            ts_pd512::remove_by_tombstoning(rand_quot, rand_rem, pd);
            const __m512i pd1 = *pd;
            auto temp_s = pds_difference_to_string(rand_quot, rand_rem, &pd0, &pd1);
            ts_pd512::clear_all_tombstones_wrapper(pd);
            // ts_pd512::clear_all_tombstones_super_naive(pd);
            const __m512i pd2 = *pd;
            ts_pd512::add(rand_quot, rand_rem, pd);
            const __m512i pd3 = *pd;
            auto temp_res = pd_distance(&pd0, &pd3);
            if (temp_res != 0) {
                std::cout << "Failed after the following repetition: " << (i) << std::endl;

                // std::cout << "quot:              " << quot << std::endl;
                std::cout << "rand_quot:         " << rand_quot << std::endl;
                std::cout << "rand_rem:          " << (uint16_t) rand_rem << std::endl;
                std::cout << "rand_rem_index:    " << rand_rem_index << std::endl;
                std::cout << "\n\npd0:" << std::endl;
                print_pd(&pd0);
                std::cout << std::string(80, '.') << std::endl;
                std::cout << "\n\npd1:" << std::endl;
                print_pd(&pd1);
                std::cout << std::string(80, '.') << std::endl;
                std::cout << "\n\npd2:" << std::endl;
                print_pd(&pd2);
                std::cout << std::string(80, '.') << std::endl;
                std::cout << "\n\npd3:" << std::endl;
                print_pd(&pd3);
                std::cout << std::string(80, '.') << std::endl;
                // auto diff_01 = pds_difference_to_string(rand_quot, rand_rem, &pd0, &pd1);
                // auto diff_12 = pds_difference_to_string(rand_quot, rand_rem, &pd1, &pd2);
                // auto diff_23 = pds_difference_to_string(rand_quot, rand_rem, &pd2, &pd3);
                // auto old_vs_curr=  pds_difference_to_string(rand_quot, rand_rem, &old_pd, &mid_pd);

                assert(0);
            }
        }
        return true;
    }

    /**
     * @brief Prints standart info of two PD's. Sometimes "quot" and "rem" are used naturally.
     * 
     * @param quot 
     * @param rem 
     * @param old_pd 
     * @param new_pd 
     */
    void print_pds_difference(int64_t quot, uint8_t rem, const __m512i *old_pd, const __m512i *new_pd) {
        std::cout << "quot:              " << quot << std::endl;
        std::cout << "rem:               " << (uint16_t) rem << std::endl;

        if (!compare_headers(old_pd, new_pd)) {
            std::cout << "Headers differ" << std::endl;
            size_t temp0_select_quot = pd512_plus::count_zeros_up_to_the_kth_one(old_pd, quot) + quot;
            size_t temp1_select_quot = pd512_plus::count_zeros_up_to_the_kth_one(new_pd, quot) + quot;
            // size_t temp2_select_quot = pd512_plus::count_zeros_up_to_the_kth_one(&temp_pd2, quot) + quot;

            size_t temp0_select_quot_m1 = pd512_plus::count_zeros_up_to_the_kth_one(old_pd, quot - 1) + quot - 1;
            size_t temp1_select_quot_m1 = pd512_plus::count_zeros_up_to_the_kth_one(new_pd, quot - 1) + quot - 1;
            // size_t temp2_select_quot_m1 = pd512_plus::count_zeros_up_to_the_kth_one(&temp_pd2, quot - 1) + quot - 1;
            std::cout << "temp0_select_quot: " << temp0_select_quot << std::endl;
            std::cout << "temp1_select_quot: " << temp1_select_quot << std::endl;
            // std::cout << "temp2_select_quot: " << temp2_select_quot << std::endl;
            std::cout << "temp0_select_quot_m1: " << temp0_select_quot_m1 << std::endl;
            std::cout << "temp1_select_quot_m1: " << temp1_select_quot_m1 << std::endl;
            // std::cout << "temp2_select_quot_m1: " << temp2_select_quot_m1 << std::endl;
        }
        v_pd512_plus::print_headers_extended(old_pd);
        v_pd512_plus::print_headers_extended(new_pd);
        // v_pd512_plus::print_headers_extended(&temp_pd2);

        std::cout << "temp0 cap: " << pd512::get_capacity_naive(old_pd) << std::endl;
        std::cout << "temp1 cap: " << pd512::get_capacity_naive(new_pd) << std::endl;
        // std::cout << "temp2 cap: " << pd512::get_capacity_naive(&temp_pd2) << std::endl;
        if (!compare_bodies(old_pd, new_pd)) {
            size_t arr[ts_pd512::MAX_CAPACITY];
            // Next line is useful.
            // diff_bodies(old_pd, new_pd, arr);
            print_body(old_pd);
            print_body(new_pd);
            // v_ts_pd512::print_body(&temp_pd2);
        }
    }

    void print_pd(const __m512i *pd) {
        std::cout << std::string(80, '~') << std::endl;
        assert(pd512::get_capacity(pd) == pd512::get_capacity_naive(pd));
        std::cout << "pd capacity:" << pd512::get_capacity(pd) << std::endl;
        v_pd512_plus::print_headers_extended(pd);
        print_body(pd);
        std::cout << std::string(80, '~') << std::endl;
    }

    auto pds_difference_to_string(int64_t quot, uint8_t rem, const __m512i *old_pd, const __m512i *new_pd) -> std::stringstream {
        std::stringstream ss;
        ss << "quot:              " << quot << std::endl;
        ss << "rem:               " << (uint16_t) rem << std::endl;

        if (!compare_headers(old_pd, new_pd)) {
            ss << "Headers differ" << std::endl;
            size_t temp0_select_quot = pd512_plus::count_zeros_up_to_the_kth_one(old_pd, quot) + quot;
            size_t temp1_select_quot = pd512_plus::count_zeros_up_to_the_kth_one(new_pd, quot) + quot;
            // size_t temp2_select_quot = pd512_plus::count_zeros_up_to_the_kth_one(&temp_pd2, quot) + quot;

            size_t temp0_select_quot_m1 = pd512_plus::count_zeros_up_to_the_kth_one(old_pd, quot - 1) + quot - 1;
            size_t temp1_select_quot_m1 = pd512_plus::count_zeros_up_to_the_kth_one(new_pd, quot - 1) + quot - 1;
            // size_t temp2_select_quot_m1 = pd512_plus::count_zeros_up_to_the_kth_one(&temp_pd2, quot - 1) + quot - 1;
            ss << "temp0_select_quot: " << temp0_select_quot << std::endl;
            ss << "temp1_select_quot: " << temp1_select_quot << std::endl;
            // ss << "temp2_select_quot: " << temp2_select_quot << std::endl;
            ss << "temp0_select_quot_m1: " << temp0_select_quot_m1 << std::endl;
            ss << "temp1_select_quot_m1: " << temp1_select_quot_m1 << std::endl;
            // ss << "temp2_select_quot_m1: " << temp2_select_quot_m1 << std::endl;
        }
        ss << headers_extended_to_string(old_pd).str();
        ss << headers_extended_to_string(new_pd).str();
        // v_pd512_plus::print_headers_extended(&temp_pd2);

        ss << "temp0 cap: " << pd512::get_capacity_naive(old_pd) << std::endl;
        ss << "temp1 cap: " << pd512::get_capacity_naive(new_pd) << std::endl;
        // std::cout << "temp2 cap: " << pd512::get_capacity_naive(&temp_pd2) << std::endl;
        if (!compare_bodies(old_pd, new_pd)) {
            size_t arr[ts_pd512::MAX_CAPACITY];
            // diff_bodies(old_pd, new_pd, arr);
            ss << body_to_string(old_pd).str();
            ss << body_to_string(new_pd).str();
        }
        return ss;
    }

    auto headers_extended_to_string(const __m512i *pd) -> std::stringstream {
        std::stringstream ss;
        // constexpr uint64_t h1_mask = (1ULL << (101ul - 64ul)) - 1ul;
        const uint64_t h0 = _mm_extract_epi64(_mm512_castsi512_si128(*pd), 0);
        const uint64_t h1 = _mm_extract_epi64(_mm512_castsi512_si128(*pd), 1);
        ss << std::string(84, '-') << std::endl;
        ss << "h0: " << v_pd512_plus::bin_print_header_spaced2(h0);
        ss << "\t(#1,#0): (" << _mm_popcnt_u64(h0) << ", " << (64 - _mm_popcnt_u64(h0)) << ")" << std::endl;
        ss << "h1: " << v_pd512_plus::bin_print_header_spaced2(h1);
        ss << "\t(#1,#0): (" << _mm_popcnt_u64(h1) << ", " << (64 - _mm_popcnt_u64(h1)) << ")" << std::endl;
        ss << std::string(84, '-') << std::endl;
        return ss;
    }

    auto headers_extended_to_string2(const __m512i *pd) -> std::stringstream {
        std::stringstream ss;
        constexpr uint64_t h1_mask = (1ULL << (101ul - 64ul)) - 1ul;
        const uint64_t h0 = _mm_extract_epi64(_mm512_castsi512_si128(*pd), 0);
        const uint64_t h1 = _mm_extract_epi64(_mm512_castsi512_si128(*pd), 1);
        ss << std::string(84, '-') << std::endl;
        ss << "h0: " << v_pd512_plus::bin_print_header_spaced2(h0);
        ss << "\t(#1,#0): (" << _mm_popcnt_u64(h0) << ", " << (64 - _mm_popcnt_u64(h0)) << ")" << std::endl;
        ss << "h1: " << v_pd512_plus::bin_print_header_spaced2(h1);
        auto h1_pop = _mm_popcnt_u64(h1 & h1_mask);
        ss << "\t(#1,#0): (" << h1_pop << ", " << (37 - h1_pop) << ")" << std::endl;
        ss << std::string(84, '-') << std::endl;
        return ss;
    }

    auto body_to_string(const __m512i *pd) -> std::stringstream {
        std::stringstream ss;

        ss << std::string(80, '=') << std::endl;
        uint8_t temp_arr[ts_pd512::MAX_CAPACITY];
        memcpy(temp_arr, &((uint8_t *) pd)[ts_pd512::kBytes2copy], ts_pd512::MAX_CAPACITY);
        for (size_t i = 0; i < 6; i++) {
            for (size_t j = 0; j < 8; j++) {
                size_t temp_index = i * 8 + j;
                ss << std::setw(3) << (uint16_t) temp_arr[temp_index] << " | ";
            }
            ss << std::endl;
        }
        for (size_t i = 0; i < 3; i++) {
            size_t temp_index = 48 + i;
            ss << std::setw(3) << (uint16_t) temp_arr[temp_index] << " | ";
        }
        ss << std::endl;
        ss << std::string(80, '=') << std::endl;
        return ss;
    }

    auto pd_to_string(const __m512i *pd) -> std::stringstream {
        std::stringstream ss;

        assert(pd512::get_capacity(pd) == pd512::get_capacity_naive(pd));
        ss << "pd capacity:" << pd512::get_capacity(pd) << std::endl;
        ss << headers_extended_to_string(pd).str();
        ss << body_to_string(pd).str();
        return ss;
    }

}// namespace v_ts_pd512


// for printing
namespace v_ts_pd512 {
    void p_format_word(uint64_t x) {
        std::string res = to_bin(x, 64);
        std::cout << space_string(res) << std::endl;
    }

    auto format_word_to_string(uint64_t x) -> std::string {
        std::string res = to_bin(x, 64) + "\n";
        return space_string(res);
        // std::cout << space_string(res) << std::endl;
    }

    auto to_bin(uint64_t x, size_t length) -> std::string {
        assert(length <= 64);
        uint64_t b = 1ULL;
        std::string res = "";
        for (size_t i = 0; i < length; i++) {
            res += (b & x) ? "1" : "0";
            b <<= 1ul;
        }
        return res;
    }

    auto space_string(std::string s) -> std::string {
        std::string new_s = "";
        for (size_t i = 0; i < s.size(); i += 4) {
            if (i) {
                if (i % 16 == 0) {
                    new_s += "|";
                } else if (i % 4 == 0) {
                    new_s += ".";
                }
            }
            new_s += s.substr(i, 4);
        }
        return new_s;
    }


}// namespace v_ts_pd512

namespace ts_pd512 {

    auto get_specific_quot_capacity_naive2(int64_t quot, const __m512i *pd) -> int {
        assert(quot < 50);
        // assert(quot <= _mm_popcnt_u64(get_clean_header(pd)));
        if (quot == 0) {
            return pd512_plus::count_zeros_up_to_the_kth_one(pd, 0);
        }
        return pd512_plus::count_zeros_up_to_the_kth_one(pd, quot) - pd512_plus::count_zeros_up_to_the_kth_one(pd, quot - 1);
    }
}// namespace ts_pd512


namespace ts_pd512 {

    inline void evict_random_element(int64_t quot, uint8_t rem, __m512i *pd) {
        assert(0);
    }

    /**
     * @brief Finding the last `remainder` in the "run" of the quotient `quot`, and replacing it with rem.
     * This functions only changes one byte in the body. (Does not change the header).
     *  
     * Assertion error if there is no element in the filter with quotient `quot`.
     * This function probably induce non-uniform evacuation policy (w.r.t. insertion time), and therefore will probably be replaced.  
     * @param quot 
     * @param rem 
     * @param pd 
     */
    inline void evict_random_element_with_the_same_quot_naive(int64_t quot, uint8_t rem, __m512i *pd) {
        assert(ts_pd512::get_specific_quot_capacity_naive2(quot, pd));
        const unsigned __int128 *h = (const unsigned __int128 *) pd;
        constexpr unsigned __int128 kLeftoverMask = (((unsigned __int128) 1) << (50 + 51)) - 1;
        const unsigned __int128 header = (*h) & kLeftoverMask;
        constexpr unsigned kBytes2copy = (50 + 51 + CHAR_BIT - 1) / CHAR_BIT;

        assert(pd512::popcount128(header) == 50);
        const unsigned fill = pd512::select128(header, 50 - 1) - (50 - 1);
        assert((fill <= 14) || (fill == pd512::pd_popcount(pd)));
        assert((fill == 51) == pd512::pd_full(pd));


        const uint64_t end = pd512::select128(header, quot);
        const uint64_t end_fingerprint = end - quot;

        // memmove(&((uint8_t *) pd)[kBytes2copy + end_fingerprint + 1],
        //         &((const uint8_t *) pd)[kBytes2copy + end_fingerprint],
        //         sizeof(*pd) - (kBytes2copy + end_fingerprint + 1));
        ((uint8_t *) pd)[kBytes2copy + end_fingerprint] = rem;

        assert(find(quot, rem, pd));
        // return true;
    }


    inline void evict_random_element_with_new_quot_super_naive(int64_t quot, uint8_t rem, __m512i *pd) {
        constexpr unsigned kBytes2copy = (50 + 51 + CHAR_BIT - 1) / CHAR_BIT;
        size_t index = (rand() % QUOTS);

        const size_t rand_quot = pd512_plus::count_ones_up_to_the_kth_zero(pd, index);
        //todo: add in assertion here of something.
        const uint8_t rand_rem = ((uint8_t *) pd)[kBytes2copy + index];

        //todo: smart remove here
        remove(rand_quot, rand_rem, pd);
        add(quot, rem, pd);
    }


    inline void evict_random_element_with_the_same_quot(int64_t quot, uint8_t rem, __m512i *pd) {
        assert(get_specific_quot_capacity_naive2(quot, pd));
        const __m512i target = _mm512_set1_epi8(rem);
        // compare_less or equal.
        uint64_t v = _mm512_cmp_epu8_mask(target, *pd, 2) >> 13ul;
    }

    inline void evict_random_element_naive(int64_t quot, uint8_t rem, __m512i *pd) {
        assert(pd512::pd_full_naive(pd));
        if (get_specific_quot_capacity_naive2(quot, pd)) {
            evict_random_element_with_the_same_quot_naive(quot, rem, pd);
            return;
        }
    }

}// namespace ts_pd512
