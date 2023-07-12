#ifndef SRC_D_UTIL_D_PATH_H_
#define SRC_D_UTIL_D_PATH_H_
#include "src/util/algo.h"
#include "src/util/vec.h"

#include <assert.h>
#include <string.h>

#include <string_view>

////////////////////////////////////////////////////////////////////////////////
// Some helpers related to manipulating file paths.

namespace tlo {
namespace detail {
static std::string_view
path_delimp_to_sview(std::string_view path, const void * p) {
    if (p == nullptr) {
        return { "", 0 };
    }

    const char * s = reinterpret_cast<const char *>(p) + 1;
    assert(s > path.data());
    assert(s <= path.data() + path.length());
    return { s, path.length() - static_cast<size_t>(s - path.data()) };
}
static std::string_view
path_getr_delim(std::string_view path, char delim) {
    const void * p = memrchr(reinterpret_cast<const void *>(path.data()), delim,
                             path.length());
    return path_delimp_to_sview(path, p);
}

static std::string_view
path_get_delim(std::string_view path, char delim) {
    const void * p = memchr(reinterpret_cast<const void *>(path.data()), delim,
                            path.length());
    return path_delimp_to_sview(path, p);
}

}  // namespace detail

static std::string_view
path_get_filename(std::string_view path) {
    std::string_view r = detail::path_getr_delim(path, '/');
    if (r.length() == 0) {
        return path;
    }
    return r;
}

static std::string_view
path_get_fulldir(std::string_view path) {
    const void * p = memrchr(reinterpret_cast<const void *>(path.data()), '/',
                             path.length());
    if (p == nullptr) {
        return { "", 0 };
    }
    return std::string_view{
        path.data(),
        static_cast<size_t>(reinterpret_cast<const char *>(p) - path.data())
    };
}

static std::string_view
path_get_file_end_ext(std::string_view path) {
    return detail::path_getr_delim(path_get_filename(path), '.');
}

static std::string_view
path_get_file_start_ext(std::string_view path) {
    return detail::path_get_delim(path_get_filename(path), '.');
}

static std::string_view
path_join(tlo::vec_t<char> * buf, std::string_view next) {
    if (buf == nullptr) {
        return {};
    }
    while (!buf->empty() && buf->back() == '\0') {
        buf->pop_back();
    }

    if (buf->empty() || buf->back() != '/') {
        buf->emplace_back('/');
    }
    size_t base_sz = buf->size();
    auto   begin   = next.begin();
    for (; begin != next.end() && *begin == '/'; ++begin) {
    }
    std::copy(begin, next.end(), std::back_inserter(*buf));
    buf->emplace_back('\0');
    return { buf->data(), base_sz + next.length() + 1 };
}

static std::string_view
path_join(tlo::vec_t<char> * buf, const char * s) {
    return path_join(buf, std::string_view{ s, strlen(s) });
}

}  // namespace tlo


#endif
