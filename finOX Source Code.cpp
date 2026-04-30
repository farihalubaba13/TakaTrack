#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <map>
#include <string>
#include <vector>
#include <sstream>
#include <ctime>
#include <cctype>
#include <windows.h> // For colors

using namespace std;

// ====================== COLOR FUNCTIONS ======================
enum Color {
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

void setColor(Color color) {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, color);
}

void resetColor() {
    setColor(WHITE);
}

void printColored(const string& text, Color color) {
    setColor(color);
    cout << text;
    resetColor();
}

// ====================== DATA STRUCTURES ======================
struct FamilyMember {
    string name;
    string relation;
    double monthlyIncome;
};

struct BudgetCategory {
    string name;
    double allocatedAmount;
    double spentAmount;
    double availableAmount() const { return allocatedAmount - spentAmount; }
};

struct BudgetPlan {
    string name;
    string district;
    double totalIncome;
    vector<BudgetCategory> categories;
    string focus;
    string month; // Added month tracking
};

struct MonthlyRecord {
    string month;
    string year;
    BudgetPlan budgetPlan;
    double totalSpent;
    double totalSaved;
};

// ====================== GLOBAL VARIABLES ======================
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

// ====================== ENHANCED BUDGET PLANS CATALOG ======================
vector<BudgetPlan> budgetPlansCatalog = {
    // Single Person Categories
    {"Single Student", "", 0, {
        {"Groceries & Food", 0, 0},
        {"Hostel Rent", 0, 0},
        {"Utilities", 0, 0},
        {"Transport", 0, 0},
        {"Books & Supplies", 0, 0},
        {"Emergency Fund", 0, 0},
        {"Entertainment", 0, 0},
        {"Savings", 0, 0}
    }, "Focus: Education & Basics", ""},

    {"Young Professional", "", 0, {
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
    }, "Focus: Career Growth", ""},

    {"Single Parent (1 Child)", "", 0, {
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
    }, "Focus: Child Support", ""},

    // Couple Categories
    {"Newly Married Couple", "", 0, {
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
    }, "Focus: Building Foundation", ""},

    {"Working Couple (DINK)", "", 0, {
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
    }, "Focus: Wealth Building", ""},

    // Family with Children
    {"Young Family (1 Child)", "", 0, {
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
    }, "Focus: Child Development", ""},

    {"Growing Family (2 Children)", "", 0, {
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
    }, "Focus: Education & Growth", ""}
};

// ====================== UTILITY FUNCTIONS ======================
void clearInputBuffer() {
    cin.clear();
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
}

int getValidInput(int min, int max) {
    int input;
    while (true) {
        if (cin >> input && input >= min && input <= max) {
            clearInputBuffer();
            return input;
        }
        printColored("Invalid input. Please enter a number between " + to_string(min) + " and " + to_string(max) + ": ", RED);
        clearInputBuffer();
    }
}

double getValidDouble(double minInclusive) {
    double val;
    while (true) {
        if (cin >> val && val >= minInclusive) {
            clearInputBuffer();
            return val;
        }
        printColored("Invalid number. Please enter a value >= " + to_string(minInclusive) + ": ", RED);
        clearInputBuffer();
    }
}

void pressEnterToContinue() {
    printColored("\nPress Enter to continue...", GRAY);
    clearInputBuffer();
    cin.get();
}

// ====================== NAVIGATION FUNCTIONS ======================
bool askToGoBack(const string& targetMenu = "previous menu") {
    printColored("\nPress 0 to go back to " + targetMenu + ", or any other key to continue: ", GRAY);
    string input;
    getline(cin, input);
    return (input == "0");
}

bool askToGoMainMenu() {
    printColored("\nPress 0 to go back to Main Menu, or any other key to continue: ", GRAY);
    string input;
    getline(cin, input);
    return (input == "0");
}

bool confirmAction(const string& action) {
    printColored("\nAre you sure you want to " + action + "? (1=Yes, 0=No): ", YELLOW);
    int choice = getValidInput(0, 1);
    return (choice == 1);
}

// ====================== DATE AND TIME FUNCTIONS ======================
string getCurrentMonth() {
    vector<string> months = {
        "January", "February", "March", "April", "May", "June",
        "July", "August", "September", "October", "November", "December"
    };

    time_t now = time(0);
    tm* ltm = localtime(&now);
    return months[ltm->tm_mon];
}

string getCurrentYear() {
    time_t now = time(0);
    tm* ltm = localtime(&now);
    return to_string(1900 + ltm->tm_year);
}

vector<string> getAllMonths() {
    return {
        "January", "February", "March", "April", "May", "June",
        "July", "August", "September", "October", "November", "December"
    };
}

// ====================== COST OF LIVING CALCULATION ======================
string getCostOfLivingLevel(const string& district) {
    map<string, string> costMap = {
        {"dhaka", "High"}, {"chattogram", "High"}, {"sylhet", "High"},
        {"khulna", "Medium"}, {"rajshahi", "Medium"}, {"barisal", "Medium"},
        {"rangpur", "Low"}, {"mymensingh", "Low"}
    };

    string lowerDistrict = district;
    transform(lowerDistrict.begin(), lowerDistrict.end(), lowerDistrict.begin(), ::tolower);

    if (costMap.find(lowerDistrict) != costMap.end()) {
        return costMap[lowerDistrict];
    }
    return "Medium";
}

// ====================== BUDGET CALCULATION FUNCTIONS ======================
void calculateBudgetPlan() {
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

    // Apply allocations
    vector<double> percentages = budgetTable[familyType][costOfLivingLevel];

    budgetPlan.categories.clear();
    budgetPlan.name = familyType;
    budgetPlan.district = district;
    budgetPlan.totalIncome = totalIncome;
    budgetPlan.month = currentMonth; // Set current month

    // Set focus based on cost level
    if (costOfLivingLevel == "Low") budgetPlan.focus = "Balanced Growth";
    else if (costOfLivingLevel == "Medium") budgetPlan.focus = "Wealth Building";
    else budgetPlan.focus = "Essential Spending";

    // Find matching plan from catalog
    for (const auto& plan : budgetPlansCatalog) {
        if (plan.name == familyType) {
            for (size_t i = 0; i < min(percentages.size(), plan.categories.size()); i++) {
                BudgetCategory cat = plan.categories[i];
                cat.allocatedAmount = totalIncome * (percentages[i] / 100.0);
                cat.spentAmount = 0;
                budgetPlan.categories.push_back(cat);
            }
            break;
        }
    }
}

// ====================== DISPLAY FUNCTIONS ======================
void displayBudgetSummary() {
    printColored("\n================================================\n", CYAN);
    printColored("               BUDGET SUMMARY\n", CYAN);
    printColored("================================================\n", CYAN);

    cout << "Month: "; printColored(currentMonth + " " + currentYear, YELLOW); cout << "\n";
    cout << "Family Type: "; printColored(familyType, YELLOW); cout << "\n";
    cout << "District: "; printColored(district, YELLOW);
    cout << " (Cost Level: "; printColored(costOfLivingLevel, YELLOW); cout << ")\n";
    cout << "Total Income: "; printColored("BDT " + to_string((int)budgetPlan.totalIncome), GREEN); cout << "\n";
    cout << "Focus: "; printColored(budgetPlan.focus, MAGENTA); cout << "\n\n";

    printColored("--------------------------------------------------------------\n", GRAY);
    printColored("Category                     | Allocated    | Spent      | Available\n", WHITE);
    printColored("--------------------------------------------------------------\n", GRAY);

    double totalAllocated = 0, totalSpent = 0, totalAvailable = 0;

    for (const auto& cat : budgetPlan.categories) {
        cout << " ";
        string displayName = cat.name.length() > 28 ? cat.name.substr(0, 25) + "..." : cat.name;
        printColored(displayName, WHITE);
        cout << string(30 - displayName.length(), ' ') << "| ";
        printColored(to_string((int)cat.allocatedAmount), BRIGHT_GREEN);
        cout << string(12 - to_string((int)cat.allocatedAmount).length(), ' ') << "| ";
        printColored(to_string((int)cat.spentAmount), cat.spentAmount > cat.allocatedAmount * 0.8 ? RED : YELLOW);
        cout << string(10 - to_string((int)cat.spentAmount).length(), ' ') << "| ";
        printColored(to_string((int)cat.availableAmount()), cat.availableAmount() < 0 ? RED : BRIGHT_GREEN);
        cout << "\n";

        totalAllocated += cat.allocatedAmount;
        totalSpent += cat.spentAmount;
        totalAvailable += cat.availableAmount();
    }

    printColored("--------------------------------------------------------------\n", GRAY);
    cout << " "; printColored("TOTAL", BRIGHT_WHITE);
    cout << string(25, ' ') << "| ";
    printColored(to_string((int)totalAllocated), BRIGHT_GREEN);
    cout << string(12 - to_string((int)totalAllocated).length(), ' ') << "| ";
    printColored(to_string((int)totalSpent), BRIGHT_YELLOW);
    cout << string(10 - to_string((int)totalSpent).length(), ' ') << "| ";
    printColored(to_string((int)totalAvailable), totalAvailable < 0 ? BRIGHT_RED : BRIGHT_GREEN);
    cout << "\n";
    printColored("==============================================================\n", GRAY);

    // Show monthly summary
    double savingsRate = ((totalIncome - totalSpent) / totalIncome) * 100;
    cout << "\nMonthly Summary:\n";
    cout << "Total Spent: "; printColored("BDT " + to_string((int)totalSpent), YELLOW); cout << "\n";
    cout << "Total Saved: "; printColored("BDT " + to_string((int)totalAvailable), BRIGHT_GREEN); cout << "\n";
    cout << "Savings Rate: "; printColored(to_string((int)savingsRate) + "%", savingsRate >= 20 ? BRIGHT_GREEN : YELLOW); cout << "\n";

    // Ask to go back to main menu
    if (askToGoMainMenu()) {
        return;
    }
}

// ====================== PREVIOUS MONTHS FUNCTIONS ======================
void saveCurrentMonthRecord() {
    if (!budgetPlanned) return;

    MonthlyRecord record;
    record.month = currentMonth;
    record.year = currentYear;
    record.budgetPlan = budgetPlan;

    // Calculate totals
    record.totalSpent = 0;
    for (const auto& cat : budgetPlan.categories) {
        record.totalSpent += cat.spentAmount;
    }
    record.totalSaved = budgetPlan.totalIncome - record.totalSpent;

    // Check if this month already exists
    bool monthExists = false;
    for (auto& existingRecord : monthlyRecords) {
        if (existingRecord.month == currentMonth && existingRecord.year == currentYear) {
            existingRecord = record; // Update existing record
            monthExists = true;
            break;
        }
    }

    if (!monthExists) {
        monthlyRecords.push_back(record);
    }

    // Save to file
    ofstream file("monthly_history.txt");
    if (file) {
        file << monthlyRecords.size() << "\n";
        for (const auto& rec : monthlyRecords) {
            file << rec.month << "\n" << rec.year << "\n";
            file << rec.budgetPlan.name << "\n" << rec.budgetPlan.district << "\n";
            file << rec.budgetPlan.totalIncome << "\n" << rec.budgetPlan.focus << "\n";
            file << rec.totalSpent << "\n" << rec.totalSaved << "\n";
            file << rec.budgetPlan.categories.size() << "\n";
            for (const auto& cat : rec.budgetPlan.categories) {
                file << cat.name << "\n" << cat.allocatedAmount << "\n" << cat.spentAmount << "\n";
            }
        }
    }
}

void loadMonthlyHistory() {
    ifstream file("monthly_history.txt");
    if (!file) return;

    monthlyRecords.clear();
    size_t recordCount;
    file >> recordCount;
    file.ignore();

    for (size_t i = 0; i < recordCount; i++) {
        MonthlyRecord record;
        getline(file, record.month);
        getline(file, record.year);
        getline(file, record.budgetPlan.name);
        getline(file, record.budgetPlan.district);
        file >> record.budgetPlan.totalIncome;
        file.ignore();
        getline(file, record.budgetPlan.focus);
        file >> record.totalSpent >> record.totalSaved;
        file.ignore();

        size_t categoryCount;
        file >> categoryCount;
        file.ignore();

        for (size_t j = 0; j < categoryCount; j++) {
            BudgetCategory cat;
            getline(file, cat.name);
            file >> cat.allocatedAmount >> cat.spentAmount;
            file.ignore();
            record.budgetPlan.categories.push_back(cat);
        }

        monthlyRecords.push_back(record);
    }
}

void viewPreviousMonths() {
    if (monthlyRecords.empty()) {
        printColored("\nNo previous months data found.\n", YELLOW);
        pressEnterToContinue();
        return;
    }

    printColored("\n================================================\n", BRIGHT_CYAN);
    printColored("           PREVIOUS MONTHS HISTORY\n", BRIGHT_CYAN);
    printColored("================================================\n", BRIGHT_CYAN);

    // Show summary of all months
    printColored("Monthly Summary:\n", BRIGHT_WHITE);
    printColored("--------------------------------------------------------------\n", GRAY);
    printColored("Month-Year     | Income      | Spent       | Saved       | Savings %\n", WHITE);
    printColored("--------------------------------------------------------------\n", GRAY);

    for (const auto& record : monthlyRecords) {
        cout << " " << record.month << " " << record.year;
        cout << string(13 - (record.month.length() + record.year.length()), ' ') << "| ";
        printColored(to_string((int)record.budgetPlan.totalIncome), BRIGHT_GREEN);
        cout << string(12 - to_string((int)record.budgetPlan.totalIncome).length(), ' ') << "| ";
        printColored(to_string((int)record.totalSpent), YELLOW);
        cout << string(12 - to_string((int)record.totalSpent).length(), ' ') << "| ";
        printColored(to_string((int)record.totalSaved), BRIGHT_GREEN);
        cout << string(12 - to_string((int)record.totalSaved).length(), ' ') << "| ";

        double savingsRate = (record.totalSaved / record.budgetPlan.totalIncome) * 100;
        printColored(to_string((int)savingsRate) + "%", savingsRate >= 20 ? BRIGHT_GREEN : YELLOW);
        cout << "\n";
    }
    printColored("--------------------------------------------------------------\n", GRAY);

    // Option to view details of specific month
    printColored("\nView details of specific month? (1=Yes, 0=No): ", BRIGHT_WHITE);
    int viewDetails = getValidInput(0, 1);

    if (viewDetails == 1) {
        printColored("\nSelect month to view details:\n", CYAN);
        for (size_t i = 0; i < monthlyRecords.size(); i++) {
            cout << " " << i+1 << ". " << monthlyRecords[i].month << " " << monthlyRecords[i].year << "\n";
        }
        printColored(" Enter your choice (1-" + to_string(monthlyRecords.size()) + "): ", BRIGHT_WHITE);

        int choice = getValidInput(1, monthlyRecords.size());
        const MonthlyRecord& selectedRecord = monthlyRecords[choice - 1];

        printColored("\n================================================\n", CYAN);
        printColored("          BUDGET DETAILS FOR " + selectedRecord.month + " " + selectedRecord.year + "\n", CYAN);
        printColored("================================================\n", CYAN);

        cout << "Family Type: "; printColored(selectedRecord.budgetPlan.name, YELLOW); cout << "\n";
        cout << "Total Income: "; printColored("BDT " + to_string((int)selectedRecord.budgetPlan.totalIncome), GREEN); cout << "\n";
        cout << "Total Spent: "; printColored("BDT " + to_string((int)selectedRecord.totalSpent), YELLOW); cout << "\n";
        cout << "Total Saved: "; printColored("BDT " + to_string((int)selectedRecord.totalSaved), BRIGHT_GREEN); cout << "\n";

        double savingsRate = (selectedRecord.totalSaved / selectedRecord.budgetPlan.totalIncome) * 100;
        cout << "Savings Rate: "; printColored(to_string((int)savingsRate) + "%", savingsRate >= 20 ? BRIGHT_GREEN : YELLOW); cout << "\n\n";

        printColored("Category-wise Spending:\n", BRIGHT_WHITE);
        printColored("--------------------------------------------------------------\n", GRAY);
        for (const auto& cat : selectedRecord.budgetPlan.categories) {
            double usagePercent = (cat.spentAmount / cat.allocatedAmount) * 100;
            cout << " " << cat.name << ": ";
            printColored("BDT " + to_string((int)cat.spentAmount) + " / BDT " + to_string((int)cat.allocatedAmount),
                        usagePercent > 100 ? RED : (usagePercent > 80 ? YELLOW : GREEN));
            cout << " (" << (int)usagePercent << "%)\n";
        }
    }
     // Ask to go back to main menu
    if (askToGoMainMenu()) {
        return;
    }

    pressEnterToContinue();
}

// ====================== SPENDING MANAGEMENT ======================
void updateCategorySpending() {
    if (!budgetPlanned) {
        printColored("ERROR: Please create a budget plan first!\n", RED);
        pressEnterToContinue();
        return;
    }

    int choice;
    do {
        printColored("\nSelect category to update spending:\n", CYAN);
        for (size_t i = 0; i < budgetPlan.categories.size(); ++i) {
            cout << " " << (i + 1) << ". " << budgetPlan.categories[i].name << "\n";
        }
        printColored(" 0. Back to Main Menu\n", GRAY);
        printColored(" Enter your choice: ", BRIGHT_WHITE);

        choice = getValidInput(0, budgetPlan.categories.size());

        if (choice > 0) {
            BudgetCategory& cat = budgetPlan.categories[choice - 1];
            printColored("\nUpdating: ", CYAN); printColored(cat.name + "\n", YELLOW);
            cout << "Allocated: "; printColored("BDT " + to_string((int)cat.allocatedAmount) + "\n", GREEN);
            cout << "Already Spent: "; printColored("BDT " + to_string((int)cat.spentAmount) + "\n", YELLOW);
            cout << "Available: "; printColored("BDT " + to_string((int)cat.availableAmount()) + "\n",
                  cat.availableAmount() < 0 ? RED : BRIGHT_GREEN);
            printColored(" Enter amount spent (or 0 to cancel): ", BRIGHT_WHITE);

            double amount = getValidDouble(0.0);

            if (amount == 0.0) {
                printColored("Update cancelled.\n", RED);
                continue;
            }

            if (amount > cat.availableAmount()) {
                printColored("Warning: This exceeds the available amount by BDT ", YELLOW);
                printColored(to_string((int)(amount - cat.availableAmount())) + "\n", RED);
                printColored(" Do you want to proceed? (1=Yes, 0=No): ", YELLOW);

                int proceed = getValidInput(0, 1);

                if (proceed != 1) {
                    printColored("Update cancelled.\n", RED);
                    continue;
                }
            }

            cat.spentAmount += amount;
            printColored("Update successful!\n", GREEN);
            cout << "New spent amount: "; printColored("BDT " + to_string((int)cat.spentAmount) + "\n", YELLOW);
            cout << "New available amount: "; printColored("BDT " + to_string((int)cat.availableAmount()) + "\n", BRIGHT_GREEN);

            // Auto-save the monthly record after spending update
            saveCurrentMonthRecord();

            // Ask to go back
            if (askToGoBack()) {
                break;
            }
        }
    } while (choice != 0);
}

// ====================== FILE OPERATIONS ======================
void saveBudgetPlan() {
    if (!budgetPlanned) {
        printColored("ERROR: No budget plan to save.\n", RED);
        return;
    }

    ofstream file("budget_plan.txt");
    if (!file) {
        printColored("ERROR: Error opening file for saving!\n", RED);
        return;
    }

    file << budgetPlan.name << "\n"
         << budgetPlan.district << "\n"
         << budgetPlan.totalIncome << "\n"
         << budgetPlan.focus << "\n";
    file << budgetPlan.categories.size() << "\n";

    for (const auto& cat : budgetPlan.categories) {
        file << cat.name << "\n" << cat.allocatedAmount << "\n" << cat.spentAmount << "\n";
    }

    file << familyMembers.size() << "\n";
    for (const auto& member : familyMembers) {
        file << member.name << "\n" << member.relation << "\n" << member.monthlyIncome << "\n";
    }

    // Also save current month record
    saveCurrentMonthRecord();

    printColored("Budget plan saved successfully!\n", GREEN);

    // Ask to go back to main menu
    if (askToGoMainMenu()) {
        return;
    }
}

void loadBudgetPlan() {
    ifstream file("budget_plan.txt");
    if (!file) {
        printColored("No saved budget data found.\n", BLUE);
        return;
    }

    getline(file, budgetPlan.name);
    getline(file, budgetPlan.district);
    file >> budgetPlan.totalIncome;
    file.ignore();
    getline(file, budgetPlan.focus);

    size_t categoryCount;
    file >> categoryCount;
    file.ignore();

    budgetPlan.categories.clear();
    for (size_t i = 0; i < categoryCount; i++) {
        BudgetCategory cat;
        getline(file, cat.name);
        file >> cat.allocatedAmount >> cat.spentAmount;
        file.ignore();
        budgetPlan.categories.push_back(cat);
    }

    size_t memberCount;
    file >> memberCount;
    file.ignore();

    familyMembers.clear();
    totalIncome = 0;
    for (size_t i = 0; i < memberCount; i++) {
        FamilyMember member;
        getline(file, member.name);
        getline(file, member.relation);
        file >> member.monthlyIncome;
        file.ignore();
        familyMembers.push_back(member);
        totalIncome += member.monthlyIncome;
    }

    familyType = budgetPlan.name;
    costOfLivingLevel = getCostOfLivingLevel(budgetPlan.district);
    budgetPlanned = true;

    printColored("Budget plan loaded successfully!\n", GREEN);

        // Ask to go back to main menu
    if (askToGoMainMenu()) {
        return;
    }
}


// ====================== CLEAR DATA FUNCTION ======================
void clearAllData() {
    printColored("\n================================================\n", MAGENTA);
    printColored("               CLEAR ALL DATA\n", MAGENTA);
    printColored("================================================\n", MAGENTA);

    printColored("WARNING: This action will permanently delete:\n\n", BRIGHT_RED);
    printColored("1. Current budget plan\n", BRIGHT_WHITE);
    printColored("2. Family member information\n", BRIGHT_WHITE);
    printColored("3. Spending records\n", BRIGHT_WHITE);
    printColored("4. Monthly history\n", BRIGHT_WHITE);
    printColored("5. All saved files\n", BRIGHT_WHITE);

    printColored("\nThis action cannot be undone!\n", BRIGHT_RED);

    if (!confirmAction("clear all data")) {
        printColored("\nData clearance cancelled.\n", BLUE);

        // Ask to go back to main menu
        if (askToGoMainMenu()) {
            return;
        }
        pressEnterToContinue();
        return;
    }

    // Reset all global variables
    userName.clear();
    familyMembers.clear();
    totalIncome = 0;
    familyType.clear();
    district.clear();
    costOfLivingLevel.clear();
    budgetPlan = BudgetPlan(); // Reset to default
    budgetPlanned = false;
    monthlyRecords.clear();

    // Delete saved files
    remove("budget_plan.txt");
    remove("monthly_history.txt");

    printColored("\nAll data has been successfully cleared!\n", GREEN);
    printColored("The application will now start fresh.\n", GREEN);

    // Ask to go back to main menu
    if (askToGoMainMenu()) {
        return;
    }
    pressEnterToContinue();
}


// ====================== BUDGET PLANNER ======================
void budgetPlanner() {
    printColored("\n================================================\n", BRIGHT_CYAN);
    printColored("               BUDGET PLANNER\n", BRIGHT_CYAN);
    printColored("================================================\n", BRIGHT_CYAN);

    printColored(" Enter your name: ", BRIGHT_WHITE);
    clearInputBuffer();
    getline(cin, userName);

    printColored(" Enter your income (BDT): ", BRIGHT_WHITE);
    totalIncome = getValidDouble(0.0);

    char addMore;
    familyMembers.clear();

        do {
        FamilyMember member;
        printColored(" Enter family member name (or '0' to go back to main menu, 'done' if no more): ", BRIGHT_WHITE);
        string name;
        getline(cin, name);

        if (name == "0") {
            if (confirmAction("go back to main menu")) {
                return;
            } else {
                continue;
            }
        }

        if (name == "done" || name == "DONE" || name == "Done") {
            break;
        }
        member.name = name;

        vector<string> relations = {"Myself", "Spouse", "Father", "Mother", "Son", "Daughter", "Other"};

        printColored(" Select relation:\n", CYAN);
        for (size_t i = 0; i < relations.size(); i++) {
            cout << " " << i+1 << ". " << relations[i] << "\n";
        }
        printColored(" Enter your choice (1-" + to_string(relations.size()) + "): ", BRIGHT_WHITE);
        int relationChoice = getValidInput(1, relations.size());
        member.relation = relations[relationChoice - 1];

        printColored(" Enter " + member.name + "'s income (BDT): ", BRIGHT_WHITE);
        member.monthlyIncome = getValidDouble(0.0);

        familyMembers.push_back(member);
        totalIncome += member.monthlyIncome;

        printColored(" Add another family member? (y/n): ", CYAN);
        cin >> addMore;
        clearInputBuffer();
    } while (addMore == 'y' || addMore == 'Y');

    printColored("\nFamily Members:\n", CYAN);
    for (const auto& member : familyMembers) {
        cout << " - " << member.name << " (" << member.relation << "): ";
        printColored("BDT " + to_string((int)member.monthlyIncome) + "\n", GREEN);
    }
    cout << " Total Income: "; printColored("BDT " + to_string((int)totalIncome) + "\n\n", BRIGHT_GREEN);

        // Display family types in categories
    int typeChoice;
    bool validFamilyType = false;

    while (!validFamilyType) {
        printColored(" Select your family type:\n", CYAN);
        printColored("\n SINGLE PERSON:\n", BRIGHT_YELLOW);
        for (size_t i = 0; i < 3; i++) {
            cout << " " << i+1 << ". " << budgetPlansCatalog[i].name << "\n";
        }

        printColored("\n COUPLES:\n", BRIGHT_YELLOW);
        for (size_t i = 3; i < 5; i++) {
            cout << " " << i+1 << ". " << budgetPlansCatalog[i].name << "\n";
        }

        printColored("\n FAMILIES WITH CHILDREN:\n", BRIGHT_YELLOW);
        for (size_t i = 5; i < 7; i++) {
            cout << " " << i+1 << ". " << budgetPlansCatalog[i].name << "\n";
        }

        printColored(" 0. Back to Main Menu\n", GRAY);
        printColored(" Enter your choice (0-" + to_string(budgetPlansCatalog.size()) + "): ", BRIGHT_WHITE);

        typeChoice = getValidInput(0, budgetPlansCatalog.size());

        if (typeChoice == 0) {
            if (confirmAction("go back to main menu")) {
                return;
            } else {
                // Restart family type selection
                continue;
            }
        } else {
            validFamilyType = true;
        }
    }

    familyType = budgetPlansCatalog[typeChoice - 1].name;

    bool validDistrict = false;

    while (!validDistrict) {
        printColored(" Enter your district (or '0' to go back): ", BRIGHT_WHITE);
        getline(cin, district);

        if (district == "0") {
            if (confirmAction("go back to family type selection")) {
                return;
            } else {
                continue;
            }
        } else {
            validDistrict = true;
        }
    }

    costOfLivingLevel = getCostOfLivingLevel(district);
    cout << " Cost of Living Level: "; printColored(costOfLivingLevel + "\n", YELLOW);

    calculateBudgetPlan();
    displayBudgetSummary();

    budgetPlanned = true;

    // Save this month's record
    saveCurrentMonthRecord();

    printColored("Budget planning completed successfully!\n", GREEN);

    // Ask to go back to main menu
    if (askToGoMainMenu()) {
        return;
    }

    pressEnterToContinue();
}

// ====================== MAIN MENU ======================
void mainMenu() {
    int choice;

    do {
        printColored("\n================================================\n", BRIGHT_CYAN);
        printColored("           BUDGET PLANNER APPLICATION\n", CYAN);
        printColored("================================================\n\n", BRIGHT_CYAN);

        cout << " Current Month: "; printColored(currentMonth + " " + currentYear, YELLOW); cout << "\n";
        cout << " Family Type: ";
        if (budgetPlanned) printColored(familyType + "\n", GREEN);
        else printColored("Not Set\n", RED);
        cout << " Total Income: ";
        if (budgetPlanned) printColored("BDT " + to_string((int)totalIncome) + "\n", GREEN);
        else printColored("Not Set\n", RED);

        printColored("\n================================================\n", BRIGHT_YELLOW);
        printColored("                   MAIN MENU", BRIGHT_YELLOW);
        printColored("\n================================================\n", BRIGHT_YELLOW);
        printColored("  1. Budget Planner\n", WHITE);
        printColored("  2. View Budget Summary\n", WHITE);
        printColored("  3. Update Category Spending\n", WHITE);
        printColored("  4. View Previous Months\n", WHITE); // NEW OPTION
        printColored("  5. Save Budget Plan\n", WHITE);
        printColored("  6. Load Budget Plan\n", WHITE);
        printColored("  7. Destroy All Data\n", WHITE);
        printColored("  0. Exit\n", WHITE);
        printColored("------------------------------------------------\n", GRAY);
        printColored(" Enter your choice: ", BRIGHT_WHITE);

        choice = getValidInput(0, 7);

        switch (choice) {
            case 1: budgetPlanner(); break;
            case 2:
                if (budgetPlanned) displayBudgetSummary();
                else printColored("ERROR: Please create a budget plan first!\n", RED);
                pressEnterToContinue();
                break;
            case 3: updateCategorySpending(); break;
            case 4: viewPreviousMonths(); break; // NEW FEATURE
            case 5: saveBudgetPlan(); break;
            case 6: loadBudgetPlan(); break;
            case 7: clearAllData(); break;
            case 0:
                printColored("\nSaving data before exit...\n", YELLOW);
                saveBudgetPlan();
                printColored("Thank you for using Budget Planner Application!\n", BRIGHT_GREEN);
                break;
        }
    } while (choice != 0);
}

// ====================== MAIN FUNCTION ======================
int main() {
    // Set console title
    SetConsoleTitleA("Budget Planner Application");

    printColored("================================================\n", BRIGHT_GREEN);
    printColored("               WELCOME TO finOX \n", BRIGHT_MAGENTA);
    printColored("================================================\n\n", BRIGHT_GREEN);

    // Initialize current month and year
    currentMonth = getCurrentMonth();
    currentYear = getCurrentYear();
    cout << " Current Month: "; printColored(currentMonth + " " + currentYear + "\n\n", BRIGHT_YELLOW);

    // Try to load existing data
    loadBudgetPlan();
    loadMonthlyHistory(); // Load previous months data

    mainMenu();

    printColored("\nPress Enter to exit...", GRAY);
    clearInputBuffer();
    cin.get();
    return 0;
}
