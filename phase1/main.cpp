#include "match_submissions.hpp"
#include "../tokenizer.hpp"
#include<iostream>

// You should NOT modify ANYTHING in this file.
extern std::array<int, 5> match_submissions(std::vector<int> &submission1, 
        std::vector<int> &submission2);

double execute_and_verify_testcase(std::string test_dir) {
    tokenizer_t file_one(test_dir + "/one.cpp");
    tokenizer_t file_two(test_dir + "/two.cpp");
    std::vector<int> submission1 = file_one.get_tokens();
    std::vector<int> submission2 = file_two.get_tokens();
    std::array<int, 5> output = match_submissions(submission1, submission2);
    // // PRINTING THE TOKENS**********************************************************************
    // for(int i = 0;i<submission1.size();i++) {
    //     std::cout<<submission1[i]<<" ";
    // }
    // std::cout<<"this is where the first submission ends"<<"\n";
    // std::cout<<"\n";
    // std::cout<<"\n";
    // std::cout<<"\n";
    // for(int i = 0;i<submission2.size();i++) {
    //     std::cout<<submission2[i]<<" ";
    // }
    // std::cout<<"\n";
    // std::cout<<"\n";
    // std::cout<<"\n";
    // // PRINTING THE TOKENS**********************************************************************

    std::ifstream in(test_dir + "/expected.txt");
    std::array<int, 5> expected;
    in >> expected[0] >> expected[1] >> expected[2] >> 
            expected[3] >> expected[4];
    in.close();

    std::array<double, 5> results;
    results[0] = (output[0] == expected[0]) ? 1.0 : 0.0;
    results[1] = (1.0 * std::min(output[1], expected[1])) / 
            std::max(output[1], expected[1]);
    results[2] = (1.0 * std::min(output[2], expected[2])) /
            std::max(output[2], expected[2]);
    results[3] = std::pow(1.1, -std::abs(output[3] - expected[3]));
    results[4] = std::pow(1.1, -std::abs(output[4] - expected[4]));
//     ******************************************************************printing results
    std::cout << "My results: ";
    for(int i=0; i<5; i++){std::cout << output[i] << " ";}
    std::cout << std::endl;
    std::cout << "Ex results: ";
    for(int i=0; i<5; i++){std::cout << expected[i] << " ";}
    std::cout << std::endl;
//     ******************************************************************printing results
    return (2.0 * results[0] + results[1] + results[2] + 
            0.5 * (results[3] + results[4]));
}

int main(void) {
    double total_score = 0.0;
    double score1 = execute_and_verify_testcase("testcases/one");
    std::cout << "Testcase 1: " << score1 << " / 5.0" << std::endl;
    double score2 = execute_and_verify_testcase("testcases/two");
    std::cout << "Testcase 2: " << score2 << " / 5.0" << std::endl;
    double score3 = execute_and_verify_testcase("testcases/three");
    std::cout << "Testcase 3: " << score3 << " / 5.0" << std::endl;
    total_score += score1 + score2 + score3;
    std::cout << "Total score: " << total_score << " / 15.0" << std::endl;
    return 0;
}
