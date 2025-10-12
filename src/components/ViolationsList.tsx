import { AlertTriangle } from "lucide-react";
import { Card } from "@/components/ui/card";
import { Alert, AlertDescription } from "@/components/ui/alert";

interface ViolationsListProps {
  violations: Array<{
    type: string;
    message: string;
    time?: number;
    channel?: number;
  }>;
}

export const ViolationsList = ({ violations }: ViolationsListProps) => {
  return (
    <Card className="p-6">
      <div className="flex items-center gap-2 mb-4">
        <AlertTriangle className="h-5 w-5 text-warning" />
        <h3 className="text-lg font-semibold">
          Violations ({violations.length})
        </h3>
      </div>
      <div className="space-y-3">
        {violations.map((violation, index) => (
          <Alert key={index} variant="destructive" className="border-warning bg-warning/5">
            <AlertDescription className="flex items-start gap-3">
              <AlertTriangle className="h-4 w-4 mt-0.5 shrink-0" />
              <div className="flex-1">
                <p className="font-medium text-warning">{violation.type}</p>
                <p className="text-sm text-foreground mt-1">{violation.message}</p>
                {(violation.time !== undefined || violation.channel !== undefined) && (
                  <div className="flex gap-4 mt-2 text-xs text-muted-foreground">
                    {violation.time !== undefined && (
                      <span>Time: {violation.time}</span>
                    )}
                    {violation.channel !== undefined && (
                      <span>Channel: {violation.channel}</span>
                    )}
                  </div>
                )}
              </div>
            </AlertDescription>
          </Alert>
        ))}
      </div>
    </Card>
  );
};
