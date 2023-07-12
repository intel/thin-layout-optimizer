#include <assert.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/random.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
int
main(int argc, char ** argv) {
    assert(
        argc > 4 &&
        "Usage: prog <out-dir> <max_line_size> <num_lines> <compressability:0-20>");

    const char * outdir    = argv[1];
    uint32_t     ln_sz     = atoi(argv[2]);
    uint32_t     num_lines = atoi(argv[3]);
    uint32_t     cscore    = atoi(argv[4]);

    assert(ln_sz && num_lines && cscore && "Invalid arguments");
    assert(cscore < 20 && "Invalid cscore");
    uint32_t * sizes = (uint32_t *)malloc(num_lines * sizeof(uint32_t));
    uint8_t *  buf   = (uint8_t *)malloc(ln_sz);
    assert(sizes && buf && "Bad Alloc");

    assert(getrandom(sizes, num_lines * sizeof(uint32_t), 0) ==
           num_lines * sizeof(uint32_t));

    uint32_t cstore_mod = 256 / cscore;

    char out_path[512];
    int  res = snprintf(out_path, 512, "%s/generated-test-compressed-reader-input-c%u",
                        outdir, cscore);
    assert(res > 0 && res < 512);

    int fd = open(out_path, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    assert(fd > 0 && "Unable to create output file");

    for (uint32_t i = 0; i < num_lines; ++i) {
        uint32_t sz = sizes[i] % ln_sz;
        if (sz == 0) {
            sz = 1;
        }

        assert(getrandom(buf, sz, 0) == sz);
        for (uint32_t j = 0; j < sz; ++j) {
            buf[j] %= cstore_mod;
        }

        assert(write(fd, buf, sz) == sz);
    }
    close(fd);
}
