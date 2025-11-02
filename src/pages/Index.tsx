import { useState } from "react";
import { Header } from "@/components/Header";
import { FileUpload } from "@/components/FileUpload";
import { ValidationResults, ValidationResult } from "@/components/ValidationResults";
import { toast } from "sonner";
import { validateWithWasm } from "@/wasm/validator";


const Index = () => {
  const [validationResult, setValidationResult] = useState<ValidationResult | null>(null);
  const [isLoading, setIsLoading] = useState(false);

const handleValidate = async (instanceFile: File, submissionFile: File, verbose: boolean) => {
  setIsLoading(true);
  try {
    const [instanceText, submissionText] = await Promise.all([
      instanceFile.text(),
      submissionFile.text(),
    ]);

    const result = await validateWithWasm(instanceText, submissionText, verbose);
    setValidationResult(result);
    toast.success("Validation complete!");
  } catch (e: any) {
    setValidationResult({ status: "ERROR", error_message: e?.message ?? "WASM error" } as any);
    toast.error("Validation failed");
  } finally {
    setIsLoading(false);
  }
};

  const handleLoadSample = async () => {
    setIsLoading(true);
    
    try {
      // Mock loading sample files
      await new Promise((resolve) => setTimeout(resolve, 800));
      
      const sampleResult: ValidationResult = {
        status: "INVALID",
        score: {
          total: 245,
          base: 280,
          bonuses: 40,
          switches: { count: 5, S: 5, total: -25 },
          early_late: { early: 2, late: 3, T: 10, total: -50 },
        },
        violations: [
          {
            type: "PriorityBlockViolation",
            message: "Channel 1 is not allowed during priority block [720-780)",
            time: 780,
            channel: 1,
          },
          {
            type: "OverlapViolation",
            message: "Programs overlap on channel 0: [600-720) and [660-780)",
            time: 660,
            channel: 0,
          },
        ],
        verbose: [
          "Loading sample instance file...",
          "Validating channel availability...",
          "Processing program schedules...",
          "Bonus activated for program n2: +40 points",
          "⚠️ Priority block violation detected @ 780",
          "⚠️ Program overlap detected @ 660 on channel 0",
          "Channel switches: 5 × -5 = -25 points",
          "Early/late penalties: (2 + 3) × -10 = -50 points",
          "Validation complete: INVALID",
        ],
        timeline: [
          { program_id: "n2", channel_id: 0, genre: "News", start: 600, end: 720 },
          { program_id: "s2", channel_id: 1, genre: "Sports", start: 720, end: 840 },
          { program_id: "m2", channel_id: 0, genre: "Music", start: 840, end: 960 },
        ],
        validator_version: "1.0.0",
        elapsed_ms: 38,
      };

      setValidationResult(sampleResult);
      toast.success("Sample files loaded");
    } catch (error) {
      toast.error("Failed to load sample files");
    } finally {
      setIsLoading(false);
    }
  };

  return (
    <div className="min-h-screen flex flex-col">
      <Header />
      
      <main className="flex-1 container mx-auto max-w-7xl px-4 py-8">
        <div className="space-y-8">
          <FileUpload
            onValidate={handleValidate}
            onLoadSample={handleLoadSample}
            isLoading={isLoading}
          />

          {validationResult && <ValidationResults result={validationResult} />}
        </div>
      </main>

      <footer className="border-t py-6 px-4 mt-12">
        <div className="container mx-auto max-w-7xl text-center text-sm text-muted-foreground">
          © 2025 Smart TV Validator | Built with C++ & TypeScript
        </div>
      </footer>
    </div>
  );
};

export default Index;
