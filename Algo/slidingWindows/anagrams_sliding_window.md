# Anagrams and Sliding Window Algorithm

## What is an Anagram?

An **anagram** is a word or phrase formed by rearranging the letters of another word or phrase, using all the original letters exactly once.

### Examples:
- "listen" and "silent" are anagrams
- "triangle" and "integral" are anagrams
- "abc" and "cab" are anagrams
- "abc" and "ab" are NOT anagrams (different lengths)

### Key Properties:
1. Both strings must have the same length
2. Both strings must contain the same characters with the same frequency
3. Order doesn't matter

## Common Anagram Problems

### 1. Check if Two Strings are Anagrams
**Approach:** Sort both strings or use frequency count comparison.
- **Time:** O(n log n) for sorting, O(n) for frequency count
- **Space:** O(1) for sorting in-place, O(k) for frequency map (k = alphabet size)

### 2. Find All Anagrams in a String (Sliding Window)
**Problem:** Given a string `s` and a pattern `p`, find all starting indices of `p`'s anagrams in `s`.

**Example:**
```
Input: s = "cbaebabacd", p = "abc"
Output: [0, 6]
Explanation:
- Substring at index 0: "cba" is an anagram of "abc"
- Substring at index 6: "bac" is an anagram of "abc"
```

## Sliding Window Solution for Finding Anagrams

### Algorithm Overview:

The sliding window technique is optimal for this problem because:
1. We need to check every substring of length `len(p)` in string `s`
2. Instead of recalculating character frequency for each window from scratch
3. We maintain a window and slide it, updating frequencies incrementally

### Steps:

1. **Initialize:**
   - Create frequency map for pattern `p`
   - Create frequency map for first window of size `len(p)` in string `s`

2. **Compare:**
   - If both frequency maps match, we found an anagram at index 0

3. **Slide Window:**
   - Remove leftmost character from window (decrease its frequency)
   - Add new rightmost character to window (increase its frequency)
   - Compare frequency maps after each slide
   - If they match, record the starting index

### Time Complexity: O(n)
- We traverse string `s` once: O(n)
- Each frequency comparison takes O(26) = O(1) for lowercase English letters
- Total: O(n)

### Space Complexity: O(1)
- Fixed size frequency arrays (26 characters for English alphabet)

## C++ Implementation

### Method 1: Using Frequency Arrays (Most Efficient)

```cpp
#include <iostream>
#include <vector>
#include <string>

std::vector<int> findAnagrams(const std::string& s, const std::string& p) {
    std::vector<int> result;
    int sLen = s.length();
    int pLen = p.length();
    
    if (sLen < pLen) return result;
    
    // Frequency arrays for pattern and window
    std::vector<int> pFreq(26, 0);
    std::vector<int> windowFreq(26, 0);
    
    // Build frequency array for pattern
    for (char c : p) {
        pFreq[c - 'a']++;
    }
    
    // Build frequency array for first window
    for (int i = 0; i < pLen; i++) {
        windowFreq[s[i] - 'a']++;
    }
    
    // Check if first window is an anagram
    if (pFreq == windowFreq) {
        result.push_back(0);
    }
    
    // Slide the window
    for (int i = pLen; i < sLen; i++) {
        // Add new character to window
        windowFreq[s[i] - 'a']++;
        // Remove old character from window
        windowFreq[s[i - pLen] - 'a']--;
        
        // Check if current window is an anagram
        if (pFreq == windowFreq) {
            result.push_back(i - pLen + 1);
        }
    }
    
    return result;
}

// Example usage
int main() {
    std::string s = "cbaebabacd";
    std::string p = "abc";
    
    std::vector<int> indices = findAnagrams(s, p);
    
    std::cout << "Anagram indices: ";
    for (int idx : indices) {
        std::cout << idx << " ";
    }
    std::cout << std::endl;
    
    return 0;
}
```

### Method 2: Using Unordered Map (More Flexible)

```cpp
#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>

std::vector<int> findAnagramsMap(const std::string& s, const std::string& p) {
    std::vector<int> result;
    int sLen = s.length();
    int pLen = p.length();
    
    if (sLen < pLen) return result;
    
    std::unordered_map<char, int> pMap, windowMap;
    
    // Build map for pattern
    for (char c : p) {
        pMap[c]++;
    }
    
    // Build map for first window
    for (int i = 0; i < pLen; i++) {
        windowMap[s[i]]++;
    }
    
    // Check first window
    if (pMap == windowMap) {
        result.push_back(0);
    }
    
    // Slide window
    for (int i = pLen; i < sLen; i++) {
        // Add new character
        windowMap[s[i]]++;
        
        // Remove old character
        windowMap[s[i - pLen]]--;
        if (windowMap[s[i - pLen]] == 0) {
            windowMap.erase(s[i - pLen]);
        }
        
        // Check current window
        if (pMap == windowMap) {
            result.push_back(i - pLen + 1);
        }
    }
    
    return result;
}
```

### Method 3: Optimized with Match Counter

```cpp
#include <iostream>
#include <vector>
#include <string>

std::vector<int> findAnagramsOptimized(const std::string& s, const std::string& p) {
    std::vector<int> result;
    int sLen = s.length();
    int pLen = p.length();
    
    if (sLen < pLen) return result;
    
    std::vector<int> freq(26, 0);
    
    // Build frequency difference
    for (int i = 0; i < pLen; i++) {
        freq[p[i] - 'a']++;
        freq[s[i] - 'a']--;
    }
    
    // Count non-zero frequencies
    int diff = 0;
    for (int count : freq) {
        if (count != 0) diff++;
    }
    
    if (diff == 0) result.push_back(0);
    
    // Slide window
    for (int i = pLen; i < sLen; i++) {
        int newChar = s[i] - 'a';
        int oldChar = s[i - pLen] - 'a';
        
        // Add new character
        if (freq[newChar] == 0) diff++;
        freq[newChar]--;
        if (freq[newChar] == 0) diff--;
        
        // Remove old character
        if (freq[oldChar] == 0) diff++;
        freq[oldChar]++;
        if (freq[oldChar] == 0) diff--;
        
        if (diff == 0) {
            result.push_back(i - pLen + 1);
        }
    }
    
    return result;
}
```

## Other Anagram Problems Using Sliding Window

### 1. Longest Substring with At Most K Distinct Characters
While not strictly anagrams, uses similar sliding window with frequency counting.

### 2. Minimum Window Substring
Find the smallest substring of `s` that contains all characters of `t` (with frequencies).

### 3. Permutation in String
Check if one string contains a permutation of another string (anagram variant).

## Key Takeaways

1. **Anagrams** require same characters with same frequencies
2. **Sliding window** is efficient when checking all fixed-size substrings
3. **Frequency arrays/maps** track character counts
4. **Incremental updates** make sliding window O(n) instead of O(n²)
5. Always consider:
   - Edge cases (empty strings, pattern longer than text)
   - Character set size (lowercase letters, all ASCII, Unicode)
   - Whether to handle duplicates

## Practice Problems

1. **LeetCode 438**: Find All Anagrams in a String
2. **LeetCode 567**: Permutation in String
3. **LeetCode 76**: Minimum Window Substring
4. **LeetCode 242**: Valid Anagram
5. **LeetCode 49**: Group Anagrams

## Complexity Comparison

| Approach | Time | Space | Best For |
|----------|------|-------|----------|
| Brute Force (sort each window) | O(n × m log m) | O(m) | Small inputs |
| Sliding Window + Frequency Array | O(n) | O(26) = O(1) | Fixed alphabet |
| Sliding Window + Hash Map | O(n) | O(k) | Variable character sets |
| Optimized Match Counter | O(n) | O(26) = O(1) | Best performance |

Where:
- n = length of string `s`
- m = length of pattern `p`
- k = number of unique characters
