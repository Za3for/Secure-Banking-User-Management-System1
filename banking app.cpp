#include <iostream>
#include <fstream>
#include <string>
#include <ctime>
#include <vector>
#include <iomanip>

using namespace std;

// Colors for better UI
#define RESET "\033[0m"
#define RED "\033[31m"
#define GREEN "\033[32m"
#define YELLOW "\033[33m"
#define CYAN "\033[36m"

const char ENCRYPTION_KEY = 'K';

// --- Utility Functions ---

string processCipher(string data)
{
    string output = data;
    for (int i = 0; i < data.size(); i++)
    {
        output[i] = data[i] ^ ENCRYPTION_KEY;
    }
    return output;
}

string sanitize(string input)
{
    string clean = "";
    for (char c : input)
    {
        if (c != '|' && c != '\n')
            clean += c;
    }
    return clean;
}

vector<string> split(string s, char del)
{
    vector<string> tokens;
    size_t start = 0, end = s.find(del);
    while (end != string::npos)
    {
        tokens.push_back(s.substr(start, end - start));
        start = end + 1;
        end = s.find(del, start);
    }
    tokens.push_back(s.substr(start));
    return tokens;
}

int getValidatedInt(string prompt)
{
    int value;
    while (true)
    {
        cout << prompt;
        if (cin >> value)
            return value;
        cout << RED << "[!] Invalid input. Numbers only.\n"
             << RESET;
        cin.clear();
        cin.ignore(1000, '\n');
    }
}

string getValidatedPIN()
{
    string pin;
    while (true)
    {
        cout << "Set 4-Digit PIN: ";
        cin >> pin;
        bool isDigit = true;
        for (char c : pin)
            if (!isdigit(c))
                isDigit = false;
        if (pin.length() == 4 && isDigit)
            return pin;
        cout << RED << "[!] Error: PIN must be exactly 4 digits.\n"
             << RESET;
    }
}

// --- Data Management ---

bool isNameTaken(string name)
{
    ifstream inFile("bank_system.dat");
    string line;
    if (inFile.is_open())
    {
        while (getline(inFile, line))
        {
            string decrypted = processCipher(line);
            vector<string> data = split(decrypted, '|');
            if (data.size() > 1 && data[1] == name)
                return true;
        }
    }
    return false;
}

void saveUserToFile(string id, string name, string age, string city, string pass, double balance)
{
    string record = id + "|" + name + "|" + age + "|" + city + "|" + pass + "|" + to_string(balance);
    string encrypted = processCipher(record);
    ofstream outFile("bank_system.dat", ios::app);
    if (outFile.is_open())
    {
        outFile << encrypted << endl;
        outFile.close();
    }
}

// Update balance in file after transactions
void updateFileData(string id, double newBalance)
{
    ifstream inFile("bank_system.dat");
    vector<string> allUsers;
    string line;
    if (inFile.is_open())
    {
        while (getline(inFile, line))
        {
            string decrypted = processCipher(line);
            vector<string> data = split(decrypted, '|');
            if (data[0] == id)
            {
                data[5] = to_string(newBalance);
                string updatedLine = data[0];
                for (int i = 1; i < data.size(); i++)
                    updatedLine += "|" + data[i];
                allUsers.push_back(processCipher(updatedLine));
            }
            else
            {
                allUsers.push_back(line);
            }
        }
        inFile.close();
        ofstream outFile("bank_system.dat");
        for (const string &u : allUsers)
            outFile << u << endl;
    }
}

// --- Features ---

void adminSearchUser()
{
    string searchName, line;
    cout << CYAN << "\n[ADMIN] Enter name to search: " << RESET;
    getline(cin >> ws, searchName);
    ifstream inFile("bank_system.dat");
    bool found = false;
    if (inFile.is_open())
    {
        while (getline(inFile, line))
        {
            string decrypted = processCipher(line);
            vector<string> data = split(decrypted, '|');
            if (data.size() >= 6 && data[1].find(searchName) != string::npos)
            {
                if (!found)
                {
                    cout << "\n"
                         << left << setw(10) << "ID" << setw(15) << "NAME" << setw(10) << "AGE" << setw(15) << "CITY" << setw(15) << "BALANCE" << endl;
                    cout << string(65, '-') << endl;
                }
                cout << left << setw(10) << data[0] << setw(15) << data[1] << setw(10) << data[2] << setw(15) << data[3] << setw(15) << "$" + data[5] << endl;
                found = true;
            }
        }
        if (!found)
            cout << RED << "[!] No user matches your search." << RESET << endl;
        inFile.close();
    }
}

void adminShowAllUsers()
{
    ifstream inFile("bank_system.dat");
    string line;
    bool hasData = false;
    if (inFile.is_open())
    {
        cout << CYAN << "\n--- ALL REGISTERED USERS ---" << RESET << endl;
        cout << left << setw(10) << "ID" << setw(15) << "NAME" << setw(10) << "AGE" << setw(15) << "CITY" << setw(15) << "BALANCE" << endl;
        cout << string(70, '=') << endl;
        while (getline(inFile, line))
        {
            string decrypted = processCipher(line);
            vector<string> data = split(decrypted, '|');
            if (data.size() >= 6)
            {
                cout << left << setw(10) << data[0] << setw(15) << data[1] << setw(10) << data[2] << setw(15) << data[3] << setw(15) << "$" + data[5] << endl;
                hasData = true;
            }
        }
        if (!hasData)
            cout << YELLOW << "[!] System is empty." << RESET << endl;
        inFile.close();
    }
}

void registerUser()
{
    string name, city, pass;
    int age;
    cout << "\n--- New Registration ---" << endl;
    cout << "Name: ";
    getline(cin >> ws, name);
    name = sanitize(name);
    if (isNameTaken(name))
    {
        cout << RED << "[!] Error: Username taken.\n"
             << RESET;
        return;
    }
    age = getValidatedInt("Age: ");
    cout << "City: ";
    getline(cin >> ws, city);
    pass = getValidatedPIN();
    string id = to_string(1000 + rand() % 9000);
    saveUserToFile(id, name, to_string(age), sanitize(city), pass, 0.0);
    cout << GREEN << "[SUCCESS] Created ID: " << id << RESET << endl;
}

bool loginUser(string &uName, double &uBalance, string &uId)
{
    string inputName, inputPass, line;
    cout << "\n--- Login ---" << endl;
    cout << "Username: ";
    getline(cin >> ws, inputName);
    cout << "4-Digit PIN: ";
    cin >> inputPass;
    ifstream inFile("bank_system.dat");
    if (inFile.is_open())
    {
        while (getline(inFile, line))
        {
            string decrypted = processCipher(line);
            vector<string> tokens = split(decrypted, '|');
            if (tokens.size() >= 6 && tokens[1] == inputName && tokens[4] == inputPass)
            {
                uId = tokens[0];
                uName = tokens[1];
                uBalance = stod(tokens[5]);
                return true;
            }
        }
    }
    return false;
}

// --- Main System ---

int main()
{
    srand(time(0));
    int choice = 0;
    while (true)
    {
        cout << YELLOW << "\n==========================================" << RESET;
        cout << "\n     SECURE BANKING SYSTEM v4.0 (PIN)     ";
        cout << YELLOW << "\n==========================================" << RESET;
        cout << "\n1. Register Account\n2. User Login\n3. Admin: Search User\n4. Admin: View All Users\n5. Exit System\nChoice: ";

        if (!(cin >> choice))
        {
            cin.clear();
            cin.ignore(1000, '\n');
            continue;
        }

        if (choice == 1)
        {
            registerUser();
        }
        else if (choice == 2)
        {
            string uName, uId;
            double uBalance;
            if (loginUser(uName, uBalance, uId))
            {
                int bChoice = 0;
                while (bChoice != 4)
                {
                    cout << GREEN << "\nWelcome " << uName << " (ID: " << uId << ")" << RESET;
                    cout << "\n1. Balance\n2. Deposit\n3. Withdraw\n4. Logout\nChoice: ";
                    cin >> bChoice;
                    if (bChoice == 1)
                        cout << "Current Balance: $" << uBalance << endl;
                    else if (bChoice == 2)
                    {
                        double amt;
                        cout << "Deposit Amount: ";
                        cin >> amt;
                        uBalance += amt;
                        updateFileData(uId, uBalance);
                    }
                    else if (bChoice == 3)
                    {
                        double amt;
                        cout << "Withdraw Amount: ";
                        cin >> amt;
                        if (amt <= uBalance)
                        {
                            uBalance -= amt;
                            updateFileData(uId, uBalance);
                        }
                        else
                            cout << RED << "Insufficient funds!" << RESET << endl;
                    }
                }
            }
            else
                cout << RED << "[!] Invalid credentials." << RESET << endl;
        }
        else if (choice == 3)
        {
            adminSearchUser();
        }
        else if (choice == 4)
        {
            adminShowAllUsers();
        }
        else if (choice == 5)
        {
            break;
        }
    }
    return 0;
}