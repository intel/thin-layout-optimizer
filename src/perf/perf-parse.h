#ifndef SRC_D_PERF_D_PERF_PARSE_H_
#define SRC_D_PERF_D_PERF_PARSE_H_


#include "src/perf/perf-sample.h"

#include <limits>
#include <string_view>
#include <type_traits>

////////////////////////////////////////////////////////////////////////////////
// Helpers for parsing a single output line from perf data dump.
// Parses ascii text (so output for `perf script ...`)

namespace tlo {
namespace perf {

static constexpr size_t k_parse_done       = 0;
static constexpr size_t k_parse_error      = std::numeric_limits<size_t>::max();
static constexpr size_t k_parse_incomplete = k_parse_error - 1;

// Parser mmap/fork/comm lines.
size_t parse_info_line(const std::string_view buf, info_sample_t * sample_out);

// First function called on a new line. Returns `k_parse_done` if this was just
// a simple sample and offset to start of lbr data if it was an lbr sample
size_t parse_sample_line(const std::string_view buf,
                         simple_sample_t *      sample_out);
static size_t
parse_sample_line(const char * buf, size_t len, simple_sample_t * sample_out) {
    return parse_sample_line(std::string_view{ buf, len }, sample_out);
}

// Called only after `parse_sample_line` was called on the line. Parses an lbr
// entry.
size_t parse_lbr_line(const std::string_view buf,
                      size_t                 off,
                      lbr_sample_t *         sample_out);

static size_t
parse_lbr_line(const char *   buf,
               size_t         len,
               size_t         off,
               lbr_sample_t * sample_out) {
    return parse_lbr_line(std::string_view(buf, len), off, sample_out);
}


}  // namespace perf
}  // namespace tlo

#endif
