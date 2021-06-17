#include <stdint.h>

#define EXPORT  __attribute__ ((visibility ("default")))

void* doInitialize_sim();

// =================================
//       PHARO API WRAPPERS
// =================================

// EXPORT void* initialize_sim(){
//     return doInitialize_sim();
// }

// uint32_t read_register(void* sim, int nb) {
// //    sim_t* real_sim = (sim_t*) sim;
// //    return real_sim->get_core(0)->get_state()->XPR[nb];
// }

// void write_register(void* sim, int nb, uint32_t value) {
// //    sim_t* real_sim = (sim_t*) sim;
// //    real_sim->get_core(0)->get_state()->XPR.write(nb, value);
// }