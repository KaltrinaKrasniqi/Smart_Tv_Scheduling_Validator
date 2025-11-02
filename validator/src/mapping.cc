#include <emscripten/emscripten.h>
#include "validator.hh"
#include <string>
#include <cstring>
#include <cstdlib>

using namespace tvv;

extern "C" {

EMSCRIPTEN_KEEPALIVE
const char* validate_json(const char* instance_json,
                          const char* submission_json,
                          int verbose) {
  Result r = validate(
      instance_json ? std::string(instance_json) : std::string(),
      submission_json ? std::string(submission_json) : std::string(),
      verbose != 0);

  std::string result_str = to_json(r);


  char* buffer = (char*)std::malloc(result_str.size() + 1);
  if (!buffer) {
  
    const char* OOM = "{\"status\":\"ERROR\",\"message\":\"out_of_memory_allocating_result\"}";
    char* oom = (char*)std::malloc(std::strlen(OOM)+1);
    if (!oom) return nullptr;
    std::memcpy(oom, OOM, std::strlen(OOM)+1);
    return oom;
  }

  std::memcpy(buffer, result_str.c_str(), result_str.size() + 1);
  return buffer;
}

EMSCRIPTEN_KEEPALIVE
void free_buffer(const char* p) {
  if (p) std::free((void*)p);
}

} // extern "C"
