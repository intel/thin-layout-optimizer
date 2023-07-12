#ifndef SRC_D_PERF_D_PERF_FILE_H_
#define SRC_D_PERF_D_PERF_FILE_H_

#include "src/perf/perf-parse.h"
#include "src/perf/perf-stats.h"

#include "src/util/file-reader.h"


////////////////////////////////////////////////////////////////////////////////
// Contains code for parsing perf file from a file reader.
//
// We parse both mmap events and LBR/sample events here.


namespace tlo {
namespace perf {

static bool
create_perf_cmdline(std::string_view       cmdline,
                    std::string_view       input_file,
                    preader_t::cmdline_t * outbuf) {
    if (cmdline.length() + input_file.length() >= outbuf->size()) {
        return false;
    }
    auto it = std::copy(cmdline.begin(), cmdline.end(), outbuf->begin());
    it      = std::copy(input_file.begin(), input_file.end(), it);
    *it     = '\0';
    return true;
}


// Create the cmdline for `perf script` to get dump of mmap events
static bool
create_perf_info_cmdline(std::string_view       input_file,
                         preader_t::cmdline_t * outbuf) {
    return create_perf_cmdline(
        "perf script -F comm,pid,tid,time  --show-mmap-events --show-task-events -i ",
        input_file, outbuf);
}

// Create the cmdline for `perf script` to get dump of LBR/sample events.
static bool
create_perf_events_cmdline(std::string_view       input_file,
                           preader_t::cmdline_t * outbuf) {
    return create_perf_cmdline(
        "perf script -F comm,pid,tid,time,ip,dso,brstack -i ", input_file,
        outbuf);
}

// Parses entire file and accumulates the samples intos pstats.
// The important stuff is in perf-parse / perf-stats
bool collect_perf_file_events(file_reader_t * fr_events, perf_stats_t * pstats);
bool collect_perf_file_info(file_reader_t * fr_map, perf_stats_t * pstats);

}  // namespace perf
}  // namespace tlo


#endif
