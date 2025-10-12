import { Download, FileJson, FileSpreadsheet } from "lucide-react";
import { Button } from "@/components/ui/button";
import { Card } from "@/components/ui/card";
import { ValidationResult } from "./ValidationResults";
import { toast } from "sonner";

interface ExportButtonsProps {
  result: ValidationResult;
}

export const ExportButtons = ({ result }: ExportButtonsProps) => {
  const downloadJSON = () => {
    const dataStr = JSON.stringify(result, null, 2);
    const dataBlob = new Blob([dataStr], { type: "application/json" });
    const url = URL.createObjectURL(dataBlob);
    const link = document.createElement("a");
    link.href = url;
    link.download = `validation-report-${Date.now()}.json`;
    link.click();
    URL.revokeObjectURL(url);
    toast.success("JSON report downloaded");
  };

  const downloadCSV = () => {
    if (!result.score) {
      toast.error("No score data to export");
      return;
    }

    const csvRows = [
      ["Component", "Value", "Details"],
      ["Base Program Scores", result.score.base, "Sum of program scores"],
      ["Time Preference Bonuses", result.score.bonuses, "Optimal time bonuses"],
      [
        "Channel Switch Penalties",
        result.score.switches.total,
        `${result.score.switches.count} switches × S(${result.score.switches.S})`,
      ],
      [
        "Early/Late Penalties",
        result.score.early_late.total,
        `T(${result.score.early_late.T}) × (${result.score.early_late.early} early + ${result.score.early_late.late} late)`,
      ],
      ["Total Score", result.score.total, ""],
    ];

    const csvContent = csvRows.map((row) => row.join(",")).join("\n");
    const blob = new Blob([csvContent], { type: "text/csv" });
    const url = URL.createObjectURL(blob);
    const link = document.createElement("a");
    link.href = url;
    link.download = `validation-report-${Date.now()}.csv`;
    link.click();
    URL.revokeObjectURL(url);
    toast.success("CSV report downloaded");
  };

  return (
    <Card className="p-6">
      <h3 className="text-lg font-semibold mb-4">Export Results</h3>
      <div className="flex gap-3">
        <Button onClick={downloadJSON} variant="outline" className="flex-1">
          <FileJson className="mr-2 h-4 w-4" />
          Download JSON
        </Button>
        <Button onClick={downloadCSV} variant="outline" className="flex-1">
          <FileSpreadsheet className="mr-2 h-4 w-4" />
          Export CSV
        </Button>
      </div>
    </Card>
  );
};
