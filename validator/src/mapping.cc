#include <emscripten/emscripten.h>
#include "validator.hh"
#include <string>
#include <cstring>
#include <cstdlib>

using namespace tvv;

extern "C" {

EMSCRIPTEN_KEEPALIVE
char* validate_json(
  const char* instance_json,
  const char* submission_json,
  int verbose,
  int* out_len
) {
  Result r = validate(
    instance_json ? std::string(instance_json) : std::string(),
    submission_json ? std::string(submission_json) : std::string(),
    verbose != 0
  );

  std::string result_str = to_json(r);
  
  char* buffer = (char*)std::malloc(result_str.size());
  if (!buffer) {
    *out_len = 0;
    return nullptr;
  }

  std::memcpy(buffer, result_str.data(), result_str.size());

  *out_len = (int)result_str.size();

  return buffer;
}

EMSCRIPTEN_KEEPALIVE
void free_buffer(char* p) {
  if (p) std::free(p);
}

} // extern "C"
