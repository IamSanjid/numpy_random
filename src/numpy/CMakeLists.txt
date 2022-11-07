add_library(numpy STATIC
    "npy_common.h"
    "npy_math.h"
    "numpyconfig.h"
    "_numpyconfig.h"
    "random/bitgen.h"
    "random/distributions.h"
    "random/distributions/distributions.c"
    "random/distributions/logfactorial.c"
    "random/distributions/logfactorial.h"
    "random/distributions/random_hypergeometric.c"
    "random/distributions/random_mvhg_count.c"
    "random/distributions/random_mvhg_marginals.c"
    "random/distributions/ziggurat_constants.h"
    "random/legacy/legacy-distributions.c"
    "random/legacy/legacy-distributions.h"
)

if (MSVC)
    target_compile_options(numpy PRIVATE
        /wd4242
        /wd4244
        /wd4245
        /wd4254
        /wd4800
    )
else()
    target_compile_options(numpy PRIVATE
        -Wno-conversion

        -Wno-sign-conversion

        $<$<CXX_COMPILER_ID:Clang>:-fsized-deallocation>
    )
endif()

create_target_directory_groups(numpy)