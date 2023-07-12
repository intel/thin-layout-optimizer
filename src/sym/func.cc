#include "src/util/algo.h"

#include "src/sym/dso.h"
#include "src/sym/func.h"

#include "src/util/vec.h"
#include "src/util/verbosity.h"

#include <string_view>

#include <stddef.h>
#include <stdio.h>
namespace tlo {
namespace sym {

std::string_view
func_t::label(vec_t<char> * buf, std::string_view postfix) const {
    auto write_to_buf = [buf](char const * s, size_t slen) {
        std::copy(s, s + slen, std::back_inserter(*buf));
    };


    if (dso() != nullptr) {
        const std::string_view dso_filename = dso()->filename();
        write_to_buf(dso_filename.data(), dso_filename.length());
    }
    write_to_buf("::", 2);
    write_to_buf(name_.str(), name_.len());
    const std::string_view ident_prefix = ident_.prefix();
    write_to_buf(ident_prefix.data(), ident_prefix.length());
    write_to_buf(ident_.str(), ident_.len());
    if (!postfix.empty()) {
        write_to_buf(postfix.data(), postfix.length());
    }
    buf->emplace_back('\0');
    assert(!buf->empty() && (*buf)[buf->size() - 1] == '\0');
    return { buf->data(), buf->size() - 1 };
}

void
func_t::dump(int vlvl, FILE * fp, const char * prefix) const {
    if (!has_verbosity(vlvl)) {
        return;
    }
    vec_t<char> chars{};
    label(&chars);

    (void)fprintf(fp, "%s%s (%d: %lx, %lx)\n", prefix, chars.data(),
                  has_elfinfo() ? 1 : 0, start_addr(), end_addr());
}
}  // namespace sym
}  // namespace tlo
