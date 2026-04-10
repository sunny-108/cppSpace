#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>

std::vector<int> anargams(const std::string& s, const std::string& p){
    std::vector<int> result;
    auto sl = s.length();
    auto pl = p.length();

    if (sl < pl) result;
    std::unordered_map<char, int> smap;
    std::unordered_map<char, int> wmap;
    //create frequency map
    for(auto c : p) smap[c]++;

    for(int i =0; i< pl; ++i) wmap[s[i]]++;

    // Check first window
    if (smap == wmap) {
        result.push_back(0);
    }

    //wmap.clear();

    for(auto i=pl; i < sl; i++){

        //wmap.clear(s[i-1]);
    
    }
}//