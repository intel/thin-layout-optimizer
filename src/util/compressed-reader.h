#ifndef SRC_D_UTIL_D_COMPRESSED_READER_H_

////////////////////////////////////////////////////////////////////////////////
// Hook for compressed reader (if we don't have zstd, then its an empty reader)
// and we will fail if we try to read a compressed file.

#define SRC_D_UTIL_D_COMPRESSED_READER_H_

#ifdef TLO_ZSTD
# include "src/util/compressed-reader-impl.h"
#else
# include "src/util/empty-reader.h"
namespace tlo {
using creader_t = detail::empty_reader_impl_t<1>;
}
#endif

#endif
