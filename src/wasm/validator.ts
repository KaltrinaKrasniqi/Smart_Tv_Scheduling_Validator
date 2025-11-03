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

export async function validateWithWasm(instanceText: string, submissionText: string, verbose: boolean) {
  const { mod, validate_ptr, free_buffer } = await initValidator();
  const ptr: number = validate_ptr(instanceText, submissionText, verbose ? 1 : 0);
  if (!ptr) throw new Error("native returned null pointer");
  const json = mod.UTF8ToString(ptr);
  free_buffer(ptr);
  return JSON.parse(json);
}