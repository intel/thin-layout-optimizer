#include "src/perf/perf-file.h"
#include "src/perf/perf-parse.h"


namespace tlo {
namespace perf {

static size_t
handle_maybe_err(size_t res, size_t err_cnt, const char * line) {
    if (TLO_UNLIKELY(res == k_parse_error)) {
        if (tlo::has_verbosity(2)) {
            TLO_perr("Warning: Bad Line ->\n\"%s\"\n", line);
        }
        else {
            TLO_perr("\rWarning: Bad Line(%zu)", ++err_cnt);
        }
    }
    return err_cnt;
}

bool
collect_perf_file_info(file_reader_t * fr_map, perf_stats_t * pstats) {
    bool             ret = false;
    std::string_view buf;
    progress_bar_t   progress(fr_map->nbytes_total(), 0, "Info Events Parsed");
    size_t           err_cnt = 0;
    for (;;) {
        // Parse each line in the file (until empty).
        buf = fr_map->nextline();
        progress.update_progress(fr_map->nbytes_read());
        if (buf.empty()) {
            pstats->finalize_mappings();
            return ret;
        }
        info_sample_t sample;  // NOLINT
        const size_t  res = parse_info_line(buf, &sample);
        if (res == k_parse_done) {
            assert(sample.active());
            if (sample.is_mmap()) {
                ret |= pstats->collect_mmap_sample(sample);
            }
            else if (sample.is_comm()) {
            }
            else if (sample.is_fork()) {
                ret |= pstats->collect_fork_sample(sample);
            }
        }
        err_cnt = handle_maybe_err(res, err_cnt, buf.data());
    }
}

bool
collect_perf_file_events(file_reader_t * fr_events, perf_stats_t * pstats) {
    bool             ret = false;
    std::string_view buf;
    progress_bar_t progress(fr_events->nbytes_total(), 0, "Perf Events Parsed");
    size_t         err_cnt = 0;
    for (;;) {
        // Parse each line in the file (until empty).
        buf = fr_events->nextline();
        progress.update_progress(fr_events->nbytes_read());
        if (buf.empty()) {
            return ret;
        }
        // NOTE: We MUST consume the sample before starting the next line.
        // Some of what the sample stores are just pointers to locations in
        // the parsed line (strings for sym/dso).
        lbr_sample_t sample;  // NOLINT
        size_t       res = parse_sample_line(buf, &sample);
        // If return is k_parse_done this was a simple sample.
        if (res == k_parse_done) {
            ret |= pstats->collect_simple_sample_stats(&sample);
        }
        else if (res != k_parse_error) {
            // Otherwise it was an LBR sample.
            res = parse_lbr_line(buf, res, &sample);
            if (res == k_parse_done) {
                ret |= pstats->collect_lbr_sample_stats(&sample);
            }
        }
        err_cnt = handle_maybe_err(res, err_cnt, buf.data());
    }
}

}  // namespace perf
}  // namespace tlo
