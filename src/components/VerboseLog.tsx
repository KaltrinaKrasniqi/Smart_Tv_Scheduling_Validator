import { useState } from "react";
import { Card } from "@/components/ui/card";
import { Button } from "@/components/ui/button";
import { ChevronDown, ChevronUp, Terminal } from "lucide-react";

interface VerboseLogProps {
  logs: string[];
}

export const VerboseLog = ({ logs }: { logs: string[] }) => {
  return (
    <Card className="p-6">
      <h3 className="text-xl font-semibold mb-4">Verbose Log ({logs.length} entries)</h3>
      <ol className="space-y-1 font-mono text-sm">
        {logs.map((line, i) => (
          <li key={i} className="text-muted-foreground">
            <span className="mr-2 text-foreground">{i + 1}.</span>
            {line}
          </li>
        ))}
      </ol>
    </Card>
  );
};