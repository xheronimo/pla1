#include <iostream>
#include <vector>
#include <algorithm>

/**
 * @brief Removes duplicates from the given vector.
 * 
 * This function takes a vector by reference, sorts it, and removes duplicate
 * elements while preserving the order of unique elements.
 * 
 * @param vec A reference to the vector from which duplicates will be removed.
 */
void removeDuplicates(std::vector<int>& vec) {
    // Sort the vector to bring duplicates together
    std::sort(vec.begin(), vec.end());
    
    // Use the unique function to remove duplicates
    auto last = std::unique(vec.begin(), vec.end());
    vec.erase(last, vec.end());
}

int main() {
    std::vector<int> numbers = {5, 3, 6, 3, 8, 5, 10, 6};

    std::cout << "Original numbers: ";
    for (const int& num : numbers) {
        std::cout << num << " ";
    }
    std::cout << std::endl;

    removeDuplicates(numbers);

    std::cout << "Numbers after removing duplicates: ";
    for (const int& num : numbers) {
        std::cout << num << " ";
    }
    std::cout << std::endl;

    return 0;
}