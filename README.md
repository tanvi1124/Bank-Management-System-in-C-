# Bank Management Application (C++)

A console-based banking system built with object-oriented programming and
file handling, as required by the assignment brief: deposit, withdrawal,
balance check, and persistent, securely-stored customer records.

## Files

| File              | Purpose                                                              |
|-------------------|-----------------------------------------------------------------------|
| `BankAccount.h/.cpp` | The `BankAccount` class — one customer's record (name, account number, balance, encrypted PIN, account type) plus the operations on it (`deposit`, `withdraw`, etc). |
| `Bank.h/.cpp`        | The `Bank` class — owns all file I/O. Creates, finds, updates, and deletes account records in `accounts.dat`, and writes every transaction to `transactions.log`. |
| `main.cpp`           | The console menu and input validation. Talks only to `Bank`, never touches files directly. |
| `Makefile`           | One-command build. |

## How to build and run

```bash
make        # compiles to ./bank_app
./bank_app
```

or directly:

```bash
g++ -std=c++17 -Wall main.cpp Bank.cpp BankAccount.cpp -o bank_app
./bank_app
```

No external libraries are required — only the C++ standard library.

## Features

- **Open a new account** — Savings or Current, with a minimum opening
  deposit of Rs. 500 for Savings accounts. Account numbers are assigned
  automatically starting at 1001.
- **Deposit** — adds funds to an account after verifying account number + PIN.
- **Withdraw** — verifies funds are sufficient and that a Savings account
  won't drop below the Rs. 500 minimum balance.
- **Balance enquiry** — quick balance check after authentication.
- **View account details** — full record (name, type, balance, date opened).
- **Change PIN** — requires the current PIN.
- **Close account** — requires confirmation; removes the record from the file.
- **Admin: view all accounts** — lists every account on file, gated behind a
  separate passcode (`9999` in this demo) to show that administrative
  functions should have their own access control, separate from customer PINs.

Every successful transaction (account opened, deposit, withdrawal, account
closed) is appended to `transactions.log` with a timestamp, giving a simple
audit trail.

## Object-oriented design

- **Encapsulation** — all of `BankAccount`'s fields are private; everything
  goes through methods (`deposit()`, `withdraw()`, `verifyPin()`, …).
- **Separation of concerns** — `BankAccount` knows nothing about files;
  `Bank` knows nothing about the console; `main.cpp` knows nothing about
  file formats. Each class has one job.
- **Data hiding for security** — the PIN is never stored or compared in
  plain text. It's passed through a reversible XOR cipher before being
  written to disk, so opening `accounts.dat` in a hex editor does not
  reveal customer PINs directly.

## How persistence works

`accounts.dat` is a binary file. Every `BankAccount` object is a
fixed-size, "plain old data" record (fixed-length `char` arrays instead of
`std::string`, no pointers), so the whole object can be written or read in
one call:

```cpp
file.write(reinterpret_cast<char*>(&account), sizeof(BankAccount));
```

This is what lets `Bank::overwriteRecord()` find one customer's record by
scanning the file, then seek back to that exact byte offset and overwrite
just that record — an update without needing a database engine.

## Notes on security (important for grading / honesty)

This project demonstrates the *concept* of not persisting secrets in plain
text, using a simple reversible XOR cipher for the PIN. **This is not
real cryptographic security** — a production banking system would:
- Hash PINs/passwords with a salted algorithm (e.g. bcrypt/Argon2) and
  never decrypt them, only compare hashes.
- Encrypt the data file at rest and over any network transport.
- Use a real authentication/authorization layer instead of a hardcoded
  admin passcode.

These simplifications are called out in code comments and are reasonable
for an academic/learning project, but should not be mistaken for
production-grade security.

## Tested behavior

The program was built and exercised end-to-end (open account → deposit →
withdraw → balance check → view details → change PIN → admin view →
re-launch to confirm persistence) with both successful and failing inputs
(wrong PIN, withdrawal that would breach the minimum balance). All cases
behaved as expected.
