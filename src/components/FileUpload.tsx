import { useState, useCallback } from "react";
import { Upload, FileJson, CheckCircle2 } from "lucide-react";
import { Button } from "@/components/ui/button";
import { Checkbox } from "@/components/ui/checkbox";
import { Label } from "@/components/ui/label";
import { Card } from "@/components/ui/card";

interface FileUploadProps {
  onValidate: (instanceFile: File, submissionFile: File, verbose: boolean) => void;
  onLoadSample: () => void;
  isLoading: boolean;
}

export const FileUpload = ({ onValidate, onLoadSample, isLoading }: FileUploadProps) => {
  const [instanceFile, setInstanceFile] = useState<File | null>(null);
  const [submissionFile, setSubmissionFile] = useState<File | null>(null);
  const [verbose, setVerbose] = useState(false);
  const [dragActive, setDragActive] = useState<"instance" | "submission" | null>(null);

  const handleDrag = useCallback((e: React.DragEvent, type: "instance" | "submission") => {
    e.preventDefault();
    e.stopPropagation();
    if (e.type === "dragenter" || e.type === "dragover") {
      setDragActive(type);
    } else if (e.type === "dragleave") {
      setDragActive(null);
    }
  }, []);

  const handleDrop = useCallback((e: React.DragEvent, type: "instance" | "submission") => {
    e.preventDefault();
    e.stopPropagation();
    setDragActive(null);
    
    if (e.dataTransfer.files && e.dataTransfer.files[0]) {
      const file = e.dataTransfer.files[0];
      if (file.type === "application/json" || file.name.endsWith(".json")) {
        if (type === "instance") {
          setInstanceFile(file);
        } else {
          setSubmissionFile(file);
        }
      }
    }
  }, []);

  const handleFileChange = (e: React.ChangeEvent<HTMLInputElement>, type: "instance" | "submission") => {
    if (e.target.files && e.target.files[0]) {
      if (type === "instance") {
        setInstanceFile(e.target.files[0]);
      } else {
        setSubmissionFile(e.target.files[0]);
      }
    }
  };

  const handleValidate = () => {
    if (instanceFile && submissionFile) {
      onValidate(instanceFile, submissionFile, verbose);
    }
  };

  const FileDropZone = ({ 
    type, 
    file, 
    onChange 
  }: { 
    type: "instance" | "submission"; 
    file: File | null;
    onChange: (e: React.ChangeEvent<HTMLInputElement>) => void;
  }) => (
    <div
      className={`
        relative border-2 border-dashed rounded-lg p-8 text-center transition-all
        ${dragActive === type ? "border-primary bg-primary/5" : "border-border hover:border-primary/50"}
        ${file ? "bg-success/5 border-success" : ""}
      `}
      onDragEnter={(e) => handleDrag(e, type)}
      onDragLeave={(e) => handleDrag(e, type)}
      onDragOver={(e) => handleDrag(e, type)}
      onDrop={(e) => handleDrop(e, type)}
    >
      <input
        type="file"
        id={`${type}-file`}
        accept=".json"
        onChange={onChange}
        className="absolute inset-0 w-full h-full opacity-0 cursor-pointer"
      />
      <div className="flex flex-col items-center gap-3">
        {file ? (
          <>
            <CheckCircle2 className="h-12 w-12 text-success" />
            <p className="font-medium text-foreground">{file.name}</p>
            <p className="text-sm text-muted-foreground">
              {(file.size / 1024).toFixed(2)} KB
            </p>
          </>
        ) : (
          <>
            <FileJson className="h-12 w-12 text-muted-foreground" />
            <div>
              <p className="font-medium text-foreground">
                {type === "instance" ? "Instance file" : "Submission file"}
              </p>
              <p className="text-sm text-muted-foreground mt-1">
                Drop JSON file or click to browse
              </p>
            </div>
          </>
        )}
      </div>
    </div>
  );

  return (
    <Card className="p-6">
      <h2 className="text-2xl font-semibold mb-6">Upload Files</h2>
      
      <div className="grid md:grid-cols-2 gap-6 mb-6">
        <div>
          <Label htmlFor="instance-file" className="text-base mb-3 block">
            Instance File (Problem Definition)
          </Label>
          <FileDropZone
            type="instance"
            file={instanceFile}
            onChange={(e) => handleFileChange(e, "instance")}
          />
        </div>
        
        <div>
          <Label htmlFor="submission-file" className="text-base mb-3 block">
            Submission File (Solution)
          </Label>
          <FileDropZone
            type="submission"
            file={submissionFile}
            onChange={(e) => handleFileChange(e, "submission")}
          />
        </div>
      </div>

      <div className="flex items-center space-x-2 mb-6">
        <Checkbox 
          id="verbose" 
          checked={verbose}
          onCheckedChange={(checked) => setVerbose(checked as boolean)}
        />
        <Label
          htmlFor="verbose"
          className="text-sm font-medium leading-none peer-disabled:cursor-not-allowed peer-disabled:opacity-70 cursor-pointer"
        >
          Enable verbose mode (detailed logs)
        </Label>
      </div>

      <div className="flex gap-3">
        <Button
          onClick={handleValidate}
          disabled={!instanceFile || !submissionFile || isLoading}
          className="flex-1"
          size="lg"
        >
          <Upload className="mr-2 h-4 w-4" />
          {isLoading ? "Validating..." : "Validate"}
        </Button>
        
        <Button
          onClick={onLoadSample}
          variant="outline"
          disabled={isLoading}
          size="lg"
        >
          Use Sample Files
        </Button>
      </div>
    </Card>
  );
};
