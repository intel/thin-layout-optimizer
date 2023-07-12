#include "src/perf/perf-parse.h"
#include "src/perf/perf-parse-state.h"
#include "src/system/br-insn.h"
#include "src/util/compiler.h"
#include "src/util/macro.h"

#include "src/util/verbosity.h"

#include <string_view>
#include <tuple>

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//////////////////////////////////////////////////////////////////////
// Parses perf file.
// NOTE: Expects input standard perf.
// Tests perf.5.15 - perf.6.3

namespace tlo {
namespace perf {


// NOLINTNEXTLINE(readability-avoid-unconditional-preprocessor-if)
#if 0
# define PARSE_ASSERT(cond) assert(cond);
#else
# define PARSE_ASSERT(cond)                                                    \
     if (TLO_UNLIKELY(!(cond))) {                                              \
         TLO_perrv("Parse Error: %s:%d: %s\n", __FILE__, __LINE__,             \
                   TLO_STRINGIFY(cond));                                       \
         return k_parse_error;                                                 \
     }
#endif


static size_t
parse_sample_hdr(const std::string_view buf, sample_hdr_t * sample_out) {
    parse_state_t parser = { buf, 0 };
    // <str:comm> <u32:pid>/<u32:tid> <u32:ts_hi>.<u32:ts_lo>:
    bool err;

    // <str::comm>
    char const * tmp_s;
    char const * end_s;
    size_t       slen;
    PARSE_ASSERT(parser.skip_ws());
    tmp_s = parser.cur();
    // Somewhat complicated as command lines can pretty arbitrary characters.
    // Estimate is we first skip to next whitespace. Then to '/'. This hopefully
    // gets us to pid/tid field. Then we backtrack through digits then ws to end
    // of command.
    PARSE_ASSERT(parser.skip_to_ws());
    PARSE_ASSERT(parser.skip_to('/'));
    PARSE_ASSERT(parser.skip_back_before_digit());
    PARSE_ASSERT(parser.skip_back_before_ws());
    end_s = parser.cur();
    PARSE_ASSERT(parser.at_ws());
    PARSE_ASSERT(end_s > tmp_s);
    slen = static_cast<size_t>(end_s - tmp_s);
    PARSE_ASSERT(small_str_t<char const *>::fits(slen));
    const small_str_t<char const *> comm(tmp_s, static_cast<uint16_t>(slen));


    // <u32:pid>/<u32:tid>
    uint32_t pid, tid;
    PARSE_ASSERT(parser.skip_ws());
    std::tie(pid, err) = parser.get_decint<uint32_t>();
    PARSE_ASSERT(!err);
    PARSE_ASSERT(parser.at_c('/'));
    PARSE_ASSERT(parser.skip_fwd(1));
    std::tie(tid, err) = parser.get_decint<uint32_t>();
    PARSE_ASSERT(!err);
    PARSE_ASSERT(parser.at_ws());
    PARSE_ASSERT(parser.skip_ws());

    // <u32:ts_hi>.<u32:ts_lo>:
    uint64_t ts_before_decimal, ts_after_decimal, ts;
    std::tie(ts_before_decimal, err) = parser.get_decint<uint64_t>();
    PARSE_ASSERT(!err);
    PARSE_ASSERT(parser.at_c('.'));
    PARSE_ASSERT(parser.skip_fwd(1));
    std::tie(ts_after_decimal, err) = parser.get_decint<uint64_t>();
    PARSE_ASSERT(!err);
    PARSE_ASSERT(parser.at_c(':'));
    PARSE_ASSERT(parser.skip_fwd(1));
    PARSE_ASSERT(parser.at_ws());
    PARSE_ASSERT(parser.skip_ws());
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
    ts = (ts_before_decimal << 32U) + ts_after_decimal;

    *sample_out = { pid, tid, ts, comm };
    return parser.bytes_parsed();
}
static size_t
parse_fork_line(const std::string_view buf,
                size_t                 pre_parsed,
                sample_fork_t *        sample_out) {
    // <{hdr}> PERF_RECORD_FORK(<u32:cpid>:<u32:ctid>):(<u32:ppid>:<u32:ptid>)
    parse_state_t parser = { buf, pre_parsed };
    bool          err;

    assert(sample_out != nullptr);

    // PERF_RECORD_FORK
    PARSE_ASSERT(parser.at_str("PERF_RECORD_FORK"));
    PARSE_ASSERT(parser.skip_str("PERF_RECORD_FORK"));
    PARSE_ASSERT(parser.skip_ws());

    //(<u32:cpid>/<u32:ctid>):
    PARSE_ASSERT(parser.at_c('('));
    PARSE_ASSERT(parser.skip_fwd(1));
    uint32_t cpid, ctid;
    std::tie(cpid, err) = parser.get_decint<uint32_t>();
    PARSE_ASSERT(!err);
    PARSE_ASSERT(parser.at_c(':'));
    PARSE_ASSERT(parser.skip_fwd(1));
    std::tie(ctid, err) = parser.get_decint<uint32_t>();
    PARSE_ASSERT(!err);
    PARSE_ASSERT(parser.at_c(')'));
    PARSE_ASSERT(parser.skip_fwd(1));
    PARSE_ASSERT(parser.at_c(':'));
    PARSE_ASSERT(parser.skip_fwd(1));
    PARSE_ASSERT(parser.skip_ws());

    //(<u32:ppid>/<u32:ptid>)
    PARSE_ASSERT(parser.at_c('('));
    PARSE_ASSERT(parser.skip_fwd(1));
    uint32_t ppid, ptid;
    std::tie(ppid, err) = parser.get_decint<uint32_t>();
    PARSE_ASSERT(!err);
    PARSE_ASSERT(parser.at_c(':'));
    PARSE_ASSERT(parser.skip_fwd(1));
    std::tie(ptid, err) = parser.get_decint<uint32_t>();
    PARSE_ASSERT(!err);
    PARSE_ASSERT(parser.at_c(')'));
    PARSE_ASSERT(parser.skip_fwd(1));
    PARSE_ASSERT(parser.skip_to_or_ws('\n'));

    *sample_out = { ppid, ptid, cpid, ctid };

    return k_parse_done;
}

static size_t
parse_comm_line(const std::string_view buf,
                size_t                 pre_parsed,
                sample_comm_t *        sample_out) {
    // <{hdr}> PERF_RECORD_COMM <str:exec or empty>: <str:new
    // comm>:<u32:pid>/<u32:tid>
    parse_state_t parser = { buf, pre_parsed };
    bool          err;

    assert(sample_out != nullptr);

    // PERF_RECORD_COMM
    PARSE_ASSERT(parser.at_str("PERF_RECORD_COMM"));
    PARSE_ASSERT(parser.skip_str("PERF_RECORD_COMM"));
    PARSE_ASSERT(parser.skip_ws());


    // <std::exec or empty>
    const bool exec = parser.at_str("exec");
    PARSE_ASSERT(parser.skip_to(':'));
    PARSE_ASSERT(parser.at_c(':'));
    PARSE_ASSERT(parser.skip_fwd(1));
    PARSE_ASSERT(parser.skip_ws());

    // <std::new comm>:
    char const * tmp_s;
    char const * end_s;
    size_t       slen;
    tmp_s = parser.cur();
    PARSE_ASSERT(parser.skip_to('\n'));
    PARSE_ASSERT(parser.skip_back_to_c(':'));
    end_s = parser.cur();
    PARSE_ASSERT(end_s > tmp_s);
    slen = static_cast<size_t>(end_s - tmp_s);
    PARSE_ASSERT(small_str_t<char const *>::fits(slen));
    const small_str_t<char const *> comm(tmp_s, static_cast<uint16_t>(slen));
    PARSE_ASSERT(parser.at_c(':'));
    PARSE_ASSERT(parser.skip_fwd(1));
    PARSE_ASSERT(parser.skip_ws());

    // <u32:pid>/<u32:tid>
    uint32_t pid, tid;
    std::tie(pid, err) = parser.get_decint<uint32_t>();
    PARSE_ASSERT(!err);
    PARSE_ASSERT(parser.at_c('/'));
    PARSE_ASSERT(parser.skip_fwd(1));
    std::tie(tid, err) = parser.get_decint<uint32_t>();
    PARSE_ASSERT(!err);
    PARSE_ASSERT(parser.skip_to_or_ws('\n'));

    *sample_out = { pid, tid, exec, comm };

    return k_parse_done;
}

static size_t
parse_mmap_line(const std::string_view buf,
                size_t                 pre_parsed,
                sample_mmap_t *        sample_out) {
    // clang-format off
    // <{hdr}> PERF_RECORD_MMAP2 <u32:pid>/<u32:tid>: [<0x64:base>(<0x64:size>) @ <0x64:off> <str:unused>]: <str:perms> <str:dso>
    // clang-format on
    parse_state_t parser = { buf, pre_parsed };
    bool          err;

    assert(sample_out != nullptr);

    // PERF_RECORD_MMAP2
    PARSE_ASSERT(parser.at_str("PERF_RECORD_MMAP2"));
    PARSE_ASSERT(parser.skip_str("PERF_RECORD_MMAP2"));
    PARSE_ASSERT(parser.at_ws());
    PARSE_ASSERT(parser.skip_ws());

    // <u32:pid>/<u32:tid>:
    uint32_t pid, tid;
    std::tie(pid, err) = parser.get_decint<uint32_t>();
    PARSE_ASSERT(!err);
    PARSE_ASSERT(parser.at_c('/'));
    PARSE_ASSERT(parser.skip_fwd(1));
    std::tie(tid, err) = parser.get_decint<uint32_t>();
    PARSE_ASSERT(!err);
    PARSE_ASSERT(parser.at_c(':'));
    PARSE_ASSERT(parser.skip_fwd(1));
    PARSE_ASSERT(parser.skip_ws());
    PARSE_ASSERT(parser.skip_to('['));

    // [<0x64:base>(<0x64:size>) @ <0x64:off> <str:unused>]
    uint64_t base, size, off;
    PARSE_ASSERT(parser.at_c('['));
    PARSE_ASSERT(parser.skip_fwd(1));
    std::tie(base, err) = parser.get_hexint<uint64_t>();
    PARSE_ASSERT(!err);
    PARSE_ASSERT(parser.at_c('('));
    PARSE_ASSERT(parser.skip_fwd(1));
    std::tie(size, err) = parser.get_hexint<uint64_t>();
    PARSE_ASSERT(!err);
    PARSE_ASSERT(parser.at_c(')'));
    PARSE_ASSERT(parser.skip_fwd(1));
    PARSE_ASSERT(parser.skip_to('@'));
    PARSE_ASSERT(parser.at_c('@'));
    PARSE_ASSERT(parser.skip_fwd(1));
    PARSE_ASSERT(parser.skip_ws());
    std::tie(off, err) = parser.get_hexint<uint64_t>();
    PARSE_ASSERT(!err);
    PARSE_ASSERT(parser.skip_to(']'));
    PARSE_ASSERT(parser.skip_fwd(1));
    PARSE_ASSERT(parser.at_c(':'));
    PARSE_ASSERT(parser.skip_fwd(1));
    PARSE_ASSERT(parser.skip_ws());

    // <str:perms>
    bool read   = false;
    bool write  = false;
    bool exec   = false;
    bool shared = false;
    bool priv   = false;
    while (!parser.at_ws()) {
        read |= parser.at_c('r');
        write |= parser.at_c('w');
        exec |= parser.at_c('x');
        shared |= parser.at_c('s');
        priv |= parser.at_c('p');
        parser.skip_fwd(1);
    }

    // <str::dso>
    char const * tmp_s;
    char const * end_s;
    size_t       slen;
    PARSE_ASSERT(parser.at_ws());
    PARSE_ASSERT(parser.skip_ws());
    tmp_s = parser.cur();
    PARSE_ASSERT(parser.skip_to_or_ws('\n'));
    end_s = parser.cur();
    PARSE_ASSERT(end_s > tmp_s);
    slen = static_cast<size_t>(end_s - tmp_s);
    PARSE_ASSERT(small_str_t<char const *>::fits(slen));
    const small_str_t<char const *> dso(tmp_s, static_cast<uint16_t>(slen));


    *sample_out = { base,
                    size,
                    off,
                    pid,
                    tid,
                    static_cast<uint8_t>(read),
                    static_cast<uint8_t>(write),
                    static_cast<uint8_t>(exec),
                    static_cast<uint8_t>(shared),
                    static_cast<uint8_t>(priv),
                    dso };
    return k_parse_done;
}

// perf script -F comm,pid,tid,time  --show-mmap-events --show-task-events
size_t
parse_info_line(const std::string_view buf, info_sample_t * sample_out) {
    // <{hdr}> <PERF_RECORD_MMAP2,PERF_RECORD_FORK,PERF_RECORD_COMM>...
    sample_out->reset();
    assert(!sample_out->active());
    size_t pre_parsed = parse_sample_hdr(buf, &(sample_out->hdr_));
    if (pre_parsed == k_parse_error) {
        return k_parse_error;
    }

    const parse_state_t parser = { buf, pre_parsed };
    if (parser.at_str("PERF_RECORD_MMAP2")) {
        sample_out->use_mmap();
        pre_parsed = parse_mmap_line(buf, pre_parsed, sample_out->get_mmap());
        assert(pre_parsed != k_parse_done || sample_out->is_mmap());
    }

    else if (parser.at_str("PERF_RECORD_FORK")) {
        sample_out->use_fork();
        pre_parsed = parse_fork_line(buf, pre_parsed, sample_out->get_fork());
        assert(pre_parsed != k_parse_done || sample_out->is_fork());
    }

    else if (parser.at_str("PERF_RECORD_COMM")) {
        sample_out->use_comm();
        pre_parsed = parse_comm_line(buf, pre_parsed, sample_out->get_comm());
        assert(pre_parsed != k_parse_done || sample_out->is_comm());
    }
    return pre_parsed;
}

static size_t
parse_sample_dso(parse_state_t * parser, small_str_t<char const *> * dso_out) {
    // (<str::dso>)
    char const * tmp_s;
    char const * end_s = nullptr;
    size_t       slen;
    PARSE_ASSERT(parser->at_c('('));
    PARSE_ASSERT(parser->skip_fwd(1));

    PARSE_ASSERT(parser->skip_ws());
    tmp_s = parser->cur();
    PARSE_ASSERT(parser->skip_to_or_ws(')'));

    if (parser->at_ws()) {
        if (parser->at_str(" (deleted)")) {
            end_s = parser->cur();
            PARSE_ASSERT(parser->skip_str(" (deleted)"));
        }
        else if (parser->at_str(" spaces/clang)")) {
            PARSE_ASSERT(parser->cur() > tmp_s);
            const std::string_view tmp_sview = {
                tmp_s, static_cast<size_t>(parser->cur() - tmp_s)
            };
            if (tmp_sview.ends_with(
                    "tools/clang/test/Driver/Output/clang_f_opts.c.tmp.r/with")) {
                parser->skip_str(" spaces/clang");
                end_s = parser->cur();
            }
        }

        if (end_s == nullptr) {
            // NOLINTNEXTLINE(cert-dcl03-c,hicpp-static-assert,misc-static-assert)
            PARSE_ASSERT(0 && "Unknown Space in directory path!");
        }
    }
    else {
        end_s = parser->cur();
    }
    PARSE_ASSERT(parser->at_c(')'));
    PARSE_ASSERT(end_s > tmp_s);
    slen = static_cast<size_t>(end_s - tmp_s);
    PARSE_ASSERT(small_str_t<char const *>::fits(slen));
    *dso_out = { tmp_s, static_cast<uint16_t>(slen) };
    PARSE_ASSERT(parser->at_c(')'));
    PARSE_ASSERT(parser->skip_fwd(1));
    return k_parse_done;
}

// perf script -F comm,pid,tid,time,ip,dso,brstack --no-demangle
size_t
parse_sample_line(const std::string_view buf, simple_sample_t * sample_out) {

    // <{hdr}> <0x64:ip> (<str:dso>)
    const size_t pre_parsed = parse_sample_hdr(buf, &(sample_out->hdr_));
    if (pre_parsed == k_parse_error) {
        return k_parse_error;
    }

    parse_state_t parser = { buf, pre_parsed };
    bool          err;

    // <0x64:ip>
    uint64_t ip;
    std::tie(ip, err) = parser.get_hexint<uint64_t>();
    PARSE_ASSERT(parser.skip_ws());

    // (<str::dso>)
    small_str_t<char const *> dso{ "", 0 };
    PARSE_ASSERT(dso.empty());
    PARSE_ASSERT(parse_sample_dso(&parser, &dso) == k_parse_done);
    PARSE_ASSERT(!dso.empty());
    parser.skip_to_or_ws('\n');
    parser.skip_ws();


    sample_out->loc_ = { ip, {}, dso };
    return (parser.at_end() || parser.at_c('\n')) ? k_parse_done
                                                  : parser.bytes_parsed();
}

static size_t
parse_lbr_entry_prepare(const std::string_view buf, size_t off) {
    parse_state_t parser = { buf, off };
    if (parser.skip_ws() && !parser.at_end()) {
        return parser.bytes_parsed();
    }
    return tlo::perf::k_parse_incomplete;
}

static size_t
parse_lbr_entry_flags(const std::string_view buf,
                      size_t                 off,
                      lbr_br_sample_t *      sample_out) {
    // perf < 5.15
    // <char:predicted>/<char:in_tx>/<char:abort>/<u32:cycles>
    // perf < 6.3.0
    // <char:predicted>/<char:in_tx>/<char:abort>/<u32:cycles>/<str:brtype>
    // perf >= 6.3.0
    // <char:predicted>/<char:in_tx>/<char:abort>/<u32:cycles>/<str:brtype>/<str:brdesc>

    parse_state_t parser = { buf, off };
    uint32_t      predicted, cycles;
    bool          in_tx, aborted, err;
    // <char:predicted>/
    PARSE_ASSERT(parser.at_c('P') || parser.at_c('M') || parser.at_c('-'));
    predicted = parser.at_c('-')
                    ? lbr_br_sample_t::k_unknown
                    : (parser.at_c('P') ? lbr_br_sample_t::k_pred
                                        : lbr_br_sample_t::k_mispred);
    PARSE_ASSERT(parser.skip_fwd(1));
    PARSE_ASSERT(parser.at_c('/'));
    PARSE_ASSERT(parser.skip_fwd(1));

    // <char:in_tx>/
    PARSE_ASSERT(parser.at_c('X') || parser.at_c('-'));
    in_tx = parser.at_c('X');
    PARSE_ASSERT(parser.skip_fwd(1));
    PARSE_ASSERT(parser.at_c('/'));
    PARSE_ASSERT(parser.skip_fwd(1));

    // <char:abort>/
    PARSE_ASSERT(parser.at_c('A') || parser.at_c('-'));
    aborted = parser.at_c('A');
    PARSE_ASSERT(parser.skip_fwd(1));
    PARSE_ASSERT(parser.at_c('/'));
    PARSE_ASSERT(parser.skip_fwd(1));

    // <u32:cycles>/
    std::tie(cycles, err) = parser.get_decint<uint32_t>();
    PARSE_ASSERT(!err);
    if (parser.at_c('/')) {
        parser.skip_to_ws();
    }

    sample_out->cycles_    = cycles;
    sample_out->predicted_ = predicted & 3U;
    sample_out->in_tx_     = in_tx;
    sample_out->aborted_   = aborted;
    sample_out->br_insn_   = {};
    return parser.bytes_parsed();
}


static size_t
parse_lbr_entry_till_slash(const std::string_view buf,
                           size_t                 off,
                           sample_loc_t *         sample_out) {
    // <u64:mapped_addr>/s*(<str:dso>)/
    parse_state_t parser = { buf, off };
    uint64_t      mapped_addr;
    bool          err;

    // <u64:mapped_addr>
    std::tie(mapped_addr, err) = parser.get_hexint<uint64_t>();
    PARSE_ASSERT(!err);
    PARSE_ASSERT(parser.skip_ws());

    // (<str::dso>)
    small_str_t<char const *> dso{ "", 0 };
    PARSE_ASSERT(dso.empty());
    PARSE_ASSERT(parse_sample_dso(&parser, &dso) == k_parse_done);
    PARSE_ASSERT(!dso.empty());
    PARSE_ASSERT(parser.skip_to('/'));

    PARSE_ASSERT(parser.at_c('/'));
    PARSE_ASSERT(parser.skip_fwd(1));
    *sample_out = { mapped_addr, {}, dso };
    return parser.bytes_parsed();
}


size_t
parse_lbr_line(const std::string_view buf,
               size_t                 off,
               lbr_sample_t *         sample_out) {
    uint32_t i;
    for (i = 0; i < lbr_sample_t::k_max_lbr_samples; ++i) {
        off = parse_lbr_entry_prepare(buf, off);
        if (off == tlo::perf::k_parse_incomplete) {
            break;
        }
        const uint32_t index = lbr_sample_t::k_max_lbr_samples - i - 1;

        off = parse_lbr_entry_till_slash(buf, off,
                                         &(sample_out->samples_[index].from_));
        PARSE_ASSERT(off != tlo::perf::k_parse_error);
        off = parse_lbr_entry_till_slash(buf, off,
                                         &(sample_out->samples_[index].to_));
        PARSE_ASSERT(off != tlo::perf::k_parse_error);
        off = parse_lbr_entry_flags(buf, off, &(sample_out->samples_[index]));
        PARSE_ASSERT(off != tlo::perf::k_parse_error);
    }
    if (i == 0) {
        return tlo::perf::k_parse_incomplete;
    }

    sample_out->num_samples_ = i;
    if (i != lbr_sample_t::k_max_lbr_samples) {
        memmove(&sample_out->samples_[0],
                &sample_out->samples_[lbr_sample_t::k_max_lbr_samples - i],
                i * sizeof(lbr_br_sample_t));
    }
    return tlo::perf::k_parse_done;
}

}  // namespace perf
}  // namespace tlo
