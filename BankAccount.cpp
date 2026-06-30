#include "BankAccount.h"
#include <iostream>
#include <iomanip>
#include <cstring>
#include <sstream>

const double BankAccount::MIN_SAVINGS_BALANCE = 500.0;

static const char XOR_KEY = 0x5A; // simple reversible XOR cipher key

std::string BankAccount::encryptPin(const std::string &rawPin) {
    std::string out = rawPin;
    for (char &c : out) c = static_cast<char>(c ^ XOR_KEY);
    return out;
}

std::string BankAccount::decryptPin(const std::string &encPin) {
    // XOR is its own inverse
    return encryptPin(encPin);
}

BankAccount::BankAccount() {
    accountNumber = 0;
    std::memset(name, 0, sizeof(name));
    std::memset(pin, 0, sizeof(pin));
    std::memset(accountType, 0, sizeof(accountType));
    balance = 0.0;
    createdOn = std::time(nullptr);
}

BankAccount::BankAccount(int accNum,
                          const std::string &holderName,
                          const std::string &rawPin,
                          double initialBalance,
                          const std::string &type) {
    accountNumber = accNum;

    std::memset(name, 0, sizeof(name));
    std::strncpy(name, holderName.c_str(), sizeof(name) - 1);

    std::memset(pin, 0, sizeof(pin));
    std::string enc = encryptPin(rawPin);
    std::strncpy(pin, enc.c_str(), sizeof(pin) - 1);

    std::memset(accountType, 0, sizeof(accountType));
    std::strncpy(accountType, type.c_str(), sizeof(accountType) - 1);

    balance = initialBalance;
    createdOn = std::time(nullptr);
}

void BankAccount::deposit(double amount) {
    balance += amount;
}

bool BankAccount::withdraw(double amount) {
    double minBalance = (std::strcmp(accountType, "Savings") == 0) ? MIN_SAVINGS_BALANCE : 0.0;
    if (balance - amount < minBalance) {
        return false; // would breach minimum balance rule / insufficient funds
    }
    balance -= amount;
    return true;
}

double BankAccount::getBalance() const {
    return balance;
}

int BankAccount::getAccountNumber() const {
    return accountNumber;
}

std::string BankAccount::getName() const {
    return std::string(name);
}

std::string BankAccount::getAccountType() const {
    return std::string(accountType);
}

time_t BankAccount::getCreatedOn() const {
    return createdOn;
}

bool BankAccount::verifyPin(const std::string &inputPin) const {
    // pin[] holds encrypted bytes followed by zero padding. Because PINs are
    // restricted to digits 0-9 (enforced by the UI layer), no encrypted byte
    // is ever 0x00, so the buffer behaves like a normal null-terminated
    // C string and this comparison is safe and exact.
    std::string storedEncrypted(pin);
    std::string inputEncrypted = encryptPin(inputPin);
    return storedEncrypted == inputEncrypted;
}

void BankAccount::setPin(const std::string &newRawPin) {
    std::memset(pin, 0, sizeof(pin));
    std::string enc = encryptPin(newRawPin);
    std::strncpy(pin, enc.c_str(), sizeof(pin) - 1);
}

void BankAccount::setName(const std::string &newName) {
    std::memset(name, 0, sizeof(name));
    std::strncpy(name, newName.c_str(), sizeof(name) - 1);
}

void BankAccount::display() const {
    char timeBuf[26];
    std::strncpy(timeBuf, std::ctime(&createdOn), sizeof(timeBuf));

    std::cout << "------------------------------------------\n";
    std::cout << "Account Number : " << accountNumber << "\n";
    std::cout << "Holder Name    : " << name << "\n";
    std::cout << "Account Type   : " << accountType << "\n";
    std::cout << std::fixed << std::setprecision(2);
    std::cout << "Balance        : Rs. " << balance << "\n";
    std::cout << "Opened On      : " << timeBuf;
    std::cout << "------------------------------------------\n";
}

void BankAccount::displayRow() const {
    std::cout << std::left << std::setw(10) << accountNumber
               << std::setw(22) << name
               << std::setw(12) << accountType
               << std::fixed << std::setprecision(2)
               << "Rs. " << balance << "\n";
}
