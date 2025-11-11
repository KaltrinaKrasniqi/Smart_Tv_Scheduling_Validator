#include "validator.hh"
#include "rules.hh"
#include "json.hpp"
#include <chrono>
#include <iostream>
#include <algorithm>
#include <unordered_map>
#include <functional>
#include <unordered_set>

using nlohmann::json;
namespace tvv {

static void collectInputOverlapsAsViolations(
  const nlohmann::json& input,
  std::vector<Violation>& out,
  std::function<void(const std::string&)> logv,
  std::unordered_set<std::string>& overlapped_prog_ids 
);

static std::string to_json_score(const Score& s) {
  json j;
  j["total"] = s.total;
  j["base"] = s.base;
  j["bonuses"] = s.bonuses;
  j["switches"] = { {"count", s.switches.count}, {"S", s.switches.S}, {"total", s.switches.total} };
  j["early_late"] = { {"early", s.early_late.early}, {"late", s.early_late.late},
                      {"T", s.early_late.T}, {"total", s.early_late.total} };
  return j.dump();
}

std::string to_json(const Result& r) {
  json j;
  j["status"] = r.status;
  j["score"] = json::parse(to_json_score(r.score));
  j["violations"] = json::array();
  for (auto& v: r.violations) {

    json jv; 
    jv["code"]=v.code; 
    jv["msg"]=v.msg;       
    jv["message"]=v.msg;     
    jv["t"]=v.t;
    j["violations"].push_back(jv);
  }
  j["timeline"] = json::array();
  for (auto& t: r.timeline) {
    json jt; jt["program_id"]=t.program_id; jt["channel_id"]=t.channel_id;
    jt["genre"]=t.genre; jt["start"]=t.start; jt["end"]=t.end;
    j["timeline"].push_back(jt);
  }
  j["validator_version"] = r.validator_version;
  j["elapsed_ms"] = r.elapsed_ms;
  if (!r.error_message.empty()) j["error_message"] = r.error_message;
  if (!r.debug.empty()) {
  j["debug"] = r.debug;
  j["verbose"] = r.debug;
}

  return j.dump();
}

Result validate(const std::string& instance_json,
                const std::string& submission_json,
                bool verbose) {
  Result result;
  std::vector<std::string> dbg;
  auto logv = [&](std::string s){ if (verbose) dbg.push_back(std::move(s)); };

 
  json jIns, jSub;
  try {
    jIns = json::parse(instance_json);
    jSub = json::parse(submission_json);
    logv("Parsed JSON (instance & submission) OK.");
  } catch (const std::exception& e) {
    result.status = "ERROR";
    result.error_message = std::string("JSON parse error: ") + e.what();
    return result;
  }

  if (!validateInputStructure(jIns)) {
    result.status = "ERROR";
    result.error_message = "Input structure validation failed.";
    return result;
  }
    if (!validateOutputStructure(jSub)) {
    result.status = "ERROR";
    result.error_message = "Output structure validation failed.";
    return result;
  }

  logv("Schema validation OK.");

  if (!validateOpeningAndClosingTime(jIns) ||
      !validateChannelsCount(jIns) ||
      !validatePriorityBlocks(jIns) ||
      !validateTimePreferences(jIns)) {
        
    result.status = "ERROR";
    result.error_message = "Input validation failed.";
    return result;
  }
  logv("Instance constraints OK.");

if (!validateProgramsExistInInput(jIns, jSub)) {
  result.status = "ERROR";
  result.error_message = "Output validation failed.";
  return result;
}
logv("Output reference checks OK.");


if (!validateProgramAndChannel(jSub, jIns)) {
    result.status = "ERROR";
    result.error_message = "Program and channel validation failed.";
    return result;
}

logv("Program->Channel mapping OK.");

  Instance ins;
  Submission sub;
  try {
    ins = parse_instance(instance_json);
    sub = parse_submission(submission_json);
    logv("Parsed to internal structs OK.");
  } catch (const std::exception& e) {
    result.status = "ERROR";
    result.error_message = std::string("Parsing to structs failed: ") + e.what();
    return result;
  }

  std::vector<TimelineItem> tl;
  tl.reserve(sub.items.size());
  for (const auto& it : sub.items) {
    auto f = ins.program_by_id.find(it.program_id);
    std::string g = (f == ins.program_by_id.end()) ? "" : f->second->genre;
    tl.push_back(TimelineItem{it.program_id, it.channel_id, g, it.start, it.end});
  }
  std::sort(tl.begin(), tl.end(), [](const TimelineItem& a, const TimelineItem& b){
    if (a.start != b.start) return a.start < b.start;
    if (a.end   != b.end  ) return a.end   < b.end;
    if (a.channel_id != b.channel_id) return a.channel_id < b.channel_id;
    return a.program_id < b.program_id;
  });
  logv("Built timeline with " + std::to_string(tl.size()) + " items.");

 
std::vector<char> valid_mask(tl.size(), 1);
std::vector<Violation> all_violations;
auto add_violation = [&](Violation v){
  all_violations.push_back(std::move(v));
};
 
// MIN_CONTIGUOUS_DURATION
for (size_t i = 0; i < tl.size(); ++i) {
  const auto& t = tl[i];
  int W = t.end - t.start;


  int L = 0;
  {
    auto itP = ins.program_by_id.find(t.program_id);
    if (itP != ins.program_by_id.end() && itP->second) {
  
      L = itP->second->end - itP->second->start;
    } else {

      logv("[WARN] Program id not found in instance map for " + t.program_id + " while checking MIN_CONTIGUOUS_DURATION.");
 
      add_violation(Violation{
        "PROGRAM_NOT_IN_INSTANCE",
        "INVALID: Program '" + t.program_id + "' not found in instance when checking duration constraints.",
        t.start
      });
      valid_mask[i] = 0;
      continue;
    }
  }

  const int D = ins.min_duration;

  if (L >= D) {

    if (W < D) {
      add_violation(Violation{
        "MIN_CONTIGUOUS_DURATION_UNDER_D",
        "INVALID: Program '" + t.program_id + "' scheduled for " + std::to_string(W) +
        " min, which is less than required minimum of " + std::to_string(D) + " min.",
        t.start
      });
      valid_mask[i] = 0;
      logv("[VIOL] MIN_CONTIGUOUS_DURATION_UNDER_D at " + std::to_string(t.start) + " for " + t.program_id);
    }
  } else {
    if (W != L) {
      add_violation(Violation{
        "SHORT_PROGRAM_MUST_BE_FULL",
        "INVALID: Program '" + t.program_id + "' is shorter than D (" + std::to_string(L) +
        " min < " + std::to_string(D) + " min) and must be scheduled in full; got " + std::to_string(W) + " min.",
        t.start
      });
      valid_mask[i] = 0;
      logv("[VIOL] SHORT_PROGRAM_MUST_BE_FULL at " + std::to_string(t.start) + " for " + t.program_id);
    }
  }
}


// MAX_GENRE_RUN
{
  int run = 0;
  std::string last;
  for (size_t i = 0; i < tl.size(); ++i) {
    const auto& t = tl[i];
    if (t.genre.empty()) { last.clear(); run = 0; continue; }

    if (t.genre == last) {
      run++;
    } else {
      last = t.genre;
      run = 1;
    }

    if (run > ins.max_same_genre) {
      add_violation(Violation{
        "MAX_GENRE_RUN",
        "INVALID: More than " + std::to_string(ins.max_same_genre) +
        " consecutive programs of genre '" + t.genre + "'. Offending program: '" + t.program_id + "'.",
        t.start
      });
      valid_mask[i] = 0;
      logv("[VIOL] MAX_GENRE_RUN at " + std::to_string(t.start) + " for " + t.program_id + " (genre " + t.genre + ")");
    }
  }
}

  
// PRIORITY_BLOCK_CHANNEL
auto in_block = [](int s1,int e1,int s2,int e2){
  return !(e1 <= s2 || e2 <= s1);
};
for (size_t i = 0; i < tl.size(); ++i) {
  const auto& t = tl[i];
  for (const auto& b : ins.priority_blocks) {
    if (!b.allowed_channels.empty() && in_block(t.start, t.end, b.start, b.end)) {
      bool allowed = std::find(b.allowed_channels.begin(), b.allowed_channels.end(),
                               t.channel_id) != b.allowed_channels.end();
      if (!allowed) {
        add_violation(Violation{
          "PRIORITY_BLOCK_CHANNEL",
          "INVALID: Program '" + t.program_id + "' is scheduled in Channel " + std::to_string(t.channel_id) +
          " during the priority block [" + std::to_string(b.start) + "-" + std::to_string(b.end) + "], but this channel is not allowed in this block.",
          t.start
        });
        valid_mask[i] = 0;
        logv("[VIOL] PRIORITY_BLOCK_CHANNEL at " + std::to_string(t.start) + " for " + t.program_id);
      }
    }
  }
}


//  OUTSIDE_WINDOW → INVALID
{
  const int O = ins.opening_time;
  const int E = ins.closing_time;
  for (size_t i = 0; i < tl.size(); ++i) {
    const auto& t = tl[i];

    if (t.start < O || t.end > E) {
      add_violation(Violation{
        "OUTSIDE_WINDOW",
        "INVALID: Program '" + t.program_id + "' is scheduled outside the global window [" +
          std::to_string(O) + "," + std::to_string(E) + ").",
        t.start
      });
      valid_mask[i] = 0;
      logv("[VIOL] OUTSIDE_WINDOW at " + std::to_string(t.start) + " for " + t.program_id);
    }
  }
}


// OUTPUT_OVERLAP
{
  std::vector<size_t> active;
  active.reserve(tl.size()); 


  std::unordered_set<std::string> reported_overlaps;
  auto mk_key = [&](const TimelineItem& X){
    return X.program_id + "|ch" + std::to_string(X.channel_id) +
           "|" + std::to_string(X.start) + "-" + std::to_string(X.end);
  };

  for (size_t i = 0; i < tl.size(); ++i) {
    const auto& C = tl[i];

  
    size_t w = 0;
    for (size_t r = 0; r < active.size(); ++r) {
      const auto& A = tl[active[r]];
      if (A.end > C.start) active[w++] = active[r];
    }
    active.resize(w);

 
    for (size_t r = 0; r < active.size(); ++r) {
      size_t iPrev = active[r];
      const auto& A = tl[iPrev];

      if ((C.start < A.end) && (C.end > A.start)) {

        valid_mask[iPrev] = 0;
        valid_mask[i]     = 0;

        std::string k1 = mk_key(A);
        std::string k2 = mk_key(C);
        std::string pair_key = (k1 < k2) ? (k1 + "||" + k2) : (k2 + "||" + k1);

        if (reported_overlaps.insert(pair_key).second) {
          add_violation(Violation{
            "OUTPUT_OVERLAP",
            "INVALID: Overlap between '" + A.program_id + "' [ch " + std::to_string(A.channel_id) +
              ", " + std::to_string(A.start) + "-" + std::to_string(A.end) + "] and '" +
              C.program_id + "' [ch " + std::to_string(C.channel_id) + ", " +
              std::to_string(C.start) + "-" + std::to_string(C.end) + "].",
            std::min(A.start, C.start)
          });
          logv("[VIOL] OUTPUT_OVERLAP " + A.program_id + " (ch " + std::to_string(A.channel_id) +
               ") <-> " + C.program_id + " (ch " + std::to_string(C.channel_id) + ")");
        }
      }
    }

    active.push_back(i);
  }
}

std::unordered_set<std::string> overlapped_in_input;
collectInputOverlapsAsViolations(jIns, all_violations, logv, overlapped_in_input);

for (size_t i = 0; i < tl.size(); ++i) {
  if (overlapped_in_input.find(tl[i].program_id) != overlapped_in_input.end()) {
    if (valid_mask[i]) {
      valid_mask[i] = 0;
      add_violation(Violation{
        "INPUT_OVERLAP",
        "INVALID: Referenced program '" + tl[i].program_id +
        "' overlaps with another program in the input; excluded from scoring.",
        tl[i].start
      });
      logv("[VIOL] INPUT_OVERLAP → exclude from score (ref in submission): " + tl[i].program_id);
    }
  }
}


bool any_invalid = std::any_of(valid_mask.begin(), valid_mask.end(),
                               [](char v){ return v == 0; });

std::vector<TimelineItem> filtered;
if (any_invalid) {
  filtered.reserve(tl.size());
  for (size_t i = 0; i < tl.size(); ++i) {
    if (valid_mask[i]) filtered.push_back(tl[i]);
  }
  logv("[DIAGNOSTIC] INVALID detected. Evaluating score on valid subset only: " +
       std::to_string(filtered.size()) + " / " + std::to_string(tl.size()) + " items.");
}

 
EvalOutput eval = any_invalid
  ? evaluate(ins, filtered, verbose)
  : evaluate(ins, tl,       verbose);

result.status     = any_invalid ? "INVALID" : "VALID";
result.violations = std::move(all_violations);
result.timeline = tl;
result.score.base     = eval.base;
result.score.bonuses  = eval.bonuses;
result.score.switches.count = eval.switches;
result.score.switches.S     = ins.S;
result.score.switches.total = eval.switches * ins.S;
result.score.early_late.early = eval.early;
result.score.early_late.late  = eval.late;
result.score.early_late.T     = ins.T;
result.score.early_late.total = (eval.early + eval.late) * ins.T;
result.score.total = eval.total;

if (verbose) {
  dbg.insert(dbg.end(), eval.debug.begin(), eval.debug.end());
  if (any_invalid) dbg.push_back("[DIAGNOSTIC] Score computed on valid subset only.");
  result.debug = dbg;
} else {
  result.debug.clear();
}


return result;

}

bool validateInputStructure(const nlohmann::json& input) {
    std::vector<std::string> required_fields = {
        "opening_time", "closing_time", "min_duration", "max_consecutive_genre", 
        "channels_count", "switch_penalty", "termination_penalty", "priority_blocks", 
        "time_preferences", "channels"
    };

    for (const auto& field : required_fields) {
        if (input.find(field) == input.end()) {
            std::cout << "Error: Missing required field " << field << " in input file." << std::endl;
            return false;
        }
    }
    return true;
}

bool validateOpeningAndClosingTime(const nlohmann::json& input) {
    int opening_time = input["opening_time"];
    int closing_time = input["closing_time"];
    
    if (opening_time >= closing_time) {
        std::cout << "Error: Opening time cannot be greater than or equal to closing time." << std::endl;
        return false;
    }
    return true;
}

bool validateChannelsCount(const nlohmann::json& input) {
    int channels_count = input["channels_count"];
    int actual_channels_count = input["channels"].size();
    
    if (channels_count != actual_channels_count) {
        std::cout << "Error: Channels count mismatch. Expected " << channels_count << ", but found " << actual_channels_count << "." << std::endl;
        return false;
    }
    return true;
}

bool validatePriorityBlocks(const nlohmann::json& input) {
    int opening_time = input["opening_time"];
    int closing_time = input["closing_time"];
    const nlohmann::json& priority_blocks = input["priority_blocks"];
    
    for (const auto& block : priority_blocks) {
        int start = block["start"];
        int end = block["end"];
        const nlohmann::json& allowed_channels = block["allowed_channels"];
        
        if (start < opening_time || end > closing_time) {
            std::cout << "Error: Priority block " << start << "-" << end << " is out of valid time range [" << opening_time << ", " << closing_time << "]." << std::endl;
            return false;
        }

        for (const auto& channel_id : allowed_channels) {
            if (channel_id < 0 || channel_id >= input["channels_count"]) {
                std::cout << "Error: Invalid channel " << channel_id << " in priority block." << std::endl;
                return false;
            }
        }
    }
    return true;
}

bool validateTimePreferences(const nlohmann::json& input) {
    int opening_time = input["opening_time"];
    int closing_time = input["closing_time"];
    const nlohmann::json& time_preferences = input["time_preferences"];
    
    for (const auto& preference : time_preferences) {
        int start = preference["start"];
        int end = preference["end"];
        std::string preferred_genre = preference["preferred_genre"];
        
        if (start < opening_time || end > closing_time) {
            std::cout << "Error: Time preference " << start << "-" << end << " is out of valid time range [" << opening_time << ", " << closing_time << "]." << std::endl;
            return false;
        }


        if (preferred_genre.empty()) {
            std::cout << "Error: Preferred genre is empty or invalid." << std::endl;
            return false;
        }
    }
    return true;
}

bool validatePrograms(const nlohmann::json& input) {
    int opening_time = input["opening_time"];
    int closing_time = input["closing_time"];
    const nlohmann::json& channels = input["channels"];
    
    for (const auto& channel : channels) {
        const nlohmann::json& programs = channel["programs"];
        
        for (const auto& program : programs) {
            int start = program["start"];
            int end = program["end"];
            
            if (start < opening_time || end > closing_time || start >= end) {
                std::cout << "Error: Program " << program["program_id"] << " has invalid start or end time." << std::endl;
                return false;
            }
        }
    }
    return true;
}


bool validateProgramOverlapInInput(const nlohmann::json& input) {
    for (const auto& channel : input["channels"]) {
        const nlohmann::json& programs = channel["programs"];
        

        for (size_t i = 0; i < programs.size(); ++i) {
            int start_i = programs[i]["start"];
            int end_i = programs[i]["end"];
            
            for (size_t j = i + 1; j < programs.size(); ++j) {
                int start_j = programs[j]["start"];
                int end_j = programs[j]["end"];
                

                if ((start_i < end_j) && (end_i > start_j)) {
                    std::cout << "Error: Programs in channel " << channel["channel_id"] << " overlap. Programs " 
                              << programs[i]["program_id"] << " and " << programs[j]["program_id"] << " overlap." << std::endl;
                    return false;
                }
            }
        }
    }
    return true;
}

bool validateOutputStructure(const nlohmann::json& output) {
    if (output.find("scheduled_programs") == output.end()) {
        std::cout << "Error: Missing 'scheduled_programs' in output file." << std::endl;
        return false;
    }
    if (!output["scheduled_programs"].is_array()) {
        std::cout << "Error: 'scheduled_programs' should be an array." << std::endl;
        return false;
    }
    return true;
}


bool validateNumPrograms(const nlohmann::json& output) {
    if (output.find("num_programs") == output.end()) {
        std::cout << "Error: Missing 'num_programs' in output file." << std::endl;
        return false;
    }

    size_t num_programs_output = output["scheduled_programs"].size();
    int num_programs = output["num_programs"];

    if (num_programs != num_programs_output) {
        std::cout << "Error: 'num_programs' in output file does not match the number of scheduled programs." << std::endl;
        return false;
    }

    return true;
}

bool validateProgramTimes(const nlohmann::json& output, int opening_time, int closing_time) {
    const nlohmann::json& scheduled_programs = output["scheduled_programs"];
    
    for (const auto& program : scheduled_programs) {
        int start = program["start"];
        int end = program["end"];
        
        if (start < opening_time || end > closing_time) {
            std::cout << "Error: Program " << program["program_id"] << " has invalid start or end time." << std::endl;
            return false;
        }
    }
    return true;
}

bool validateProgramOverlap(const nlohmann::json& output) {
    std::unordered_map<int, std::vector<std::pair<int, int>>> channel_times;
    const nlohmann::json& scheduled_programs = output["scheduled_programs"];
    
    for (const auto& program : scheduled_programs) {
        int channel_id = program["channel_id"];
        int start = program["start"];
        int end = program["end"];
        
        for (const auto& existing_program : channel_times[channel_id]) {
            if ((start < existing_program.second) && (end > existing_program.first)) {
                std::cout << "Error: Overlap detected in channel " << channel_id << " between programs." << std::endl;
                return false;
            }
        }
        
        channel_times[channel_id].push_back({start, end});
    }
    
    return true;
}

bool validateProgramsExistInInput(const nlohmann::json& input, const nlohmann::json& output) {
    std::unordered_map<std::string, bool> input_programs;
    for (const auto& channel : input["channels"]) {
        for (const auto& program : channel["programs"]) {
            input_programs[program["program_id"]] = true;
        }
    }

    for (const auto& scheduled_program : output["scheduled_programs"]) {
        std::string program_id = scheduled_program["program_id"];
        
        if (input_programs.find(program_id) == input_programs.end()) {
            std::cout << "Error: Program " << program_id << " in output file does not exist in input file." << std::endl;
            return false;
        }
    }
    return true;
}

bool validateProgramAndChannel(const nlohmann::json& output, const nlohmann::json& input) {
    const nlohmann::json& scheduled_programs = output["scheduled_programs"];

    for (const auto& program : scheduled_programs) {
        int channel_id = program["channel_id"];
        std::string program_id = program["program_id"];

        auto channel_it = std::find_if(input["channels"].begin(), input["channels"].end(),
            [channel_id](const nlohmann::json& channel) {
                return channel["channel_id"] == channel_id;
            });

        if (channel_it == input["channels"].end()) {
            std::cout << "Error: Channel ID " << channel_id << " in output file does not exist in input file." << std::endl;
            return false;
        }

        bool program_found = false;
        for (const auto& p : channel_it->at("programs")) {
            if (p["program_id"] == program_id) {
                program_found = true;
                break;
            }
        }

        if (!program_found) {
            std::cout << "Error: Program ID " << program_id << " does not belong to Channel " << channel_id << " in input file." << std::endl;
            return false;
        }
        
        auto program_in_output = std::find_if(scheduled_programs.begin(), scheduled_programs.end(),
            [program_id](const nlohmann::json& p) {
                return p["program_id"] == program_id;
            });

        if (program_in_output != scheduled_programs.end()) {
            int output_channel = program_in_output->at("channel_id");
            if (output_channel != channel_id) {
                std::cout << "Error: Program ID " << program_id << " is scheduled in channel " 
                          << output_channel << " in output file, but it should be in channel " 
                          << channel_id << " based on the input file." << std::endl;
                return false;
            }
        }
    }

    return true;
}

static void collectInputOverlapsAsViolations(const nlohmann::json& input,
                                             std::vector<Violation>& out,
                                             std::function<void(const std::string&)> logv,
                                             std::unordered_set<std::string>& overlapped_prog_ids) {  // <- SHTUAR
  for (const auto& channel : input["channels"]) {
    int ch = channel["channel_id"];
    const auto& programs = channel["programs"];

    struct Seg { int s,e; std::string id; };
    std::vector<Seg> segs;
    segs.reserve(programs.size());
    for (const auto& p : programs) {
      segs.push_back(Seg{ p["start"], p["end"], p["program_id"] });
    }
    std::sort(segs.begin(), segs.end(), [](const Seg& a, const Seg& b){
      if (a.s != b.s) return a.s < b.s;
      if (a.e != b.e) return a.e < b.e;
      return a.id < b.id;
    });

    std::vector<size_t> active;
    for (size_t i = 0; i < segs.size(); ++i) {
      const auto& C = segs[i];
      size_t w = 0;
      for (size_t r = 0; r < active.size(); ++r) {
        const auto& A = segs[active[r]];
        if (A.e > C.s) active[w++] = active[r];
      }
      active.resize(w);

      for (size_t r = 0; r < active.size(); ++r) {
        const auto& A = segs[active[r]];
        if ((C.s < A.e) && (C.e > A.s)) {

        overlapped_prog_ids.insert(A.id);
        overlapped_prog_ids.insert(C.id);


        logv("[WARN] INPUT_OVERLAP in input ch=" + std::to_string(ch) + " " + A.id + " <-> " + C.id);
      }

      }
      active.push_back(i);
    }
  }
}


} // namespace tvv
