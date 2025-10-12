import {
  Table,
  TableBody,
  TableCell,
  TableHead,
  TableHeader,
  TableRow,
} from "@/components/ui/table";

interface ScoreBreakdownProps {
  score: {
    total: number;
    base: number;
    bonuses: number;
    switches: { count: number; S: number; total: number };
    early_late: { early: number; late: number; T: number; total: number };
  };
}

export const ScoreBreakdown = ({ score }: ScoreBreakdownProps) => {
  return (
    <div>
      <h3 className="text-lg font-semibold mb-4">Score Breakdown</h3>
      <Table>
        <TableHeader>
          <TableRow>
            <TableHead className="w-1/2">Component</TableHead>
            <TableHead className="text-right">Value</TableHead>
            <TableHead>Details</TableHead>
          </TableRow>
        </TableHeader>
        <TableBody>
          <TableRow>
            <TableCell className="font-medium">Base Program Scores</TableCell>
            <TableCell className="text-right font-mono">{score.base}</TableCell>
            <TableCell className="text-muted-foreground">Σ program scores</TableCell>
          </TableRow>
          <TableRow>
            <TableCell className="font-medium">Time Preference Bonuses</TableCell>
            <TableCell className="text-right font-mono text-success">+{score.bonuses}</TableCell>
            <TableCell className="text-muted-foreground">Optimal time slot bonuses</TableCell>
          </TableRow>
          <TableRow>
            <TableCell className="font-medium">Channel Switch Penalties</TableCell>
            <TableCell className="text-right font-mono text-error">
              {score.switches.total}
            </TableCell>
            <TableCell className="text-muted-foreground">
              {score.switches.count} switches × S({score.switches.S})
            </TableCell>
          </TableRow>
          <TableRow>
            <TableCell className="font-medium">Early/Late Penalties</TableCell>
            <TableCell className="text-right font-mono text-error">
              {score.early_late.total}
            </TableCell>
            <TableCell className="text-muted-foreground">
              T({score.early_late.T}) × ({score.early_late.early} early + {score.early_late.late} late)
            </TableCell>
          </TableRow>
          <TableRow className="bg-muted/30 font-semibold">
            <TableCell className="text-lg">Total Score</TableCell>
            <TableCell className="text-right text-lg font-mono">{score.total}</TableCell>
            <TableCell></TableCell>
          </TableRow>
        </TableBody>
      </Table>
    </div>
  );
};
