import { useState } from "react";
import { Header } from "@/components/Header";
import { FileUpload } from "@/components/FileUpload";
import { ValidationResults, ValidationResult } from "@/components/ValidationResults";
import { toast } from "sonner";

const Index = () => {
  const [validationResult, setValidationResult] = useState<ValidationResult | null>(null);
  const [isLoading, setIsLoading] = useState(false);

  const handleValidate = async (
    instanceFile: File,
    submissionFile: File,
    verbose: boolean
  ) => {
    setIsLoading(true);
    
    try {
      // Read file contents
      const instanceText = await instanceFile.text();
      const submissionText = await submissionFile.text();
      
      // Parse JSON to validate
      const instanceData = JSON.parse(instanceText);
      const submissionData = JSON.parse(submissionText);

      // Create FormData for backend
      const formData = new FormData();
      formData.append("instance", instanceFile);
      formData.append("submission", submissionFile);
      formData.append("verbose", verbose.toString());

      // TODO: Replace with actual backend endpoint
      // const response = await fetch("/api/validate", {
      //   method: "POST",
      //   body: formData,
      // });
      // const result = await response.json();

      // Mock response for demonstration
      await new Promise((resolve) => setTimeout(resolve, 1500));
      
      const mockResult: ValidationResult = {
        status: "VALID",
        score: {
          total: 380,
          base: 300,
          bonuses: 90,
          switches: { count: 2, S: 5, total: -10 },
          early_late: { early: 0, late: 0, T: 10, total: 0 },
        },
        violations: [],
        verbose: verbose ? [
          "Loading instance file...",
          "Validating channel availability...",
          "Processing program schedules...",
          "Bonus activated for program n1: +30 points",
          "Bonus activated for program n3: +60 points",
          "Channel switch detected @ 840: Ch0 → Ch1 (-5 points)",
          "Channel switch detected @ 1020: Ch1 → Ch0 (-5 points)",
          "No early/late penalties detected",
          "All priority blocks satisfied",
          "Validation complete: VALID",
        ] : undefined,
        timeline: [
          { program_id: "n1", channel_id: 0, genre: "News", start: 540, end: 600 },
          { program_id: "s1", channel_id: 0, genre: "Sports", start: 600, end: 720 },
          { program_id: "m1", channel_id: 1, genre: "Music", start: 840, end: 900 },
          { program_id: "d1", channel_id: 1, genre: "Documentary", start: 900, end: 1020 },
          { program_id: "mov1", channel_id: 0, genre: "Movies", start: 1020, end: 1200 },
        ],
        validator_version: "1.0.0",
        elapsed_ms: 42,
      };

      setValidationResult(mockResult);
      toast.success("Validation complete!");
    } catch (error) {
      console.error("Validation error:", error);
      const errorResult: ValidationResult = {
        status: "ERROR",
        error_message: error instanceof Error ? error.message : "Failed to validate files",
        validator_version: "1.0.0",
        elapsed_ms: 0,
      };
      setValidationResult(errorResult);
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
