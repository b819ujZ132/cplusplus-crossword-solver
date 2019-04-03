// CrosswordSolver.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

/*
* There are so many additional optimizations that could be made, but at some point, enough is enough for
* a coding challenge. Below I highlight some things that I would invest time into if this application was
* to become an investment.
*
* NOTES:
*   Performance is worst with known letters of high-frequency and words of highest length frequency. (Ex. 'E_____' 6)
*   Examples such as 'EY_' 3 are fast.
*   The addendum here is that retrieval is fast enough for all cases that the end user cannot notice any
*   delays.
*
* Performance analysis
* Statistical analysis of datasets (looking for improved distributions here to speed up the final search)
* Input validation
* Robustness, i.e., what if I swap the dictionary out for another one
* Machine learning (train a bot to identify statistical variables or combinatorials that provide an improved distribution)
* UI
*/

#include "pch.h"
#include <algorithm>
#include <cstring>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

/*
* Order of relative letter frequency in English language based on external study.
*
* How this 'map' works is simple. 'a' is the 3rd most frequently used letter in the Dictionary. 'b' is
* the 20th most frequently used letter in the Dictionary and so on.
*
* 'map' is quoted because as you can plainly see, it isn't actually a map to the layman. However, the
* reason I call it a map is because the indices map to letters of the alphabet. We use this to our
* advantage later on.
*
* NOTE: In an ideal world, this should be computed off the data set itself. I could have optimized here
*       by running a statistical analysis on the dictionary provided, but the time required was not in
*       scope.
*/
const std::vector<unsigned char> Order =
{
  3, 20, 12, 10, 1, 16, 17, 8, 5, 23, // A, B, C, D, E, F, G, H, I, J
  22, 11, 14, 6, 4, 19, 25, 9, 7, 2,  // K, L, M, N, O, P, Q, R, S, T
  13, 21, 15, 24, 18, 26              // U, V, W, X, Y, Z
};

/*
* Simple struct to hold original and compressed versions of strings.
* 
* NOTE: Initially, I tried to hold the compressed version as a C string to eliminate the conversion, but
*       it merely amounts to a single indirection. Cost is low on dataset of current size.
*/
struct Word
{
  Word(const std::string o, const std::string c)
    : original(o), compressed(c)
  {
  }
  const std::string original;
  const std::string compressed;
};

/*
* Simple utility function to sort and remove duplicate characters in a std::string.
*/
static std::string Compress(std::string word)
{
  std::sort(word.begin(), word.end());
  word.erase(std::unique(word.begin(), word.end()), word.end());

  return word;
}

/*
* Function does several things:
*   1) Creates a top-level vector of size 14. Why you might ask? A simple analysis of the dataset highlights
*      that the dataset is comprised of words of 2 <= word.length <= 15. Each index of the vector represents
*      a length. Words of length 2 map to index 0 and so on thereby enabling simple arithmetic to identify
*      the desired word length bucket at lookup time.
*   2) Creates a sub-level of vectors of size 26. This one should be pretty obvious, so I will not ask my
*      rhetorical question this time. The alphabet is represented at this level. The neat thing about the
*      solution below is that you do not see me leverage an expensive map structure of any kind. Here again
*      we can leverage simple arithmetic and known ASCII tables to identify the desired alphabet bucket at
*      lookup time. In case confirmation is needed, 'A' maps to index 0 and so on.
*   3) You likely asked the question "How exactly do we identify the desired alphabet bucket?", however
*      before I answer that, let me briefly highlight that the next step in our function is to start a loop
*      to iterate through our dictionary resource. For each word, we create a Word struct composed of the
*      original word straight from the dictionary as well as a compressed version of the word, i.e., one
*      without duplicate information and sorted. Now back to the fun part which gets way more interesting in
*      the lookup. The shared pointer to Word struct is now pushed back into all sub-level vectors whose
*      index maps to a character in the compressed version of the word. Now let's see it in action!
*/
static void ReadDictionary(const std::shared_ptr<std::vector<std::shared_ptr<std::vector<std::shared_ptr<std::vector<std::shared_ptr<Word>>>>>>> dictionary)
{
  // Create length vectors and add to top-level vector
  // Max word length is 15
  // Min word length is 2
  for (auto i = 0; i < 14; ++i)
  {
    // std::vector::emplace_back is much more efficient than std::vector::push_back
    dictionary->emplace_back(std::make_shared<std::vector<std::shared_ptr<std::vector<std::shared_ptr<Word>>>>>());

    // Create alphabet vectors
    // A-Z is 26 characters
    const auto p = dictionary->at(i);
    for (auto j = 0; j < 26; ++j)
    {
      p->emplace_back(std::make_shared<std::vector<std::shared_ptr<Word>>>());
    }
  }

  // File I/O
  std::ifstream inf("english.csv");

  if (!inf)
  {
    std::cerr << "Dictionary resource could not be opened." << std::endl;
    exit(1);
  }

  while (inf)
  {
    std::string w;
    std::getline(inf, w);

    if (w.empty())
    {
      continue;
    }

    // std::shared_ptr for Word struct
    // Indirection is okay for a coding challenge, but resource acquisition rate would need to be
    // monitored more closely for a production application.
    const auto word = std::make_shared<Word>(w, Compress(w));

    // Insert shared pointer to Word in mapped index if character in compressed word.
    const char * p = (word->compressed).c_str();
    const auto top = dictionary->at(w.length() - 2);
    for (char c = *p; c; c = *++p)
    {
      const auto bot = top->at(c - 'A');
      bot->push_back(word);
    }
  }
}

/*
* Simple utility function to identify the 'smallest' lookup bucket via the Order constant defined above.
*
* NOTE: 'smallest' is in quotations because in the preload process, I did not actually identify the smallest
*       lookup bucket. Instead, I leveraged an external letter frequency study to make an educated guess as
*       to which bucket is the smallest.
*/
inline static unsigned char GetLowestFrequencyIndex(const char * known)
{
  // Index of least frequent letter in known input.
  unsigned char idx = 0;
  unsigned char max = 0;
  for (char c = *known; c; c = *++known)
  {
    if (max < Order[c - 'A'])
    {
      idx = c - 'A';
      max = Order[c - 'A'];
    }
  }

  return idx;
}

/*
* Look at how small this lookup function is! Someone must have done their homework and done some pre-optimizations.
* There are still so many more optimizations that could be made, and honestly, I may invest some time into training
* a machine learning algorithm to identify potential places for optimization. I apologize if some of this 
* commentary may be over the top for you. I am geniunely having a good time writing this with my cat sitting in my
* lap.
*
* On to the actual outline of this function. This function does as little complex work as possible to reduce the
* time required to fetch potential words for the end-user.
*   1) The known letter list is passed to our utility function which returns the index of the known letter of lowest
*      frequency based on the external dataset above. Remember that 'A' maps to index 0 and so on. Revisit the Order
*      'map' if further clarification is needed. Also, note that we create a copy of the known letter list and strip
*      it to make it more readily consumable.
*   2) Next, we remove the known letter of lowest frequency from the checklist. The known letter of lowest frequency
*      is guaranteed to exist in Words in the bucket that is retrieved via the index mentioned above.
*   3) The bucket is retrieved via the index from step 1.
*   4) We search against all Words in the bucket using the compressed version of the words for the remaining
*      known letters. Push those results into our preliminary results vector.
*   5) Now that we have narrowed the search space fairly significantly, it is time to incorporate our positional data.
*      The known positional data is compared to that of each word in the search space then the results are returned.
*/
static std::vector<std::shared_ptr<Word>> Lookup(std::shared_ptr<std::vector<std::shared_ptr<std::vector<std::shared_ptr<Word>>>>> dictionary, const char * known)
{
  // Remove positional data.
  std::string ks = _strdup(known);
  ks.erase(std::remove(ks.begin(), ks.end(), '_'), ks.end());

  // Index for lowest frequency letter. Used to retrieve the 'smallest' bucket.
  const unsigned char idx = GetLowestFrequencyIndex(ks.c_str());

  // Known minus corresponding index character. No need to check if it exists twice.
  ks.erase(std::remove(ks.begin(), ks.end(), (idx + 'A')), ks.end());

  // Create the return object and get a handle to the 'smallest' bucket.
  std::vector<std::shared_ptr<Word>> matches1;
  const auto bucket = dictionary->at(idx);
  std::copy_if(bucket->begin(), bucket->end(), std::back_inserter(matches1), [k = ks.c_str()](std::shared_ptr<Word> w) {
    auto b = k;
    auto p = w->compressed.c_str();
    for (char c = *b; c; c = *++b)
    {
      if (strchr(p, c) == nullptr)
      {
        return false;
      }
    }

    return true;
  });

  // Now that we have shrunk the search space further, apply positional data requirements.
  std::vector<std::shared_ptr<Word>> matches2;
  std::copy_if(matches1.begin(), matches1.end(), std::back_inserter(matches2), [known](std::shared_ptr<Word> w) {
    auto b = known;
    auto p = w->original.c_str();
    for (char c = *b; c; )
    {
      if (c != *p && c != '_')
      {
        return false;
      }

      c = *++b;
      ++p;
    }

    return true;
  });

  return matches2;
}

int main()
{
  auto dictionary = std::make_shared<std::vector<std::shared_ptr<std::vector<std::shared_ptr<std::vector<std::shared_ptr<Word>>>>>>>();

  // Pre-optimizations.
  std::cout << "Loading in dictionary...\n";
  ReadDictionary(dictionary);
  std::cout << "Finished loading dictionary.\n";
  
  while (true)
  {
    std::string input;

    // Ignore complex validation below for the sake of speed.
    std::cout << "Please enter the number of letters.\n";
    int length = 0;
    std::getline(std::cin, input);

    std::stringstream(input) >> length;

    if (length <= 1 && length > 15)
    {
      std::cerr << "Invalid number of letters.\n";
      exit(1);
    }

    // In an effort to optimize even further, I retrieve the shared pointer to the first desired bucket here.
    // Remember the top-level vector is indexed based on word length.
    const auto ptr = dictionary->at(length - 2);

    std::cout << "Please enter the known letters and positions capitalized and without spaces, i.e., 'B_C_'.\n";
    std::getline(std::cin, input);

    std::transform(input.begin(), input.end(), input.begin(), ::toupper);

    // Pass the first bucket and the known letters and positions.
    const auto results = Lookup(ptr, input.c_str());

    // If empty, let end user know.
    if (results.empty())
    {
      std::cout << "No known words." << std::endl;
      continue;
    }

    // All the printing. I chose printing even though file I/O would have been faster.
    // The program executes very fast, but of course standard out is a bit slow.
    std::cout << "Possible answers:\n";
    for (const auto word : results)
    {
      std::cout << word->original << std::endl;
    }
    std::cout << std::endl;

    // No cleanup here. Execution is contained, and it is fine to let the OS cleanup for us.
  }  
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
