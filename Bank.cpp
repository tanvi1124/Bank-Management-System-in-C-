#include "Bank.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <ctime>
#include <iomanip>
#include <cstring>

Bank::Bank(const std::string &accFile, const std::string &transactionLogFile)
    : accountsFile(accFile), logFile(transactionLogFile) {
    // Make sure the data file exists so later ifstream opens don't fail.
    std::ofstream touch(accountsFile, std::ios::app | std::ios::binary);
    touch.close();
}

void Bank::logTransaction(int accNum, const std::string &type, double amount, double balanceAfter) {
    std::ofstream log(logFile, std::ios::app);
    if (!log) return;

    time_t now = std::time(nullptr);
    char timeBuf[26];
    std::strncpy(timeBuf, std::ctime(&now), sizeof(timeBuf));
    // ctime() output ends with '\n' -- strip it for a clean one-line log entry
    for (char &c : timeBuf) if (c == '\n') c = '\0';

    log << "[" << timeBuf << "] Acc#" << accNum
        << " | " << type
        << " | Amount: " << std::fixed << std::setprecision(2) << amount
        << " | Balance After: " << balanceAfter << "\n";
}

int Bank::countAccounts() const {
    std::ifstream in(accountsFile, std::ios::binary);
    if (!in) return 0;
    BankAccount tmp;
    int count = 0;
    while (in.read(reinterpret_cast<char*>(&tmp), sizeof(BankAccount))) {
        count++;
    }
    return count;
}

bool Bank::accountExists(int accNum) const {
    std::ifstream in(accountsFile, std::ios::binary);
    if (!in) return false;
    BankAccount tmp;
    while (in.read(reinterpret_cast<char*>(&tmp), sizeof(BankAccount))) {
        if (tmp.getAccountNumber() == accNum) return true;
    }
    return false;
}

int Bank::createAccount(const std::string &name, const std::string &pin,
                         double initialDeposit, const std::string &type) {
    int newAccNum = 1001 + countAccounts(); // simple, deterministic numbering scheme
    BankAccount acc(newAccNum, name, pin, initialDeposit, type);

    std::ofstream out(accountsFile, std::ios::binary | std::ios::app);
    if (!out) return -1;
    out.write(reinterpret_cast<char*>(&acc), sizeof(BankAccount));
    out.close();

    logTransaction(newAccNum, "ACCOUNT OPENED", initialDeposit, initialDeposit);
    return newAccNum;
}

bool Bank::overwriteRecord(const BankAccount &updated) {
    std::fstream io(accountsFile, std::ios::binary | std::ios::in | std::ios::out);
    if (!io) return false;

    BankAccount tmp;
    std::streampos pos = 0;
    while (io.read(reinterpret_cast<char*>(&tmp), sizeof(BankAccount))) {
        if (tmp.getAccountNumber() == updated.getAccountNumber()) {
            io.seekp(pos);
            io.write(reinterpret_cast<const char*>(&updated), sizeof(BankAccount));
            return true;
        }
        pos = io.tellg();
    }
    return false; // account number not found
}

bool Bank::authenticate(int accNum, const std::string &pin, BankAccount &outAcc) {
    std::ifstream in(accountsFile, std::ios::binary);
    if (!in) return false;

    BankAccount tmp;
    while (in.read(reinterpret_cast<char*>(&tmp), sizeof(BankAccount))) {
        if (tmp.getAccountNumber() == accNum && tmp.verifyPin(pin)) {
            outAcc = tmp;
            return true;
        }
    }
    return false;
}

bool Bank::deposit(int accNum, const std::string &pin, double amount, double &newBalance) {
    if (amount <= 0) return false;
    BankAccount acc;
    if (!authenticate(accNum, pin, acc)) return false;

    acc.deposit(amount);
    if (!overwriteRecord(acc)) return false;

    newBalance = acc.getBalance();
    logTransaction(accNum, "DEPOSIT", amount, newBalance);
    return true;
}

bool Bank::withdraw(int accNum, const std::string &pin, double amount, double &newBalance) {
    if (amount <= 0) return false;
    BankAccount acc;
    if (!authenticate(accNum, pin, acc)) return false;

    if (!acc.withdraw(amount)) return false; // insufficient funds / breaches min balance
    if (!overwriteRecord(acc)) return false;

    newBalance = acc.getBalance();
    logTransaction(accNum, "WITHDRAWAL", amount, newBalance);
    return true;
}

bool Bank::getBalance(int accNum, const std::string &pin, double &balance) {
    BankAccount acc;
    if (!authenticate(accNum, pin, acc)) return false;
    balance = acc.getBalance();
    return true;
}

bool Bank::getAccountDetails(int accNum, const std::string &pin, BankAccount &acc) {
    return authenticate(accNum, pin, acc);
}

bool Bank::changePin(int accNum, const std::string &oldPin, const std::string &newPin) {
    BankAccount acc;
    if (!authenticate(accNum, oldPin, acc)) return false;
    acc.setPin(newPin);
    return overwriteRecord(acc);
}

bool Bank::changeName(int accNum, const std::string &pin, const std::string &newName) {
    BankAccount acc;
    if (!authenticate(accNum, pin, acc)) return false;
    acc.setName(newName);
    return overwriteRecord(acc);
}

bool Bank::closeAccount(int accNum, const std::string &pin, double &refundedBalance) {
    BankAccount target;
    if (!authenticate(accNum, pin, target)) return false;
    refundedBalance = target.getBalance();

    // Rewrite the file without this one record.
    std::ifstream in(accountsFile, std::ios::binary);
    if (!in) return false;
    std::vector<BankAccount> remaining;
    BankAccount tmp;
    while (in.read(reinterpret_cast<char*>(&tmp), sizeof(BankAccount))) {
        if (tmp.getAccountNumber() != accNum) remaining.push_back(tmp);
    }
    in.close();

    std::ofstream out(accountsFile, std::ios::binary | std::ios::trunc);
    if (!out) return false;
    for (const auto &a : remaining) {
        out.write(reinterpret_cast<const char*>(&a), sizeof(BankAccount));
    }
    out.close();

    logTransaction(accNum, "ACCOUNT CLOSED", refundedBalance, 0.0);
    return true;
}

void Bank::displayAllAccounts() const {
    std::ifstream in(accountsFile, std::ios::binary);
    if (!in) {
        std::cout << "No accounts found.\n";
        return;
    }

    BankAccount tmp;
    int count = 0;
    std::cout << std::left << std::setw(10) << "AccNo"
               << std::setw(22) << "Name"
               << std::setw(12) << "Type"
               << "Balance\n";
    std::cout << "--------------------------------------------------------\n";
    while (in.read(reinterpret_cast<char*>(&tmp), sizeof(BankAccount))) {
        tmp.displayRow();
        count++;
    }
    if (count == 0) std::cout << "No accounts found.\n";
}
