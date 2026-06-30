/*
 * Bank Management Application
 * ----------------------------
 * A console-based banking system demonstrating:
 *   - Object-Oriented Programming (BankAccount, Bank classes;
 *     encapsulation, constructors, member functions)
 *   - File handling for persistent storage (accounts.dat, transactions.log)
 *   - Core banking operations: open account, deposit, withdraw,
 *     balance inquiry, account details, PIN/name change, close account
 *
 * Compile:  g++ -std=c++17 -Wall main.cpp Bank.cpp BankAccount.cpp -o bank_app
 * Run:      ./bank_app
 */

#include <iostream>
#include <limits>
#include <iomanip>
#include <algorithm>
#include <cctype>
#include <cstdlib>
#include "Bank.h"

// ---------- small input-helper functions ----------

void clearInputError() {
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

// If stdin has been closed/exhausted (e.g. input redirected from a file
// that ran out, or the terminal sent EOF), there is no way to get further
// input. Without this check, the read* loops below would spin forever
// re-printing their error message. Exit cleanly instead.
void exitIfInputClosed() {
    if (std::cin.eof()) {
        std::cout << "\nInput stream closed unexpectedly. Exiting.\n";
        std::exit(0);
    }
}

int readInt(const std::string &prompt) {
    int value;
    while (true) {
        std::cout << prompt;
        if (std::cin >> value) {
            clearInputError();
            return value;
        }
        exitIfInputClosed();
        std::cout << "Invalid input. Please enter a whole number.\n";
        clearInputError();
    }
}

double readPositiveDouble(const std::string &prompt) {
    double value;
    while (true) {
        std::cout << prompt;
        if (std::cin >> value && value > 0) {
            clearInputError();
            return value;
        }
        exitIfInputClosed();
        std::cout << "Invalid input. Please enter a positive amount.\n";
        clearInputError();
    }
}

std::string readLine(const std::string &prompt) {
    std::string value;
    std::cout << prompt;
    std::getline(std::cin, value);
    exitIfInputClosed();
    return value;
}

// PIN is restricted to 4-6 digits. This restriction is what makes the
// simple XOR-based encryption in BankAccount safe to use as a fixed-size
// null-terminated buffer (see BankAccount::verifyPin for why).
std::string readPin(const std::string &prompt) {
    std::string pin;
    while (true) {
        pin = readLine(prompt);
        bool valid = !pin.empty() && pin.size() <= 6 &&
                     std::all_of(pin.begin(), pin.end(), ::isdigit);
        if (valid) return pin;
        std::cout << "PIN must be 4-6 digits only. Please try again.\n";
    }
}

std::string readAccountType() {
    while (true) {
        std::cout << "Account type (1 = Savings, 2 = Current): ";
        int choice;
        if (std::cin >> choice) {
            clearInputError();
            if (choice == 1) return "Savings";
            if (choice == 2) return "Current";
        } else {
            exitIfInputClosed();
            clearInputError();
        }
        std::cout << "Please enter 1 or 2.\n";
    }
}

// ---------- menu actions ----------

void doCreateAccount(Bank &bank) {
    std::cout << "\n--- Open New Account ---\n";
    std::string name = readLine("Full name: ");
    std::string pin = readPin("Set a 4-6 digit PIN: ");
    std::string type = readAccountType();

    double minDeposit = (type == "Savings") ? BankAccount::MIN_SAVINGS_BALANCE : 0.0;
    std::cout << "Minimum opening deposit for " << type << " account: Rs. "
               << std::fixed << std::setprecision(2) << minDeposit << "\n";

    double deposit;
    while (true) {
        deposit = readPositiveDouble("Initial deposit amount: Rs. ");
        if (deposit >= minDeposit) break;
        std::cout << "Initial deposit must be at least Rs. " << minDeposit << ".\n";
    }

    int accNum = bank.createAccount(name, pin, deposit, type);
    if (accNum > 0) {
        std::cout << "\nAccount created successfully!\n";
        std::cout << "Your account number is: " << accNum << "\n";
        std::cout << "Please remember your account number and PIN -- you will need both for every transaction.\n";
    } else {
        std::cout << "Account creation failed due to a file error.\n";
    }
}

void doDeposit(Bank &bank) {
    std::cout << "\n--- Deposit ---\n";
    int accNum = readInt("Account number: ");
    std::string pin = readPin("PIN: ");
    double amount = readPositiveDouble("Amount to deposit: Rs. ");

    double newBalance;
    if (bank.deposit(accNum, pin, amount, newBalance)) {
        std::cout << std::fixed << std::setprecision(2);
        std::cout << "Deposit successful. New balance: Rs. " << newBalance << "\n";
    } else {
        std::cout << "Deposit failed. Check the account number and PIN.\n";
    }
}

void doWithdraw(Bank &bank) {
    std::cout << "\n--- Withdraw ---\n";
    int accNum = readInt("Account number: ");
    std::string pin = readPin("PIN: ");
    double amount = readPositiveDouble("Amount to withdraw: Rs. ");

    double newBalance;
    if (bank.withdraw(accNum, pin, amount, newBalance)) {
        std::cout << std::fixed << std::setprecision(2);
        std::cout << "Withdrawal successful. New balance: Rs. " << newBalance << "\n";
    } else {
        std::cout << "Withdrawal failed. Possible reasons: wrong account number/PIN, "
                     "insufficient funds, or it would breach the minimum balance requirement.\n";
    }
}

void doCheckBalance(Bank &bank) {
    std::cout << "\n--- Balance Enquiry ---\n";
    int accNum = readInt("Account number: ");
    std::string pin = readPin("PIN: ");

    double balance;
    if (bank.getBalance(accNum, pin, balance)) {
        std::cout << std::fixed << std::setprecision(2);
        std::cout << "Current balance: Rs. " << balance << "\n";
    } else {
        std::cout << "Could not retrieve balance. Check the account number and PIN.\n";
    }
}

void doAccountDetails(Bank &bank) {
    std::cout << "\n--- Account Details ---\n";
    int accNum = readInt("Account number: ");
    std::string pin = readPin("PIN: ");

    BankAccount acc;
    if (bank.getAccountDetails(accNum, pin, acc)) {
        acc.display();
    } else {
        std::cout << "Could not retrieve account. Check the account number and PIN.\n";
    }
}

void doChangePin(Bank &bank) {
    std::cout << "\n--- Change PIN ---\n";
    int accNum = readInt("Account number: ");
    std::string oldPin = readPin("Current PIN: ");
    std::string newPin = readPin("New PIN: ");

    if (bank.changePin(accNum, oldPin, newPin)) {
        std::cout << "PIN updated successfully.\n";
    } else {
        std::cout << "Could not update PIN. Check the account number and current PIN.\n";
    }
}

void doCloseAccount(Bank &bank) {
    std::cout << "\n--- Close Account ---\n";
    int accNum = readInt("Account number: ");
    std::string pin = readPin("PIN: ");

    std::cout << "Are you sure you want to close this account? (y/n): ";
    std::string confirm;
    std::getline(std::cin, confirm);
    if (confirm.empty() || (confirm[0] != 'y' && confirm[0] != 'Y')) {
        std::cout << "Account closure cancelled.\n";
        return;
    }

    double refunded;
    if (bank.closeAccount(accNum, pin, refunded)) {
        std::cout << std::fixed << std::setprecision(2);
        std::cout << "Account closed. Final balance of Rs. " << refunded
                   << " has been recorded as refunded.\n";
    } else {
        std::cout << "Could not close account. Check the account number and PIN.\n";
    }
}

void doAdminView(Bank &bank) {
    // A hardcoded admin password is NOT production-safe -- this exists only
    // to demonstrate that administrative/listing functions should sit behind
    // their own access control, separate from customer PINs.
    std::string adminPass = readPin("Enter admin passcode: "); // reusing digit-only reader for simplicity
    if (adminPass != "9999") {
        std::cout << "Incorrect admin passcode.\n";
        return;
    }
    std::cout << "\n--- All Accounts (" << bank.countAccounts() << ") ---\n";
    bank.displayAllAccounts();
}

void printMenu() {
    std::cout << "\n============= BANK MANAGEMENT SYSTEM =============\n";
    std::cout << " 1. Open New Account\n";
    std::cout << " 2. Deposit\n";
    std::cout << " 3. Withdraw\n";
    std::cout << " 4. Balance Enquiry\n";
    std::cout << " 5. View Account Details\n";
    std::cout << " 6. Change PIN\n";
    std::cout << " 7. Close Account\n";
    std::cout << " 8. Admin: View All Accounts\n";
    std::cout << " 9. Exit\n";
    std::cout << "====================================================\n";
}

int main() {
    Bank bank; // uses accounts.dat and transactions.log in the current directory

    std::cout << "Welcome to the C++ Bank Management Application.\n";

    bool running = true;
    while (running) {
        printMenu();
        int choice = readInt("Enter your choice (1-9): ");

        switch (choice) {
            case 1: doCreateAccount(bank); break;
            case 2: doDeposit(bank); break;
            case 3: doWithdraw(bank); break;
            case 4: doCheckBalance(bank); break;
            case 5: doAccountDetails(bank); break;
            case 6: doChangePin(bank); break;
            case 7: doCloseAccount(bank); break;
            case 8: doAdminView(bank); break;
            case 9:
                std::cout << "Thank you for using the Bank Management Application. Goodbye!\n";
                running = false;
                break;
            default:
                std::cout << "Please choose a number between 1 and 9.\n";
        }
    }
    return 0;
}
