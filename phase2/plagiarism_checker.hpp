#include "structures.hpp"
// -----------------------------------------------------------------------------
#include <vector>
#include <memory>
#include <unordered_map>
#include <set>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include<unordered_set>

// You are free to add any STL includes above this comment, below the --line--.
// DO NOT add "using namespace std;" or include any other files/libraries.
// Also DO NOT add the include "bits/stdc++.h"

// OPTIONAL: Add your helper functions and classes here

class plagiarism_checker_t {
    // You should NOT modify the public interface of this class.
public:
    plagiarism_checker_t(void);
    plagiarism_checker_t(std::vector<std::shared_ptr<submission_t>> __submissions);
    ~plagiarism_checker_t(void);
    void add_submission(std::shared_ptr<submission_t> __submission);

protected:
    // TODO: Add members and function signatures here
    // Helper functions
    void process_submission(std::shared_ptr<submission_t> __submission);
    bool check_plagiarism(const std::vector<int>& tokens1, const std::vector<int>& tokens2, long &patchwork_count);

    // Data members
    std::mutex mtx; // Mutex for thread safety
    std::condition_variable cv; // Condition variable for thread synchronization
    std::vector<std::shared_ptr<submission_t>> existing_submissions; // Vector for storing existing submissions
    std::unordered_map<long , std::shared_ptr<submission_t>> existingmap;
    std::unordered_map<long, std::vector<int>> tokenized_data; // Map for storing tokenized submissions
    std::vector<std::thread> threads; // Set of threads processing submissions
    bool shutdown = false; // Flag to indicate if threads should stop
    // End TODO
    // std::unordered_map<long,long> timestamps;
    std::unordered_map<long, std::chrono::system_clock::time_point> timestamps;
    std::unordered_set<long> flaggedid; 

};