# Sliding Window Technique (Coding/Algorithms)

## What is Sliding Window Technique?

The **Sliding Window Technique** is a powerful problem-solving pattern used to solve problems involving **arrays, strings, or lists** where you need to find something about a **continuous subset** of elements.

## Simple Analogy

Imagine looking through a window at a row of houses:
- You can see 3 houses at a time (window size)
- Instead of rebuilding your view from scratch, you just **slide** the window one house at a time
- This is much faster than starting over each time!

```
Houses: [🏠][🏡][🏘️][🏚️][🏗️][🏛️][🏢]
         |====|                    ← Window sees houses 1,2,3
            |====|                 ← Slide right, sees 2,3,4
               |====|              ← Slide right, sees 3,4,5
```

## Why Use Sliding Window?

### Without Sliding Window (Brute Force):
```cpp
// Find max sum of 3 consecutive elements
// Time: O(n*k) where k is window size
for (int i = 0; i <= n-3; i++) {
    int sum = arr[i] + arr[i+1] + arr[i+2];  // Recalculate every time
    max_sum = max(max_sum, sum);
}
```

### With Sliding Window:
```cpp
// Time: O(n) - Much faster!
int sum = arr[0] + arr[1] + arr[2];  // Initial window
for (int i = 3; i < n; i++) {
    sum = sum - arr[i-3] + arr[i];   // Slide: remove left, add right
    max_sum = max(max_sum, sum);
}
```

**Result**: From O(n×k) to O(n) - HUGE improvement! 🚀

## Types of Sliding Window

### 1. Fixed Size Window
Window size stays constant throughout.

**Example**: Find maximum sum of any 3 consecutive elements
```
Array: [2, 1, 5, 1, 3, 2]
Window size: 3

[2, 1, 5] → sum = 8
   [1, 5, 1] → sum = 7
      [5, 1, 3] → sum = 9  ← Maximum!
         [1, 3, 2] → sum = 6
```

### 2. Variable Size Window
Window expands and shrinks based on conditions.

**Example**: Find smallest subarray with sum ≥ 7
```
Array: [2, 1, 5, 2, 3, 2]

[2] → sum=2 (too small, expand)
[2, 1] → sum=3 (too small, expand)
[2, 1, 5] → sum=8 ≥ 7 ✓ (found! try to shrink)
   [1, 5] → sum=6 (too small)
   [1, 5, 2] → sum=8 ≥ 7 ✓ (found!)
      [5, 2] → sum=7 ≥ 7 ✓ (smaller! answer is size 2)
```

## How It Works

### The Two Pointer Approach

```
Array: [a][b][c][d][e][f][g]
        ↑           ↑
       left       right
       (start)     (end)
```

1. **Expand**: Move `right` pointer to add elements
2. **Process**: Check if window meets condition
3. **Shrink**: Move `left` pointer to reduce window
4. **Repeat**: Until you've checked all possibilities

## Pattern Recognition

### When to Use Sliding Window?

✅ **Use sliding window if problem asks for:**
- Maximum/minimum sum of k consecutive elements
- Longest substring with k distinct characters
- Smallest subarray with sum ≥ target
- Find all anagrams in a string
- Maximum consecutive 1's after flipping k 0's

✅ **Keywords that hint sliding window:**
- "Consecutive" / "Contiguous"
- "Subarray" / "Substring"
- "Window" / "Range"
- "k elements"

## Common Problem Patterns

### Pattern 1: Fixed Window Max/Min

**Problem**: Maximum sum of k consecutive elements

```cpp
int maxSum(vector<int>& arr, int k) {
    int n = arr.size();
    if (n < k) return -1;
    
    // Calculate first window
    int window_sum = 0;
    for (int i = 0; i < k; i++) {
        window_sum += arr[i];
    }
    
    int max_sum = window_sum;
    
    // Slide the window
    for (int i = k; i < n; i++) {
        window_sum = window_sum - arr[i-k] + arr[i];  // Slide!
        max_sum = max(max_sum, window_sum);
    }
    
    return max_sum;
}
```

**Visualization**:
```
arr = [1, 4, 2, 10, 2, 3, 1, 0, 20], k = 4

Step 1: [1, 4, 2, 10] → sum = 17
Step 2:    [4, 2, 10, 2] → sum = 18
Step 3:       [2, 10, 2, 3] → sum = 17
Step 4:          [10, 2, 3, 1] → sum = 16
Step 5:              [2, 3, 1, 0] → sum = 6
Step 6:                 [3, 1, 0, 20] → sum = 24 ← MAX!
```

### Pattern 2: Variable Window with Condition

**Problem**: Longest substring with at most k distinct characters

```cpp
int longestSubstring(string s, int k) {
    unordered_map<char, int> char_count;
    int left = 0, max_len = 0;
    
    for (int right = 0; right < s.length(); right++) {
        // Expand: Add character to window
        char_count[s[right]]++;
        
        // Shrink: If too many distinct chars, remove from left
        while (char_count.size() > k) {
            char_count[s[left]]--;
            if (char_count[s[left]] == 0) {
                char_count.erase(s[left]);
            }
            left++;
        }
        
        // Update max length
        max_len = max(max_len, right - left + 1);
    }
    
    return max_len;
}
```

**Visualization**:
```
s = "araaci", k = 2

Step 1: [a] → {a:1} ✓ len=1
Step 2: [a,r] → {a:1,r:1} ✓ len=2
Step 3: [a,r,a] → {a:2,r:1} ✓ len=3
Step 4: [a,r,a,a] → {a:3,r:1} ✓ len=4 ← MAX so far
Step 5: [a,r,a,a,c] → {a:3,r:1,c:1} ✗ TOO MANY! Shrink...
        [r,a,a,c] → {r:1,a:2,c:1} ✗ Still too many! Shrink...
        [a,a,c] → {a:2,c:1} ✓ len=3
Step 6: [a,a,c,i] → {a:2,c:1,i:1} ✗ Shrink...
        [a,c,i] → {a:1,c:1,i:1} ✗ Shrink...
        [c,i] → {c:1,i:1} ✓ len=2

Answer: 4
```

### Pattern 3: Two Pointers Approaching

**Problem**: Find pair with target sum in sorted array

```cpp
vector<int> twoSum(vector<int>& arr, int target) {
    int left = 0, right = arr.size() - 1;
    
    while (left < right) {
        int sum = arr[left] + arr[right];
        
        if (sum == target) {
            return {left, right};
        } else if (sum < target) {
            left++;   // Need larger sum
        } else {
            right--;  // Need smaller sum
        }
    }
    
    return {-1, -1};
}
```

## Step-by-Step Example

**Problem**: Find length of longest substring without repeating characters

```
Input: "abcabcbb"
Output: 3 (substring "abc")
```

**Solution Process**:

```cpp
int lengthOfLongestSubstring(string s) {
    unordered_set<char> window;
    int left = 0, max_len = 0;
    
    for (int right = 0; right < s.length(); right++) {
        // If character already in window, shrink from left
        while (window.count(s[right])) {
            window.erase(s[left]);
            left++;
        }
        
        // Add current character
        window.insert(s[right]);
        max_len = max(max_len, right - left + 1);
    }
    
    return max_len;
}
```

**Execution Trace**:
```
s = "abcabcbb"

right=0: window={a}, left=0, len=1
right=1: window={a,b}, left=0, len=2
right=2: window={a,b,c}, left=0, len=3 ← MAX
right=3: 'a' exists! Remove from left
         window={b,c}, left=1
         window={b,c,a}, left=1, len=3
right=4: 'b' exists! Remove from left
         window={c,a}, left=2
         window={c,a,b}, left=2, len=3
right=5: 'c' exists! Remove from left
         window={a,b}, left=3
         window={a,b,c}, left=3, len=3
right=6: 'b' exists! Remove from left
         window={c,b}, left=5, len=2
right=7: 'b' exists! Remove from left
         window={b}, left=7, len=1

Answer: 3
```

## Classic Problems to Practice

### Easy
1. **Maximum Average Subarray I** (Fixed window)
2. **Contains Duplicate II** (Fixed window with hash)
3. **Minimum Size Subarray Sum** (Variable window)

### Medium
4. **Longest Substring Without Repeating Characters**
5. **Longest Repeating Character Replacement**
6. **Permutation in String**
7. **Find All Anagrams in String**
8. **Fruit Into Baskets**
9. **Max Consecutive Ones III**

### Hard
10. **Minimum Window Substring**
11. **Sliding Window Maximum**
12. **Substring with Concatenation of All Words**

## Templates

### Template 1: Fixed Window
```cpp
int fixedWindow(vector<int>& arr, int k) {
    // Initialize window
    int window_sum = 0;
    for (int i = 0; i < k; i++) {
        window_sum += arr[i];
    }
    
    int result = window_sum;
    
    // Slide window
    for (int i = k; i < arr.size(); i++) {
        window_sum = window_sum - arr[i-k] + arr[i];
        result = processWindow(window_sum);  // Your logic
    }
    
    return result;
}
```

### Template 2: Variable Window
```cpp
int variableWindow(vector<int>& arr, condition) {
    int left = 0, result = 0;
    
    for (int right = 0; right < arr.size(); right++) {
        // Expand window: add arr[right]
        addToWindow(arr[right]);
        
        // Shrink window if condition violated
        while (conditionViolated()) {
            removeFromWindow(arr[left]);
            left++;
        }
        
        // Update result
        result = updateResult(right - left + 1);
    }
    
    return result;
}
```

### Template 3: Character Frequency Window
```cpp
string windowWithFrequency(string s, string pattern) {
    unordered_map<char, int> pattern_freq, window_freq;
    
    // Build pattern frequency
    for (char c : pattern) {
        pattern_freq[c]++;
    }
    
    int left = 0, matched = 0;
    
    for (int right = 0; right < s.length(); right++) {
        char right_char = s[right];
        
        // Expand: add character
        if (pattern_freq.count(right_char)) {
            window_freq[right_char]++;
            if (window_freq[right_char] == pattern_freq[right_char]) {
                matched++;
            }
        }
        
        // Check if all matched
        while (matched == pattern_freq.size()) {
            // Process valid window
            
            // Shrink: remove from left
            char left_char = s[left];
            if (pattern_freq.count(left_char)) {
                if (window_freq[left_char] == pattern_freq[left_char]) {
                    matched--;
                }
                window_freq[left_char]--;
            }
            left++;
        }
    }
}
```

## Time & Space Complexity

| Operation | Time | Space |
|-----------|------|-------|
| Fixed Window | O(n) | O(1) or O(k) |
| Variable Window | O(n) | O(k) |
| With Hash Map | O(n) | O(k) |

Where:
- n = length of array/string
- k = window size or distinct elements

## Key Concepts Summary

1. **Window** = Contiguous subarray/substring
2. **Sliding** = Move window efficiently without recalculating
3. **Two Pointers** = `left` and `right` define window boundaries
4. **Optimization** = Reduce O(n²) or O(n×k) to O(n)

## Tips & Tricks

### ✅ Do's
- Identify if problem needs fixed or variable window
- Use hash map/set to track elements in window
- Update result after expanding/shrinking
- Handle edge cases (empty array, k > n)

### ❌ Don'ts
- Don't recalculate entire window each time
- Don't forget to shrink window when needed
- Don't use nested loops (defeats the purpose!)
- Don't confuse window boundaries (use right-left+1 for size)

## Practice Strategy

1. **Start with fixed window problems** (easier)
2. **Move to variable window** (more complex logic)
3. **Practice pattern recognition** (when to use sliding window)
4. **Master the templates** (adapt to different problems)
5. **Solve 10-15 problems** (build muscle memory)

---

**Remember**: Sliding Window = Efficiently process contiguous subarrays! 🪟✨

**Key Insight**: Instead of recalculating from scratch, just update by:
- **Adding** the new element entering the window
- **Removing** the old element leaving the window
