#include <stdint.h>

extern uint8_t* test_rust();

bool
test_rust_wrapper() {
  return test_rust();
}
