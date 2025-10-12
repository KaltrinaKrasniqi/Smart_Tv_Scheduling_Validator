import { Info } from "lucide-react";
import { Button } from "@/components/ui/button";
import {
  Dialog,
  DialogContent,
  DialogDescription,
  DialogHeader,
  DialogTitle,
  DialogTrigger,
} from "@/components/ui/dialog";
import validatorLogo from "@/assets/validator-logo.png";

export const Header = () => {
  return (
    <header className="bg-header text-white py-6 px-4 shadow-lg">
      <div className="container mx-auto max-w-7xl">
        <div className="flex items-center justify-between">
          <div className="flex items-center gap-4">
            <img 
              src={validatorLogo} 
              alt="Smart TV Validator Logo" 
              className="w-12 h-12 object-contain"
            />
            <div>
              <h1 className="text-3xl font-bold tracking-tight">
                Smart TV Scheduling Validator
              </h1>
              <p className="text-white/80 text-sm mt-1">
                C++ validation backend | version 1.0.0
              </p>
            </div>
          </div>
          
          <Dialog>
            <DialogTrigger asChild>
              <Button variant="ghost" size="icon" className="text-white hover:bg-white/10">
                <Info className="h-5 w-5" />
              </Button>
            </DialogTrigger>
            <DialogContent className="max-w-2xl">
              <DialogHeader>
                <DialogTitle>About the Validator</DialogTitle>
                <DialogDescription asChild>
                  <div className="space-y-4 text-foreground">
                    <p>
                      The Smart TV Scheduling Validator is a professional tool for validating 
                      program scheduling solutions against competition constraints.
                    </p>
                    <div className="space-y-2">
                      <h4 className="font-semibold">Key Parameters:</h4>
                      <ul className="list-disc list-inside space-y-1 text-sm">
                        <li><strong>D</strong> - Base score per program</li>
                        <li><strong>R</strong> - Time preference bonus multiplier</li>
                        <li><strong>S</strong> - Channel switch penalty</li>
                        <li><strong>T</strong> - Early/Late time slot penalty</li>
                        <li><strong>O</strong> - Early threshold (minutes)</li>
                        <li><strong>E</strong> - Late threshold (minutes)</li>
                      </ul>
                    </div>
                    <p className="text-sm">
                      Upload an instance JSON file (problem definition) and a submission JSON 
                      file (your solution) to validate the scheduling and calculate the score.
                    </p>
                  </div>
                </DialogDescription>
              </DialogHeader>
            </DialogContent>
          </Dialog>
        </div>
      </div>
    </header>
  );
};
