# DES and AES assignment

C++17 implementation of Twofish for parts 1 and 2. The professor has approved
using Twofish for the Feistel cipher exercise.

## Part 1: the cipher

We use the full [Twofish design](https://www.schneier.com/wp-content/uploads/2016/02/paper-twofish-paper.pdf)
with these choices:

| Choice | Explanation |
| --- | --- |
| 128-bit blocks | Twofish's standard block size, split into four 32-bit words that form two 64-bit halves. |
| 16 rounds | We keep the full Twofish round count so we implement the original algorithm rather than a reduced version. |
| Round function F | Nonlinear S-boxes, matrix mixing, rotations, and addition combine the data with two round keys. This is stronger than a trivial XOR-only construction. |
| Key schedule | A 128-, 192-, or 256-bit key produces S-box key material and 40 subkeys: 8 for input/output whitening and 32 for the rounds. We keep the standard schedule to match Twofish. |

**Encryption:** XOR the block with the input whitening keys, run the 16 Feistel
rounds, then apply the final word ordering and output whitening. Each round
computes F from one half, mixes it into the other half, and swaps the halves.

**Decryption:** undo output whitening and word ordering, run the rounds backwards
with the round keys in reverse order, and undo input whitening. The XORs and
rotations are reversed; F itself does not need to be inverted.

**Confusion:** the nonlinear, key-dependent S-boxes make the relationship between
the key and ciphertext complicated.

**Diffusion:** matrix mixing, word additions, rotations, and repeated rounds
spread changes across the block.

[text.cpp](feistel/text.cpp) handles messages of any length with PKCS#7 padding.
It always adds padding, including for empty input and exact block lengths.
Decryption checks and removes that padding to recover the original bytes.
Blocks are encrypted independently (ECB), so repeated blocks remain visible.

## Part 2: correctness tests

[feistel_tests.cpp](tests/feistel_tests.cpp) is one simple test program. It uses
five random keys of each supported size and checks 12 messages for each key:

- Empty input, one character, odd-length text, and ordinary text.
- Messages of 15, 16, and 17 bytes, around the block boundary.
- A repeated `ABCD` pattern and a longer 1,000-byte message.
- Embedded NUL bytes, all 256 byte values, and random binary data.

For each case, it verifies `decrypt(encrypt(message, key), key) == message`,
including every binary byte. The random generator uses seed 42 so the tests
are repeatable. Three published Twofish vectors also check encryption and
decryption against known results, one for each key size.

Result: **180 message tests and 3 published test vectors passed** on Windows
with an MSVC Release build. A failure prints a message and returns a nonzero
exit code.

## Build and run

```sh
cmake -S . -B build
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
```

On Windows, run the demo or see the test output directly:

```powershell
.\build\Release\feistel.exe
.\build\Release\feistel_tests.exe
```

With a single-configuration generator, the executables are `build/feistel`
and `build/feistel_tests` instead.

The implementation and tests cover parts 1 and 2. Include the explanation,
source code, test cases, and results in the assignment's final PDF report.
