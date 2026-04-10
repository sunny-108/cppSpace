#include <unordered_map>
#include <iostream>
#include <string>

using char_int_map = std::unordered_map<char, int>;

void printMap(const char_int_map& wordMap){
    std::cout<<std::endl;
    for(const auto& element : wordMap){
        std::cout<<" --> "<<element.first<<" : "<<element.second<<std::endl;
    }
}

char_int_map countCharFrequency(const std::string& word){
    char_int_map freqMap;
    for(auto ch : word) freqMap[ch]++;
    return freqMap;
}

int main(){
    std::string myname = "sunny shivam";
    char_int_map mymap = countCharFrequency(myname);
    printMap(mymap);
    mymap.erase('s');
    printMap(mymap);
    return 0;
}