#include <ruckig_c/ruckig.h>

int main(void) {
    ruckig_diagnostics_t diagnostics;
    ruckig_diagnostics_init(&diagnostics);
    return RUCKIG_C_VERSION_MAJOR == 0
        && diagnostics.struct_size == sizeof(diagnostics)
        && RUCKIG_DIAGNOSTIC_SCOPE_INPUT == 1
        && RUCKIG_DIAGNOSTIC_ZERO_LIMIT == 7
        ? 0
        : 1;
}
