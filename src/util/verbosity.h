#include <stdint.h>
#include <stdio.h>
#include <time.h>

#include <cstddef>
#include <limits>

#include "src/util/compiler.h"


////////////////////////////////////////////////////////////////////////////////
// Just macros for printing w/ verbosity.

#ifndef SRC_D_UTIL_D_VERBOSITY_H_
# define SRC_D_UTIL_D_VERBOSITY_H_


namespace tlo {

# define TLO_print(...)    TLO_fprint(TLO_STDOUT, __VA_ARGS__)
# define TLO_printv(...)   TLO_fprintv(TLO_STDOUT, __VA_ARGS__)
# define TLO_printvv(...)  TLO_fprintvv(TLO_STDOUT, __VA_ARGS__)
# define TLO_printvvv(...) TLO_fprintvvv(TLO_STDOUT, __VA_ARGS__)


# define TLO_perr(...)    TLO_fprint(stderr, __VA_ARGS__)
# define TLO_perrv(...)   TLO_fprintv(stderr, __VA_ARGS__)
# define TLO_perrvv(...)  TLO_fprintvv(stderr, __VA_ARGS__)
# define TLO_perrvvv(...) TLO_fprintvvv(stderr, __VA_ARGS__)

# define TLO_fprint(fp, ...)                                                   \
  TLO_fprint_if(tlo::has_verbosity(0), fp, __VA_ARGS__)
# define TLO_fprintv(fp, ...)                                                  \
  TLO_fprint_if(tlo::has_verbosity(1), fp, __VA_ARGS__)
# define TLO_fprintvv(fp, ...)                                                 \
  TLO_fprint_if(tlo::has_verbosity(2), fp, __VA_ARGS__)
# define TLO_fprintvvv(fp, ...)                                                \
  TLO_fprint_if(tlo::has_verbosity(3), fp, __VA_ARGS__)

# define TLO_print_ifv(vlvl, ...)                                              \
  TLO_fprint_if(tlo::has_verbosity(vlvl), TLO_STDOUT, __VA_ARGS__)
# define TLO_fprint_ifv(vlvl, fp, ...)                                         \
  TLO_fprint_if(tlo::has_verbosity(vlvl), fp, __VA_ARGS__)
# define TLO_print_if(c, ...) TLO_fprint_if(c, TLO_STDOUT, __VA_ARGS__)
# define TLO_perr_if(c, ...)  TLO_fprint_if(c, stderr, __VA_ARGS__)

# define TLO_fprint_if(c, fp, ...)                                             \
  if ((c)) {                                                                   \
   (void)fprintf(fp, __VA_ARGS__);                                             \
  }


extern int g_verbosity;
static bool
has_verbosity(int lvl) {
# ifdef TLO_VERBOSITY
    if (lvl <= (TLO_VERBOSITY)) {
        return true;
    }
# endif
    return (lvl <= g_verbosity);
}

void set_verbosity(int lvl);

struct progress_bar_t {
    size_t       cur_progress_;
    size_t       next_progress_bar_;
    size_t       chunk_;
    const char * desc_;
    FILE *       fp_;
    long         last_ts_;
    bool         done_;
    bool         cnt_mode_;

    static constexpr long k_sec_to_ns = 1000L * 1000L * 1000L;

    static constexpr size_t k_perc_granularity = 100;
    static constexpr size_t k_one_million      = 1000U * 1000U;
    static constexpr size_t k_cnt_granularity  = 5U * k_one_million;
    progress_bar_t(size_t       endgoal,
                   int          vlvl,
                   const char * desc = nullptr,
                   FILE *       fp   = stdout) {
        cur_progress_      = 0;
        next_progress_bar_ = 0;
        chunk_             = 0;
        desc_              = desc;
        fp_                = fp;
        cnt_mode_ = endgoal >= (std::numeric_limits<size_t>::max() / 2LU);
        if (has_verbosity(vlvl)) {
            chunk_ = endgoal / k_perc_granularity;
            done_  = false;
        }
        else {
            done_ = true;
        }
        last_ts_ = 0;
    }
    ~progress_bar_t() {
        if (cnt_mode_) {
            (void)fprintf(fp_, "\n");
            (void)fflush(fp_);
        }
    }

    bool
    reasonable_time_since() {
        TLO_DISABLE_WREDUNDANT_TAGS
        struct timespec ts;
        TLO_REENABLE_WREDUNDANT_TAGS
        if (clock_gettime(CLOCK_MONOTONIC, &ts) != 0) {
            return true;
        }
        long cur_ts = ts.tv_sec * k_sec_to_ns + ts.tv_nsec;
        if ((cur_ts - last_ts_) >= (k_sec_to_ns / 2)) {
            last_ts_ = cur_ts;
            return true;
        }
        return false;
    }


    void
    check_progress() {
        if (done_) {
            return;
        }
        if (cur_progress_ >= next_progress_bar_) {
            bool do_print = reasonable_time_since();

            if (cnt_mode_) {
                TLO_fprint_if(do_print, fp_, "\rProgress %s: %zuM",
                              desc_ == nullptr ? "" : desc_,
                              cur_progress_ / k_one_million);
                next_progress_bar_ = cur_progress_ + k_cnt_granularity;
            }
            else {
                size_t cnt = next_progress_bar_ / chunk_;
                done_      = cnt >= k_perc_granularity;
                do_print |= done_;
                TLO_fprint_if(do_print, fp_, "\rProgress %s: %%%zu",
                              desc_ == nullptr ? "" : desc_, cnt);
                next_progress_bar_ += chunk_;

                if (done_) {
                    TLO_fprint_if(do_print, fp_, "\n");
                }
            }
            if (do_print) {
                (void)fflush(fp_);
            }
        }
    }


    void
    add_progress(size_t addby) {
        update_progress(cur_progress_ + addby);
    }

    void
    update_progress(size_t setto) {
        cur_progress_ = setto;
        check_progress();
    }
};

# undef TLO_VERBOSITY

}  // namespace tlo

#endif
