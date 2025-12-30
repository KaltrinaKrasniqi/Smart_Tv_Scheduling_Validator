let modulePromise: Promise<any> | null = null;

function loadScriptOnce(src: string): Promise<void> {
  return new Promise((resolve, reject) => {
    if (document.querySelector(`script[data-wasm="${src}"]`)) return resolve();
    const s = document.createElement("script");
    s.src = src;
    s.async = true;
    s.dataset.wasm = src;
    s.onload = () => resolve();
    s.onerror = () => reject(new Error(`Failed to load ${src}`));
    document.head.appendChild(s);
  });
}

export async function initValidator() {
  if (!modulePromise) {
    await loadScriptOnce("/wasm/validator.js");

    const factory = (window as any).createValidatorModule as (opts?: any) => Promise<any>;
    modulePromise = factory({ locateFile: (p: string) => `/wasm/${p}` });
  }
  const mod = await modulePromise;

  const validate_ptr = mod.cwrap("validate_json", "number", ["string", "string", "number"]);
  const free_buffer  = mod.cwrap("free_buffer", "void", ["number"]);

  return { mod, validate_ptr, free_buffer };
}

export async function validateWithWasm(
  instanceText: string,
  submissionText: string,
  verbose: boolean
) {
  const { mod } = await initValidator();

  const validate = mod.cwrap(
    "validate_json",
    "number",
    ["string", "string", "number", "number"]
  );

  const outLenPtr = mod._malloc(4);

  const resultPtr = validate(
    instanceText,
    submissionText,
    verbose ? 1 : 0,
    outLenPtr
  );

  if (!resultPtr) {
    mod._free(outLenPtr);
    throw new Error("WASM: validate_json returned null pointer");
  }

  const len = mod.getValue(outLenPtr, "i32");

  if (len <= 0) {
    mod._free(resultPtr);
    mod._free(outLenPtr);
    throw new Error("WASM: invalid result length");
  }

  const jsonStr = mod.UTF8ToString(resultPtr, len);

  mod._free(resultPtr);
  mod._free(outLenPtr);

  // Parse JSON result
  return JSON.parse(jsonStr);
}
