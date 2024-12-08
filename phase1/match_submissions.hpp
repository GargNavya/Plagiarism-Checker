#include <array>
#include <iostream>
#include <span>
#include <vector>
#include <cmath>
// -----------------------------------------------------------------------------
#include<unordered_map>
#include<algorithm>
#include<set>
// You are free to add any STL includes above this comment, below the --line--.
// DO NOT add "using namespace std;" or include any other files/libraries.
// Also DO NOT add the include "bits/stdc++.h"

#define THRESHOLD 0.8

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

// This function iterates over pairs of matched_arrays in A and as per the threshold, checks if a pair can be counted as a single approximate match
void resolve(std::unordered_multimap<double, std::vector<int>>& A, double threshold, std::unordered_multimap<double, std::vector<int>>& B){
    // make a vector of keys from multimap A
    std::vector<std::pair<const double, std::vector<int>>> key_values1;
    for(const auto& pair : A){
        key_values1.push_back(pair);
    }

    // iterate through each key in key_values1
    for(int i=0; i<key_values1.size(); i++){
        std::vector<int> matches_values_copy = key_values1[i].second;
        // insert the current pair from A into B as an individual match
        B.insert({key_values1[i].first, matches_values_copy});
        // check subsequent pairs for approximate matches up to 50 entries ahead
        for(int j=i+1; j<50; j++){
            // std::cout << j << " ";
            int new_i = (key_values1[i].second[0] < key_values1[j].second[0])? key_values1[i].second[0] : key_values1[j].second[0];
            // calculate the maximum length span between two matched arrays
            int max_length = (key_values1[i].second[0]+key_values1[i].second[1] - key_values1[j].second[0] > key_values1[j].second[0]+key_values1[j].second[1] - key_values1[i].second[0])? key_values1[i].second[0]+key_values1[i].second[1] - key_values1[j].second[0] : key_values1[j].second[0]+key_values1[j].second[1] - key_values1[i].second[0];
            // calculate the new accuracy as a weighted average of the pair's accuracies and lengths
            double new_acc = (double(key_values1[i].second[1]*key_values1[i].first + key_values1[j].second[1]*key_values1[j].first) / double(max_length));
            // if the calculated accuracy meets or exceeds the threshold, insert the combined match into B
            if( new_acc >= threshold ){
                B.insert({new_acc, {new_i, max_length}});
            }
        }
        // std::cout << "\n**\n__\n";
    }
}

std::array<int, 5> match_submissions(std::vector<int> &submission1, std::vector<int> &submission2) {
    std::array<int, 5> result = {0, 0, 0, 0, 0};
    int len1 = submission1.size();
    int len2 = submission2.size();

    // FINDING EXACT MATCHES
    // matches will store in <j-i : {i, len}> format, where i is the starting index of matched pattern in submission1 and j is the starting index of matched pattern in submission2 and len is the length of matched pattern
    std::unordered_multimap<int, std::vector<int>> matches;

    int len = 10;   // Length of minimum pattern match
    // Finding all the exact matches of length >= len in the two arrays and storing them in matches
    for(int i=0; i < len1-len+1; i++){
        std::vector<int> pattern = std::vector<int>(submission1.begin()+i, submission1.begin()+i+len);      // Extracting a subarray of size len at index i in submission1
        std::vector<int> matching_starts_indices = KMP(submission2, pattern);           // Finding all the occurences of 'pattern' in submission2 
        for(int j : matching_starts_indices){
            int diff = j-i;
            if(matches.count(diff) == 0){
                matches.insert({diff, {i, len}});
            }else{
                // Merging the overlapping matched subarrays
                auto range = matches.equal_range(diff);
                for(auto it = range.first; it != range.second; it++){
                    int i_stored = it->second[0];
                    int len_stored = it->second[1];
                    if( ((i <= i_stored) && (i_stored <= i + len)) || ((i_stored <= i) && (i <= i_stored + len_stored))){
                        int i_new = (i < i_stored)? i : i_stored;
                        int len_new = ( (i+len > i_stored+len_stored)? (i+len):(i_stored+len_stored) ) - i_new;
                        it->second = {i_new, len_new};
                    }
                }
            }
        }
    }

    if(matches.size() == 0){
        return {0, 0, 0, 0, 0};
    }

    // FINDING APPROXIMATE MATCHES
    // Format: <accuracy: {i, len}>
    std::unordered_multimap<double, std::vector<int>> approx_matches;
    std::unordered_multimap<double, std::vector<int>> approx_matches1;
    std::unordered_multimap<double, std::vector<int>> approx_matches2;
    std::unordered_multimap<double, std::vector<int>> approx_matches3;

    // RESOLVE:
    // Add pairs of exact matches which satisfy the THRESHOLD if counted as a single matched array
    std::vector<std::vector<int>> key_values;
    for(int key = -len1; key < len2; key++){
        auto range = matches.equal_range(key);
        for(auto value_it = range.first; value_it != range.second; value_it++){
            std::vector<int> value = value_it->second;
            key_values.push_back(value);
        }
    }
    
    for(int i=0; i<key_values.size(); i++){
        std::vector<int> matches_values_copy = key_values[i];
        approx_matches.insert({1.0, matches_values_copy});
        for(int j=i+1; j<key_values.size(); j++){
            int new_i = (key_values[i][0] < key_values[j][0])? key_values[i][0] : key_values[j][0];
            int max_length = (key_values[i][0]+key_values[i][1] - key_values[j][0] > key_values[j][0]+key_values[j][1] - key_values[i][0])? key_values[i][0]+key_values[i][1] - key_values[j][0] : key_values[j][0]+key_values[j][1] - key_values[i][0];
            double new_acc = (double(key_values[i][1] + key_values[j][1]) / double(max_length));
            if( new_acc >= THRESHOLD ){
                approx_matches.insert({new_acc, {new_i, max_length}});
            }
        }
    }

    // Resolve one degree more
    std::vector<std::pair<const double, std::vector<int>>> key_values1;
    for(const auto& pair : approx_matches){
        key_values1.push_back(pair);
    }
    for(int i=0; i<key_values1.size(); i++){
        std::vector<int> matches_values_copy = key_values1[i].second;
        approx_matches1.insert({key_values1[i].first, matches_values_copy});
        for(int j=i+1; j<key_values1.size(); j++){
            int new_i = (key_values1[i].second[0] < key_values1[j].second[0])? key_values1[i].second[0] : key_values1[j].second[0];
            int max_length = (key_values1[i].second[0]+key_values1[i].second[1] - key_values1[j].second[0] > key_values1[j].second[0]+key_values1[j].second[1] - key_values1[i].second[0])? key_values1[i].second[0]+key_values1[i].second[1] - key_values1[j].second[0] : key_values1[j].second[0]+key_values1[j].second[1] - key_values1[i].second[0];
            double new_acc = (double(key_values1[i].second[1]*key_values1[i].first + key_values1[j].second[1]*key_values1[j].first) / double(max_length));
            if( new_acc >= THRESHOLD ){
                approx_matches1.insert({new_acc, {new_i, max_length}});
            }
        }
    }

    resolve(approx_matches1, THRESHOLD, approx_matches2);
    resolve(approx_matches2, THRESHOLD, approx_matches3);


    int total_len_10_20 = 0;
    int longest_length = 0;
    int start_10_20 = len1;
    int start_longest = 0;
    std::set<int> matched_indices;
    for(const auto& pair : matches){
        int i = pair.second[0];
        int length = pair.second[1];
        for(int j=i; j<i+length; j++){
            matched_indices.insert(j);
        }
        if(i < start_10_20){
            start_10_20 = i;
        }
    }
    total_len_10_20 = matched_indices.size();
    for(const auto& pair : approx_matches3){
        int i = pair.second[0];
        int length = pair.second[1];
        if(length > longest_length){
            longest_length = length;
            start_longest = i;
        }
    }
    bool flag = (total_len_10_20 >= 0.2*len1);
    result = {flag, total_len_10_20, longest_length, start_10_20, start_longest};
    return result;
}