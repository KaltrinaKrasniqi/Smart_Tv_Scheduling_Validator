#include "rules.hh"
#include "validator.hh"
#include "json.hpp"
#include <stdexcept>
#include <algorithm>
#include <unordered_set>
#include <iostream>



using nlohmann::json;
namespace tvv {


static int as_int(const json& j, const char* k) {
  if (!j.contains(k) || !j[k].is_number_integer())
    throw std::runtime_error(std::string("Missing/int field: ") + k);
  return j[k].get<int>();
}
static std::string as_str(const json& j, const char* k) {
  if (!j.contains(k) || !j[k].is_string())
    throw std::runtime_error(std::string("Missing/string field: ") + k);
  return j[k].get<std::string>();
}


static inline bool overlaps(int s1,int e1,int s2,int e2) {
  return !(e1 <= s2 || e2 <= s1);
}

Instance parse_instance(const std::string& txt) {
  auto j = json::parse(txt);
  Instance ins;

  ins.opening_time   = as_int(j, "opening_time");
  ins.closing_time   = as_int(j, "closing_time");
  ins.min_duration   = j.value("min_duration", 1);
  ins.max_same_genre = j.value("max_consecutive_genre", 999);
  ins.S = j.value("switch_penalty", 0);
  ins.T = j.value("termination_penalty", 0);

  if (!j.contains("channels") || !j["channels"].is_array())
    throw std::runtime_error("Missing/array: channels");

  ins.channels.reserve(j["channels"].size());

  for (auto& jc : j["channels"]) {
    ins.channels.push_back(Channel{});
    Channel& C = ins.channels.back();

    if (jc.contains("channel_id") && jc["channel_id"].is_number_integer())
      C.id = jc["channel_id"].get<int>();
    else if (jc.contains("id") && jc["id"].is_number_integer())
      C.id = jc["id"].get<int>();
    else
      throw std::runtime_error("Missing/int field: channel_id (or id)");

    if (!jc.contains("programs") || !jc["programs"].is_array())
      throw std::runtime_error("Missing/array: channels[].programs");


    C.programs.reserve(jc["programs"].size());

    for (auto& jp : jc["programs"]) {
      C.programs.push_back(Program{});
      Program& p = C.programs.back();

      p.id    = as_str(jp, "program_id"); 
      p.start = as_int(jp, "start");
      p.end   = as_int(jp, "end");
      p.genre = jp.value("genre", std::string{});
      p.score = jp.value("score", 0);

      if (ins.program_by_id.count(p.id))
        throw std::runtime_error("Duplicate program_id: " + p.id);

      ins.program_by_id[p.id] = &p;
    }

    ins.channel_by_id[C.id] = &C;
  }

  if (j.contains("priority_blocks")) {
    for (auto& pb : j["priority_blocks"]) {
      PriorityBlock b;
      b.start = as_int(pb, "start");
      b.end   = as_int(pb, "end");
      if (pb.contains("allowed_channels"))
        for (auto& ac : pb["allowed_channels"])
          b.allowed_channels.push_back(ac.get<int>());
      ins.priority_blocks.push_back(b);
    }
  }

  if (j.contains("time_preferences")) {
    for (auto& tp : j["time_preferences"]) {
      TimePreference t;
      t.start = as_int(tp, "start");
      t.end   = as_int(tp, "end");
      t.preferred_genre = tp.value("preferred_genre", std::string{});
      t.bonus = tp.value("bonus", 0);
      ins.time_prefs.push_back(t);
    }
  }

  return ins;
}

Submission parse_submission(const std::string& txt) {
  auto j = json::parse(txt);
  Submission s;
  const json* arr = nullptr;
  if (j.contains("schedule") && j["schedule"].is_array()) arr = &j["schedule"];
  else if (j.contains("scheduled_programs") && j["scheduled_programs"].is_array()) arr = &j["scheduled_programs"];
  else throw std::runtime_error("Missing/array: schedule (or scheduled_programs)");

  s.items.reserve(arr->size()); 

  for (auto& it : *arr) {
    SubmissionItem si;
    si.program_id = as_str(it, "program_id"); 
    si.channel_id = as_int(it, "channel_id"); 
    si.start      = as_int(it, "start");
    si.end        = as_int(it, "end");
    s.items.push_back(si);
  }
  return s;
}

// ------------------ evaluation ------------------
EvalOutput evaluate(const Instance& ins,
                    const std::vector<TimelineItem>& sorted_tl, bool verbose) {
  EvalOutput out;
  out.timeline = sorted_tl;
   auto logv = [&](const std::string& s){
    if (verbose) out.debug.push_back(s);
  };

  logv("=== EVALUATE START ===");
  logv("Items: " + std::to_string(sorted_tl.size()));


  // Base points
  int base_sum = 0;
  for (const auto& t : sorted_tl) {
    auto itp = ins.program_by_id.find(t.program_id);
    if (itp != ins.program_by_id.end()){
      base_sum += itp->second->score;
      logv(" + base: " + t.program_id + " → " + std::to_string(itp->second->score));
  }else{
    logv(" ! base: " + t.program_id + " not in instance");
  }
}
  out.base = base_sum;
  logv("Base total = " + std::to_string(out.base));

// Bonus points  (must have at least D minutes inside preferred interval)

int bonus_sum = 0;
const int D = ins.min_duration;

for (const auto& t : sorted_tl) {
  if (t.genre.empty()) continue;

  for (const auto& pref : ins.time_prefs) {
    if (pref.preferred_genre.empty()) continue;
    if (t.genre != pref.preferred_genre) continue;

    int inter_start = std::max(t.start, pref.start);
    int inter_end   = std::min(t.end,   pref.end);
    int inter_len   = std::max(0, inter_end - inter_start);

    if (inter_len >= D) {
      bonus_sum += pref.bonus;
      logv("[BONUS] +" + std::to_string(pref.bonus) + " for " + t.program_id +
           " (genre " + t.genre + ") with " + std::to_string(inter_len) +
           " min inside [" + std::to_string(pref.start) + "-" + std::to_string(pref.end) + "] (>= D=" +
           std::to_string(D) + ")");
    } else {
      logv("[NO BONUS] " + t.program_id + " has only " + std::to_string(inter_len) +
           " min inside preferred interval [" + std::to_string(pref.start) + "-" +
           std::to_string(pref.end) + "] (< D=" + std::to_string(D) + ")");
    }
  }
}

out.bonuses = bonus_sum;
logv("Bonus total = " + std::to_string(out.bonuses));

  // Switch penalty
  int switches = 0;
  for (size_t i = 1; i < sorted_tl.size(); ++i) {
    if (sorted_tl[i].channel_id != sorted_tl[i-1].channel_id) {
      switches++;
            logv("[SWITCH] " + sorted_tl[i-1].program_id + "(ch " +
           std::to_string(sorted_tl[i-1].channel_id) + ") -> " +
           sorted_tl[i].program_id + "(ch " +
           std::to_string(sorted_tl[i].channel_id) + ")");
    }
  }
  out.switches = switches;
  const int switches_pen = switches * ins.S;
    logv("Switches=" + std::to_string(out.switches) + " S=" + std::to_string(ins.S) +
       " penalty=" + std::to_string(switches_pen));

  // T penalty for early/late termination
  int delay_count = 0; 
  int early_count = 0; 
  for (const auto& t : sorted_tl) {
    auto itp = ins.program_by_id.find(t.program_id);
    if (itp == ins.program_by_id.end()) continue;
    const Program* P = itp->second;
    if (t.start > P->start) delay_count += 1;
    if (t.end   < P->end)   early_count += 1;
  }
  out.late  = delay_count;
  out.early = early_count;
  const int early_late_pen = (delay_count + early_count) * ins.T;
    logv("Early=" + std::to_string(out.early) + " Late=" + std::to_string(out.late) +
       " T=" + std::to_string(ins.T) +
       " penalty=" + std::to_string(early_late_pen));


  out.timeline = sorted_tl;        
  out.total = out.base + out.bonuses - switches_pen - early_late_pen;
    logv("[TOTAL] " + std::to_string(out.total));
  logv("=== EVALUATE END ===");

  return out;
}

} // namespace tvv
