# Blockchain_project1

A beginner C++ project for learning how a Feistel cipher works. The program starts with two integer halves, encrypts them, then decrypts them to recover the original values.

This is a toy learning cipher and is not secure for protecting real data.

## What it demonstrates

- 16 Feistel rounds.
- A simple round function: `right ^ key`, where `^` means bitwise XOR.
- Round keys generated with `masterKey + roundIndex`.
- Decryption using the round keys in reverse order.

The code is in [feistel/feistel.cpp](feistel/feistel.cpp). Try changing `left`, `right`, or `masterKey` in `main()` and running it again.

## Build and run

Install CMake and a C++ compiler, then run these commands from the project folder:

```sh
cmake -S . -B build
cmake --build build --config Release
```

On Windows with the Visual Studio generator, run:

```powershell
.\build\Release\feistel.exe
```

On Linux with a single-configuration generator, run:

```sh
./build/feistel
```

## Example output

With `left = 5`, `right = 9`, and `masterKey = 30`:

```text
Before: 5 | 9
Encrypt:  50 | 23
decrypt:  5 | 9
```
