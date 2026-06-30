#ifndef BANK_H
#define BANK_H

#include "BankAccount.h"
#include <string>

/*
 * Bank
 * ----
 * Owns all persistence logic. Nothing outside this class ever touches
 * the data files directly -- main.cpp only ever talks to Bank, and Bank
 * is the only place that opens accounts.dat / transactions.log.
 *
 * accounts.dat is a binary file: a flat sequence of BankAccount records,
 * written with .write(reinterpret_cast<char*>(&acc), sizeof(BankAccount)).
 * Because every account is a fixed-size record, a given account can be
 * located, updated in place (seekp + overwrite), or removed (rewrite the
 * file without it) without needing a database engine.
 */
class Bank {
private:
    std::string accountsFile;
    std::string logFile;

    void logTransaction(int accNum, const std::string &type, double amount, double balanceAfter);
    bool overwriteRecord(const BankAccount &updated); // find by account number, rewrite that one record

public:
    explicit Bank(const std::string &accFile = "accounts.dat",
                  const std::string &transactionLogFile = "transactions.log");

    // Account lifecycle
    int  createAccount(const std::string &name, const std::string &pin,
                        double initialDeposit, const std::string &type);
    bool closeAccount(int accNum, const std::string &pin, double &refundedBalance);

    // Authenticated operations (account number + PIN required for all of these)
    bool authenticate(int accNum, const std::string &pin, BankAccount &outAcc);
    bool deposit(int accNum, const std::string &pin, double amount, double &newBalance);
    bool withdraw(int accNum, const std::string &pin, double amount, double &newBalance);
    bool getBalance(int accNum, const std::string &pin, double &balance);
    bool getAccountDetails(int accNum, const std::string &pin, BankAccount &acc);
    bool changePin(int accNum, const std::string &oldPin, const std::string &newPin);
    bool changeName(int accNum, const std::string &pin, const std::string &newName);

    // Administrative
    void displayAllAccounts() const;
    int  countAccounts() const;
    bool accountExists(int accNum) const;
};

#endif // BANK_H
