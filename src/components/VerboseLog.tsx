import { useState } from "react";
import { Card } from "@/components/ui/card";
import { Button } from "@/components/ui/button";
import { ChevronDown, ChevronUp, Terminal } from "lucide-react";

interface VerboseLogProps {
  logs: string[];
}

export const VerboseLog = ({ logs }: VerboseLogProps) => {
  const [isExpanded, setIsExpanded] = useState(false);

  return (
    <Card className="p-6">
      <div className="flex items-center justify-between mb-4">
        <div className="flex items-center gap-2">
          <Terminal className="h-5 w-5 text-muted-foreground" />
          <h3 className="text-lg font-semibold">Verbose Log</h3>
          <span className="text-sm text-muted-foreground">({logs.length} entries)</span>
        </div>
        <Button
          variant="ghost"
          size="sm"
          onClick={() => setIsExpanded(!isExpanded)}
        >
          {isExpanded ? (
            <>
              <ChevronUp className="h-4 w-4 mr-1" />
              Collapse
            </>
          ) : (
            <>
              <ChevronDown className="h-4 w-4 mr-1" />
              Expand
            </>
          )}
        </Button>
      </div>

      {isExpanded && (
        <div className="bg-muted/30 rounded-lg p-4 max-h-96 overflow-y-auto">
          <pre className="text-xs font-mono text-foreground whitespace-pre-wrap">
            {logs.map((log, index) => (
              <div key={index} className="py-1 border-b border-border/50 last:border-0">
                <span className="text-muted-foreground mr-3">{index + 1}.</span>
                {log}
              </div>
            ))}
          </pre>
        </div>
      )}
    </Card>
  );
};
