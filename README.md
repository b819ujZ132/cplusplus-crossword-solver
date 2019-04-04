# cplusplus-crossword-solver
A crossword puzzle solver.

## How it works
1. Loads dictionary resource into memory. Top-level map index is based on word length, and bottom-level map index is based on alphabet.
2. User input is consumed.
3. Inputted length is used to retrieve bottom-level map.
4. The bucket in the bottom-level map is selected based on the lowest frequency letter of the input. This reduces the search space significantly. Take a look at https://en.wikipedia.org/wiki/Letter_frequency.
5. Each letter in our compressed input (_s removed and sorted) is compared to each compressed word (words are stored in original form and compressed, i.e., { original = "banana", compressed = "abn" }) in our bucket. If a letter is not found, the loop short circuits and continues onto the next word in the bucket. Otherwise, the word is added to a new bucket for one last passthrough.
6. The final passthrough now takes into account the positional data the user inputted, i.e., "_ANA_A". Each original word in our recently created bucket is compared to the unmodified user input. If an underscore is hit, that position is ignored. Otherwise, the characters of every position are compared. If all characters at known positions match, the word is added to our return bucket.

NOTE: No actual maps were used here because that would be inefficient.

## Inputs
Input comes in two steps.

1. User inputs an integer value representing the length of the word (Ex. 5).
2. User inputs a string value representing the known characters at the known positions (Ex. A_PL_).

## Outputs
Output is piped to stdout. With inputs that result in a large output, the stdout lag is noticeable. One can always pipe the output to a file if need be.
