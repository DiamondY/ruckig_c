#include <ruckig_c/ruckig.h>

int main() {
    ruckig_t* otg = 0;
    return ruckig_create(&otg, 1, 0.001) == RUCKIG_WORKING ? (ruckig_destroy(otg), 0) : 1;
}
