import { useState } from "react";
import { Card } from "@/components/ui/card";
import { ChevronDown, ChevronUp } from "lucide-react";
import { Button } from "@/components/ui/button";

interface TimelineVisualizationProps {
  timeline: Array<{
    program_id: string;
    channel_id: number;
    genre: string;
    start: number;
    end: number;
  }>;
}

const genreColors: Record<string, string> = {
  news: "bg-genre-news",
  sports: "bg-genre-sports",
  music: "bg-genre-music",
  documentary: "bg-genre-documentary",
  movies: "bg-genre-movies",
};

export const TimelineVisualization = ({ timeline }: TimelineVisualizationProps) => {
  const [isExpanded, setIsExpanded] = useState(false);

  // Group programs by channel
  const channels = timeline.reduce((acc, program) => {
    if (!acc[program.channel_id]) {
      acc[program.channel_id] = [];
    }
    acc[program.channel_id].push(program);
    return acc;
  }, {} as Record<number, typeof timeline>);

  // Find time range
  const minTime = Math.min(...timeline.map((p) => p.start));
  const maxTime = Math.max(...timeline.map((p) => p.end));
  const timeRange = maxTime - minTime;

  const formatTime = (minutes: number) => {
    const hours = Math.floor(minutes / 60);
    const mins = minutes % 60;
    return `${hours.toString().padStart(2, "0")}:${mins.toString().padStart(2, "0")}`;
  };

  return (
    <Card className="p-6">
      <div className="flex items-center justify-between mb-4">
        <h3 className="text-lg font-semibold">Timeline Visualization of the output file</h3>
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
        <div className="space-y-6">
          <div className="flex gap-4 flex-wrap text-xs">
            <div className="flex items-center gap-2">
              <div className="w-4 h-4 rounded bg-genre-news" />
              <span>News</span>
            </div>
            <div className="flex items-center gap-2">
              <div className="w-4 h-4 rounded bg-genre-sports" />
              <span>Sports</span>
            </div>
            <div className="flex items-center gap-2">
              <div className="w-4 h-4 rounded bg-genre-music" />
              <span>Music</span>
            </div>
            <div className="flex items-center gap-2">
              <div className="w-4 h-4 rounded bg-genre-documentary" />
              <span>Documentary</span>
            </div>
            <div className="flex items-center gap-2">
              <div className="w-4 h-4 rounded bg-genre-movies" />
              <span>Movies</span>
            </div>
          </div>

          <div className="space-y-4">
            {Object.entries(channels)
              .sort(([a], [b]) => Number(a) - Number(b))
              .map(([channelId, programs]) => (
                <div key={channelId}>
                  <div className="text-sm font-medium mb-2">Channel {channelId}</div>
                  <div className="relative h-12 bg-muted/30 rounded-lg">
                    {programs.map((program, idx) => {
                      const left = ((program.start - minTime) / timeRange) * 100;
                      const width = ((program.end - program.start) / timeRange) * 100;
                      const genreColor = genreColors[program.genre.toLowerCase()] || "bg-muted";

                      return (
                        <div
                          key={idx}
                          className={`absolute top-1 bottom-1 ${genreColor} rounded cursor-pointer hover:opacity-80 transition-opacity group`}
                          style={{
                            left: `${left}%`,
                            width: `${width}%`,
                          }}
                          title={`${program.program_id} | ${program.genre} | ${formatTime(program.start)} - ${formatTime(program.end)}`}
                        >
                          <div className="absolute -top-16 left-1/2 -translate-x-1/2 bg-card border border-border rounded px-2 py-1 text-xs whitespace-nowrap opacity-0 group-hover:opacity-100 transition-opacity pointer-events-none shadow-lg z-10">
                            <div className="font-medium">{program.program_id}</div>
                            <div className="text-muted-foreground">{program.genre}</div>
                            <div className="text-muted-foreground">
                              {formatTime(program.start)} - {formatTime(program.end)}
                            </div>
                          </div>
                        </div>
                      );
                    })}
                  </div>
                </div>
              ))}
          </div>

          <div className="flex justify-between text-xs text-muted-foreground pt-2 border-t">
            <span>{formatTime(minTime)}</span>
            <span>{formatTime(maxTime)}</span>
          </div>
        </div>
      )}
    </Card>
  );
};
