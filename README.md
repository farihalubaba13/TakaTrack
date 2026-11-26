# TakaTrack

A console-based monthly budget planner for families in Bangladesh, written in C++.

## About the Project

TakaTrack helps a person or a family plan their monthly budget in Taka (BDT). You enter your income, family members, family type, district and rent. The program then divides your income into categories like food, rent, utilities, transport, education and savings.

After making the plan, the program checks if the budget is practical for your family size and area. For example, it checks if there is enough money for food and if you are saving enough. If something is wrong, it fixes the plan by moving money between categories. During the month you can record what you spend, and later you can look at your previous months.

This project was first called finOX. Later it was renamed to TakaTrack.

## Purpose

Most budgeting apps are either too complicated or need a paid subscription. I wanted to make something simple that fits the cost of living in Bangladesh. This project was also good practice for file handling, structs, vectors, maps and input checking in C++.

## Features

- Register and log in with full name, email, username and password
- Password is hidden with `*` while typing and is saved as a hash, not as plain text
- Every user has their own saved data
- Step-by-step budget planner with 7 family types (from Single Student to Growing Family)
- Pick a district from a list, or type any other district
- Choose the cost of living level yourself (High, Medium or Low)
- Enter the rent you really pay, or 0 if you live in your own house
- Practicality check that fixes the plan automatically
- Add spending to any category, and correct a wrong entry
- Monthly history with total spent, total saved and savings rate
- Color-coded output (green = good, yellow = careful, red = over budget)
- At any input step, type `b` to go back or `m` to go to the main menu
- My Profile option to change name, email and password

## Technologies Used

- C++ (C++14)
- `windows.h` for console colors
- `conio.h` for hidden password typing
- STL: `vector`, `map`, `string`
- Text files (`fstream`) for saving data

## Requirements

- Windows
- A C++ compiler that supports C++14 or newer, for example MinGW g++ (it comes with Code::Blocks)

## How to Compile and Run

**Using Code::Blocks**

1. Open `takatrack.cpp` in Code::Blocks.
2. Go to **Settings > Compiler** and tick **"Have g++ follow the C++14 ISO C++ language standard [-std=c++14]"**.
3. Press **F9** (Build and run).

**Using the command line**

```
g++ -std=c++14 takatrack.cpp -o TakaTrack.exe
TakaTrack.exe
```

## How to Use

1. Start the program. Choose **Register** to make an account, or **Login** if you already have one.
2. From the main menu, choose **1 (Budget Planner)** and answer the questions.
3. The program makes your plan, checks it and shows the budget summary.
4. During the month, use **3 (Update Category Spending)** to add what you spend.
5. Use **2** to see your current budget and **4** to see previous months.

Main menu:

```
 1. Budget Planner
 2. View Budget Summary
 3. Update Category Spending
 4. View Previous Months
 5. Save Budget Plan
 6. Load Budget Plan
 7. Destroy All Data
 8. Check Budget Practicality
 9. My Profile
10. Logout
 0. Exit
```

The program saves data in text files in the same folder:

- `users.txt` - user accounts
- `budget_plan_<username>.txt` - the current plan of that user
- `monthly_history_<username>.txt` - the monthly records of that user

These files hold personal data, so they are not uploaded to GitHub (see `.gitignore`).

## How the Practicality Check Works

First, the program finds the household size and an estimated minimum living cost. Then it puts the income in one of three levels: **Low** (below the minimum), **Tight** (below 1.5 times the minimum) or **Comfortable**.

It checks these rules:

- Rent should be 40% of income or less
- Food should be at least the minimum for each person
- Savings should be at least 10% (Comfortable), 7% (Tight) or 5% (Low)
- The emergency fund should be at least 5%, 4% or 3%

If a rule is broken, money is moved from non-essential spending first, then from extra savings, then a small part from other essentials. Rent that the user enters is never changed. If the real problem is very high rent or very low income, the program says so instead of hiding it.

Estimated minimum costs used (BDT per month):

| Cost level | First person | Each extra person | Food per person |
|------------|--------------|-------------------|-----------------|
| High       | 15,000       | 9,000             | 4,000           |
| Medium     | 12,000       | 7,000             | 3,500           |
| Low        | 10,000       | 6,000             | 3,000           |

These numbers are estimates based on Numbeo cost of living prices for Bangladesh (March 2026). The category percentages for each family type are my own design.

## Project Structure

```
├── takatrack.cpp   (full source code)
├── README.md
└── .gitignore
```

The whole program is in one file. Inside, it is divided into sections: colors, data structures, utility and navigation functions, cost of living, budget calculation, display, practicality check, monthly history, spending, file operations, user accounts, budget planner and main menu.

## Limitations

- Works only on Windows
- The password hashing is fine for a student project, but it is not bank-level security
- The cost figures are estimates and need updating when prices change

## Academic Information

- **Course:** SDP I (Software Development Project I)
- **Year:** 1st Year
- **Semester:** 2nd Semester
