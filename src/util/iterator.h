#ifndef SRC_D_UTIL_D_ITERATOR_H_
#define SRC_D_UTIL_D_ITERATOR_H_

////////////////////////////////////////////////////////////////////////////////
// Simple helper for making iterable sequence from [N...M)

#include "src/util/type-info.h"


namespace tlo {

template<typename T_t>
struct cnt_iterator_t {
    static_assert(std::is_integral<T_t>::value || std::is_pointer<T_t>::value);
    T_t lb_;
    T_t ub_;

    struct cnt_iterator_impl_t {
        T_t cur_;

        T_t
        operator*() const {
            return cur_;
        }

        cnt_iterator_impl_t &
        operator++() {
            ++cur_;
            return *this;
        }


        bool
        operator!=(cnt_iterator_impl_t const & other) {
            return cur_ != other.cur_;
        }
    };

    cnt_iterator_impl_t
    begin() const {
        return cnt_iterator_impl_t{ lb_ };
    }

    cnt_iterator_impl_t
    end() const {
        return cnt_iterator_impl_t{ ub_ };
    }
};


template<typename T_t>
cnt_iterator_t<T_t>
make_cnt_iterator(T_t lb, T_t ub) {
    static_assert(has_okay_type_traits<cnt_iterator_t<T_t>>::value);
    return cnt_iterator_t<T_t>{ lb, ub };
}

}  // namespace tlo

#endif
