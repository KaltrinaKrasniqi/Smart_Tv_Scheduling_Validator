#pragma once
#include <string>
#include <unordered_map>
#include <vector>

namespace tvv {

struct Program {
  std::string id;
  int start=0, end=0;
  std::string genre;
  int score=0;
};

struct Channel {
  int id=0;
  std::vector<Program> programs;
};

struct PriorityBlock {
  int start=0, end=0;
  std::vector<int> allowed_channels;
};

struct TimePreference {
  int start=0, end=0;
  std::string preferred_genre;
  int bonus=0;
};

struct Instance {
  int opening_time=0;    
  int closing_time=0;    
  int min_duration=1;     
  int max_same_genre=999; 
  int S=0;                
  int T=0;                
  std::vector<Channel> channels;
  std::vector<PriorityBlock> priority_blocks;
  std::vector<TimePreference> time_prefs;


  std::unordered_map<std::string, const Program*> program_by_id;
  std::unordered_map<int, const Channel*> channel_by_id;
};

struct SubmissionItem {
  std::string program_id;
  int channel_id=0;
  int start=0, end=0;
};

struct Submission {
  std::vector<SubmissionItem> items;
};

/**
 * @brief Parses an instance JSON text into an Instance.
 * @param json_text Raw JSON string.
 * @return Instance Populated instance with lookup maps.
 * @throws std::exception on JSON parse or structural errors.
 */
Instance parse_instance(const std::string& json_text);

/**
 * @brief Parses a submission JSON text into a Submission.
 * @param json_text Raw JSON string.
 * @return Submission Populated submission items.
 * @throws std::exception on JSON parse or structural errors.
 */
Submission parse_submission(const std::string& json_text);


struct EvalOutput {
  int base=0, bonuses=0;
  int switches=0, early=0, late=0;
  int total=0;
  std::vector<std::string> debug;
  std::vector<struct Violation> violations;
  std::vector<struct TimelineItem> timeline;
};

/**
 * @brief Computes score/violations on a sorted timeline.
 *
 * Applies base points per program, time-preference bonuses,
 * switch (S) penalties, and early/late (T) penalties.
 *
 * @param ins Parsed instance with rules/bonuses/penalties.
 * @param sorted_tl Timeline items sorted by start time.
 * @param verbose If true, fills detailed debug logs.
 * @return EvalOutput Scoring totals, violations, and logs.
 */
EvalOutput evaluate(const Instance& ins,
                    const std::vector<TimelineItem>& sorted_tl, bool verbose);

} // namespace tvv
