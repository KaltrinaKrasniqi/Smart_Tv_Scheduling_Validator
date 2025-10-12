import { CheckCircle2, AlertTriangle, XCircle, Clock, Code } from "lucide-react";
import { Card } from "@/components/ui/card";
import { Badge } from "@/components/ui/badge";
import {
  Table,
  TableBody,
  TableCell,
  TableHead,
  TableHeader,
  TableRow,
} from "@/components/ui/table";
import { ScoreBreakdown } from "./ScoreBreakdown";
import { ViolationsList } from "./ViolationsList";
import { TimelineVisualization } from "./TimelineVisualization";
import { VerboseLog } from "./VerboseLog";
import { ExportButtons } from "./ExportButtons";

export interface ValidationResult {
  status: "VALID" | "INVALID" | "ERROR";
  score?: {
    total: number;
    base: number;
    bonuses: number;
    switches: { count: number; S: number; total: number };
    early_late: { early: number; late: number; T: number; total: number };
  };
  violations?: Array<{
    type: string;
    message: string;
    time?: number;
    channel?: number;
  }>;
  verbose?: string[];
  timeline?: Array<{
    program_id: string;
    channel_id: number;
    genre: string;
    start: number;
    end: number;
  }>;
  validator_version?: string;
  elapsed_ms?: number;
  error_message?: string;
}

interface ValidationResultsProps {
  result: ValidationResult;
}

export const ValidationResults = ({ result }: ValidationResultsProps) => {
  const getStatusConfig = (status: string) => {
    switch (status) {
      case "VALID":
        return {
          icon: CheckCircle2,
          color: "success",
          label: "VALID",
          className: "bg-success/10 text-success border-success",
        };
      case "INVALID":
        return {
          icon: AlertTriangle,
          color: "warning",
          label: "INVALID",
          className: "bg-warning/10 text-warning border-warning",
        };
      case "ERROR":
        return {
          icon: XCircle,
          color: "error",
          label: "ERROR",
          className: "bg-error/10 text-error border-error",
        };
      default:
        return {
          icon: AlertTriangle,
          color: "muted",
          label: "UNKNOWN",
          className: "bg-muted text-muted-foreground",
        };
    }
  };

  const statusConfig = getStatusConfig(result.status);
  const StatusIcon = statusConfig.icon;

  return (
    <div className="space-y-6 animate-in fade-in slide-in-from-bottom-4 duration-500">
      <Card className="p-6">
        <div className="flex items-start justify-between mb-6">
          <div className="flex items-center gap-4">
            <div className={`p-3 rounded-lg ${statusConfig.className}`}>
              <StatusIcon className="h-8 w-8" />
            </div>
            <div>
              <h2 className="text-2xl font-semibold">Validation Results</h2>
              <div className="flex items-center gap-4 mt-2 text-sm text-muted-foreground">
                {result.elapsed_ms && (
                  <span className="flex items-center gap-1">
                    <Clock className="h-4 w-4" />
                    {result.elapsed_ms}ms
                  </span>
                )}
                {result.validator_version && (
                  <span className="flex items-center gap-1">
                    <Code className="h-4 w-4" />
                    v{result.validator_version}
                  </span>
                )}
              </div>
            </div>
          </div>
          <Badge variant="outline" className={statusConfig.className}>
            {statusConfig.label}
          </Badge>
        </div>

        {result.status === "ERROR" && result.error_message && (
          <div className="bg-error/10 border border-error rounded-lg p-4">
            <p className="text-error font-medium">Error:</p>
            <p className="text-foreground mt-1">{result.error_message}</p>
          </div>
        )}

        {result.score && <ScoreBreakdown score={result.score} />}
      </Card>

      {result.violations && result.violations.length > 0 && (
        <ViolationsList violations={result.violations} />
      )}

      {result.timeline && result.timeline.length > 0 && (
        <TimelineVisualization timeline={result.timeline} />
      )}

      {result.verbose && result.verbose.length > 0 && (
        <VerboseLog logs={result.verbose} />
      )}

      <ExportButtons result={result} />
    </div>
  );
};
