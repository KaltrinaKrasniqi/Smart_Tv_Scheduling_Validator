let mod: any;

export async function initWasm() {
  if (mod) return mod;
  
  const factory = (await import("public/wasm/validator.js")).default;
  mod = await factory({ locateFile: (p: string) => `/wasm/${p}` });
  return mod;
}

export async function validateWithWasm(instanceStr: string, submissionStr: string, verbose: boolean) {
  const m = await initWasm();


  const validate_ptr = m.cwrap("validate_json", "number", ["string", "string", "number"]);
  const free_buffer  = m.cwrap("free_buffer", "void", ["number"]);


  const ptr: number = validate_ptr(instanceStr, submissionStr, verbose ? 1 : 0);
  if (!ptr) throw new Error("native returned null pointer");


  const jsonStr: string = m.UTF8ToString(ptr);
  free_buffer(ptr);

  return JSON.parse(jsonStr);
}



