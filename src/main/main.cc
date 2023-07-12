#include "src/cfg/cfg.h"
#include "src/perf/perf-file.h"
#include "src/perf/perf-saver.h"
#include "src/util/algo.h"
#include "src/util/file-reader.h"
#include "src/util/global-stats.h"
#include "src/util/vec.h"
#include "src/util/verbosity.h"

#include <vector>

#include <errno.h>
#include <getopt.h>
#include <stdio.h>
#include <unistd.h>

// #define TLO_DEBUG
#include "src/util/debug.h"


static const char *
dump_ordered_funcs(FILE *                                         fp,
                   const tlo::sym::dso_t *                        dso,
                   const tlo::vec_t<tlo::cfg_func_order_info_t> & ordered_funcs,
                   bool                                           overwrite) {
    (void)overwrite;
    size_t i = 0;
    // Output order for each function in the DSO.
    (void)fprintf(fp,
                  "commuse_dsodep_buildid_self_or_order,"
                  "name_or_has_section,"
                  "section_or_func_name,"
                  "ordering,"
                  "id,"
                  "incoming_weight,"
                  "outgoing_weight\n");
    (void)fprintf(fp, "self,%s,,,,,\n", dso->str());
    for (const tlo::strbuf_t<> & dso_dep : dso->deps_) {
        (void)fprintf(fp, "dsodep,%s,,,,,\n", dso_dep.str());
    }
    if (dso->has_comm_uses()) {
        for (const tlo::strbuf_t<> & comm_use : dso->comm_uses()) {
            (void)fprintf(fp, "commuse,%s,,,,,\n", comm_use.str());
        }
    }
    if (dso->has_buildids()) {
        for (const tlo::strbuf_t<> & buildid : dso->buildids()) {
            (void)fprintf(fp, "buildid,%s,,,,,\n", buildid.str());
        }
    }

    tlo::basic_uset<tlo::strbuf_t<>> dup_names{};
    for (const auto & func_order_info : ordered_funcs) {
        const auto * func_clump = func_order_info.fc_;
        if (!func_clump->in_dso(dso)) {
            continue;
        }
        if (!func_clump->is_findable()) {
            continue;
        }
        for (const auto & func : func_clump->funcs_) {
            if (dup_names.emplace(func.name_).second) {
                (void)fprintf(fp, "order,%d,%s,%zu,%zu,%lf,%lf\n", 0,
                              func.str(), i++, func_order_info.order_,
                              func_order_info.weight_in_,
                              func_order_info.weight_out_);
            }
        }
    }
    return i == 0 ? "No functions to order" : nullptr;
}


#define TLO_PRINT_USR_ERR(...) TLO_fprint_ifv(-1, stderr, __VA_ARGS__)

static const char *
dump_ordered_funcs(char const *                                   path,
                   const tlo::sym::dso_t *                        dso,
                   const tlo::vec_t<tlo::cfg_func_order_info_t> & ordered_funcs,
                   bool                                           overwrite) {
    // Don't overwrite.
    // TODO: Make this an user-option.
    if (tlo::file_ops::exists(path)) {
        return "Duplicate DSO Files";
    }
    // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
    FILE * fp = fopen(path, "w+");
    if (fp == nullptr) {
        // NOLINTNEXTLINE(*magic*)
        std::array<char, 512> error_buf;
        TLO_PRINT_USR_ERR(
            "Failed to open: %s\n\t-> %s\n", path,
            strerror_r(errno, error_buf.data(), error_buf.size()));
        return "IO Error";
    }
    const char * ret = dump_ordered_funcs(fp, dso, ordered_funcs, overwrite);
    if (ret != nullptr) {
        (void)remove(path);
    }
    // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
    (void)fclose(fp);
    return ret;
}


// Output orders for each findable dso.
template<typename T_dso_it_t>
static bool
write_all(std::string_view                               output_dir,
          T_dso_it_t                                     dso_it,
          const tlo::vec_t<tlo::cfg_func_order_info_t> & ordered_funcs,
          bool                                           overwrite) {

    if (tlo::file_ops::is_dir(output_dir)) {
        if (!tlo::file_ops::is_empty_dir(output_dir)) {
            if (!overwrite) {
                (void)printf(
                    "Directory %s already exists and contains records. Delete? [y/n]\n",
                    output_dir.data());
                int c;
                do {
                    c = getchar();
                } while (c == '\n' || c == ' ' || c == '\t');
                if (c != 'y') {
                    return false;
                }
            }
            if (!tlo::file_ops::clear_dir(output_dir)) {
                (void)fprintf(stderr, "Unable to clear output dir: %s\n",
                              output_dir.data());
                return false;
            }
        }
    }
    else if (!tlo::file_ops::make_dir_p(output_dir.data())) {
        (void)fprintf(stderr, "Unable to create output dir: %s\n",
                      output_dir.data());
        return false;
    }

    tlo::vec_t<char> base_path;
    base_path.reserve(256);  // NOLINT(*magic*)

    auto write_to_buf = [&base_path](char const * s, size_t slen) {
        std::copy(s, s + slen, std::back_inserter(base_path));
    };


    bool ret = false;
    write_to_buf(output_dir.data(), output_dir.length());
    write_to_buf("/ordering-", strlen("/ordering-"));
    const size_t base_len = base_path.size();
    for (const auto * dso : dso_it) {
        // Only output for dsos we can actually find.
        if (!dso->is_findable()) {
            continue;
        }
        base_path.resize(base_len);
        const std::string_view filename = dso->complete_filename();
        size_t                 cur_idx  = base_path.size();
        write_to_buf(filename.data(), filename.length());
        for (; cur_idx < base_path.size(); ++cur_idx) {
            if (base_path[cur_idx] == '/') {
                base_path[cur_idx] = '-';
            }
        }
        // Make sure nullterm is there!
        static_assert(sizeof(".txt") == 5);  // NOLINT(*magic*)
        write_to_buf(".txt", sizeof(".txt"));
        const std::string_view path{ base_path.data(), base_path.size() };
        char const *           err_or_null =
            dump_ordered_funcs(path.data(), dso, ordered_funcs, overwrite);
        if (err_or_null != nullptr) {
            TLO_perrv("Failed to write ordering for %s ->\n\t%s\n", dso->str(),
                      err_or_null);
        }
        else {
            ret = true;
        }
    }
    return ret;
}
static std::string_view
process_input_files(std::string_view   infile,
                    std::string_view   root_path,
                    tlo::vec_t<char> * path_buf,
                    std::string_view   zst_filename,
                    std::string_view   ascii_filename,
                    const char *       desc) {
    if (!infile.empty() && tlo::file_ops::exists(infile.data())) {
        return infile;
    }

    if (!root_path.empty() && tlo::file_ops::is_dir(root_path.data())) {
        path_buf->clear();
        std::copy(root_path.begin(), root_path.end(),
                  std::back_inserter(*path_buf));
        if (tlo::file_ops::exists(
                tlo::path_join(path_buf, zst_filename).data())) {
            TLO_PRINT_USR_ERR("Warning: defaulting to \"%s\" as %s file\n",
                              path_buf->data(), desc);
            return { path_buf->data(), path_buf->size() - 1U };
        }

        path_buf->clear();
        std::copy(root_path.begin(), root_path.end(),
                  std::back_inserter(*path_buf));
        if (tlo::file_ops::exists(
                tlo::path_join(path_buf, ascii_filename).data())) {
            TLO_PRINT_USR_ERR("Warning: defaulting to \"%s\" as %s file\n",
                              path_buf->data(), desc);
            return { path_buf->data(), path_buf->size() - 1U };
        }

        path_buf->clear();
        std::copy(root_path.begin(), root_path.end(),
                  std::back_inserter(*path_buf));
        if (tlo::file_ops::exists(
                tlo::path_join(path_buf, "perf.data").data())) {
            TLO_PRINT_USR_ERR("Warning: defaulting to \"%s\" as %s file\n",
                              path_buf->data(), desc);
            return { path_buf->data(), path_buf->size() - 1U };
        }
    }

    if (infile.empty()) {
        TLO_PRINT_USR_ERR("Error: Missing %s file argument\n", desc);
    }
    else {
        TLO_PRINT_USR_ERR("Error: \"%s\" file argument does not exist at: %s\n",
                          desc, infile.data());
    }
    // NOLINTNEXTLINE(bugprone-string-constructor)
    return { "", 0 };
}

static void
usage(const char * progname) {
    TLO_PRINT_USR_ERR(
        "Usage: %s\n"
        "\t[-h][--help]\t\tDiplay this message\n"
        "\t[-v][-vv][-vvv]\t\tSet verbosity\n"
        "\t[--silent]\t\tDisable all error outputs\n"
        "\t[-p][--perf]\t\tperf.data file or text file with processed results of events\n"
        "\t[-r][--root]\t\troot path with dsos\n"
        "\t[-m][--map]\t\ttext file with processed results of info events\n"
        "\t[-o][--out]\t\tOutput directory\n"
        "\t[-w][--overwrite]\t\tOverwrite existing files without asking\n"
        "\t[--silent]\t\tSilence almost all outputs\n"
        "\t[--save]\t\tSave state from profile. Can be reloaded/combined\n"
        "\t[--reload]\t\tReload state(s). Multiple can be specified as CSV.\n"
        "\t[--dot]\t\tDump CFG as DOT file.\n"
        "\t[--dot-dso]\t\tDSO to dump as DOT file (only relevent if --dot is specified).\n"
        "\t[--no-normalize]\t\tDon't normalize save states.\n"
        "\t[--force-no-normalize]\t\tDon't normalize save states even if one is already scaled.\n"
        "\t[--add-scale]\t\tAdd a custom scaling factor to the output.\n"
        "\t[--use-custom-scale]\t\tUse custom scaling factors from save states.\n"
        "\t[--dump]\t\tDump stats.\n"
        "Can also specify 'stdin' and pass the states to reload from through stdin.\n",
        progname);
}

static bool
check_can_overwrite(bool overwrite, const char * path, const char * path_desc) {
    if (overwrite || !tlo::file_ops::exists(path)) {
        return true;
    }
    (void)printf("%s file %s already exists. Delete? [y/n]\n", path_desc, path);
    int c;
    do {
        c = getchar();
    } while (c == '\n' || c == ' ' || c == '\t');
    return c == 'y';
}

int
main(int argc, char ** argv) {
    // NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)
    TLO_DISABLE_WREDUNDANT_TAGS
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays,hicpp-avoid-c-arrays,modernize-avoid-c-arrays)
    struct option cmdline_options[] = {
        { "h", no_argument, nullptr, 0 },
        { "help", no_argument, nullptr, 0 },
        { "p", required_argument, nullptr, 1 },
        { "perf", required_argument, nullptr, 1 },
        { "r", required_argument, nullptr, 2 },
        { "root", required_argument, nullptr, 2 },
        { "m", required_argument, nullptr, 3 },
        { "map", required_argument, nullptr, 3 },
        { "v", no_argument, nullptr, 4 },
        { "vv", no_argument, nullptr, 5 },
        { "vvv", no_argument, nullptr, 6 },
        { "o", required_argument, nullptr, 7 },
        { "out", required_argument, nullptr, 7 },
        { "w", no_argument, nullptr, 8 },
        { "overwrite", no_argument, nullptr, 8 },
        { "silent", no_argument, nullptr, 9 },
        { "save", required_argument, nullptr, 10 },
        { "reload", required_argument, nullptr, 11 },
        { "dot", required_argument, nullptr, 12 },
        { "dot-dso", required_argument, nullptr, 13 },
        { "no-normalize", no_argument, nullptr, 14 },
        { "force-no-normalize", no_argument, nullptr, 15 },
        { "add-scale", required_argument, nullptr, 16 },
        { "use-custom-scale", no_argument, nullptr, 17 },
        { "dump", no_argument, nullptr, 18 },
        { nullptr, 0, nullptr, 0 },
    };
    TLO_REENABLE_WREDUNDANT_TAGS

    // NOLINTBEGIN(bugprone-string-constructor)
    bool                            dump_stats = false;
    std::string_view                perf_file{ "", 0 };
    std::string_view                root_path{ "", 0 };
    std::string_view                info_file{ "", 0 };
    std::string_view                savefile{ "", 0 };
    char *                          reload_infiles = nullptr;
    std::string_view                output_dir{ "", 0 };
    std::string_view                dot_file{ "", 0 };
    std::string_view                dot_dso{ "", 0 };
    tlo::perf::perf_state_scaling_t scaling_todo{};
    // NOLINTEND(bugprone-string-constructor)
    bool overwrite = false;
    for (;;) {
        int opt_index, res;
        // NOLINTNEXTLINE(concurrency-mt-unsafe,cppcoreguidelines-pro-bounds-array-to-pointer-decay,hicpp-no-array-decay)
        res = getopt_long_only(argc, argv, "", cmdline_options, &opt_index);

        if (res == -1) {
            break;
        }
        switch (res) {
            default:
                // Help
            case 0:
                usage(argv[0]);
                return 1;

                // perf
            case 1:
                perf_file = { optarg, strlen(optarg) };
                break;
                // root
            case 2:
                root_path = { optarg, strlen(optarg) };
                break;
                // info
            case 3:
                info_file = { optarg, strlen(optarg) };
                break;
                // verbosity
            case 4:
            case 5:
            case 6:
                tlo::set_verbosity(res - 3);
                break;
                // Output directory
            case 7:
                output_dir = { optarg, strlen(optarg) };
                break;
                // Overwrite files w.o asking
            case 8:
                overwrite = true;
                break;
                // Disable verbosity.
            case 9:
                tlo::set_verbosity(-2);
                break;
                // Set output save file
            case 10:
                savefile = { optarg, strlen(optarg) };
                break;
                // Set reload from save file
            case 11:
                reload_infiles = optarg;
                break;
                // Set CFG dump to DOT file
            case 12:
                dot_file = optarg;
                break;
                // Set DSO to dump as DOT file
            case 13:
                dot_dso = optarg;
                break;
                // Set no scaling
            case 14:
                scaling_todo.set_no_scale();
                break;
                // Set force no scaling
            case 15:
                scaling_todo.set_force_no_scale();
                // Add an custom output scaling factor
                break;
            case 16: {
                char *       end = optarg;
                const double val = std::strtod(optarg, &end);
                if (end == optarg || !scaling_todo.set_add_scale(val)) {
                    TLO_PRINT_USR_ERR(
                        "Unable to convert argument to --add-scale to double: \"%s\"\n",
                        optarg);
                    return 1;
                }
            } break;
                // Use custom scaling factor
            case 17:
                scaling_todo.set_use_local_scale();
                break;
                // Dump stats
            case 18:
                dump_stats = true;
                break;
        }
    }
    // NOLINTEND(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)

    tlo::sym::sym_state_t              ss{};
    tlo::vec_t<tlo::perf::perf_func_t> funcs;
    tlo::vec_t<tlo::perf::perf_edge_t> edges;

    // Ensure output
    if (output_dir.empty() && savefile.empty() && dot_file.empty()) {
        TLO_PRINT_USR_ERR(
            "Must provide an output dir, savefile, or dot_file\n");
        return 1;
    }

    if (reload_infiles == nullptr) {
        // Check root_path valid.
        if (!root_path.empty() && !tlo::file_ops::is_dir(root_path.data())) {
            TLO_PRINT_USR_ERR("Root directory: %s does not exist\n",
                              root_path.data());
            return 1;
        }

        tlo::vec_t<char> perf_path_buf{};
        tlo::vec_t<char> info_path_buf{};
        // If no perf file then try:
        //  - root_path/profile.zst
        //  - root_path/perf.data
        perf_file =
            process_input_files(perf_file, root_path, &perf_path_buf,
                                "profile.zst", "profile.txt", "Perf Events");
        if (perf_file.empty()) {
            return 1;
        }

        if (info_file.empty() && perf_file.ends_with(".data")) {
            info_file = perf_file;
            TLO_PRINT_USR_ERR(
                "Warning: defaulting to \"%s\" as Info Events file\n",
                info_file.data());
        }
        info_file = process_input_files(info_file, root_path, &info_path_buf,
                                        "info-events.zst", "info-events.txt",
                                        "Info Events");
        if (info_file.empty()) {
            return 1;
        }

        // Open user file(s).
        tlo::file_reader_t fr_events, fr_map;
        if (info_file.ends_with(".data")) {
            tlo::preader_t::cmdline_t cmdline;
            if (!tlo::perf::create_perf_info_cmdline(info_file, &cmdline)) {
                TLO_PRINT_USR_ERR("Perf filename too long!\n");
                return 1;  // NOLINT(*magic*)
            }
            fr_map.init(cmdline.data());
        }
        else {
            fr_map.init(info_file.data());
        }

        if (perf_file.ends_with(".data")) {
            tlo::preader_t::cmdline_t cmdline;
            if (!tlo::perf::create_perf_events_cmdline(perf_file, &cmdline)) {
                TLO_PRINT_USR_ERR("Perf filename too long!\n");
                return 1;  // NOLINT(*magic*)
            }
            fr_events.init(cmdline.begin());
        }
        else {
            fr_events.init(perf_file.data());
        }

        if (!fr_events.active()) {
            TLO_PRINT_USR_ERR("Unable to read file: \"%s\"\n",
                              perf_file.data());
            return 1;  // NOLINT(*magic*)
        }

        if (!fr_map.active()) {
            TLO_PRINT_USR_ERR("Unable to read file: \"%s\"\n",
                              info_file.data());
            return 1;  // NOLINT(*magic*)
        }

        // Collect all samples from the file.
        // Set root path for DSOs.
        tlo::sym::dso_t::set_dso_root_path(root_path);

        tlo::perf::perf_stats_t stats{ &ss };
        bool res = tlo::perf::collect_perf_file_info(&fr_map, &stats);
        if (!res || !stats.valid()) {
            if (dump_stats) {
                stats.dump();
            }
            TLO_PRINT_USR_ERR("Error collecting stats from perf file: \"%s\"\n",
                              info_file.data());
            return 1;  // NOLINT(*magic*)
        }
        fr_map.cleanup();

        res = tlo::perf::collect_perf_file_events(&fr_events, &stats);
        if (!res || !stats.valid()) {
            if (dump_stats) {
                stats.dump();
            }
            TLO_PRINT_USR_ERR("Error collecting stats from perf file: \"%s\"\n",
                              perf_file.data());
            return 1;  // NOLINT(*magic*)
        }
        fr_events.cleanup();

        // Collect function / call stats.
        stats.filter_and_clump(tlo::perf::perf_stats_func_filter_t{},
                               tlo::perf::perf_stats_edge_filter_t{},
                               tlo::perf::perf_stats_function_order_clumper_t{},
                               &funcs, &edges);
    }
    else {
        if (!root_path.empty() || !info_file.empty() || !perf_file.empty()) {
            TLO_PRINT_USR_ERR(
                "Warning: Ignoring root/perf/info arguments are only using saved-stats\n");
        }
        std::span<char> stdin_buf{};
        if (std::strncmp(reload_infiles, "stdin", strlen("stdin")) == 0) {
            size_t off = 0;
            // NOLINTNEXTLINE(*magic*)
            stdin_buf = { tlo::arr_alloc<char>(4096), 4096U };
            for (;;) {
                const ssize_t nread = read(STDIN_FILENO, stdin_buf.data() + off,
                                           stdin_buf.size() - 1U - off);
                if (nread <= 0) {
                    if (errno == EAGAIN) {
                        continue;
                    }
                    break;
                }
                off += static_cast<size_t>(nread);
                assert(off < stdin_buf.size());
                stdin_buf.data()[off] = '\0';
                if (off == (stdin_buf.size() - 1U)) {
                    stdin_buf = { tlo::arr_realloc(stdin_buf.data(),
                                                   stdin_buf.size(),
                                                   stdin_buf.size() * 2U),
                                  stdin_buf.size() * 2U };
                }
            }

            reload_infiles = stdin_buf.data();
        }

        char * end = reload_infiles + std::strlen(reload_infiles);
        tlo::vec_t<std::string_view> reload_infile_paths{};
        for (;;) {
            char * next = reinterpret_cast<char *>(
                std::memchr(reload_infiles, ',',
                            static_cast<uintptr_t>(end - reload_infiles)));
            if (next == nullptr) {
                break;
            }
            reload_infile_paths.emplace_back(
                reload_infiles, static_cast<uintptr_t>(next - reload_infiles));
            *next          = '\0';
            reload_infiles = next + 1;
        }
        reload_infile_paths.emplace_back(
            reload_infiles, static_cast<uintptr_t>(end - reload_infiles));
        TLO_printv("Processing %zu input states\n", reload_infile_paths.size());
        const tlo::perf::perf_state_reloader_t reloader{ &ss };

        if (!reloader.reload_state(&reload_infile_paths, &funcs, &edges,
                                   &scaling_todo)) {
            TLO_PRINT_USR_ERR("Unable to reload state from reloads\n");
            return 1;  // NOLINT(*magic*)
        }

        if (!stdin_buf.empty()) {
            tlo::arr_free(stdin_buf.data(), stdin_buf.size());
        }
    }

    if (funcs.empty() || edges.empty()) {
        TLO_PRINT_USR_ERR(
            "No functions(%d) or edges(%d) from profile... Something almost certainly went wrong\n",
            funcs.empty(), edges.empty());
        return 1;  // NOLINT(*magic*)
    }

    tlo::global_stats_dump(0);
    int no_outdir_ret = 0;
    if (!savefile.empty()) {
        const tlo::perf::perf_state_saver_t saver{ &ss };
        if (check_can_overwrite(overwrite, savefile.data(), "Save")) {
            if (!saver.save_state(savefile.data(), &funcs, &edges,
                                  &scaling_todo)) {
                TLO_PRINT_USR_ERR("Error saving state to: \"%s\"\n",
                                  savefile.data());
                no_outdir_ret = 1;
            }
        }
        else {
            no_outdir_ret = 1;
        }
    }

    if (output_dir.empty() && dot_file.empty()) {
        if (no_outdir_ret != 0) {
            TLO_PRINT_USR_ERR(
                "No output directory and saving to save failed\n");
            return no_outdir_ret;
        }
        return 0;
    }


    if (!output_dir.empty() || !dot_file.empty()) {
        tlo::cfg_t::cfg_prepare(&funcs, &edges);
        // Use collected functions / edges to build CFG.
        const tlo::cfg_t cg(funcs, edges);
        if (!cg.valid()) {
            TLO_PRINT_USR_ERR("Error constructing cfg\n");
            return 1;  // NOLINT(*magic*)
        }

        if (!dot_file.empty()) {
            const tlo::sym::dso_t * dso_to_dump = nullptr;
            if (!dot_dso.empty()) {
                for (const tlo::sym::dso_t * dso : ss.dsos()) {
                    if (dso->filename() == dot_dso) {
                        dso_to_dump = dso;
                        break;
                    }
                }
                if (dso_to_dump == nullptr) {
                    TLO_print("Unable to find dot dso: %s\n", dot_dso.data());
                }
            }

            if (check_can_overwrite(overwrite, dot_file.data(), "DOT")) {
                // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
                FILE * fp = fopen(dot_file.data(), "w+");
                if (fp == nullptr) {
                    TLO_PRINT_USR_ERR("Error opening DOT file: %s\n",
                                      dot_file.data());
                }
                else {
                    cg.dump_dot(fp, dso_to_dump);
                    // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
                    (void)fclose(fp);
                }
            }
        }

        if (!output_dir.empty()) {
            // Order functions in the CFG.
            tlo::vec_t<tlo::cfg_func_order_info_t> ordered_funcs;
            cg.order_nodes(tlo::cfg_t::order_algorithm::k_hfsort_c3,
                           &ordered_funcs);
            // Write result out.
            if (!write_all(output_dir, ss.dsos(), ordered_funcs, overwrite)) {
                TLO_PRINT_USR_ERR("No DSOs wrote out succesfully\n");
                return 1;  // NOLINT(*magic*)
            }
        }
    }
    return 0;
}
