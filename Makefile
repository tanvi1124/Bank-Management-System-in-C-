CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2
TARGET = bank_app
SOURCES = main.cpp Bank.cpp BankAccount.cpp

$(TARGET): $(SOURCES) Bank.h BankAccount.h
	$(CXX) $(CXXFLAGS) $(SOURCES) -o $(TARGET)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET) accounts.dat transactions.log

.PHONY: run clean
