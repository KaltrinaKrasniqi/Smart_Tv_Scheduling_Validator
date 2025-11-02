#pragma once
#include <string>
#include <vector>
#include "json.hpp"
using nlohmann::json;

namespace tvv {

struct SwitchStats { int count=0; int S=0; int total=0; };
struct EarlyLateStats { int early=0; int late=0; int T=0; int total=0; };

struct Score {
  int total=0;
  int base=0;
  int bonuses=0;
  SwitchStats switches;
  EarlyLateStats early_late;
};

struct Violation {
  std::string code; 
  std::string msg;
  int t = -1;         
};

struct TimelineItem {
  std::string program_id;
  int channel_id = -1;
  std::string genre;
  int start = 0;
  int end   = 0;
};

struct Result {
  std::string status = "VALID"; // "VALID" | "INVALID" | "ERROR"
  Score score;
  std::vector<Violation> violations;
  std::vector<TimelineItem> timeline;
  std::string validator_version = "1.0";
  int elapsed_ms = 0;
  std::string error_message;
  std::vector<std::string> debug;
};


/**
 * @brief Main API: validates an instance/submission pair and computes score.
 *
 * Performs structural checks, business-rule validation (input & output),
 * overlap detection, and scoring (base, bonuses, S/T penalties).
 *
 * @param instance_json The scheduling instance JSON.
 * @param submission_json The submission JSON.
 * @param verbose If true, collects detailed debug logs.
 * @return Result Structured outcome including status, violations, and score.
 */
Result validate(const std::string& instance_json,
                const std::string& submission_json,
                bool verbose);

/**
 * @brief Serializes a Result to JSON.
 * @param r The result to serialize.
 * @return JSON string representing the result payload.
 */
std::string to_json(const Result& r);

//////////////////// input checks ////////////////////

/**
 * @brief Validates required fields and basic shapes in the instance.
 * @param input Parsed instance JSON.
 * @return true if structure is valid; false otherwise.
 */
bool validateInputStructure(const nlohmann::json& input);

/**
 * @brief Ensures opening_time < closing_time and within valid bounds.
 * @param input Parsed instance JSON.
 * @return true if times are valid; false otherwise.
 */
bool validateOpeningAndClosingTime(const nlohmann::json& input);

/**
 * @brief Checks that channels exist, are unique, and well-formed.
 * @param input Parsed instance JSON.
 * @return true on success; false otherwise.
 */
bool validateChannelsCount(const nlohmann::json& input);

/**
 * @brief Validates PriorityBlock ranges and allowed channel lists.
 * @param input Parsed instance JSON.
 * @return true on success; false otherwise.
 */
bool validatePriorityBlocks(const nlohmann::json& input);

/**
 * @brief Validates TimePreference ranges and bonus definitions.
 * @param input Parsed instance JSON.
 * @return true on success; false otherwise.
 */
bool validateTimePreferences(const nlohmann::json& input);

/**
 * @brief Validates program catalog: ids, times, genres, and scores.
 * @param input Parsed instance JSON.
 * @return true on success; false otherwise.
 */
bool validatePrograms(const nlohmann::json& input);

/**
 * @brief Detects overlaps among programs within the same input channel.
 * @param input Parsed instance JSON.
 * @return true if no same-channel overlaps; false otherwise.
 */
bool validateProgramOverlapInInput(const nlohmann::json& input);

//////////////////// output checks ////////////////////

/**
 * @brief Validates presence/type of 'scheduled_programs' in submission.
 * @param output Parsed submission JSON.
 * @return true on success; false otherwise.
 */
bool validateOutputStructure(const nlohmann::json& output);

/**
 * @brief Checks the type of num_programs attribute (if exists).
 * @param output Parsed submission JSON.
 * @return true on success; false otherwise.
 */
bool validateNumPrograms(const nlohmann::json& output);

/**
 * @brief Ensures each scheduled item lies within opening/closing time.
 * @param output Parsed submission JSON.
 * @param opening_time Instance opening time.
 * @param closing_time Instance closing time.
 * @return true on success; false otherwise.
 */
bool validateProgramTimes(const nlohmann::json& output, int opening_time, int closing_time);

/**
 * @brief Detects overlaps between scheduled programs.
 * @param output Parsed submission JSON.
 * @return true if no overlaps; false otherwise.
 */
bool validateProgramOverlap(const nlohmann::json& output);

/**
 * @brief Ensures all scheduled program_ids exist in the instance catalog.
 * @param input Parsed instance JSON.
 * @param output Parsed submission JSON.
 * @return true on success; false otherwise.
 */
bool validateProgramsExistInInput(const nlohmann::json& input, const nlohmann::json& output);

/**
 * @brief Verifies that each scheduled item’s channel_id matches the program’s channel.
 * @param output Parsed submission JSON.
 * @param input Parsed instance JSON.
 * @return true on success; false otherwise.
 */
bool validateProgramAndChannel(const nlohmann::json& output, const nlohmann::json& input);


} // namespace tvv
