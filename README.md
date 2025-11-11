# Smart TV Scheduling Validator

This repository contains the **Smart TV Scheduling Validator** — a tool designed to validate TV scheduling data for the Smart Tv Scheduling problem based on a set of defined rules and constraints. This validator is part of a **diploma thesis project**. It leverages **C++** to provide a robust validation mechanism, running in the browser using **WebAssembly**.

## Hosted Version

You can access the live version of the validator here:  
[Smart TV Scheduling Validator](https://smarttvschedulingvalidator.netlify.app/)

## Features

- **Web Interface**: Upload TV schedule data and submission files to validate them against predefined constraints.
- **Validation Rules**: The validator checks multiple constraints such as program overlap, priority blocks, channel availability, and more.
  - **ERROR Status**:
    1. **JSON Parsing Errors**: Errors during the parsing of `instance` and `submission` from string to JSON.
    2. **Missing Required Fields**: Missing fields in the input file (e.g., `channels_count`, `priority_blocks`, etc.).
    3. **Output Structure Issues**: If `scheduled_programs` is missing or not an array in the output file.
    4. **Opening and Closing Time Validation**: Ensures that the opening time is less than the closing time.
    5. **Channel Count Mismatch**: Ensures that the `channels_count` matches the number of channels in the input file.
    6. **Priority Block Validation**: Ensures that priority blocks are within the opening and closing times and that channel IDs are valid.
    7. **Time Preferences**: Ensures that time blocks for time preferences are within valid time ranges, and genres are correctly defined.
    8. **Program and Channel Matching**: Checks if the `channel_id` in the output file matches the input file and if programs are correctly assigned to channels.
    9. **Program Existence Validation**: Ensures that all program IDs in the output file exist in the input file.
  - **INVALID Status**:
    1. **MIN_DURATION**: Validates that the program duration meets the minimum required duration.
    2. **MAX_GENRE_RUN**: Ensures that no genre runs consecutively more than allowed.
    3. **Priority Block Violations**: Programs violating the priority block constraints are excluded from score calculations.
    4. **Outside Time Window**: Validates that programs are scheduled within the defined opening and closing times.
    5. **Output Overlap**: Detects any overlapping programs in the output file.
    6. **Input Overlap**: Programs in the output file that refer to the ones that overlap on input file within the same channel are excluded from scoring.
  - **VALID Status**: The validator returns a valid status when no violations are found.
  - **BONUS**: Programs that overlap with preferred genres or time intervals receive bonus points. Programs with overlapping intervals for the same genre or across different intervals earn bonus points only if they run on the preferred interval for a min_duration time.
  - **SWITCH PENALTY**: Penalizes switching programs between channels.
  - **EARLY/LATE TERMINATION PENALTY**: Applies a penalty for programs starting later or finishing earlier than their scheduled times.
- **Real-time Results**: Upon validation, the results, including violation details and score breakdown, are displayed in real-time.
- **Built with C++ & TypeScript**: The core validation logic is implemented in C++, compiled to WebAssembly for performance, and the frontend is built using TypeScript and React.

## Local Setup

To run the project locally, follow these steps:

### Prerequisites
1. **Node.js** (version 14 or higher)
2. **Emscripten**: Required to build the C++ WASM module. You can install it following the [Emscripten installation guide](https://emscripten.org/docs/getting_started/downloads.html).

### Steps to Run Locally

1. **Install Emscripten**:
   First, install [Emscripten](https://emscripten.org/docs/getting_started/downloads.html) and ensure that it is properly set up on your machine.

2. **Activate the Emscripten environment**:
   Run the following command to activate the Emscripten environment in your terminal:
   ```bash
   source ~/emsdk/emsdk_env.sh
3. Clone the repository
   git clone https://github.com/your-username/smart-tv-scheduling-validator.git
   cd smart-tv-scheduling-validator
4. Install dependencies
   npm install
5. Build the C++ code with Emscripten
   cd wasm
   ./build.sh
6. Start the local development server
   npm run dev

