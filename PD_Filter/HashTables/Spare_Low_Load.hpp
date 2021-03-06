//
// Created by tomer on 9/14/20.
//
/**
 * @brief Class for storing elements that overflowed from PD in the first level.
 * This class contain 256 hashtables (2^rem). 
 * Given some element x that overflowed from some PD, with index i, quotient q, and remainder r, we insert new element y to the Table[i] where y equals 
 * concatenation of i and q.
 * We also maintain a set of counters representing the number of overflowing elemnts from each PD. if element does not have overflowing elements, it is 
 * not belong to that set. This set is used to update that some PD is no longer overflowing.
 * 
 */


#ifndef FILTERS_SPARE_LOW_LOAD_HPP
#define FILTERS_SPARE_LOW_LOAD_HPP


#include "../../Tests/printutil.hpp"
#include "../../hashutil.h"
#include "../basic_function_util.h"
#include "../macros.h"

#include <unordered_map>
#include <vector>

#define TABLE_SIZE (256)

template<typename T>
auto my_max(T a, T b) ->T{
    return (a < b) ? b : a;
}

template<typename bucket_type, size_t bucket_size, typename HashFamily = hashing::TwoIndependentMultiplyShift>
class Spare_Low_Load {

    struct Bucket {
        bucket_type bits_[bucket_size];
    };

    Bucket *Tables[TABLE_SIZE];
    unordered_map<uint64_t, uint8_t> OF_counter;
    const size_t single_table_buckets_num, max_capacity, element_length;
    size_t capacity{0};
    // const double max_load_factor;
    const bucket_type empty_slot{(bucket_type) -1};

    // size_t insert_existing_counter = 0;
    HashFamily hasher;

public:
    Spare_Low_Load(size_t max_capacity, size_t element_length)
        : max_capacity(max_capacity),
          element_length(element_length),
          //   max_load_factor(-1),
          single_table_buckets_num(my_max<size_t>(max_capacity >> 10ul, 4)),
          hasher() {
        assert(single_table_buckets_num <= MASK32);
        assert(single_table_buckets_num > 0);

        /* Todo: test changes to second argument */
        for (size_t i = 0; i < TABLE_SIZE; i++) {
            int ok = posix_memalign((void **) &Tables[i], sizeof(Bucket), sizeof(Bucket) * (single_table_buckets_num));
            if (ok != 0) {
                cout << "Failed!!!" << endl;
                return;
            }
        }
        //        Table = new Bucket[num_of_buckets];
        // constexpr auto db_midder1 = sizeof(bucket_type);
        // constexpr auto db_midder2 = sizeof(bucket_type) * CHAR_BIT;
        assert(element_length < sizeof(bucket_type) * CHAR_BIT);
        for (size_t i = 0; i < TABLE_SIZE; i++) {
            Bucket *table = Tables[i];
            for (int j = 0; j < single_table_buckets_num; ++j) {
                auto bp = table[j].bits_;
                for (int k = 0; k < bucket_size; ++k) {
                    bp[k] = empty_slot;
                }
            }
        }
        // OF_counter.resize()
    }

    virtual ~Spare_Low_Load() {
        for (size_t i = 0; i < TABLE_SIZE; i++) {
            free(Tables[i]);
        }
        // free(Table);
        // delete[] Table;
    }


    inline auto find(uint64_t pd_index, uint64_t quot, uint8_t rem) const -> bool {
        bucket_type spare_item = (pd_index << 6ul) | quot;
        const uint64_t out = hasher(spare_item);
        const uint32_t b1 = reduce32((uint32_t)(out >> 32ul), (uint32_t) single_table_buckets_num);
        const uint32_t b2 = reduce32((uint32_t) out & MASK32, (uint32_t) single_table_buckets_num);
        return find_helper(spare_item, b1, b2, rem);
    }

    void insert_to_map(uint64_t pd_index) {
        auto it = OF_counter.find(pd_index);
        if (it != OF_counter.end()) {
            it->second++;
        } else {
            OF_counter.insert({pd_index, 1});
        }
    }

    /**
     * @brief Returns true if the PD matching pd_index is not associated with any more elements in this level. 
     * 
     * @param pd_index 
     * @return true 
     * @return false 
     */
    auto remove_from_map(uint64_t pd_index) -> bool {

        auto it = OF_counter.find(pd_index);
        assert(it != OF_counter.end());
        if (it->second == 1) {
            OF_counter.erase(it);
            return true;
        }
        // If it->second == 0, then the counter overflowed.
        if (it->second)
            it->second--;
        return false;
    }

    void insert(uint64_t pd_index, uint64_t quot, uint8_t rem) {


        if (capacity >= max_capacity) {
            std::cout << "Trying to insert into fully loaded hash table" << std::endl;
            assert(false);
        }
        insert_to_map(pd_index);

        /* http://www.cs.toronto.edu/~noahfleming/CuckooHashing.pdf (Algorithm 2)*/
        bucket_type spare_item = (pd_index << 6ul) | quot;
        const uint64_t out = hasher(spare_item);
        const uint32_t b1 = reduce32((uint32_t)(out >> 32ul), (uint32_t) single_table_buckets_num);
        const uint32_t b2 = reduce32((uint32_t) out & MASK32, (uint32_t) single_table_buckets_num);
        // assert(b1 < num_of_buckets);
        // assert(b2 < num_of_buckets);
        // assert(does_bucket_contain_valid_elements(b1));
        // assert(does_bucket_contain_valid_elements(b2));

        if (insert_if_bucket_not_full(spare_item, b2, rem)) {
            return;
        }

        bucket_type hold = spare_item;
        size_t bucket_index = b1;
        for (int i = 0; i < MAX_CUCKOO_LOOP; ++i) {
            if (insert_if_bucket_not_full(hold, bucket_index, rem)) {
                return;
            }

            cuckoo_swap(&hold, &bucket_index, rem);
        }
        std::stringstream ss;
        get_info(&ss);
        std::cout << ss.str();
        assert(false);
    }


    /**
         * Picks random element in the bucket. denote by "temp_val".
         * Insert "hold" in "temp_val" position.
         * Set *hold = "temp_val".
         * Store in "bucket_index" the OTHER bucket "temp_val" can be stored in.
         * @param hold
         * @param bucket_index
         */
    inline auto cuckoo_swap(bucket_type *hold, size_t *bucket_index, uint8_t rem) -> void {
        bucket_type old_val = *hold;

        bucket_type junk = swap_elements_from_bucket(*bucket_index, *hold, rem);
        assert(junk != empty_slot);
        *hold = junk;


        const uint64_t out = hasher(*hold);
        const uint32_t b1 = reduce32((uint32_t)(out >> 32ul), (uint32_t) single_table_buckets_num);
        const uint32_t b2 = reduce32((uint32_t) out & MASK32, (uint32_t) single_table_buckets_num);

        *bucket_index = (b1 == *bucket_index) ? b2 : b1;
    }

    auto remove(uint64_t pd_index, uint64_t quot, uint8_t rem) -> bool {
        if (capacity == 0) {
            std::cout << "Trying to delete from empty hash table" << std::endl;
            //        assert(false);
        }
        bucket_type spare_item = (pd_index << 6ul) | quot;
        const uint64_t out = hasher(spare_item);
        const uint32_t b1 = reduce32((uint32_t)(out >> 32ul), (uint32_t) single_table_buckets_num);
        const uint32_t b2 = reduce32((uint32_t) out & MASK32, (uint32_t) single_table_buckets_num);


        const bool res = remove_helper(spare_item, b1, rem) || remove_helper(spare_item, b2, rem);
        assert(res);
        return remove_from_map(pd_index);
    }


    inline auto swap_elements_from_bucket(size_t bucket_index, bucket_type x, uint8_t rem) -> bucket_type {
        assert(is_bucket_full_by_index(bucket_index, rem));
        auto *bp = Tables[rem][bucket_index].bits_;

        auto temp_index = rand() & (bucket_size - 1);
        bucket_type temp = bp[temp_index];
        bp[temp_index] = x;
        return temp;
    }


    /**
         * Tries to insert "x" to bucket in "bucket_index".
         * If the bucket is not full, x will be inserted, and true will be returned.
         * Otherwise, x was not inserted, and false will be returned.
         * @param x
         * @param bucket_index
         * @return
         */
    inline auto insert_if_bucket_not_full(uint64_t spare_item, size_t bucket_index, uint8_t rem) -> bool {
        assert(bucket_size == 4);
        auto *bp = Tables[rem][bucket_index].bits_;
        if (bp[0] == empty_slot) {
            bp[0] = spare_item;
            capacity++;
            return true;
        } else if (bp[1] == empty_slot) {
            bp[1] = spare_item;
            capacity++;
            return true;
        } else if (bp[2] == empty_slot) {
            bp[2] = spare_item;
            capacity++;
            return true;
        } else if (bp[3] == empty_slot) {
            bp[3] = spare_item;
            capacity++;
            return true;
        }
        /*Add pop attempt*/
        return false;
    }


    /**
         *
         * @param x
         * @param bucket_index
         * @param location
         * table[bucket_index*bucket_size + location] = x;
         */
    inline void insert_by_bucket_index_and_location(bucket_type x, size_t bucket_index, size_t location, uint8_t rem) {
        auto *bp = Tables[rem][bucket_index].bits_;
        bp[location] = x;
        capacity++;
    }
    /* 
    auto print_bucket(uint32_t bucket_index, ostream &os = std::cout) -> ostream & {
        auto line = std::string(80, '-') + "\n";
        os << line;
        os << "bucket number " << bucket_index << " content:" << std::endl;
        auto *bp = Table[bucket_index].bits_;
        os << "[";
        bool is_slot_empty = is_empty_by_bucket_index_and_location(bucket_index, 0);
        if (!is_slot_empty)
            os << bp[0];
        else
            os << "Empty";

        for (int i = 1; i < bucket_size; ++i) {
            if (is_empty_by_bucket_index_and_location(bucket_index, i)) {
                os << ", Empty!";
            } else {
                os << ", " << bp[i];
            }
        }
        os << "]" << std::endl;
        os << line;
        return os;
    }
 */

    inline auto get_bucket(size_t bucket_index, uint8_t rem) const -> Bucket * {
        return Tables[rem][bucket_index];
    }


    auto get_byte_size() {
        return sizeof(bucket_type) * bucket_size * single_table_buckets_num * 256;
        ;
    }

    auto get_table_size() const -> const size_t {
        return single_table_buckets_num;
    }

    auto get_max_capacity() const -> const size_t {
        return max_capacity;
    }

    auto get_element_length() const -> const size_t {
        return element_length;
    }

    auto get_bucket_size() const -> const size_t {
        return bucket_size;
    }

    auto get_capacity() const -> size_t {
        return capacity;
    }

    auto get_max_load_factor() const -> const double {
        return 42;
    }

    auto get_load_factor() const -> double {
        return capacity / (double) max_capacity;
    }

    void get_info(std::stringstream *os) {

        auto line = std::string(64, '-');
        *os << line << std::endl;
        *os << "Spare max_capacity is: " << str_format(get_max_capacity()) << std::endl;
        *os << "Spare capacity is: " << str_format(get_capacity()) << std::endl;
        *os << "Byte size is: " << str_format(get_byte_size()) << std::endl;
        *os << "Spare load factor is: " << get_load_factor() << std::endl;

        // if (insert_existing_counter) {
        //     *os << "insert_existing_counter: " << insert_existing_counter << std::endl;
        //     double ratio = insert_existing_counter / (double) max_capacity;
        //     *os << "ratio to max capacity: " << ratio << std::endl;
        // }
        double waste_ratio = (sizeof(bucket_size) * CHAR_BIT) / ((double) element_length);
        *os << "Waste ratio (by not packing): " << waste_ratio << std::endl;
        *os << "element bit size: " << element_length << std::endl;

        size_t empty_buckets = count_empty_buckets();
        *os << "Number of empty buckets " << empty_buckets << "/" << single_table_buckets_num << "\t";
        *os << "ratio is: " << (empty_buckets / (double) single_table_buckets_num) << std::endl;

        // auto tp = find_empty_bucket_interval();
        // size_t start = std::get<0>(tp), end = std::get<1>(tp);
        // *os << "longest_empty_interval length is: " << end - start << std::endl;
        // *os << "start: " << start << std::endl;
        // *os << "end: " << end << std::endl;
        // if (start + 10 < end) {
        //     *os << "longest_empty_interval length is: " << end - start << std::endl;
        //     *os << "start: " << start << std::endl;
        //     *os << "end: " << end << std::endl;
        // }
        *os << line << std::endl;
        // return *os;
    }
    ////Setters

    void increase_capacity() {
        capacity++;
    }

    void decrease_capacity() {
        capacity--;
    }

    /*
        void clear_slot_bucket_index_and_location(size_t bucket_index, size_t location) {
            table[bucket_index * bucket_size + location] = EMPTY;
        }
    */

    ////validation

    /* void get_all_elements(vector<bucket_type> *vec) {
        auto resize_val = get_capacity();
        vec->resize(resize_val);
        for (size_t i = 0; i < num_of_buckets; i++) {
            auto *bp = Table[i].bits_;

            for (size_t j = 0; j < bucket_size; j++) {
                if (bp[j] == empty_slot)
                    continue;
                vec->push_back(bp[j]);
            }
        }
    } */

    auto count_empty_buckets() -> size_t {
        size_t res = 0;

        for (size_t r = 0; r < 256; r++) {
            for (size_t i = 0; i < single_table_buckets_num; i++) {
                if (is_bucket_empty(i, r))
                    res++;
            }
        }
        return res;
    }

    auto is_state_valid() -> bool;

    // auto is_bucket_elements_unique(size_t bucket_index) -> bool {
    //     auto *bp = Table[bucket_index].bits_;
    //     for (int j = 0; j < bucket_size; ++j) {
    //         if (is_empty_by_bucket_index_and_location(bucket_index, j))
    //             continue;
    //         for (int i = j + 1; i < bucket_size; ++i) {
    //             if (bp[j] == bp[i])
    //                 return false;
    //         }
    //     }
    //     return true;
    // }

    /*auto find_table_location(slot_type x) -> size_t {
            uint32_t b1 = -1, b2 = -1;
            my_hash(x, &b1, &b2);

            auto res = find_helper_table_location(x, b1);
            if (res != -1)
                return res;
            return find_helper_table_location(x, b2);


        }*/

    /**
         * @param table_index the index of table, in which the element is stored.
         * @return the element without the counter.
         */
    auto get_element_by_index(size_t table_index, uint8_t rem) -> bucket_type {
        size_t bucket_index = table_index / bucket_size;
        size_t in_bucket_index = table_index % bucket_size;
        return get_element_by_bucket_index_and_location(bucket_index, in_bucket_index, rem);
        //        assert(false);
        //        return table[table_index];
    }

    auto get_element_by_bucket_index_and_location(size_t bucket_index, size_t location, uint8_t rem) -> bucket_type {
        auto *bp = Tables[rem][bucket_index].bits_;
        return bp[location];
    }

    /*
        void update_max_cuckoo_insert(size_t i) {
            max_cuckoo_insert = (max_cuckoo_insert >= i) ? max_cuckoo_insert : i;
        }

        /**
         * Increase "cuckoo_HT_insert_counter" by "cuckoo_chain_length" * "bucket_size".
         * @param cuckoo_chain_length
         *
        void update_cuckoo_HT_insert_counter(size_t cuckoo_chain_length) {
            cuckoo_HT_insert_counter += cuckoo_chain_length * bucket_size;
        }
        */

    auto is_empty_by_index(size_t table_index, uint8_t rem) -> bool {
        size_t bucket_index = table_index / bucket_size;
        size_t in_bucket_index = table_index % bucket_size;
        return is_empty_by_bucket_index_and_location(bucket_index, in_bucket_index, rem);
        //        assert(false);
        //        return (table[table_index] == EMPTY);
    }

    auto is_empty_by_bucket_index_and_location(size_t bucket_index, size_t location, uint8_t rem) -> bool {
        auto *bp = Tables[rem][bucket_index].bits_;
        return bp[location] == empty_slot;
    }

    auto clear_slot_bucket_index_and_location(size_t bucket_index, size_t location, uint8_t rem) -> void {
        auto *bp = Tables[rem][bucket_index].bits_;
        bp[location] = empty_slot;
    }

    // inline auto is_bucket_full_by_index(uint32_t bucket_index) const -> bool {
    //     return get_bucket_capacity(bucket_index) == bucket_size;
    // }

    inline auto is_bucket_full_by_index(uint32_t bucket_index, uint8_t rem) const -> bool {
        return get_bucket_capacity(bucket_index, rem) == bucket_size;
    }


    // auto does_bucket_contain_valid_elements(uint32_t bucket_index) const -> bool {
    //     auto *bp = Table[bucket_index].bits_;
    //     for (int i = 0; i < bucket_size; ++i) {
    //         auto temp = bp[i];
    //         if (temp == empty_slot)
    //             continue;

    //         uint32_t b1, b2;
    //         my_hash(temp, &b1, &b2);
    //         bool cond = (bucket_index == b1) or (bucket_index == b2);
    //         if (!cond) {
    //             std::cout << "h5" << std::endl;
    //             std::cout << "bucket_index is: " << bucket_index << std::endl;
    //             std::cout << "temp: " << temp;
    //             std::cout << "\tbuckets: (" << b1 << ", " << b2 << ")" << std::endl;
    //             assert(false);
    //         }
    //     }
    //     return true;
    // }


    auto get_name() -> std::string {
        return "Bucket HT";
    }

    // auto in_which_bucket_is_the_element(bucket_type x) -> int {
    //     // uint32_t b1 = -1, b2 = -1;
    //     my_hash(x, &b1, &b2);

    //     // if (db_cond)
    //     // {
    //     //     std::cout << "b1: " << b1 << std::endl;
    //     //     std::cout << "b2: " << b2 << std::endl;
    //     //     std::cout << "find_helper(x, b2): " << find_helper(x, b2) << std::endl;
    //     // }
    //     if (find_helper(x, b1))
    //         return 1;
    //     if (find_helper(x, b2))
    //         return 2;
    //     return -1;
    //     // return ((find_helper(x, b1)) || find_helper(x, b2));
    // }


private:
    inline auto find_helper(uint64_t spare_item, size_t bucket_index1, size_t bucket_index2, uint8_t rem) const -> bool {
        assert(bucket_size == 4);
        //todo validate auto.
        auto table = Tables[rem];
        auto *bp1 = table[bucket_index1].bits_;
        auto *bp2 = table[bucket_index2].bits_;
        return ((bp1[0] == spare_item) | (bp1[1] == spare_item) | (bp1[2] == spare_item) | (bp1[3] == spare_item)) ||
               ((bp2[0] == spare_item) | (bp2[1] == spare_item) | (bp2[2] == spare_item) | (bp2[3] == spare_item));
    }


    // inline auto
    // find_helper(bucket_type x, size_t bucket_index) const -> bool {
    //     assert(bucket_size == 4);
    //     //        auto table_index = bucket_index * bucket_size;
    //     auto *bp = Table[bucket_index].bits_;
    //     return ((bp[0] == x) | (bp[1] == x) | (bp[2] == x) | (bp[3] == x));}


    /*
        auto find_helper_table_location(bucket_type x, size_t bucket_index) -> int {
            auto table_index = bucket_index * bucket_size;
            for (int i = 0; i < bucket_size; ++i) {
                if (is_equal(table[table_index + i], x))
                    return table_index + i;
            }
            return -1;
        }*/

    auto remove_helper(uint64_t spare_item, size_t bucket_index, uint8_t rem) -> bool {
        auto *bp = Tables[rem][bucket_index].bits_;
        for (int i = 0; i < bucket_size; ++i) {
            if (bp[i] == spare_item) {
                bp[i] = empty_slot;
                capacity--;
                return true;
            }
        }
        return false;
    }


    /**
         *
         * @param with_counter
         * @param without_counter
         * @return compares x,y first "element length" bits.
         */
    auto is_equal(bucket_type with_counter, bucket_type without_counter) const -> bool {
        bucket_type after_mask = without_counter & MASK(element_length);
        //        assert((without_counter & MASK(element_length)) == without_counter);
        return (with_counter & MASK(element_length)) == without_counter;
    }


    auto get_bucket_capacity(size_t bucket_index, uint8_t rem) const -> size_t {
        size_t res = 0;
        auto *bp = Tables[rem][bucket_index].bits_;
        for (int i = 0; i < bucket_size; ++i) {
            if (bp[i] != empty_slot) {
                res++;
            }
        }
        return res;
    }

    auto is_bucket_empty(size_t bucket_index, uint8_t rem) const -> bool {
        auto *bp = Tables[rem][bucket_index].bits_;
        for (int i = 0; i < bucket_size; ++i) {
            if (bp[i] != empty_slot) {
                return false;
            }
        }
        return true;
    }

    /** auto find_empty_bucket_interval() -> std::tuple<size_t, size_t> {
        size_t max_length = 0;
        size_t start = 0, end = 0;
        size_t temp_start = 0, temp_end = 0;
        size_t index = 0;
        // __m512i *ppd = &pd_array[0];
        while (index < num_of_buckets) {
            //            size_t temp_length = 0;
            if (is_bucket_empty(index)) {
                size_t temp_length = 1;
                size_t temp_index = index + 1;
                while ((temp_index < num_of_buckets) and (is_bucket_empty(temp_index))) {
                    temp_index++;
                    temp_length++;
                }
                if (temp_index == num_of_buckets) {
                    std::cout << "h8!" << std::endl;
                }
                temp_length = temp_index - index;
                if (temp_length > max_length) {
                    start = index;
                    end = temp_index;
                    max_length = temp_length;
                }
                index = temp_index + 1;
            } else
                index++;
        }
        return {start, end};
    }


*/
};

#endif//FILTERS_SPARE_LOW_LOAD_HPP


//Old functions

// auto find_old(bucket_type x) const -> bool {
//         // HT_lookup_counter++;
//         // bool db_cond = f(x);
//         assert((x & MASK(element_length)) == x);
//         // bool printer = f(x);

//         uint32_t b1 = -1, b2 = -1;
//         my_hash(x, &b1, &b2);
//         assert(does_bucket_contain_valid_elements(b1));
//         assert(does_bucket_contain_valid_elements(b2));
//         // if (printer) {
//         //     // std::cout << "***spare lookup***\nb1: " << b1 << std::endl;
//         //     std::cout << "***spare lookup***\nb1: " << b1 << std::endl;
//         //     std::cout << "b2: " << b2 << std::endl;
//         //     bool find1 = (find_helper(x, b1));
//         //     bool find2 = (find_helper(x, b2));
//         //     std::cout << "find1: " << find1 << std::endl;
//         //     std::cout << "find2: " << find2 << std::endl;
//         //     std::cout << std::endl;
//         // }

//         return ((find_helper(x, b1)) || find_helper(x, b2));
//         // if (db_cond) {
//         //     std::cout << "b1: " << b1 << std::endl;
//         //     std::cout << "b2: " << b2 << std::endl;
//         //     std::cout << "find_helper(x, b2): " << find_helper(x, b2) << std::endl;
//         // }
//         // found_something |= ((find_helper(x, b1)) || find_helper(x, b2));
//     }

// void insert_old(bucket_type x) {
//     // if (find(x)) {
//     //     insert_existing_counter++;
//     // }
//     // bool printer = f(x);
//     assert((x & MASK(element_length)) == x);

//     if (capacity >= max_capacity) {
//         std::cout << "Trying to insert into fully loaded hash table" << std::endl;
//         assert(false);
//     }
//     // capacity++;
//     /* http://www.cs.toronto.edu/~noahfleming/CuckooHashing.pdf (Algorithm 2)*/

//     uint32_t b1 = -1, b2 = -1;

//     my_hash(x, &b1, &b2);
//     /* if (printer) {
//             std::cout << "***spare insert***\nb1: " << b1 << std::endl;
//             std::cout << "b2: " << b2 << std::endl;
//             bool find1 = (find_helper(x, b1));
//             bool find2 = (find_helper(x, b2));
//             std::cout << "find1: " << find1 << std::endl;
//             std::cout << "find2: " << find2 << std::endl;
//             std::cout << std::endl;

//         } */
//     assert(does_bucket_contain_valid_elements(b1));
//     assert(does_bucket_contain_valid_elements(b2));

//     if (insert_if_bucket_not_full(x, b2)) {
//         /* if (printer) {
//                 std::cout << "was inserted into b1" << std::endl;
//             } */
//         return;
//     }

//     auto hold = x;
//     size_t bucket_index = b1;
//     for (int i = 0; i < MAX_CUCKOO_LOOP; ++i) {
//         // assert(does_bucket_contain_valid_elements(bucket_index));

//         if (insert_if_bucket_not_full(hold, bucket_index)) {

//             // assert(does_bucket_contain_valid_elements(bucket_index));
//             // std::cout << "/* message */" << std::endl;
//             return;
//         }

//         bucket_type old_hold = hold;
//         uint32_t old_bucket_index = bucket_index;

//         cuckoo_swap(&hold, &bucket_index);
//         // line_counter++;
//         // assert(find_helper(old_hold, old_bucket_index));
//         // assert(does_bucket_contain_valid_elements(bucket_index));
//         //
//         // if (printer) {
//         //     uint32_t j1, j2;
//         //     my_hash(hold, &j1, &j2);
//         //     std::cout << "was swapped to " << hold << std::endl;
//         //     std::cout << "b1: " << j1 << std::endl;
//         //     std::cout << "b2: " << j2 << std::endl;
//         //     std::cout << "i is: " << i << std::endl;
//         // }
//         // if (old_hold == hold) {
//         //     std::cout << "swap counter" << swap_counter << std::endl;
//         //     assert(false);
//         // }
//     }
//     std::stringstream ss;
//     get_info(&ss);
//     std::cout << ss.str();
//     assert(false);
// }

// void very_old_insert(bucket_type x) {
//     /* bool was_flipped_now = false;
//         // HT_insert_counter++;
// //        bool ic_cond = (HT_insert_counter == 1118697);
//         // bool ic_cond = false;
//         //if (ic_cond) {
//             std::cout << std::string(128, '@') << std::endl;
//         }
//         // if (el3002_was_inserted) {
//         //     if (!h()) {
//         //         std::cout << "HT_insert_counter: " << HT_insert_counter << std::endl;
//         //         std::cout << "x: " << x << std::endl;
//         //         assert(false);
//         //     }
//         // }
//         // bool printer = false;
//         //bool skip_assert = false;
//         if (f(x) or ic_cond) {
//             if (!el3002_was_inserted) {
//                 std::cout << std::string(80, '*') << std::endl;
//                 std::cout << "insert index: " << HT_insert_counter << std::endl;
//                 std::cout << "el3002 was flipped! " << std::endl;
//                 std::cout << std::string(80, '*') << std::endl;
//                 skip_assert = true;
//                 was_flipped_now = true;
//             }
//             printer = true;
// //            el3002_was_inserted = true;
//         }*/
//     assert((x & MASK(element_length)) == x);
//     bool printer = f(x);

//     if (capacity >= max_capacity) {
//         std::cout << "Trying to insert into fully loaded hash table" << std::endl;
//         assert(false);
//     }
//     /* http://www.cs.toronto.edu/~noahfleming/CuckooHashing.pdf (Algorithm 2)*/

//     uint32_t b1 = -1, b2 = -1;

//     my_hash(x, &b1, &b2);
//     if (printer) {
//         std::cout << "***spare insert***\nb1: " << b1 << std::endl;
//         std::cout << "b2: " << b2 << std::endl;
//         bool find1 = (find_helper(x, b1));
//         bool find2 = (find_helper(x, b2));
//         std::cout << "find1: " << find1 << std::endl;
//         std::cout << "find2: " << find2 << std::endl;
//         std::cout << std::endl;
//     }
//     assert(does_bucket_contain_valid_elements(b1));
//     assert(does_bucket_contain_valid_elements(b2));

//     if (insert_if_bucket_not_full(x, b2)) {
//         if (printer) {
//             std::cout << "was inserted into b1" << std::endl;
//         }
//         return;
//         /*if (was_flipped_now) {
//                 std::cout << "Trying to insert (" << x << ") into the second bucket, (" << b2 << ").\n";
//                 std::cout << "Bucket contents After insertion attempt is:\n";
//                 print_bucket(b2);
//             }
//             if (f(x))
//                 assert(h());

//             bool will_not_assert = (!el3002_was_inserted) || h();
//             if (!will_not_assert) {
//                 std::cout << "HT_insert_counter: " << HT_insert_counter << std::endl;
//                 std::cout << "p2_cond_counter: " << p2_cond_counter << std::endl;
//                 std::cout << "swap counter: " << swap_counter << std::endl;
//                 std::cout << "x: " << x << std::endl; //2499577067
//                 assert(false);
//             }

//             if (printer)
//                 std::cout << "was inserted to b2. bucket number " << b2 << std::endl;
//             assert(does_bucket_contain_valid_elements(b2));
//             */
//     }

//     auto hold = x;
//     size_t bucket_index = b1;
//     for (int i = 0; i < MAX_CUCKOO_LOOP; ++i) {
//         assert(does_bucket_contain_valid_elements(bucket_index));
//         if (insert_if_bucket_not_full(hold, bucket_index)) {

//             assert(does_bucket_contain_valid_elements(bucket_index));
//             std::cout << "/* message */" << std::endl;
//             /*if (p2_cond) {
//                     std::cout << "T_After_T" << std::endl;
//                     std::cout << "h() := " << h() << std::endl;
//                     print_bucket(b1);
//                 }
//                 bool going_to_assert = ((!el3002_was_inserted) || h());
//                 if (!going_to_assert) {
//                     std::cout << "HT_insert_counter: " << HT_insert_counter << std::endl;
//                     std::cout << "p2_cond_counter: " << p2_cond_counter << std::endl;
//                     std::cout << "i: " << i << std::endl;
//                     std::cout << "sleeping: ";
//                     usleep(500000);
//                     std::cout << ".";
//                     usleep(300000);
//                     std::cout << ".";
//                     usleep(200000);
//                     std::cout << ".";
//                     usleep(100000);
//                     std::cout << ".";
//                     std::cout << std::endl;
//                 }
//                 assert(going_to_assert);
//                 */
//             return;
//         }

//         bucket_type old_hold = hold;
//         uint32_t old_bucket_index = bucket_index;

//         assert(does_bucket_contain_valid_elements(bucket_index));
//         cuckoo_swap(&hold, &bucket_index);
//         line_counter++;
//         assert(find_helper(old_hold, old_bucket_index));

//         assert(does_bucket_contain_valid_elements(bucket_index));

//         if (printer) {
//             std::cout << "was swapped to " << hold << std::endl;
//             std::cout << "i is: " << i << std::endl;
//         }
//         if (old_hold == hold) {
//             std::cout << "swap counter" << swap_counter << std::endl;
//             assert(false);
//         }
//     }
//     assert(false);
// }

// void cuckoo_swap_old(bucket_type *hold, size_t *bucket_index) {
//     //        assert(false);
//     // assert(!el3002_was_inserted || h());
//     assert(does_bucket_contain_valid_elements(*bucket_index));
//     swap_counter++;
//     bucket_type old_val = *hold;
//     bucket_type junk = swap_elements_from_bucket(*bucket_index, *hold);
//     assert(junk != empty_slot);
//     *hold = junk;

//     uint32_t temp_b1 = -1, temp_b2 = -1;
//     my_hash(*hold, &temp_b1, &temp_b2);
//     assert(does_bucket_contain_valid_elements(*bucket_index));
//     // assert(temp_b2 != temp_b1);

//     if (temp_b1 == *bucket_index)
//         *bucket_index = temp_b2;
//     else if (temp_b2 == *bucket_index)
//         *bucket_index = temp_b1;
//     else {
//         std::cout << "h4" << std::endl;
//         std::cout << "*hold: " << *hold;
//         std::cout << "\tbuckets: (" << temp_b1 << ", " << temp_b2 << ")" << std::endl;
//         //            my_hash(old_val, &old_b2, &old_b1);
//         //            std::cout << "\tRevered: (" << old_b1<<", " << old_b2 << ")" << std::endl;


//         std::cout << "*bucket_index: " << *bucket_index << std::endl;
//         std::cout << "num_of_buckets: " << num_of_buckets << std::endl;

//         uint32_t old_b1, old_b2;
//         my_hash(old_val, &old_b1, &old_b2);

//         std::cout << "old_val: " << old_val;
//         std::cout << "\tbuckets: (" << old_b1 << ", " << old_b2 << ")";
//         my_hash(old_val, &old_b2, &old_b1);
//         std::cout << "\tRevered: (" << old_b1 << ", " << old_b2 << ")" << std::endl;


//         std::cout << "*hold: " << *hold;
//         my_hash(*hold, &old_b1, &old_b2);
//         std::cout << "\tbuckets: (" << old_b1 << ", " << old_b2 << ")";
//         my_hash(*hold, &old_b2, &old_b1);
//         std::cout << "\tRevered: (" << old_b1 << ", " << old_b2 << ")" << std::endl;

//         std::cout << "empty_slot: " << empty_slot << std::endl;
//         assert(false);
//     }
//     assert(does_bucket_contain_valid_elements(*bucket_index));

//     // assert(!el3002_was_inserted || h());
// }
// auto swap_elements_from_bucket_old(size_t bucket_index, bucket_type x) -> bucket_type {
//     assert(is_bucket_full_by_index(bucket_index));
//     auto *bp = Table[bucket_index].bits_;

//     auto rand_bucket_index = random() % bucket_size;
//     auto correct_index = rand_bucket_index;
//     bucket_type temp = bp[correct_index];
//     if (temp != x) {
//         bp[correct_index] = x;
//         return temp;
//     }
//     if (temp == x) {
//         // std::cout << "G2" << std::endl;
//         bool did_fail = true;
//         for (int i = 1; i < bucket_size; ++i) {
//             correct_index = (rand_bucket_index + i) % bucket_size;
//             temp = bp[correct_index];
//             if (temp != x) {
//                 did_fail = false;
//                 break;
//             }
//         }
//         if (did_fail) {
//             std::cout << "insertion failed. The bucket contains is full with the same element. (" << x << ")"
//                       << std::endl;
//             print_bucket(bucket_index);
//             assert(false);
//         }
//     }
//     assert(temp != x);
//     bp[correct_index] = x;
//     if (f(x)) {
//         std::cout << "x bucket is: " << bucket_index;
//         std::cout << "\t in location :" << correct_index << std::endl;
//     }
//     return temp;
// }
