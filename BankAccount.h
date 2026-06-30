#ifndef BANKACCOUNT_H
#define BANKACCOUNT_H

#include <string>
#include <ctime>

/*
 * BankAccount
 * -----------
 * Represents a single customer account.
 *
 * NOTE ON FILE LAYOUT:
 * All members are fixed-size (no std::string, no pointers) so that an
 * object of this class can be written to / read from a binary file with
 * a single read()/write() call. This is what makes random-access
 * "find record #N and overwrite it" updates possible in Bank.cpp.
 *
 * NOTE ON SECURITY:
 * The PIN is never stored in plain text. It is passed through a simple
 * reversible XOR cipher before being written to disk (see encryptPin /
 * decryptPin in BankAccount.cpp). This is NOT cryptographically strong
 * security -- it is meant to demonstrate the *concept* of not persisting
 * secrets in plain text. A production system would use a proper salted
 * hash (e.g. bcrypt) and would never need to decrypt the PIN at all
 * (only compare hashes).
 */
class BankAccount {
private:
    int   accountNumber;
    char  name[50];
    char  pin[7];          // holds the ENCRYPTED pin bytes, not plain text
    char  accountType[12]; // "Savings" or "Current"
    double balance;
    time_t createdOn;

    static std::string encryptPin(const std::string &rawPin);
    static std::string decryptPin(const std::string &encPin);

public:
    static const double MIN_SAVINGS_BALANCE; // minimum balance a savings account must keep

    BankAccount();
    BankAccount(int accNum,
                const std::string &holderName,
                const std::string &rawPin,
                double initialBalance,
                const std::string &type);

    // Core banking operations
    void deposit(double amount);
    bool withdraw(double amount);   // returns false if funds/min-balance rule violated
    double getBalance() const;

    // Getters
    int getAccountNumber() const;
    std::string getName() const;
    std::string getAccountType() const;
    time_t getCreatedOn() const;

    // Security / account maintenance
    bool verifyPin(const std::string &inputPin) const;
    void setPin(const std::string &newRawPin);
    void setName(const std::string &newName);

    // Display helpers
    void display() const;       // full detail view
    void displayRow() const;    // one-line summary (used in "list all accounts")
};

#endif // BANKACCOUNT_H
