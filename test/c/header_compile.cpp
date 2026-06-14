#include <ruckig_c/ruckig.h>

int main() {
    ruckig_t* otg = 0;
    ruckig_diagnostics_t diagnostics;
    ruckig_diagnostics_init(&diagnostics);
    return ruckig_create(&otg, 1, 0.001) == RUCKIG_WORKING
        && diagnostics.code == RUCKIG_DIAGNOSTIC_NONE
        ? (ruckig_destroy(otg), 0)
        : 1;
}
