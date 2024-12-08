#include "plagiarism_checker.hpp"
// You should NOT add ANY other includes to this file.
#include "../tokenizer.hpp"

#include<iostream>

// Do NOT add "using namespace std;".

// TODO: Implement the methods of the plagiarism_checker_t class
// CONSTRUCTOR -------------------------------------------------------
plagiarism_checker_t::plagiarism_checker_t(void) {
  // Initialize resources for dynamic addition of submissions
  // (e.g., mutexes or condition variables if needed)
}

// ----------------- PARAMETERIZED CONSTRUCTOR -------------------
// Accepts a vector of shared pointers to submission_t objects.
// Iterates over these submissions, tokenizing each one's codefile.
// Uses tokenizer_t to extract tokens and stores them in a tokenized_data map
// for efficient comparison later.

plagiarism_checker_t::plagiarism_checker_t(
    std::vector<std::shared_ptr<submission_t>> __submissions)
    : existing_submissions(__submissions) {
  for (const auto &submission : __submissions) {
    // Tokenize each submission's codefile and store its tokens in
    // tokenized_data
    existing_submissions.push_back(submission);
    // timestamps[submission->id] = std::chrono::system_clock::time_point();
    existingmap[submission->id] = submission;
    timestamps[submission->id] = std::chrono::system_clock::time_point();
    tokenizer_t tokenizer(submission->codefile);
    tokenized_data[submission->id] = tokenizer.get_tokens();
  }
}

// DESTRUCTOR --------------------------------------------------------
plagiarism_checker_t::~plagiarism_checker_t(void) {
  {
    std::lock_guard<std::mutex> lock(mtx);
    shutdown = true; // Signal threads to stop processing
  }
  cv.notify_all(); // Wake up any threads waiting on the condition variable

  // Join all threads to ensure clean shutdown
  for (auto &t : threads) {
    if (t.joinable())
      t.join();
  }
}

// ADDING NEW SUBMISSIONS --------------------------------------------
// The add_submission method allows the plagiarism checker to accept new
// submissions dynamically. When a new submission is added, it processes
// the submission in a separate thread for concurrent execution, ensuring
// the system remains responsive even under heavy load.

void plagiarism_checker_t::add_submission(
    std::shared_ptr<submission_t> __submission) {
  // Record the timestamp when the submission is added
  auto now = std::chrono::system_clock::now();

  {
    std::lock_guard<std::mutex> lock(mtx); // Lock to ensure thread safety
    // Store the timestamp of the new submission
    timestamps[__submission->id] = now;
    existingmap[__submission->id] = __submission;
    // Add the submission to the list of existing submissions
    existing_submissions.push_back(__submission);
  }

  // Create a thread to process the submission asynchronously
  threads.push_back(std::thread(&plagiarism_checker_t::process_submission, this, __submission));
}


// -------------------- FLAG STUDENT/PROFESSOR ----------------------
void plagiarism_checker_t::process_submission(
    std::shared_ptr<submission_t> __submission) {
  // Tokenize the current submission's code
  tokenizer_t tokenizer(__submission->codefile);
  std::vector<int> tokens = tokenizer.get_tokens();
  long int existing_submissionid = __submission->id;
  long int newid = 0;

  // Get references to student and professor (to flag if plagiarism is found)
  std::shared_ptr<student_t> student = __submission->student;
  std::shared_ptr<professor_t> professor = __submission->professor;
  std::shared_ptr<student_t> student2 = nullptr;
  std::shared_ptr<professor_t> professor2 = nullptr;

  bool is_plagiarized = false;
  bool differ = false; // Flag for timestamp difference greater than 1 second
  long patchwork_count = 0;

  {
    std::lock_guard<std::mutex> lock(mtx); // Lock for thread safety

    for (const auto &[id, existing_tokens] : tokenized_data) {
      // Check plagiarism
      if(timestamps[existing_submissionid] <= timestamps[id]){
        continue;
      }
      bool is_matching = check_plagiarism(tokens, existing_tokens, patchwork_count);
      if (is_matching) {
        // Get the timestamp difference
        auto time_diff = std::chrono::duration_cast<std::chrono::seconds>(
                             timestamps[__submission->id] - timestamps[id])
                             .count();

        // Check if timestamps differ by at least 1 second
        if (std::abs(time_diff) < 1) {
          differ = true;
          // Assign `student2` from the existing submission
          if (existingmap.find(id) != existingmap.end()) {
            student2 = existingmap[id]->student;
            professor2 = existingmap[id]->professor;
          }
        }

        // Record the id of the plagiarized existing submission
        newid = id;
        is_plagiarized = true;
        break; // Exit the loop if plagiarism is detected
      }else if (patchwork_count >= 20){
        is_plagiarized = true;
        differ = false;
      }
    }
  

  // If plagiarism is detected, flag the appropriate submissions
  
    if (is_plagiarized) {
        flaggedid.insert(existing_submissionid);
        if (differ){
            if(flaggedid.find(newid) != flaggedid.end()){}
            else{
                // Flag both submissions if timestamps differ by less than 1 second
                std::shared_ptr<submission_t> submission2 = existingmap[newid];
                flaggedid.insert(newid);
                if (student2)
                    student2->flag_student(submission2);
                if (professor2)
                    professor2->flag_professor(submission2);
            }
        }
        if (student)
            student->flag_student(__submission);
        if (professor)
            professor->flag_professor(__submission);
    }
  }
  // Store the tokenized data for future comparisons
  {
    std::lock_guard<std::mutex> lock(mtx); // Lock for thread safety
    tokenized_data[__submission->id] = tokens;
    existing_submissions.push_back(__submission);
  }
}

std::vector<int> KMPTable(std::vector<int> P){
    int i=1;
    int j=0;
    int Psize = P.size();
    std::vector<int> h(Psize + 1, 0);
    h[0] = -1;
    while(i < Psize){
        if(P[j] != P[i]){
            h[i] = j;
            while(j >= 0 && P[j] != P[i]){
                j = h[j];
            }
        }else{
            h[i] = h[j];
        }
        i++; j++;
    }
    h[Psize] = j;
    return h;
}

// ---------------------- CHECK PLAGIARISM -------------------------

// function to generate KMP table / prefix table
std::vector<int> KMP(std::vector<int> T, std::vector<int> P){
    std::vector<int> h = KMPTable(P);
    std::vector<int> found;
    int i=0;
    int j=0;
    int Tsize = T.size();
    int Psize = P.size();
    while(i < Tsize){
        if(P[j] == T[i]){
            i++; j++;
            if(j == Psize){
                found.push_back(i-j);
                j = h[j];
            }
        }else{
            j = h[j];
            if(j < 0){
                i++; j++;
            }
        }
    }
    return found;
}

// function to check if two submission have threshold level of plagiarism
bool plagiarism_checker_t::check_plagiarism(const std::vector<int>& tokens1,
const std::vector<int>& tokens2, long &patchwork_count) {
    int len1 = tokens1.size();
    int len2 = tokens2.size();

    // FINDING EXACT MATCHES
    // matches will store in <j-i : {i, len}> format, where i is the starting index of matched pattern in tokens1 and j is the starting index of matched pattern in tokens2 and len is the length of matched pattern
    std::unordered_multimap<int, std::vector<int>> matches;

    int len = 15;   // Length of minimum pattern match
    // Finding all the exact matches of length >= len in the two arrays and storing them in matches
    for(int i=0; i < len1-len+1; i++){
        std::vector<int> pattern = std::vector<int>(tokens1.begin()+i,
        tokens1.begin()+i+len);                                             // Extracting a subarray of size len at index i in tokens1
        std::vector<int> matching_starts_indices = KMP(tokens2, pattern);   // Finding all the occurences of 'pattern' in tokens2
        for(int j : matching_starts_indices){
            int diff = j-i;
            if(matches.count(diff) == 0){
                matches.insert({diff, {i, len}});
                // Flag for more than 10 matches***********************
                if(matches.size() >=10){
                    return true;
                }
                // ****************************************************
            }else{
                // Merging the overlapping matched subarrays
                auto range = matches.equal_range(diff);
                bool merged = false;
                for(auto it = range.first; it != range.second; it++){
                    int i_stored = it->second[0];
                    int len_stored = it->second[1];
                    if( ((i <= i_stored) && (i_stored <= i + len)) || ((i_stored <= i) && (i <= i_stored + len_stored))){
                        int i_new = (i < i_stored)? i : i_stored;
                        int len_new = ( (i+len > i_stored+len_stored)? (i+len):(i_stored+len_stored) ) - i_new; it->second = {i_new, len_new};
                        merged = true;
                        it->second = {i_new, len_new};
                        // Checking for length >=75 *******************
                        if(len_new >=75){
                            return true;
                        }
                        // ********************************************
                    }
                }if(!merged){
                    matches.insert({diff, {i, len}});
                }
            }
        }
    }
    if(matches.size() >= 4)  patchwork_count += matches.size();
    return false;
}
// End TODO