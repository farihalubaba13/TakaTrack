#define NOMINMAX
#define _CRT_SECURE_NO_WARNINGS

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdio>
#include <ctime>
#include <fstream>
#include <iostream>
#include <limits>
#include <map>
#include <random>
#include <sstream>
#include <string>
#include <vector>
#include <windows.h> // For colors
#include <conio.h>   // For hidden password typing (_getch)

using namespace std;

//  COLOR FUNCTIONS
enum Color
{
    BLACK = 0,
    BLUE = 1,
    GREEN = 2,
    CYAN = 3,
    RED = 4,
    MAGENTA = 5,
    YELLOW = 6,
    WHITE = 7,
    GRAY = 8,
    BRIGHT_BLUE = 9,
    BRIGHT_GREEN = 10,
    BRIGHT_CYAN = 11,
    BRIGHT_RED = 12,
    BRIGHT_MAGENTA = 13,
    BRIGHT_YELLOW = 14,
    BRIGHT_WHITE = 15
};

void setColor(Color color)
{
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, color);
}

void resetColor()
{
    setColor(WHITE);
}

void printColored(const string& text, Color color)
{
    setColor(color);
    cout << text;
    resetColor();
}

//  DATA STRUCTURES
struct FamilyMember
{
    string name;
    string relation;
    double monthlyIncome;
};

struct BudgetCategory
{
    string name;
    double allocatedAmount;
    double spentAmount;
    double availableAmount() const
    {
        return allocatedAmount - spentAmount;
    }
};

struct BudgetPlan
{
    string name;
    string district;
    double totalIncome = 0;
    vector<BudgetCategory> categories;
    string focus;
    string month;
    double actualRent = -1; // NEW: rent the user really pays (-1 = use the suggested amount)
};

struct MonthlyRecord
{
    string month;
    string year;
    BudgetPlan budgetPlan;
    double totalSpent = 0;
    double totalSaved = 0;
};

//  GLOBAL VARIABLES
string userName;
vector<FamilyMember> familyMembers;
double totalIncome = 0;
string familyType;
string district;
string costOfLivingLevel;
BudgetPlan budgetPlan;
string currentMonth;
string currentYear;
bool budgetPlanned = false;
vector<MonthlyRecord> monthlyRecords; // Store previous months
string currentUser;                   // NEW: username of the logged-in user
string currentFullName;               // NEW: full name of the logged-in user
string currentEmail;                  // NEW: email of the logged-in user

bool writeBudgetPlanFile(); // defined in FILE OPERATIONS below

//  USER DATA FILES (NEW)
// Every user has their own files, so users never see each other's data.
string planFileName()
{
    return "budget_plan_" + currentUser + ".txt";
}

string historyFileName()
{
    return "monthly_history_" + currentUser + ".txt";
}

//  ENHANCED BUDGET PLANS CATALOG
vector<BudgetPlan> budgetPlansCatalog =
{
    // Single Person Categories
    {
        "Single Student", "", 0, {
            {"Groceries & Food", 0, 0},
            {"Hostel Rent", 0, 0},
            {"Utilities", 0, 0},
            {"Transport", 0, 0},
            {"Books & Supplies", 0, 0},
            {"Emergency Fund", 0, 0},
            {"Entertainment", 0, 0},
            {"Savings", 0, 0}
        }, "Focus: Education & Basics", ""
    },

    {
        "Young Professional", "", 0, {
            {"Groceries & Toiletries", 0, 0},
            {"Rent / Home Loan", 0, 0},
            {"Utilities", 0, 0},
            {"Transport", 0, 0},
            {"Emergency Fund", 0, 0},
            {"Health Insurance", 0, 0},
            {"Retirement Savings", 0, 0},
            {"Skill Development", 0, 0},
            {"Entertainment", 0, 0},
            {"Personal Care", 0, 0}
        }, "Focus: Career Growth", ""
    },

    {
        "Single Parent (1 Child)", "", 0, {
            {"Groceries & Household", 0, 0},
            {"Rent / Home Loan", 0, 0},
            {"Utilities", 0, 0},
            {"Transport", 0, 0},
            {"Child Education", 0, 0},
            {"Child Care", 0, 0},
            {"Emergency Fund", 0, 0},
            {"Insurance", 0, 0},
            {"Savings", 0, 0},
            {"Family Activities", 0, 0}
        }, "Focus: Child Support", ""
    },

    // Couple Categories
    {
        "Newly Married Couple", "", 0, {
            {"Groceries & Household", 0, 0},
            {"Rent / Home Loan", 0, 0},
            {"Utilities", 0, 0},
            {"Transport", 0, 0},
            {"Emergency Fund", 0, 0},
            {"Insurance", 0, 0},
            {"Retirement Savings", 0, 0},
            {"Home Setup", 0, 0},
            {"Entertainment", 0, 0},
            {"Personal Spending", 0, 0}
        }, "Focus: Building Foundation", ""
    },

    {
        "Working Couple (DINK)", "", 0, {
            {"Groceries & Toiletries", 0, 0},
            {"Rent / Home Loan", 0, 0},
            {"Utilities", 0, 0},
            {"Transport", 0, 0},
            {"Emergency Fund", 0, 0},
            {"Insurance", 0, 0},
            {"Retirement Savings", 0, 0},
            {"Travel & Leisure", 0, 0},
            {"Investments", 0, 0},
            {"Personal Spending", 0, 0}
        }, "Focus: Wealth Building", ""
    },

    // Family with Children
    {
        "Young Family (1 Child)", "", 0, {
            {"Groceries & Household", 0, 0},
            {"Rent / Home Loan", 0, 0},
            {"Utilities", 0, 0},
            {"Transport", 0, 0},
            {"Child Education", 0, 0},
            {"Child Care", 0, 0},
            {"Emergency Fund", 0, 0},
            {"Insurance", 0, 0},
            {"Retirement Savings", 0, 0},
            {"Family Activities", 0, 0}
        }, "Focus: Child Development", ""
    },

    {
        "Growing Family (2 Children)", "", 0, {
            {"Groceries & Household", 0, 0},
            {"Rent / Home Loan", 0, 0},
            {"Utilities", 0, 0},
            {"Transport", 0, 0},
            {"Children Education", 0, 0},
            {"Child Care", 0, 0},
            {"Emergency Fund", 0, 0},
            {"Insurance", 0, 0},
            {"Retirement Savings", 0, 0},
            {"Extracurricular", 0, 0},
            {"Family Outings", 0, 0}
        }, "Focus: Education & Growth", ""
    }
};

//  UTILITY FUNCTIONS
void clearInputBuffer()
{
    cin.clear();
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
}

void skipLine(istream& in)
{
    in.ignore(numeric_limits<streamsize>::max(), '\n');
}

string trim(const string& text)
{
    size_t start = text.find_first_not_of(" \t\r\n");
    if (start == string::npos) return "";
    size_t end = text.find_last_not_of(" \t\r\n");
    return text.substr(start, end - start + 1);
}

string toLower(string text)
{
    transform(text.begin(), text.end(), text.begin(),
              [](unsigned char c)
    {
        return static_cast<char>(tolower(c));
    });
    return text;
}

// Adds spaces to make text a fixed width.
// Never uses a negative number of spaces.
// This prevents crashes when a number is longer than the given width.
string padRight(const string& text, size_t width)
{
    if (text.length() >= width) return text;
    return text + string(width - text.length(), ' ');
}

// Whole taka, rounded to the nearest number
string toStr(double amount)
{
    return to_string(llround(amount));
}

// Calculates a percentage safely.
// Returns 0 if the divisor is zero.
double percentOf(double part, double whole)
{
    return whole > 0 ? (part / whole) * 100.0 : 0.0;
}

// Savings categories hold money that is kept, not spent
bool isSavingsCategory(const string& categoryName)
{
    string name = toLower(categoryName);
    return name.find("emergency") != string::npos || name.find("savings") != string::npos ||
           name.find("investments") != string::npos;
}

// Rent categories ("Rent / Home Loan", "Hostel Rent")
bool isHousingCategory(const string& categoryName)
{
    string name = toLower(categoryName);
    return name.find("rent") == 0 || name.find("hostel rent") != string::npos;
}

// Checks if a file exists. The file is closed again before returning.
bool fileExists(const string& fileName)
{
    ifstream file(fileName);
    return file.good();
}

int getValidInput(int minValue, int maxValue)
{
    int input;
    while (true)
    {
        if (cin >> input && input >= minValue && input <= maxValue)
        {
            clearInputBuffer();
            return input;
        }
        if (cin.eof()) exit(0); // input closed (e.g. window closed)
        printColored("Invalid input. Please enter a number between " + to_string(minValue) +
                     " and " + to_string(maxValue) + ": ", RED);
        clearInputBuffer();
    }
}

double getValidDouble(double minInclusive)
{
    double val;
    while (true)
    {
        if (cin >> val && val >= minInclusive)
        {
            clearInputBuffer();
            return val;
        }
        if (cin.eof()) exit(0); // input closed (e.g. window closed)
        printColored("Invalid number. Please enter a value >= " + toStr(minInclusive) + ": ", RED);
        clearInputBuffer();
    }
}

// The input buffer is already empty, so one getline is enough.
// The user does not need to press Enter twice.
void pressEnterToContinue()
{
    printColored("\nPress Enter to continue...", GRAY);
    string dummy;
    getline(cin, dummy);
}

//  NAVIGATION FUNCTIONS
bool askToGoBack(const string& targetMenu = "previous menu")
{
    printColored("\nPress 0 or m to go back to " + targetMenu + ", or Enter to continue: ", GRAY);
    string input;
    getline(cin, input);
    input = toLower(trim(input));
    return (input == "0" || input == "m");
}

bool confirmAction(const string& action)
{
    printColored("\nAre you sure you want to " + action + "? (1=Yes, 0=No): ", YELLOW);
    int choice = getValidInput(0, 1);
    return (choice == 1);
}

// NEW: result of every step that supports going back
enum NavAction
{
    NAV_OK,   // the user entered a normal value
    NAV_BACK, // the user typed b (go back one step)
    NAV_MAIN  // the user typed m (go to main menu)
};

// NEW: shows the navigation keys before every step
void printNavHint(const string& menuName = "Main Menu")
{
    printColored(" [b] Go Back   [m] " + menuName + "\n", GRAY);
}

// NEW: reads one line of text. "b" = back, "m" = main menu.
NavAction readLineNav(string& output)
{
    string line;
    if (!getline(cin, line))
    {
        exit(0); // input closed (e.g. window closed)
    }
    line = trim(line);
    string lower = toLower(line);
    if (lower == "b") return NAV_BACK;
    if (lower == "m") return NAV_MAIN;
    output = line;
    return NAV_OK;
}

// NEW: reads a whole number between minValue and maxValue, or b / m
NavAction readIntNav(int minValue, int maxValue, int& output)
{
    while (true)
    {
        string text;
        NavAction action = readLineNav(text);
        if (action != NAV_OK) return action;

        istringstream stream(text);
        int value;
        char extra;
        if (stream >> value && !(stream >> extra) && value >= minValue && value <= maxValue)
        {
            output = value;
            return NAV_OK;
        }
        printColored("Invalid input. Enter a number between " + to_string(minValue) + " and " +
                     to_string(maxValue) + " (or b / m): ", RED);
    }
}

// NEW: reads a number >= minInclusive, or b / m
NavAction readDoubleNav(double minInclusive, double& output)
{
    while (true)
    {
        string text;
        NavAction action = readLineNav(text);
        if (action != NAV_OK) return action;

        istringstream stream(text);
        double value;
        char extra;
        // Upper limit stops absurd numbers from breaking the calculations
        if (stream >> value && !(stream >> extra) && value >= minInclusive && value <= 1e12)
        {
            output = value;
            return NAV_OK;
        }
        printColored("Invalid amount. Enter a number from " + toStr(minInclusive) +
                     " to 1000000000000 (or b / m): ", RED);
    }
}

// NEW: yes/no question that also supports b / m
NavAction confirmActionNav(const string& action, bool& confirmed)
{
    cout << "\n";
    printNavHint();
    printColored("Are you sure you want to " + action + "? (1=Yes, 0=No): ", YELLOW);
    int choice = 0;
    NavAction result = readIntNav(0, 1, choice);
    confirmed = (choice == 1);
    return result;
}

// NEW: reads a password and shows * instead of the real characters
string readPasswordMasked()
{
    string password;
    while (true)
    {
        int ch = _getch();
        if (ch == 3 || ch == EOF) exit(0); // Ctrl+C or input closed
        if (ch == '\r' || ch == '\n')
        {
            cout << "\n";
            break;
        }
        if (ch == 0 || ch == 224) // arrow / function keys send two codes
        {
            _getch();
            continue;
        }
        if (ch == 8 || ch == 127) // backspace
        {
            if (!password.empty())
            {
                password.pop_back();
                cout << "\b \b" << flush;
            }
            continue;
        }
        if (ch >= 32 && ch < 127)
        {
            password.push_back(static_cast<char>(ch));
            cout << '*' << flush;
        }
    }
    return password;
}

// NEW: hidden password input that also supports b / m
NavAction readPasswordNav(string& output)
{
    string password = readPasswordMasked();
    string lower = toLower(trim(password));
    if (lower == "b") return NAV_BACK;
    if (lower == "m") return NAV_MAIN;
    output = password;
    return NAV_OK;
}

//  DATE AND TIME FUNCTIONS
vector<string> getAllMonths()
{
    return
    {
        "January", "February", "March", "April", "May", "June",
        "July", "August", "September", "October", "November", "December"
    };
}

string getCurrentMonth()
{
    time_t now = time(0);
    tm* ltm = localtime(&now);
    return getAllMonths()[ltm->tm_mon];
}

string getCurrentYear()
{
    time_t now = time(0);
    tm* ltm = localtime(&now);
    return to_string(1900 + ltm->tm_year);
}

//  COST OF LIVING CALCULATION
const map<string, string>& getDistrictCostMap()
{
    static const map<string, string> costMap =
    {
        {"dhaka", "High"}, {"chattogram", "High"}, {"chittagong", "High"}, {"sylhet", "High"},
        {"khulna", "Medium"}, {"rajshahi", "Medium"}, {"barisal", "Medium"}, {"barishal", "Medium"},
        {"rangpur", "Low"}, {"mymensingh", "Low"}
    };
    return costMap;
}

// NEW: districts shown to the user, with the cost level suggested by Numbeo data
vector<pair<string, string>> getDistrictList()
{
    return
    {
        {"Dhaka", "High"}, {"Chattogram", "High"}, {"Sylhet", "High"},
        {"Khulna", "Medium"}, {"Rajshahi", "Medium"}, {"Barishal", "Medium"},
        {"Rangpur", "Low"}, {"Mymensingh", "Low"}
    };
}

bool isKnownDistrict(const string& districtName)
{
    return getDistrictCostMap().count(toLower(trim(districtName))) > 0;
}

string getCostOfLivingLevel(const string& districtName)
{
    const map<string, string>& costMap = getDistrictCostMap();
    auto it = costMap.find(toLower(trim(districtName)));
    return (it != costMap.end()) ? it->second : "Medium";
}

//  BUDGET CALCULATION FUNCTIONS
// Standard percentage of income for each category of a family type
vector<double> getBudgetPercentages(const string& type, const string& level)
{
    map<string, map<string, vector<double>>> budgetTable;

    // Single Person Categories
    budgetTable["Single Student"]["Low"] = {20, 30, 8, 10, 12, 5, 8, 7};
    budgetTable["Single Student"]["Medium"] = {25, 35, 10, 12, 10, 6, 6, 6};
    budgetTable["Single Student"]["High"] = {30, 40, 12, 15, 8, 7, 4, 4};

    budgetTable["Young Professional"]["Low"] = {15, 25, 8, 10, 10, 5, 15, 5, 7, 5};
    budgetTable["Young Professional"]["Medium"] = {20, 30, 10, 12, 8, 6, 12, 4, 5, 3};
    budgetTable["Young Professional"]["High"] = {25, 35, 12, 15, 5, 7, 8, 3, 3, 2};

    budgetTable["Single Parent (1 Child)"]["Low"] = {25, 30, 8, 10, 12, 8, 5, 4, 3, 5};
    budgetTable["Single Parent (1 Child)"]["Medium"] = {28, 32, 10, 12, 10, 7, 4, 3, 2, 2};
    budgetTable["Single Parent (1 Child)"]["High"] = {32, 35, 12, 15, 8, 6, 3, 2, 1, 1};

    // Couple Categories
    budgetTable["Newly Married Couple"]["Low"] = {18, 25, 8, 10, 8, 6, 12, 5, 4, 4};
    budgetTable["Newly Married Couple"]["Medium"] = {22, 28, 10, 12, 7, 7, 10, 4, 3, 3};
    budgetTable["Newly Married Couple"]["High"] = {26, 32, 12, 14, 5, 8, 7, 3, 2, 1};

    budgetTable["Working Couple (DINK)"]["Low"] = {15, 25, 8, 10, 8, 6, 15, 8, 5, 5};
    budgetTable["Working Couple (DINK)"]["Medium"] = {18, 28, 10, 12, 7, 7, 12, 6, 4, 4};
    budgetTable["Working Couple (DINK)"]["High"] = {22, 32, 12, 14, 5, 8, 8, 4, 3, 2};

    // Family with Children
    budgetTable["Young Family (1 Child)"]["Low"] = {20, 25, 8, 10, 12, 8, 5, 4, 3, 5};
    budgetTable["Young Family (1 Child)"]["Medium"] = {25, 28, 10, 12, 10, 7, 4, 3, 2, 2};
    budgetTable["Young Family (1 Child)"]["High"] = {30, 32, 12, 14, 8, 6, 3, 2, 1, 1};

    budgetTable["Growing Family (2 Children)"]["Low"] = {22, 25, 8, 10, 15, 8, 5, 4, 3, 3, 2};
    budgetTable["Growing Family (2 Children)"]["Medium"] = {26, 28, 10, 12, 13, 7, 4, 3, 2, 2, 1};
    budgetTable["Growing Family (2 Children)"]["High"] = {30, 30, 12, 14, 12, 6, 3, 2, 1, 1, 0};

    return budgetTable[type][level];
}

// NEW: rent that the standard plan would suggest for this income
double getSuggestedRent(const string& type, const string& level, double income)
{
    vector<double> percentages = getBudgetPercentages(type, level);
    double total = 0, housing = 0;
    for (const auto& plan : budgetPlansCatalog)
    {
        if (plan.name != type) continue;
        for (size_t i = 0; i < min(percentages.size(), plan.categories.size()); i++)
        {
            total += percentages[i];
            if (isHousingCategory(plan.categories[i].name)) housing += percentages[i];
        }
    }
    return (total > 0) ? income * housing / total : 0;
}

void calculateBudgetPlan()
{
    vector<double> percentages = getBudgetPercentages(familyType, costOfLivingLevel);

    // Some rows add up to more than 100% (up to 120%).
    // This allocates more money than the family earns.
    // Adjust each row so the total allocation is exactly 100% of income.
    double percentTotal = 0;
    for (double p : percentages) percentTotal += p;

    budgetPlan.categories.clear();
    budgetPlan.name = familyType;
    budgetPlan.district = district;
    budgetPlan.totalIncome = totalIncome;
    budgetPlan.month = currentMonth;

    // Set focus based on cost level
    if (costOfLivingLevel == "Low") budgetPlan.focus = "Balanced Growth";
    else if (costOfLivingLevel == "Medium") budgetPlan.focus = "Wealth Building";
    else budgetPlan.focus = "Essential Spending";

    // Find matching plan from catalog
    for (const auto& plan : budgetPlansCatalog)
    {
        if (plan.name == familyType)
        {
            size_t count = min(percentages.size(), plan.categories.size());

            // NEW: if the user entered the rent they really pay, rent gets exactly
            // that amount and the rest of the income is shared by the other
            // categories in the same proportions as before.
            bool fixedRent = budgetPlan.actualRent >= 0;
            double rent = fixedRent ? min(budgetPlan.actualRent, totalIncome) : 0;
            double otherPercentTotal = 0;
            for (size_t i = 0; i < count; i++)
            {
                if (!isHousingCategory(plan.categories[i].name)) otherPercentTotal += percentages[i];
            }

            for (size_t i = 0; i < count; i++)
            {
                BudgetCategory cat = plan.categories[i];
                if (fixedRent && isHousingCategory(cat.name))
                {
                    cat.allocatedAmount = rent;
                }
                else if (fixedRent)
                {
                    cat.allocatedAmount = (otherPercentTotal > 0) ?
                                          (totalIncome - rent) * (percentages[i] / otherPercentTotal) : 0;
                }
                else
                {
                    cat.allocatedAmount = (percentTotal > 0) ? totalIncome * (percentages[i] / percentTotal) : 0;
                }
                cat.spentAmount = 0;
                budgetPlan.categories.push_back(cat);
            }
            break;
        }
    }
}

//  DISPLAY FUNCTIONS
void displayBudgetSummary()
{
    printColored("\n================================================\n", CYAN);
    printColored("               BUDGET SUMMARY\n", CYAN);
    printColored("================================================\n", CYAN);

    cout << "Month: ";
    printColored(currentMonth + " " + currentYear, YELLOW);
    cout << "\n";
    cout << "Family Type: ";
    printColored(familyType, YELLOW);
    cout << "\n";
    cout << "District: ";
    printColored(district, YELLOW);
    cout << " (Cost Level: ";
    printColored(costOfLivingLevel, YELLOW);
    cout << ")\n";
    cout << "Total Income: ";
    printColored("BDT " + toStr(budgetPlan.totalIncome), GREEN);
    cout << "\n";
    cout << "Rent: ";
    if (budgetPlan.actualRent >= 0) printColored("BDT " + toStr(budgetPlan.actualRent) + " (your actual rent)", YELLOW);
    else printColored("suggested by the plan", YELLOW);
    cout << "\n";
    cout << "Focus: ";
    printColored(budgetPlan.focus, MAGENTA);
    cout << "\n\n";

    const string line(66, '-');
    printColored(line + "\n", GRAY);
    printColored(" " + padRight("Category", 28) + "| " + padRight("Allocated", 11) + "| " +
                 padRight("Spent", 11) + "| Available\n", WHITE);
    printColored(line + "\n", GRAY);

    double totalAllocated = 0, totalSpent = 0, totalAvailable = 0;

    for (const auto& cat : budgetPlan.categories)
    {
        string displayName = cat.name.length() > 28 ? cat.name.substr(0, 25) + "..." : cat.name;

        cout << " ";
        printColored(padRight(displayName, 28), WHITE);
        cout << "| ";
        printColored(padRight(toStr(cat.allocatedAmount), 11), BRIGHT_GREEN);
        cout << "| ";
        printColored(padRight(toStr(cat.spentAmount), 11), cat.spentAmount > cat.allocatedAmount * 0.8 ? RED : YELLOW);
        cout << "| ";
        printColored(toStr(cat.availableAmount()), cat.availableAmount() < 0 ? RED : BRIGHT_GREEN);
        cout << "\n";

        totalAllocated += cat.allocatedAmount;
        totalSpent += cat.spentAmount;
        totalAvailable += cat.availableAmount();
    }

    printColored(line + "\n", GRAY);
    cout << " ";
    printColored(padRight("TOTAL", 28), BRIGHT_WHITE);
    cout << "| ";
    printColored(padRight(toStr(totalAllocated), 11), BRIGHT_GREEN);
    cout << "| ";
    printColored(padRight(toStr(totalSpent), 11), BRIGHT_YELLOW);
    cout << "| ";
    printColored(toStr(totalAvailable), totalAvailable < 0 ? BRIGHT_RED : BRIGHT_GREEN);
    cout << "\n";
    printColored(string(66, '=') + "\n", GRAY);

    // Monthly summary (uses the plan's income and guards against dividing by zero)
    // Money put into savings categories is kept, so it is not counted as spending
    double totalExpenses = 0, plannedSavings = 0, savedSoFar = 0;
    for (const auto& cat : budgetPlan.categories)
    {
        if (isSavingsCategory(cat.name))
        {
            plannedSavings += cat.allocatedAmount;
            savedSoFar += cat.spentAmount;
        }
        else
        {
            totalExpenses += cat.spentAmount;
        }
    }
    double totalSaved = budgetPlan.totalIncome - totalExpenses;
    double savingsRate = percentOf(totalSaved, budgetPlan.totalIncome);
    cout << "\nMonthly Summary:\n";
    cout << "Total Spent (savings not included): ";
    printColored("BDT " + toStr(totalExpenses), YELLOW);
    cout << "\n";
    cout << "Total Saved: ";
    printColored("BDT " + toStr(totalSaved), BRIGHT_GREEN);
    cout << "\n";
    cout << "Savings Rate: ";
    printColored(toStr(savingsRate) + "%", savingsRate >= 20 ? BRIGHT_GREEN : YELLOW);
    cout << "\n";
    cout << "Planned Savings: ";
    printColored("BDT " + toStr(plannedSavings) + " (" + toStr(percentOf(plannedSavings, budgetPlan.totalIncome)) +
                 "% of income), BDT " + toStr(savedSoFar) + " already put aside", BRIGHT_GREEN);
    cout << "\n";
}

//  BUDGET PRACTICALITY CHECK (NEW)
// The numbers below are estimates for Bangladesh and can be adjusted here.

const double MAX_HOUSING_SHARE = 0.40;  // rent should take at most 40% of income
const double MAX_FOOD_SHARE = 0.50;     // the food target never asks for more than 50% of income

// Estimated minimum monthly living cost for a household (BDT).
// Built from Numbeo prices (March 2026): a basic single person needs about
// 13,000-21,000 BDT (shared/outer-area rent, home cooking); a family of four
// needs about 35,000-40,000 BDT at a basic level. The first person costs the
// most; every extra person shares rent and utilities, so costs less.
double getMinLivingCost(const string& level, int people)
{
    double firstPerson = 10000, extraPerson = 6000; // Low
    if (level == "High")
    {
        firstPerson = 15000;
        extraPerson = 9000;
    }
    else if (level == "Medium")
    {
        firstPerson = 12000;
        extraPerson = 7000;
    }
    return firstPerson + extraPerson * max(0, people - 1);
}

// Estimated minimum monthly food cost for one person (BDT).
// Numbeo-based food budgets: single person home cooking 4,000-6,000 BDT,
// family of four 12,000-20,000 BDT.
double getMinFoodPerPerson(const string& level)
{
    if (level == "High") return 4000;
    if (level == "Medium") return 3500;
    return 3000;
}

// Smallest number of people each family type can have
int getMinimumHouseholdSize(const string& type)
{
    if (type == "Single Parent (1 Child)") return 2;
    if (type == "Newly Married Couple") return 2;
    if (type == "Working Couple (DINK)") return 2;
    if (type == "Young Family (1 Child)") return 3;
    if (type == "Growing Family (2 Children)") return 4;
    return 1;
}

int getHouseholdSize()
{
    // Single-person plans: parents who send money do not live in the same home
    if (familyType == "Single Student" || familyType == "Young Professional") return 1;
    return max(static_cast<int>(familyMembers.size()), getMinimumHouseholdSize(familyType));
}

// Every category belongs to one group
enum CategoryGroup
{
    GROUP_HOUSING,   // rent / home loan
    GROUP_FOOD,      // groceries
    GROUP_ESSENTIAL, // utilities, transport, education, child care, insurance
    GROUP_SAVINGS,   // emergency fund, savings, retirement, investments
    GROUP_WANTS      // entertainment, outings, personal spending, etc.
};

CategoryGroup getCategoryGroup(const string& categoryName)
{
    string name = toLower(categoryName);
    if (isHousingCategory(categoryName)) return GROUP_HOUSING;
    if (name.find("groceries") != string::npos) return GROUP_FOOD;
    if (isSavingsCategory(categoryName)) return GROUP_SAVINGS;
    if (name.find("utilities") != string::npos || name.find("transport") != string::npos ||
            name.find("education") != string::npos || name.find("child care") != string::npos ||
            name.find("books") != string::npos || name.find("insurance") != string::npos) return GROUP_ESSENTIAL;
    return GROUP_WANTS;
}

struct PracticalityRules
{
    int people;
    double minLivingCost;
    string incomeLevel; // "Low", "Tight" or "Comfortable"
    double wantsShare;
    double savingsShare;
    double emergencyShare;
    double essentialTrim; // how much other essentials may be cut during a fix
    double housingTrim;   // last resort: how much rent may be cut (cheaper housing)
    double housingCap;
    double wantsCap;
    double savingsFloor;
    double emergencyFloor;
    double foodTarget;
    bool foodTargetCapped;
};

struct PracticalityReport
{
    vector<string> problems; // can be fixed by moving money between categories
    vector<string> warnings; // cannot be fixed by the plan alone
};

PracticalityRules getPracticalityRules()
{
    PracticalityRules rules;
    double income = budgetPlan.totalIncome;

    rules.people = getHouseholdSize();
    rules.minLivingCost = getMinLivingCost(costOfLivingLevel, rules.people);

    // Lower incomes get a lower savings target and less room for non-essentials
    if (income < rules.minLivingCost)
    {
        rules.incomeLevel = "Low";
        rules.wantsShare = 0.10;
        rules.savingsShare = 0.05;
        rules.emergencyShare = 0.03;
        rules.essentialTrim = 0.30;
        rules.housingTrim = 0.20;
    }
    else if (income < rules.minLivingCost * 1.5)
    {
        rules.incomeLevel = "Tight";
        rules.wantsShare = 0.20;
        rules.savingsShare = 0.07;
        rules.emergencyShare = 0.04;
        rules.essentialTrim = 0.25;
        rules.housingTrim = 0.15;
    }
    else
    {
        rules.incomeLevel = "Comfortable";
        rules.wantsShare = 0.30;
        rules.savingsShare = 0.10;
        rules.emergencyShare = 0.05;
        rules.essentialTrim = 0.15;
        rules.housingTrim = 0.10;
    }

    rules.housingCap = income * MAX_HOUSING_SHARE;
    rules.wantsCap = income * rules.wantsShare;
    rules.savingsFloor = income * rules.savingsShare;
    rules.emergencyFloor = income * rules.emergencyShare;

    double idealFood = rules.people * getMinFoodPerPerson(costOfLivingLevel);
    rules.foodTarget = min(idealFood, income * MAX_FOOD_SHARE);
    rules.foodTargetCapped = idealFood > income * MAX_FOOD_SHARE;
    return rules;
}

string peopleText(int people)
{
    return to_string(people) + (people == 1 ? " person" : " people");
}

string shareText(double share)
{
    return to_string(static_cast<int>(share * 100 + 0.5)) + "%";
}

double getGroupTotal(CategoryGroup group)
{
    double total = 0;
    for (const auto& cat : budgetPlan.categories)
    {
        if (getCategoryGroup(cat.name) == group) total += cat.allocatedAmount;
    }
    return total;
}

bool groupExists(CategoryGroup group)
{
    for (const auto& cat : budgetPlan.categories)
    {
        if (getCategoryGroup(cat.name) == group) return true;
    }
    return false;
}

// Takes up to 'amount' from a group (shared fairly between its categories)
// without going below 'floor'. Returns how much was actually taken.
double takeFromGroup(CategoryGroup group, double amount, double floor = 0)
{
    double total = getGroupTotal(group);
    double available = total - floor;
    if (amount <= 0 || available <= 0 || total <= 0) return 0;

    double taken = min(amount, available);
    double factor = (total - taken) / total;
    for (auto& cat : budgetPlan.categories)
    {
        if (getCategoryGroup(cat.name) == group) cat.allocatedAmount *= factor;
    }
    return taken;
}

// Adds money to a group (shared fairly, or evenly if the group has nothing yet)
void addToGroup(CategoryGroup group, double amount)
{
    if (amount <= 0) return;
    double total = getGroupTotal(group);
    int count = 0;
    for (const auto& cat : budgetPlan.categories)
    {
        if (getCategoryGroup(cat.name) == group) count++;
    }
    if (count == 0) return;

    for (auto& cat : budgetPlan.categories)
    {
        if (getCategoryGroup(cat.name) == group)
        {
            cat.allocatedAmount += (total > 0) ? amount * (cat.allocatedAmount / total) : amount / count;
        }
    }
}

BudgetCategory* findCategoryContaining(const string& word)
{
    for (auto& cat : budgetPlan.categories)
    {
        if (toLower(cat.name).find(word) != string::npos) return &cat;
    }
    return nullptr;
}

PracticalityReport checkBudgetPracticality()
{
    PracticalityReport report;
    double income = budgetPlan.totalIncome;
    if (income <= 0) return report;

    PracticalityRules rules = getPracticalityRules();

    double allocated = 0;
    for (const auto& cat : budgetPlan.categories) allocated += cat.allocatedAmount;
    if (allocated > income + 1)
    {
        report.problems.push_back("Allocations (BDT " + toStr(allocated) + ") are more than the income (BDT " +
                                  toStr(income) + ").");
    }

    double housing = getGroupTotal(GROUP_HOUSING);
    if (housing > rules.housingCap + 1)
    {
        if (budgetPlan.actualRent >= 0)
        {
            // Real rent cannot be changed by the plan, so this is only a warning
            report.warnings.push_back("Your rent takes " + shareText(housing / income) +
                                      " of income (recommended: 40% or less). Cheaper housing would help.");
        }
        else
        {
            report.problems.push_back("Rent takes " + shareText(housing / income) +
                                      " of income. It should be 40% or less (BDT " + toStr(rules.housingCap) + ").");
        }
    }

    if (groupExists(GROUP_FOOD))
    {
        double food = getGroupTotal(GROUP_FOOD);
        if (food + 1 < rules.foodTarget)
        {
            report.problems.push_back("Food budget (BDT " + toStr(food) + ") is too low for " +
                                      peopleText(rules.people) + ". It needs about BDT " +
                                      toStr(rules.foodTarget) + ".");
        }
    }

    double wants = getGroupTotal(GROUP_WANTS);
    if (wants > rules.wantsCap + 1)
    {
        report.problems.push_back("Non-essential spending is " + shareText(wants / income) +
                                  " of income. For a " + rules.incomeLevel + " income it should be at most " +
                                  shareText(rules.wantsShare) + ".");
    }

    if (groupExists(GROUP_SAVINGS))
    {
        double savings = getGroupTotal(GROUP_SAVINGS);
        if (savings + 1 < rules.savingsFloor)
        {
            report.problems.push_back("Savings are only " + shareText(savings / income) +
                                      " of income. Save at least " + shareText(rules.savingsShare) +
                                      " (BDT " + toStr(rules.savingsFloor) + ").");
        }
    }

    BudgetCategory* emergency = findCategoryContaining("emergency");
    if (emergency && emergency->allocatedAmount + 1 < rules.emergencyFloor)
    {
        report.problems.push_back("Emergency Fund (BDT " + toStr(emergency->allocatedAmount) +
                                  ") is below " + shareText(rules.emergencyShare) + " of income (BDT " +
                                  toStr(rules.emergencyFloor) + ").");
    }

    // Things the plan alone cannot solve
    if (rules.incomeLevel == "Low")
    {
        report.warnings.push_back("Income is below the estimated minimum living cost of about BDT " +
                                  toStr(rules.minLivingCost) + " for " + peopleText(rules.people) +
                                  " in a " + costOfLivingLevel +
                                  "-cost district. Try to add income or find cheaper housing.");
    }
    if (rules.foodTargetCapped)
    {
        report.warnings.push_back("A full food budget for " + peopleText(rules.people) +
                                  " would need more than half of the income.");
    }
    for (const auto& cat : budgetPlan.categories)
    {
        if (cat.spentAmount > cat.allocatedAmount + 0.5)
        {
            report.warnings.push_back("Overspent on " + cat.name + " by BDT " +
                                      toStr(cat.spentAmount - cat.allocatedAmount) + ".");
        }
    }
    return report;
}

// Moves money between categories until the plan follows the rules.
// Order of priority: food first, then savings, then the emergency fund.
void fixBudgetPlan()
{
    double income = budgetPlan.totalIncome;
    if (income <= 0 || budgetPlan.categories.empty()) return;

    // Always start again from the standard plan, so running the fix again
    // gives the same result instead of cutting more and more each time.
    // Spending entered by the user is kept.
    vector<BudgetCategory> original = budgetPlan.categories;
    calculateBudgetPlan();
    if (budgetPlan.categories.size() != original.size())
    {
        budgetPlan.categories = original; // unknown plan type: leave the plan unchanged
        return;
    }
    for (size_t i = 0; i < original.size(); i++)
    {
        budgetPlan.categories[i].spentAmount = original[i].spentAmount;
    }

    PracticalityRules rules = getPracticalityRules();
    double pool = 0; // money that was freed up and still needs a category

    // 1. Allocations must add up to the income
    double allocated = 0;
    for (const auto& cat : budgetPlan.categories) allocated += cat.allocatedAmount;
    if (allocated > income + 0.01 && allocated > 0)
    {
        for (auto& cat : budgetPlan.categories) cat.allocatedAmount *= income / allocated;
    }
    else
    {
        pool += income - allocated;
    }

    // Other essentials and rent may only be trimmed a little
    double essentialTrimLeft = getGroupTotal(GROUP_ESSENTIAL) * rules.essentialTrim;
    double housingTrimLeft = 0; // set after the rent limit below

    // Collects money for a category that is short.
    // Takes from: free money -> non-essentials -> extra savings -> a small essentials trim
    //             -> a small rent trim (last resort)
    auto gather = [&](double need, bool allowSavings) -> double
    {
        double got = min(need, pool);
        pool -= got;
        if (got < need) got += takeFromGroup(GROUP_WANTS, need - got);
        if (got < need && allowSavings) got += takeFromGroup(GROUP_SAVINGS, need - got, rules.savingsFloor);
        if (got < need && essentialTrimLeft > 0)
        {
            double taken = takeFromGroup(GROUP_ESSENTIAL, min(need - got, essentialTrimLeft));
            essentialTrimLeft -= taken;
            got += taken;
        }
        if (got < need && housingTrimLeft > 0)
        {
            double taken = takeFromGroup(GROUP_HOUSING, min(need - got, housingTrimLeft));
            housingTrimLeft -= taken;
            got += taken;
        }
        return got;
    };

    // 2. Rent above 40% is cut back (look for cheaper housing)
    //    (only when the plan suggested the rent; real rent entered by the user is never changed)
    bool fixedRent = budgetPlan.actualRent >= 0;
    double housing = getGroupTotal(GROUP_HOUSING);
    if (!fixedRent && housing > rules.housingCap) pool += takeFromGroup(GROUP_HOUSING, housing - rules.housingCap);
    housingTrimLeft = fixedRent ? 0 : getGroupTotal(GROUP_HOUSING) * rules.housingTrim;

    // 3. Non-essential spending above its limit is cut back
    double wants = getGroupTotal(GROUP_WANTS);
    if (wants > rules.wantsCap) pool += takeFromGroup(GROUP_WANTS, wants - rules.wantsCap);

    // 4. Enough money for food
    if (groupExists(GROUP_FOOD))
    {
        double need = rules.foodTarget - getGroupTotal(GROUP_FOOD);
        if (need > 0) addToGroup(GROUP_FOOD, gather(need, true));
    }

    // 5. Minimum savings
    if (groupExists(GROUP_SAVINGS))
    {
        double need = rules.savingsFloor - getGroupTotal(GROUP_SAVINGS);
        if (need > 0) addToGroup(GROUP_SAVINGS, gather(need, false));
    }

    // 6. Emergency fund: first move money from other savings, then gather the rest
    BudgetCategory* emergency = findCategoryContaining("emergency");
    if (emergency && emergency->allocatedAmount < rules.emergencyFloor)
    {
        double need = rules.emergencyFloor - emergency->allocatedAmount;
        double otherSavings = getGroupTotal(GROUP_SAVINGS) - emergency->allocatedAmount;
        double moved = min(need, otherSavings);
        if (moved > 0)
        {
            double factor = (otherSavings - moved) / otherSavings;
            for (auto& cat : budgetPlan.categories)
            {
                if (&cat != emergency && getCategoryGroup(cat.name) == GROUP_SAVINGS) cat.allocatedAmount *= factor;
            }
            emergency->allocatedAmount += moved;
            need -= moved;
        }
        if (need > 0) emergency->allocatedAmount += gather(need, false);
    }

    // 7. Any money left over goes to savings
    if (pool > 0)
    {
        if (groupExists(GROUP_SAVINGS)) addToGroup(GROUP_SAVINGS, pool);
        else addToGroup(GROUP_FOOD, pool);
        pool = 0;
    }
}

void printPracticalityWarnings(const PracticalityReport& report)
{
    if (report.warnings.empty()) return;
    printColored("\nPlease note:\n", BRIGHT_YELLOW);
    for (const auto& warning : report.warnings)
    {
        printColored(" - " + warning + "\n", YELLOW);
    }
}

// Checks the current plan, explains the problems and fixes them automatically.
// Returns true if the plan was changed.
bool runPracticalityCheck()
{
    printColored("\n================================================\n", BRIGHT_CYAN);
    printColored("          BUDGET PRACTICALITY CHECK\n", BRIGHT_CYAN);
    printColored("================================================\n", BRIGHT_CYAN);

    if (budgetPlan.totalIncome <= 0)
    {
        printColored("Total income is BDT 0, so this plan cannot be practical.\n", RED);
        printColored("Please run the Budget Planner again and enter your income.\n", YELLOW);
        return false;
    }

    PracticalityRules rules = getPracticalityRules();
    cout << "Household Size: ";
    printColored(peopleText(rules.people) + "\n", YELLOW);
    cout << "Estimated Minimum Living Cost: ";
    printColored("BDT " + toStr(rules.minLivingCost) + "\n", YELLOW);
    cout << "Income Level: ";
    printColored(rules.incomeLevel + "\n",
                 rules.incomeLevel == "Low" ? RED : (rules.incomeLevel == "Tight" ? YELLOW : GREEN));

    // With an income below the minimum living cost, no plan can be fully practical
    bool lowIncome = (rules.incomeLevel == "Low");

    PracticalityReport report = checkBudgetPracticality();
    if (report.problems.empty())
    {
        if (lowIncome)
        {
            printColored("\nResult: The plan is as balanced as possible, but it is NOT fully practical\n"
                         "because the income is below the minimum living cost.\n", BRIGHT_YELLOW);
        }
        else
        {
            printColored("\nResult: Your budget plan is PRACTICAL.\n", BRIGHT_GREEN);
        }
        printPracticalityWarnings(report);
        return false;
    }

    printColored("\nResult: Your budget plan is NOT practical.\n", BRIGHT_RED);
    for (const auto& problem : report.problems)
    {
        printColored(" - " + problem + "\n", RED);
    }

    vector<BudgetCategory> before = budgetPlan.categories;
    fixBudgetPlan();

    int changedCount = 0;
    for (size_t i = 0; i < budgetPlan.categories.size() && i < before.size(); i++)
    {
        if (fabs(budgetPlan.categories[i].allocatedAmount - before[i].allocatedAmount) >= 1) changedCount++;
    }

    PracticalityReport after = checkBudgetPracticality();
    if (changedCount == 0)
    {
        // Nothing could be moved: the problems come from the income or the real rent
        printColored("\nThese problems cannot be solved by moving money between categories.\n", BRIGHT_YELLOW);
        printColored("Try to increase income or find cheaper housing.\n", YELLOW);
        printPracticalityWarnings(after);
        return false;
    }

    printColored("\nFixing the plan automatically...\n", YELLOW);
    const string line(56, '-');
    printColored(line + "\n", GRAY);
    printColored(" " + padRight("Category", 28) + "| " + padRight("Before", 11) + "| After\n", WHITE);
    printColored(line + "\n", GRAY);
    for (size_t i = 0; i < budgetPlan.categories.size(); i++)
    {
        double oldAmount = before[i].allocatedAmount;
        double newAmount = budgetPlan.categories[i].allocatedAmount;
        if (fabs(newAmount - oldAmount) < 1) continue; // only show what changed

        cout << " " << padRight(budgetPlan.categories[i].name, 28) << "| ";
        printColored(padRight(toStr(oldAmount), 11), GRAY);
        cout << "| ";
        printColored(toStr(newAmount) + "\n", newAmount > oldAmount ? BRIGHT_GREEN : YELLOW);
    }
    printColored(line + "\n", GRAY);

    if (after.problems.empty() && lowIncome)
    {
        printColored("\nPlan improved as much as possible, but it is NOT fully practical\n"
                     "because the income is below the minimum living cost.\n", BRIGHT_YELLOW);
    }
    else if (after.problems.empty())
    {
        printColored("\nPlan fixed. Your budget plan is now PRACTICAL.\n", BRIGHT_GREEN);
    }
    else
    {
        printColored("\nThe plan was improved, but these issues remain:\n", BRIGHT_YELLOW);
        for (const auto& problem : after.problems)
        {
            printColored(" - " + problem + "\n", YELLOW);
        }
        printColored("The income is too small to fix this fully. "
                     "Try to increase income or find cheaper housing.\n", YELLOW);
    }
    printPracticalityWarnings(after);
    return changedCount > 0;
}

//  PREVIOUS MONTHS FUNCTIONS
void saveCurrentMonthRecord()
{
    if (!budgetPlanned) return;

    MonthlyRecord record;
    record.month = currentMonth;
    record.year = currentYear;
    record.budgetPlan = budgetPlan;

    // Money put into savings categories is kept, so it is not counted as spending
    record.totalSpent = 0;
    for (const auto& cat : budgetPlan.categories)
    {
        if (!isSavingsCategory(cat.name)) record.totalSpent += cat.spentAmount;
    }
    record.totalSaved = budgetPlan.totalIncome - record.totalSpent;

    // Update this month if it already exists, otherwise add it
    bool monthExists = false;
    for (auto& existingRecord : monthlyRecords)
    {
        if (existingRecord.month == currentMonth && existingRecord.year == currentYear)
        {
            existingRecord = record;
            monthExists = true;
            break;
        }
    }
    if (!monthExists)
    {
        monthlyRecords.push_back(record);
    }

    ofstream file(historyFileName()); // each user has their own history file
    if (file)
    {
        file << monthlyRecords.size() << "\n";
        for (const auto& rec : monthlyRecords)
        {
            file << rec.month << "\n" << rec.year << "\n";
            file << rec.budgetPlan.name << "\n" << rec.budgetPlan.district << "\n";
            file << rec.budgetPlan.totalIncome << "\n" << rec.budgetPlan.focus << "\n";
            file << rec.totalSpent << "\n" << rec.totalSaved << "\n";
            file << rec.budgetPlan.categories.size() << "\n";
            for (const auto& cat : rec.budgetPlan.categories)
            {
                file << cat.name << "\n" << cat.allocatedAmount << "\n" << cat.spentAmount << "\n";
            }
        }
    }
}

void loadMonthlyHistory()
{
    ifstream file(historyFileName()); // each user has their own history file
    if (!file) return;

    monthlyRecords.clear();
    size_t recordCount = 0;
    if (!(file >> recordCount)) return; // empty or damaged file
    skipLine(file);

    for (size_t i = 0; i < recordCount; i++)
    {
        MonthlyRecord record;
        getline(file, record.month);
        getline(file, record.year);
        getline(file, record.budgetPlan.name);
        getline(file, record.budgetPlan.district);
        file >> record.budgetPlan.totalIncome;
        skipLine(file);
        getline(file, record.budgetPlan.focus);
        file >> record.totalSpent >> record.totalSaved;
        skipLine(file);

        size_t categoryCount = 0;
        file >> categoryCount;
        skipLine(file);

        for (size_t j = 0; j < categoryCount && file; j++)
        {
            BudgetCategory cat;
            getline(file, cat.name);
            file >> cat.allocatedAmount >> cat.spentAmount;
            skipLine(file);
            record.budgetPlan.categories.push_back(cat);
        }

        if (!file) break; // stop on a damaged record instead of reading garbage
        record.budgetPlan.month = record.month;
        monthlyRecords.push_back(record);
    }
}

void viewPreviousMonths()
{
    if (monthlyRecords.empty())
    {
        printColored("\nNo previous months data found.\n", YELLOW);
        pressEnterToContinue();
        return;
    }

    printColored("\n================================================\n", BRIGHT_CYAN);
    printColored("           PREVIOUS MONTHS HISTORY\n", BRIGHT_CYAN);
    printColored("================================================\n", BRIGHT_CYAN);

    const string line(72, '-');
    printColored("Monthly Summary:\n", BRIGHT_WHITE);
    printColored(line + "\n", GRAY);
    printColored(" " + padRight("Month-Year", 15) + "| " + padRight("Income", 11) + "| " +
                 padRight("Spent", 11) + "| " + padRight("Saved", 11) + "| Savings %\n", WHITE);
    printColored(line + "\n", GRAY);

    for (const auto& record : monthlyRecords)
    {
        double savingsRate = percentOf(record.totalSaved, record.budgetPlan.totalIncome);

        cout << " " << padRight(record.month + " " + record.year, 15) << "| ";
        printColored(padRight(toStr(record.budgetPlan.totalIncome), 11), BRIGHT_GREEN);
        cout << "| ";
        printColored(padRight(toStr(record.totalSpent), 11), YELLOW);
        cout << "| ";
        printColored(padRight(toStr(record.totalSaved), 11), BRIGHT_GREEN);
        cout << "| ";
        printColored(toStr(savingsRate) + "%", savingsRate >= 20 ? BRIGHT_GREEN : YELLOW);
        cout << "\n";
    }
    printColored(line + "\n", GRAY);

    // Step 1: view details? (b / m go back to the main menu)
    while (true)
    {
        cout << "\n";
        printNavHint();
        printColored("View details of specific month? (1=Yes, 0=No): ", BRIGHT_WHITE);
        int viewDetails = 0;
        NavAction action = readIntNav(0, 1, viewDetails);
        if (action != NAV_OK || viewDetails == 0) return;

        // Step 2: choose a month (b goes back to step 1)
        bool backToQuestion = false;
        while (true)
        {
            printColored("\nSelect month to view details:\n", CYAN);
            for (size_t i = 0; i < monthlyRecords.size(); i++)
            {
                cout << " " << i + 1 << ". " << monthlyRecords[i].month << " " << monthlyRecords[i].year << "\n";
            }
            printNavHint();
            printColored(" Enter your choice (1-" + to_string(monthlyRecords.size()) + "): ", BRIGHT_WHITE);

            int choice = 0;
            action = readIntNav(1, static_cast<int>(monthlyRecords.size()), choice);
            if (action == NAV_MAIN) return;
            if (action == NAV_BACK)
            {
                backToQuestion = true;
                break;
            }
            const MonthlyRecord& selectedRecord = monthlyRecords[choice - 1];

            printColored("\n================================================\n", CYAN);
            printColored("          BUDGET DETAILS FOR " + selectedRecord.month + " " + selectedRecord.year + "\n", CYAN);
            printColored("================================================\n", CYAN);

            cout << "Family Type: ";
            printColored(selectedRecord.budgetPlan.name, YELLOW);
            cout << "\n";
            cout << "Total Income: ";
            printColored("BDT " + toStr(selectedRecord.budgetPlan.totalIncome), GREEN);
            cout << "\n";
            cout << "Total Spent: ";
            printColored("BDT " + toStr(selectedRecord.totalSpent), YELLOW);
            cout << "\n";
            cout << "Total Saved: ";
            printColored("BDT " + toStr(selectedRecord.totalSaved), BRIGHT_GREEN);
            cout << "\n";

            double savingsRate = percentOf(selectedRecord.totalSaved, selectedRecord.budgetPlan.totalIncome);
            cout << "Savings Rate: ";
            printColored(toStr(savingsRate) + "%", savingsRate >= 20 ? BRIGHT_GREEN : YELLOW);
            cout << "\n\n";

            printColored("Category-wise Spending:\n", BRIGHT_WHITE);
            printColored(line + "\n", GRAY);
            for (const auto& cat : selectedRecord.budgetPlan.categories)
            {
                // Some categories are allocated 0 (e.g. "Family Outings"), so guard the division
                double usagePercent = percentOf(cat.spentAmount, cat.allocatedAmount);
                bool overBudget = cat.spentAmount > cat.allocatedAmount;

                cout << " " << cat.name << ": ";
                printColored("BDT " + toStr(cat.spentAmount) + " / BDT " + toStr(cat.allocatedAmount),
                             overBudget ? RED : (usagePercent > 80 ? YELLOW : GREEN));
                if (cat.allocatedAmount > 0) cout << " (" << toStr(usagePercent) << "%)\n";
                else cout << " (no allocation)\n";
            }

            // b = choose another month, Enter or m = main menu
            printColored("\nPress b to go back to the month list, or Enter / m for Main Menu: ", GRAY);
            string input;
            getline(cin, input);
            if (toLower(trim(input)) == "b") continue;
            return;
        }
        if (backToQuestion) continue;
    }
}

//  SPENDING MANAGEMENT
void updateCategorySpending()
{
    if (!budgetPlanned)
    {
        printColored("ERROR: Please create a budget plan first!\n", RED);
        pressEnterToContinue();
        return;
    }

    while (true)
    {
        printColored("\nSelect category to update spending:\n", CYAN);
        for (size_t i = 0; i < budgetPlan.categories.size(); ++i)
        {
            cout << " " << (i + 1) << ". " << budgetPlan.categories[i].name << "\n";
        }
        printColored(" 0. Back to Main Menu\n", GRAY);
        printNavHint();
        printColored(" Enter your choice: ", BRIGHT_WHITE);

        int choice = 0;
        NavAction action = readIntNav(0, static_cast<int>(budgetPlan.categories.size()), choice);
        if (action != NAV_OK || choice == 0) break; // b or m: the previous menu is the main menu

        BudgetCategory& cat = budgetPlan.categories[choice - 1];

        int subStep = 0; // 0 = choose add / correct, 1 = enter amount
        int mode = 1;    // 1 = add spending, 2 = correct a mistake
        while (true)
        {
            if (subStep == 0)
            {
                printColored("\nUpdating: ", CYAN);
                printColored(cat.name + "\n", YELLOW);
                cout << "Allocated: ";
                printColored("BDT " + toStr(cat.allocatedAmount) + "\n", GREEN);
                cout << "Already Spent: ";
                printColored("BDT " + toStr(cat.spentAmount) + "\n", YELLOW);
                cout << "Available: ";
                printColored("BDT " + toStr(cat.availableAmount()) + "\n",
                             cat.availableAmount() < 0 ? RED : BRIGHT_GREEN);

                // NEW: a wrong entry can now be corrected
                printColored("\n 1. Add spending\n", WHITE);
                printColored(" 2. Correct a mistake (reduce the spent amount)\n", WHITE);
                printNavHint();
                printColored(" Enter your choice (1-2): ", BRIGHT_WHITE);
                action = readIntNav(1, 2, mode);
                if (action == NAV_MAIN) return;
                if (action == NAV_BACK) break; // back to the category list
                if (mode == 2 && cat.spentAmount <= 0)
                {
                    printColored("Nothing has been spent in this category yet.\n", YELLOW);
                    continue;
                }
                subStep = 1;
                continue;
            }

            double amount = 0;
            if (mode == 1)
            {
                printNavHint();
                printColored(" Enter amount spent (or 0 to cancel): ", BRIGHT_WHITE);
                action = readDoubleNav(0.0, amount);
                if (action == NAV_MAIN) return;
                if (action == NAV_BACK)
                {
                    subStep = 0;
                    continue;
                }
                if (amount == 0.0)
                {
                    printColored("Update cancelled.\n", RED);
                    break;
                }

                if (amount > cat.availableAmount())
                {
                    printColored("Warning: This exceeds the available amount by BDT ", YELLOW);
                    printColored(toStr(amount - cat.availableAmount()) + "\n", RED);
                    printNavHint();
                    printColored(" Do you want to proceed? (1=Yes, 0=No): ", YELLOW);

                    int proceed = 0;
                    action = readIntNav(0, 1, proceed);
                    if (action == NAV_MAIN) return;
                    if (action == NAV_BACK) continue; // enter the amount again
                    if (proceed != 1)
                    {
                        printColored("Update cancelled.\n", RED);
                        break;
                    }
                }
                cat.spentAmount += amount;
                printColored("Update successful!\n", GREEN);
            }
            else
            {
                printNavHint();
                printColored(" Enter amount to remove (up to BDT " + toStr(cat.spentAmount) +
                             ", or 0 to cancel): ", BRIGHT_WHITE);
                action = readDoubleNav(0.0, amount);
                if (action == NAV_MAIN) return;
                if (action == NAV_BACK)
                {
                    subStep = 0;
                    continue;
                }
                if (amount == 0.0)
                {
                    printColored("Correction cancelled.\n", RED);
                    break;
                }
                if (amount > cat.spentAmount + 0.001)
                {
                    printColored("You cannot remove more than was spent.\n", RED);
                    continue;
                }
                cat.spentAmount = max(0.0, cat.spentAmount - amount);
                printColored("Correction saved!\n", GREEN);
            }

            cout << "New spent amount: ";
            printColored("BDT " + toStr(cat.spentAmount) + "\n", YELLOW);
            cout << "New available amount: ";
            printColored("BDT " + toStr(cat.availableAmount()) + "\n",
                         cat.availableAmount() < 0 ? RED : BRIGHT_GREEN);

            // Auto-save so spending isn't lost if the window is closed
            saveCurrentMonthRecord();
            writeBudgetPlanFile();

            if (askToGoBack("main menu")) return;
            break; // back to the category list
        }
    }
}

//  FILE OPERATIONS
bool writeBudgetPlanFile()
{
    if (!budgetPlanned) return false;

    ofstream file(planFileName()); // each user has their own plan file
    if (!file) return false;

    file << budgetPlan.name << "\n"
         << budgetPlan.district << "\n"
         << budgetPlan.totalIncome << "\n"
         << budgetPlan.focus << "\n";
    file << budgetPlan.categories.size() << "\n";
    for (const auto& cat : budgetPlan.categories)
    {
        file << cat.name << "\n" << cat.allocatedAmount << "\n" << cat.spentAmount << "\n";
    }

    file << familyMembers.size() << "\n";
    for (const auto& member : familyMembers)
    {
        file << member.name << "\n" << member.relation << "\n" << member.monthlyIncome << "\n";
    }

    // Remember which month this spending belongs to
    file << currentMonth << "\n" << currentYear << "\n";

    // NEW: the cost level chosen by the user and the real rent (-1 = suggested)
    file << costOfLivingLevel << "\n" << budgetPlan.actualRent << "\n";
    return static_cast<bool>(file);
}

void saveBudgetPlan()
{
    if (!budgetPlanned)
    {
        printColored("ERROR: No budget plan to save.\n", RED);
        return;
    }
    if (!writeBudgetPlanFile())
    {
        printColored("ERROR: Error opening file for saving!\n", RED);
        return;
    }
    saveCurrentMonthRecord();
    printColored("Budget plan saved successfully!\n", GREEN);
}

bool loadBudgetPlan()
{
    ifstream file(planFileName()); // each user has their own plan file
    if (!file)
    {
        printColored("No saved budget data found.\n", BLUE);
        return false;
    }

    // Read into temporaries first so a damaged file can't half-overwrite current data
    BudgetPlan loaded;
    getline(file, loaded.name);
    getline(file, loaded.district);
    file >> loaded.totalIncome;
    skipLine(file);
    getline(file, loaded.focus);

    size_t categoryCount = 0;
    file >> categoryCount;
    skipLine(file);
    for (size_t i = 0; i < categoryCount && file; i++)
    {
        BudgetCategory cat;
        getline(file, cat.name);
        file >> cat.allocatedAmount >> cat.spentAmount;
        skipLine(file);
        loaded.categories.push_back(cat);
    }

    size_t memberCount = 0;
    file >> memberCount;
    skipLine(file);
    vector<FamilyMember> loadedMembers;
    for (size_t i = 0; i < memberCount && file; i++)
    {
        FamilyMember member;
        getline(file, member.name);
        getline(file, member.relation);
        file >> member.monthlyIncome;
        skipLine(file);
        loadedMembers.push_back(member);
    }

    if (!file || loaded.categories.empty())
    {
        printColored("Saved budget file is damaged and could not be loaded.\n", RED);
        return false;
    }

    // Older save files don't have these lines, so they are optional
    string savedMonth, savedYear, savedLevel, savedRent;
    getline(file, savedMonth);
    getline(file, savedYear);
    getline(file, savedLevel);
    getline(file, savedRent);

    istringstream rentStream(savedRent);
    double rentValue = -1;
    if (rentStream >> rentValue && rentValue >= 0) loaded.actualRent = rentValue;

    budgetPlan = loaded;
    familyMembers = loadedMembers;
    familyType = budgetPlan.name;
    district = budgetPlan.district;               // was never restored before
    totalIncome = budgetPlan.totalIncome;         // previously lost the user's own income
    // Use the level the user chose; older files fall back to the district list
    if (savedLevel == "High" || savedLevel == "Medium" || savedLevel == "Low") costOfLivingLevel = savedLevel;
    else costOfLivingLevel = getCostOfLivingLevel(district);
    userName.clear();
    for (const auto& member : familyMembers)
    {
        if (member.relation == "Myself")
        {
            userName = member.name;
            break;
        }
    }
    budgetPlanned = true;

    // If the saved spending is from an earlier month, start a new month.
    // Last month's spending is already saved in the monthly history file.
    if (!savedMonth.empty() && (savedMonth != currentMonth || savedYear != currentYear))
    {
        for (auto& cat : budgetPlan.categories) cat.spentAmount = 0;
        printColored("New month detected: spending reset for " + currentMonth + " " + currentYear + ".\n", CYAN);
    }
    budgetPlan.month = currentMonth;

    printColored("Budget plan loaded successfully!\n", GREEN);
    return true;
}

//  CLEAR DATA FUNCTION
void clearAllData()
{
    printColored("\n================================================\n", MAGENTA);
    printColored("               CLEAR ALL DATA\n", MAGENTA);
    printColored("================================================\n", MAGENTA);

    printColored("WARNING: This action will permanently delete:\n\n", BRIGHT_RED);
    printColored("1. Current budget plan\n", BRIGHT_WHITE);
    printColored("2. Family member information\n", BRIGHT_WHITE);
    printColored("3. Spending records\n", BRIGHT_WHITE);
    printColored("4. Monthly history\n", BRIGHT_WHITE);
    printColored("5. All saved files\n", BRIGHT_WHITE);
    printColored("(Only for user '" + currentUser + "'. Your account itself is kept.)\n", GRAY);

    printColored("\nThis action cannot be undone!\n", BRIGHT_RED);

    bool confirmed = false;
    NavAction action = confirmActionNav("clear all data", confirmed);
    if (action != NAV_OK) return; // b or m: back to the main menu
    if (!confirmed)
    {
        printColored("\nData clearance cancelled.\n", BLUE);
        pressEnterToContinue();
        return;
    }

    userName.clear();
    familyMembers.clear();
    totalIncome = 0;
    familyType.clear();
    district.clear();
    costOfLivingLevel.clear();
    budgetPlan = BudgetPlan();
    budgetPlanned = false;
    monthlyRecords.clear();

    remove(planFileName().c_str());
    remove(historyFileName().c_str());

    printColored("\nAll data has been successfully cleared!\n", GREEN);
    printColored("The application will now start fresh.\n", GREEN);
    pressEnterToContinue();
}

//  USER ACCOUNTS (NEW)
struct UserAccount
{
    string username;
    string salt;
    string passwordHash;
    string email;
    string fullName;
};

const string USERS_FILE = "users.txt";

// Passwords are never saved as plain text. A random "salt" is mixed in
// and the result is hashed many times.
// NOTE: good enough for a learning project, but not bank-level security.
string hashPassword(const string& password, const string& salt)
{
    unsigned long long hash = 1469598103934665603ULL; // FNV-1a 64-bit
    const string data = salt + ":" + password;
    for (int round = 0; round < 20000; round++)
    {
        for (unsigned char c : data)
        {
            hash ^= c;
            hash *= 1099511628211ULL;
        }
        hash ^= static_cast<unsigned long long>(round);
    }
    ostringstream out;
    out << hex << hash;
    return out.str();
}

string generateSalt()
{
    static mt19937_64 generator(static_cast<unsigned long long>(time(0)) ^ random_device{}());
    ostringstream out;
    out << hex << generator();
    return out.str();
}

// users.txt has one account per line: username|salt|hash|email|full name
// (older files with "username salt hash" are still read correctly)
vector<UserAccount> loadUsers()
{
    vector<UserAccount> users;
    ifstream file(USERS_FILE);
    string line;
    while (getline(file, line))
    {
        line = trim(line);
        if (line.empty()) continue;

        UserAccount user;
        if (line.find('|') != string::npos)
        {
            vector<string> parts;
            string part;
            istringstream stream(line);
            while (getline(stream, part, '|')) parts.push_back(part);
            if (parts.size() < 3) continue;
            user.username = parts[0];
            user.salt = parts[1];
            user.passwordHash = parts[2];
            if (parts.size() > 3) user.email = parts[3];
            if (parts.size() > 4) user.fullName = parts[4];
        }
        else
        {
            istringstream stream(line);
            if (!(stream >> user.username >> user.salt >> user.passwordHash)) continue;
        }
        users.push_back(user);
    }
    return users;
}

bool saveUsers(const vector<UserAccount>& users)
{
    ofstream file(USERS_FILE);
    if (!file) return false;
    for (const auto& user : users)
    {
        file << user.username << "|" << user.salt << "|" << user.passwordHash << "|"
             << user.email << "|" << user.fullName << "\n";
    }
    return static_cast<bool>(file);
}

// Finds an account by username or email. Returns its index, or -1.
int findUser(const vector<UserAccount>& users, const string& usernameOrEmail)
{
    for (size_t i = 0; i < users.size(); i++)
    {
        if (users[i].username == usernameOrEmail) return static_cast<int>(i);
        if (!users[i].email.empty() && users[i].email == usernameOrEmail) return static_cast<int>(i);
    }
    return -1;
}

// Usernames are used in file names, so only safe characters are allowed
bool isValidUsername(const string& username)
{
    if (username.length() < 3 || username.length() > 20) return false;
    for (unsigned char c : username)
    {
        if (!isalnum(c) && c != '_') return false;
    }
    return true;
}

bool isValidEmail(const string& email)
{
    if (email.length() < 5 || email.length() > 60) return false;
    if (email.find(' ') != string::npos || email.find('|') != string::npos) return false;
    size_t at = email.find('@');
    if (at == string::npos || at == 0 || email.find('@', at + 1) != string::npos) return false;
    size_t dot = email.find('.', at + 2); // at least one character between @ and .
    return dot != string::npos && dot < email.length() - 1;
}

bool isValidFullName(const string& name)
{
    if (name.length() < 2 || name.length() > 40) return false;
    if (name.find('|') != string::npos) return false;
    bool hasLetter = false;
    for (unsigned char c : name)
    {
        if (c < 32) return false;
        if (isalpha(c) || c >= 128) hasLetter = true;
    }
    return hasLetter;
}

// Budget data saved before accounts existed is given to the first account
void moveOldDataToCurrentUser()
{
    bool moved = false;
    // fileExists() closes the file first; Windows cannot rename a file that is open
    if (fileExists("budget_plan.txt") && rename("budget_plan.txt", planFileName().c_str()) == 0) moved = true;
    if (fileExists("monthly_history.txt") && rename("monthly_history.txt", historyFileName().c_str()) == 0) moved = true;
    if (moved)
    {
        printColored("Your existing budget data has been moved into this account.\n", CYAN);
    }
}

void setLoggedInUser(const UserAccount& user)
{
    currentUser = user.username;
    currentFullName = user.fullName;
    currentEmail = user.email;
}

// Returns true if the account was created (the user is then logged in)
bool registerUser()
{
    printColored("\n================================================\n", BRIGHT_CYAN);
    printColored("             REGISTER NEW ACCOUNT\n", BRIGHT_CYAN);
    printColored("================================================\n", BRIGHT_CYAN);

    vector<UserAccount> users = loadUsers();
    UserAccount account;
    string password;

    int step = 0; // 0 = full name, 1 = email, 2 = username, 3 = password, 4 = confirm password
    while (step < 5)
    {
        NavAction action = NAV_OK;
        cout << "\n";
        printNavHint("Start Menu");

        if (step == 0)
        {
            printColored(" Full name: ", BRIGHT_WHITE);
            string input;
            action = readLineNav(input);
            if (action == NAV_OK)
            {
                if (!isValidFullName(input))
                {
                    printColored(" Please enter your name (2-40 characters).\n", RED);
                    continue;
                }
                account.fullName = input;
            }
        }
        else if (step == 1)
        {
            printColored(" Email address: ", BRIGHT_WHITE);
            string input;
            action = readLineNav(input);
            if (action == NAV_OK)
            {
                input = toLower(input);
                if (!isValidEmail(input))
                {
                    printColored(" Invalid email address. Example: rahim@gmail.com\n", RED);
                    continue;
                }
                if (findUser(users, input) != -1)
                {
                    printColored(" An account with this email already exists.\n", RED);
                    continue;
                }
                account.email = input;
            }
        }
        else if (step == 2)
        {
            printColored(" Choose a username (3-20 letters, numbers or _): ", BRIGHT_WHITE);
            string input;
            action = readLineNav(input);
            if (action == NAV_OK)
            {
                input = toLower(input);
                if (!isValidUsername(input))
                {
                    printColored(" Invalid username. Use 3-20 letters, numbers or _ only.\n", RED);
                    continue;
                }
                if (findUser(users, input) != -1)
                {
                    printColored(" This username is already taken. Please choose another.\n", RED);
                    continue;
                }
                account.username = input;
            }
        }
        else if (step == 3)
        {
            printColored(" Choose a password (at least 4 characters): ", BRIGHT_WHITE);
            string input;
            action = readPasswordNav(input);
            if (action == NAV_OK)
            {
                if (input.length() < 4)
                {
                    printColored(" Password is too short.\n", RED);
                    continue;
                }
                password = input;
            }
        }
        else
        {
            printColored(" Confirm password: ", BRIGHT_WHITE);
            string input;
            action = readPasswordNav(input);
            if (action == NAV_OK && input != password)
            {
                printColored(" Passwords do not match. Please choose your password again.\n", RED);
                step = 3;
                continue;
            }
        }

        if (action == NAV_MAIN) return false;
        if (action == NAV_BACK)
        {
            if (step == 0) return false; // the previous menu is the start menu
            step--;
            continue;
        }
        step++;
    }

    account.salt = generateSalt();
    account.passwordHash = hashPassword(password, account.salt);

    bool firstAccount = users.empty();
    users.push_back(account);
    if (!saveUsers(users))
    {
        printColored("ERROR: Could not save the new account.\n", RED);
        pressEnterToContinue();
        return false;
    }

    setLoggedInUser(account);
    printColored("\nAccount created! Welcome, " + currentFullName + ".\n", BRIGHT_GREEN);
    printColored("You can log in with your username (" + currentUser + ") or your email.\n", GRAY);
    if (firstAccount) moveOldDataToCurrentUser();
    return true;
}

// Returns true if the login was successful
bool loginUser()
{
    printColored("\n================================================\n", BRIGHT_CYAN);
    printColored("                    LOGIN\n", BRIGHT_CYAN);
    printColored("================================================\n", BRIGHT_CYAN);

    vector<UserAccount> users = loadUsers();
    if (users.empty())
    {
        printColored("No accounts yet. Please register first.\n", YELLOW);
        pressEnterToContinue();
        return false;
    }

    int userIndex = -1;
    int attemptsLeft = 3;
    int step = 0; // 0 = username or email, 1 = password
    while (step < 2)
    {
        NavAction action = NAV_OK;
        cout << "\n";
        printNavHint("Start Menu");

        if (step == 0)
        {
            printColored(" Username or email: ", BRIGHT_WHITE);
            string input;
            action = readLineNav(input);
            if (action == NAV_OK)
            {
                userIndex = findUser(users, toLower(input));
                if (userIndex == -1)
                {
                    printColored(" No account found. Check the spelling or register first.\n", RED);
                    continue;
                }
            }
        }
        else
        {
            printColored(" Password: ", BRIGHT_WHITE);
            string input;
            action = readPasswordNav(input);
            if (action == NAV_OK)
            {
                const UserAccount& user = users[userIndex];
                if (hashPassword(input, user.salt) != user.passwordHash)
                {
                    attemptsLeft--;
                    if (attemptsLeft == 0)
                    {
                        printColored(" Too many wrong attempts.\n", RED);
                        pressEnterToContinue();
                        return false;
                    }
                    printColored(" Wrong password. Attempts left: " + to_string(attemptsLeft) + "\n", RED);
                    continue;
                }
            }
        }

        if (action == NAV_MAIN) return false;
        if (action == NAV_BACK)
        {
            if (step == 0) return false;
            step--;
            continue;
        }
        step++;
    }

    setLoggedInUser(users[userIndex]);
    string greeting = currentFullName.empty() ? currentUser : currentFullName;
    printColored("\nWelcome back, " + greeting + "!\n", BRIGHT_GREEN);
    return true;
}

// NEW: password change with b / m on every step.
// Returns NAV_OK when changed, NAV_BACK when the user went back, NAV_MAIN for the main menu.
NavAction changePasswordSteps(UserAccount& user)
{
    string newPassword;
    int step = 0; // 0 = current password, 1 = new password, 2 = confirm
    while (step < 3)
    {
        NavAction action = NAV_OK;
        cout << "\n";
        printNavHint();
        string input;

        if (step == 0)
        {
            printColored(" Current password: ", BRIGHT_WHITE);
            action = readPasswordNav(input);
            if (action == NAV_OK && hashPassword(input, user.salt) != user.passwordHash)
            {
                printColored(" Wrong password.\n", RED);
                continue;
            }
        }
        else if (step == 1)
        {
            printColored(" New password (at least 4 characters): ", BRIGHT_WHITE);
            action = readPasswordNav(input);
            if (action == NAV_OK)
            {
                if (input.length() < 4)
                {
                    printColored(" Password is too short.\n", RED);
                    continue;
                }
                newPassword = input;
            }
        }
        else
        {
            printColored(" Confirm new password: ", BRIGHT_WHITE);
            action = readPasswordNav(input);
            if (action == NAV_OK && input != newPassword)
            {
                printColored(" Passwords do not match. Please enter the new password again.\n", RED);
                step = 1;
                continue;
            }
        }

        if (action == NAV_MAIN) return NAV_MAIN;
        if (action == NAV_BACK)
        {
            if (step == 0) return NAV_BACK;
            step--;
            continue;
        }
        step++;
    }

    user.salt = generateSalt();
    user.passwordHash = hashPassword(newPassword, user.salt);
    return NAV_OK;
}

// NEW: shows the account details and lets the user change them
void myProfile()
{
    while (true)
    {
        vector<UserAccount> users = loadUsers();
        int index = findUser(users, currentUser);
        if (index == -1)
        {
            printColored("ERROR: Account not found.\n", RED);
            pressEnterToContinue();
            return;
        }
        UserAccount& user = users[index];

        printColored("\n================================================\n", BRIGHT_CYAN);
        printColored("                  MY PROFILE\n", BRIGHT_CYAN);
        printColored("================================================\n", BRIGHT_CYAN);
        cout << " Full Name: ";
        printColored((user.fullName.empty() ? "(not set)" : user.fullName) + "\n", YELLOW);
        cout << " Email:     ";
        printColored((user.email.empty() ? "(not set)" : user.email) + "\n", YELLOW);
        cout << " Username:  ";
        printColored(user.username + "\n", YELLOW);

        printColored("\n  1. Change Name\n", WHITE);
        printColored("  2. Change Email\n", WHITE);
        printColored("  3. Change Password\n", WHITE);
        cout << "\n";
        printNavHint();
        printColored(" Enter your choice (1-3): ", BRIGHT_WHITE);

        int choice = 0;
        NavAction action = readIntNav(1, 3, choice);
        if (action != NAV_OK) return; // b or m: the previous menu is the main menu

        if (choice == 1)
        {
            cout << "\n";
            printNavHint();
            printColored(" New full name: ", BRIGHT_WHITE);
            string input;
            action = readLineNav(input);
            if (action == NAV_MAIN) return;
            if (action == NAV_BACK) continue;
            if (!isValidFullName(input))
            {
                printColored(" Please enter a name with 2-40 characters.\n", RED);
                continue;
            }
            user.fullName = input;
        }
        else if (choice == 2)
        {
            cout << "\n";
            printNavHint();
            printColored(" New email address: ", BRIGHT_WHITE);
            string input;
            action = readLineNav(input);
            if (action == NAV_MAIN) return;
            if (action == NAV_BACK) continue;
            input = toLower(input);
            if (!isValidEmail(input))
            {
                printColored(" Invalid email address. Example: rahim@gmail.com\n", RED);
                continue;
            }
            int other = findUser(users, input);
            if (other != -1 && other != index)
            {
                printColored(" Another account already uses this email.\n", RED);
                continue;
            }
            user.email = input;
        }
        else
        {
            action = changePasswordSteps(user);
            if (action == NAV_MAIN) return;
            if (action == NAV_BACK) continue;
        }

        if (saveUsers(users))
        {
            setLoggedInUser(user);
            printColored(" Profile updated successfully!\n", GREEN);
        }
        else
        {
            printColored(" ERROR: Could not save the changes.\n", RED);
        }
    }
}

// Returns true when a user is logged in, false when the user chose Exit
bool startMenu()
{
    while (true)
    {
        printColored("\n================================================\n", BRIGHT_CYAN);
        printColored("             T A K A  T R A C K\n", CYAN);
        printColored("================================================\n", BRIGHT_CYAN);
        printColored("  1. Register\n", WHITE);
        printColored("  2. Login\n", WHITE);
        printColored("  0. Exit\n", WHITE);
        printColored("------------------------------------------------\n", GRAY);
        printColored(" Enter your choice: ", BRIGHT_WHITE);

        int choice = getValidInput(0, 2);
        if (choice == 0) return false;
        if (choice == 1 && registerUser()) return true;
        if (choice == 2 && loginUser()) return true;
    }
}

// Clears the data of the logged-in user from memory (files are kept)
void resetSessionData()
{
    userName.clear();
    familyMembers.clear();
    totalIncome = 0;
    familyType.clear();
    district.clear();
    costOfLivingLevel.clear();
    budgetPlan = BudgetPlan();
    budgetPlanned = false;
    monthlyRecords.clear();
}

//  BUDGET PLANNER
// NEW: collects the other family members (not the user), one step at a time.
// Returns NAV_OK when finished, NAV_BACK for the previous step, NAV_MAIN for the main menu.
NavAction collectFamilyMembers(vector<FamilyMember>& members)
{
    vector<string> relations = {"Spouse", "Father", "Mother", "Son", "Daughter", "Other"};
    FamilyMember current;
    int subStep = 0; // 0 = name, 1 = relation, 2 = income, 3 = add another?

    if (!members.empty())
    {
        printColored("\nFamily members added so far:\n", CYAN);
        for (const auto& member : members)
        {
            cout << " - " << member.name << " (" << member.relation << ")\n";
        }
    }

    while (true)
    {
        NavAction action = NAV_OK;
        cout << "\n";
        printNavHint();

        if (subStep == 0)
        {
            printColored(" Enter family member name (or 'done' if no more): ", BRIGHT_WHITE);
            string name;
            action = readLineNav(name);
            if (action == NAV_OK)
            {
                if (toLower(name) == "done") return NAV_OK;
                if (name.empty())
                {
                    printColored(" Name cannot be empty.\n", RED);
                    continue;
                }
                current = FamilyMember();
                current.name = name;
            }
            else if (action == NAV_BACK && !members.empty())
            {
                // Go back into the last member that was added
                current = members.back();
                members.pop_back();
                subStep = 3;
                continue;
            }
        }
        else if (subStep == 1)
        {
            printColored(" Select relation for " + current.name + ":\n", CYAN);
            for (size_t i = 0; i < relations.size(); i++)
            {
                cout << " " << i + 1 << ". " << relations[i] << "\n";
            }
            printColored(" Enter your choice (1-" + to_string(relations.size()) + "): ", BRIGHT_WHITE);
            int relationChoice = 0;
            action = readIntNav(1, static_cast<int>(relations.size()), relationChoice);
            if (action == NAV_OK) current.relation = relations[relationChoice - 1];
        }
        else if (subStep == 2)
        {
            printColored(" Enter " + current.name + "'s monthly income (BDT): ", BRIGHT_WHITE);
            action = readDoubleNav(0.0, current.monthlyIncome);
        }
        else
        {
            printColored(" Add another family member? (y/n): ", CYAN);
            string answer;
            action = readLineNav(answer);
            if (action == NAV_OK)
            {
                members.push_back(current);
                if (answer.empty() || tolower(static_cast<unsigned char>(answer[0])) != 'y') return NAV_OK;
                subStep = 0;
                continue;
            }
        }

        if (action == NAV_MAIN) return NAV_MAIN;
        if (action == NAV_BACK)
        {
            if (subStep == 0) return NAV_BACK;
            subStep--;
            continue;
        }
        subStep++;
    }
}

void budgetPlanner()
{
    printColored("\n================================================\n", BRIGHT_CYAN);
    printColored("               BUDGET PLANNER\n", BRIGHT_CYAN);
    printColored("================================================\n", BRIGHT_CYAN);

    // Stores all data in local variables first.
    // Copies the data to global variables only after everything is collected.
    // This prevents an existing plan from being damaged if the user
    // returns to the main menu before finishing.
    printColored(" At any step: type b to go back, or m to go to the main menu.\n", GRAY);

    string newName;
    double myIncome = 0;
    vector<FamilyMember> otherMembers;
    int typeChoice = 0;
    string newDistrict;
    string newLevel;
    double newRent = -1; // -1 = let the plan suggest the rent

    vector<pair<string, string>> districts = getDistrictList();

    // 0 = name, 1 = income, 2 = family members, 3 = family type,
    // 4 = district, 5 = cost of living level, 6 = rent
    int step = 0;
    while (step < 7)
    {
        NavAction action = NAV_OK;

        if (step == 0)
        {
            // NOTE: clearInputBuffer() is not needed here because the input buffer
            // is already empty. Calling it was removing the name entered by the user.
            string defaultName = currentFullName.empty() ? "Me" : currentFullName;
            cout << "\n";
            printNavHint();
            printColored(" Enter your name (press Enter for '" + defaultName + "'): ", BRIGHT_WHITE);
            string input;
            action = readLineNav(input);
            if (action == NAV_OK) newName = input.empty() ? defaultName : input;
        }
        else if (step == 1)
        {
            cout << "\n";
            printNavHint();
            printColored(" Enter your monthly income (BDT): ", BRIGHT_WHITE);
            action = readDoubleNav(0.0, myIncome);
        }
        else if (step == 2)
        {
            // "Myself" is not offered here, so income can't be counted twice
            action = collectFamilyMembers(otherMembers);
        }
        else if (step == 3)
        {
            double runningTotal = myIncome;
            printColored("\nFamily Members:\n", CYAN);
            cout << " - " << newName << " (Myself): ";
            printColored("BDT " + toStr(myIncome) + "\n", GREEN);
            for (const auto& member : otherMembers)
            {
                cout << " - " << member.name << " (" << member.relation << "): ";
                printColored("BDT " + toStr(member.monthlyIncome) + "\n", GREEN);
                runningTotal += member.monthlyIncome;
            }
            cout << " Total Income: ";
            printColored("BDT " + toStr(runningTotal) + "\n\n", BRIGHT_GREEN);

            printColored(" Select your family type:\n", CYAN);
            printColored("\n SINGLE PERSON:\n", BRIGHT_YELLOW);
            for (size_t i = 0; i < 3; i++)
            {
                cout << " " << i + 1 << ". " << budgetPlansCatalog[i].name << "\n";
            }

            printColored("\n COUPLES:\n", BRIGHT_YELLOW);
            for (size_t i = 3; i < 5; i++)
            {
                cout << " " << i + 1 << ". " << budgetPlansCatalog[i].name << "\n";
            }

            printColored("\n FAMILIES WITH CHILDREN:\n", BRIGHT_YELLOW);
            for (size_t i = 5; i < 7; i++)
            {
                cout << " " << i + 1 << ". " << budgetPlansCatalog[i].name << "\n";
            }

            cout << "\n";
            printNavHint();
            printColored(" Enter your choice (1-" + to_string(budgetPlansCatalog.size()) + "): ", BRIGHT_WHITE);
            action = readIntNav(1, static_cast<int>(budgetPlansCatalog.size()), typeChoice);
        }
        else if (step == 4)
        {
            // NEW: the user picks from a list, so they know which districts are available
            printColored("\n Select your district:\n", CYAN);
            string lastLevel;
            for (size_t i = 0; i < districts.size(); i++)
            {
                if (districts[i].second != lastLevel)
                {
                    printColored("\n " + districts[i].second + " cost of living:\n", BRIGHT_YELLOW);
                    lastLevel = districts[i].second;
                }
                cout << " " << i + 1 << ". " << districts[i].first << "\n";
            }
            int otherChoice = static_cast<int>(districts.size()) + 1;
            printColored("\n " + to_string(otherChoice) + ". Other district (type the name)\n", WHITE);

            cout << "\n";
            printNavHint();
            printColored(" Enter your choice (1-" + to_string(otherChoice) + "): ", BRIGHT_WHITE);
            int districtChoice = 0;
            action = readIntNav(1, otherChoice, districtChoice);
            if (action == NAV_OK)
            {
                if (districtChoice < otherChoice)
                {
                    newDistrict = districts[districtChoice - 1].first;
                }
                else
                {
                    printNavHint();
                    printColored(" Enter your district name: ", BRIGHT_WHITE);
                    string input;
                    NavAction nameAction = readLineNav(input);
                    if (nameAction == NAV_MAIN) return;
                    if (nameAction == NAV_BACK) continue; // show the district list again
                    if (input.empty() || input.find_first_of("0123456789") != string::npos)
                    {
                        printColored(" Please enter a valid district name.\n", RED);
                        continue;
                    }
                    newDistrict = input;
                }
            }
        }
        else if (step == 5)
        {
            // NEW: the user decides the cost of living level (a suggestion is given)
            string suggested = getCostOfLivingLevel(newDistrict);
            printColored("\n Cost of living level for " + newDistrict + "\n", CYAN);
            if (isKnownDistrict(newDistrict))
            {
                printColored(" Suggested from Numbeo data: " + suggested + "\n", GRAY);
            }
            else
            {
                printColored(" " + newDistrict + " has no Numbeo data. Medium is suggested, but you know your area best.\n", GRAY);
            }
            printColored(" 1. High   (big city: high rent and prices)\n", WHITE);
            printColored(" 2. Medium (divisional or district town)\n", WHITE);
            printColored(" 3. Low    (small town or village)\n", WHITE);

            cout << "\n";
            printNavHint();
            printColored(" Choose 1-3, or press Enter to use " + suggested + ": ", BRIGHT_WHITE);
            string input;
            action = readLineNav(input);
            if (action == NAV_OK)
            {
                if (input.empty()) newLevel = suggested;
                else if (input == "1") newLevel = "High";
                else if (input == "2") newLevel = "Medium";
                else if (input == "3") newLevel = "Low";
                else
                {
                    printColored(" Please enter 1, 2, 3 or just press Enter.\n", RED);
                    continue;
                }
            }
        }
        else
        {
            // NEW: real rent makes the plan practical (rent is a fixed cost)
            double householdIncome = myIncome;
            for (const auto& member : otherMembers) householdIncome += member.monthlyIncome;
            double suggestedRent = getSuggestedRent(budgetPlansCatalog[typeChoice - 1].name, newLevel, householdIncome);

            printColored("\n How much rent (or home-loan payment) do you pay each month?\n", CYAN);
            printColored(" Enter 0 if you live in your own or family house.\n", GRAY);

            cout << "\n";
            printNavHint();
            printColored(" Monthly rent in BDT (press Enter to let the plan suggest about BDT " +
                         toStr(suggestedRent) + "): ", BRIGHT_WHITE);
            string input;
            action = readLineNav(input);
            if (action == NAV_OK)
            {
                if (input.empty())
                {
                    newRent = -1;
                }
                else
                {
                    istringstream stream(input);
                    double value;
                    char extra;
                    if (!(stream >> value) || (stream >> extra) || value < 0 || value > 1e12)
                    {
                        printColored(" Please enter an amount like 8000, 0, or just press Enter.\n", RED);
                        continue;
                    }
                    newRent = value;
                }
            }
        }

        if (action == NAV_MAIN) return; // nothing has been changed yet
        if (action == NAV_BACK)
        {
            if (step == 0) return; // the previous menu is the main menu
            step--;
            continue;
        }
        step++;
    }

    // The user is stored as a family member so their income is saved to file
    vector<FamilyMember> newMembers;
    newMembers.push_back({newName, "Myself", myIncome});
    double newTotalIncome = myIncome;
    for (const auto& member : otherMembers)
    {
        newMembers.push_back(member);
        newTotalIncome += member.monthlyIncome;
    }

    // Keep this month's spending if a plan already exists
    vector<BudgetCategory> oldCategories;
    if (budgetPlanned) oldCategories = budgetPlan.categories;

    // Commit everything
    userName = newName;
    familyMembers = newMembers;
    totalIncome = newTotalIncome;
    familyType = budgetPlansCatalog[typeChoice - 1].name;
    district = newDistrict;
    costOfLivingLevel = newLevel;
    budgetPlan.actualRent = newRent;
    cout << "\n Cost of Living Level: ";
    printColored(costOfLivingLevel + "\n", YELLOW);

    calculateBudgetPlan();
    budgetPlanned = true;

    // Carry this month's spending over to the new plan (matched by category name)
    vector<string> notMoved;
    for (const auto& oldCat : oldCategories)
    {
        if (oldCat.spentAmount <= 0) continue;
        bool moved = false;
        for (auto& cat : budgetPlan.categories)
        {
            if (cat.name == oldCat.name)
            {
                cat.spentAmount = oldCat.spentAmount;
                moved = true;
                break;
            }
        }
        if (!moved) notMoved.push_back(oldCat.name + " (BDT " + toStr(oldCat.spentAmount) + ")");
    }
    if (!notMoved.empty())
    {
        printColored(" These spending records do not fit the new plan and were not kept:\n", YELLOW);
        for (const auto& item : notMoved) printColored("  - " + item + "\n", YELLOW);
    }

    // NEW: build a practical plan straight away (enough food, savings and emergency fund)
    vector<BudgetCategory> standardPlan = budgetPlan.categories;
    fixBudgetPlan();
    for (size_t i = 0; i < budgetPlan.categories.size() && i < standardPlan.size(); i++)
    {
        if (fabs(budgetPlan.categories[i].allocatedAmount - standardPlan[i].allocatedAmount) >= 1)
        {
            printColored(" The standard plan was adjusted to fit your income, family size and area.\n", CYAN);
            break;
        }
    }

    // Final check: explains anything that is still not practical
    runPracticalityCheck();

    displayBudgetSummary();

    saveCurrentMonthRecord();
    writeBudgetPlanFile();

    printColored("\nBudget planning completed successfully!\n", GREEN);
    pressEnterToContinue();
}

//  MAIN MENU
// Returns true if the user chose Exit, false if the user logged out
bool mainMenu()
{
    int choice;
    bool loggedOut = false;

    do
    {
        printColored("\n================================================\n", BRIGHT_CYAN);
        printColored("             T A K A  T R A C K\n", CYAN);
        printColored("================================================\n\n", BRIGHT_CYAN);

        cout << " Logged in as: ";
        printColored((currentFullName.empty() ? currentUser : currentFullName + " (" + currentUser + ")") + "\n",
                     BRIGHT_MAGENTA);
        cout << " Current Month: ";
        printColored(currentMonth + " " + currentYear, YELLOW);
        cout << "\n";
        cout << " Family Type: ";
        if (budgetPlanned) printColored(familyType + "\n", GREEN);
        else printColored("Not Set\n", RED);
        cout << " Total Income: ";
        if (budgetPlanned) printColored("BDT " + toStr(totalIncome) + "\n", GREEN);
        else printColored("Not Set\n", RED);

        printColored("\n================================================\n", BRIGHT_YELLOW);
        printColored("                   MAIN MENU", BRIGHT_YELLOW);
        printColored("\n================================================\n", BRIGHT_YELLOW);
        printColored("  1. Budget Planner\n", WHITE);
        printColored("  2. View Budget Summary\n", WHITE);
        printColored("  3. Update Category Spending\n", WHITE);
        printColored("  4. View Previous Months\n", WHITE);
        printColored("  5. Save Budget Plan\n", WHITE);
        printColored("  6. Load Budget Plan\n", WHITE);
        printColored("  7. Destroy All Data\n", WHITE);
        printColored("  8. Check Budget Practicality\n", WHITE);
        printColored("  9. My Profile\n", WHITE);
        printColored(" 10. Logout\n", WHITE);
        printColored("  0. Exit\n", WHITE);
        printColored("------------------------------------------------\n", GRAY);
        printColored(" Enter your choice: ", BRIGHT_WHITE);

        choice = getValidInput(0, 10);

        switch (choice)
        {
        case 1:
            budgetPlanner();
            break;
        case 2:
            if (budgetPlanned) displayBudgetSummary();
            else printColored("ERROR: Please create a budget plan first!\n", RED);
            pressEnterToContinue();
            break;
        case 3:
            updateCategorySpending();
            break;
        case 4:
            viewPreviousMonths();
            break;
        case 5:
            saveBudgetPlan();
            pressEnterToContinue();
            break;
        case 6:
            loadBudgetPlan();
            pressEnterToContinue();
            break;
        case 7:
            clearAllData();
            break;
        case 8:
            if (budgetPlanned)
            {
                if (runPracticalityCheck())
                {
                    displayBudgetSummary();
                    saveCurrentMonthRecord();
                    writeBudgetPlanFile();
                }
            }
            else printColored("ERROR: Please create a budget plan first!\n", RED);
            pressEnterToContinue();
            break;
        case 9:
            myProfile();
            break;
        case 10:
            if (budgetPlanned)
            {
                printColored("\nSaving data before logout...\n", YELLOW);
                saveBudgetPlan();
            }
            resetSessionData();
            printColored("Logged out successfully. Goodbye, " +
                         (currentFullName.empty() ? currentUser : currentFullName) + "!\n", BRIGHT_GREEN);
            currentUser.clear();
            currentFullName.clear();
            currentEmail.clear();
            loggedOut = true;
            break;
        case 0:
            if (budgetPlanned)
            {
                printColored("\nSaving data before exit...\n", YELLOW);
                saveBudgetPlan();
            }
            printColored("Thank you for using TakaTrack!\n", BRIGHT_GREEN);
            break;
        }
    }
    while (choice != 0 && !loggedOut);

    return choice == 0;
}

//  MAIN FUNCTION
int main()
{
    SetConsoleTitleA("TakaTrack");

    printColored("================================================\n", BRIGHT_GREEN);
    printColored("              WELCOME TO TakaTrack \n", BRIGHT_MAGENTA);
    printColored("================================================\n\n", BRIGHT_GREEN);

    currentMonth = getCurrentMonth();
    currentYear = getCurrentYear();
    cout << " Current Month: ";
    printColored(currentMonth + " " + currentYear + "\n\n", BRIGHT_YELLOW);

    // NEW: register / login first, then load that user's own data
    while (true)
    {
        if (!startMenu()) break;   // Exit chosen on the start menu

        resetSessionData();
        loadMonthlyHistory();
        loadBudgetPlan();

        if (mainMenu()) break;     // Exit chosen; Logout goes back to the start menu
    }

    printColored("\nPress Enter to exit...", GRAY);
    string dummy;
    getline(cin, dummy);
    return 0;
}
