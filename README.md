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

## ECC lecture example

`ECC/ecc.h` declares the point/curve structures and functions. `ECC/ecc.cpp`
implements modular arithmetic, curve/point validation, negation, addition
(including doubling), double-and-add scalar multiplication, point order,
public-key derivation and point ElGamal encryption/decryption.
The three starter functions previously in `ECC/main.cpp` now live in `ECC/ecc.cpp`.
`ECC/main.cpp` demonstrates lecture slides 55-58; `ECC/tests.cpp` checks the
lecture results, edge cases and group laws on both small E23 curves.

To build ECC without installing the separate RSA example's GMP dependency:

```powershell
cmake -S . -B build-ecc -DBUILD_RSA=OFF
cmake --build build-ecc --config Release
ctest --test-dir build-ecc -C Release --output-on-failure
.\build-ecc\Release\ecc.exe
```

With a single-configuration generator, run `./build-ecc/ecc` instead.
The original targets are preserved; `BUILD_RSA` defaults to `ON`.

The demo uses E257(0,-4), G=(2,2), private key 101 and k=41. The first
positive multiple of G reaching infinity is 129, so its order is 129.
Expected output:

```text
Order of G: 129
Bob's public key: (197, 167)
Message: (112, 26)
k * public key: (68, 84)
C1: (136, 128)
C2: (246, 174)
Recovered message: (112, 26)
```

This is an educational implementation for small prime fields, with
3 < p <= 2^31 - 1. It uses `long long`, reduces intermediate products,
rejects singular curves and requires finite coordinates in [0,p-1].
`Point{}` means infinity. Negative scalars are supported; invalid inputs
throw `std::invalid_argument`. G and n are only needed for key/encryption
functions. Point-order discovery uses repeated addition and is intended
for the lecture's small curves, not large-curve performance measurements.

`generateKeys(privateKey, curve)` derives a key pair from a supplied private
scalar. `encryptWithK(message, publicKey, k, curve)` takes an already encoded
message point and the recipient's public key, which must be derived from G.
Both scalars must be between 1 and n-1. An infinity shared mask is rejected
(possible with composite-order examples). Callers are responsible for choosing
private scalars and a fresh random k; no random generation or text encoding is
included. The fixed demo values reproduce the lecture and are not secure keys.
This implementation is not intended for protecting real data.

Performance measurements, graphs and the written assignment report remain
separate work; the empty `ECC/benchmark.cpp` is left for that step.
