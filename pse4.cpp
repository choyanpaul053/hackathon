#include <iostream>
#include <string>
#include <cctype>
#include <algorithm>
using namespace std;

struct Result {
    string label;
    int score;      // 0 - 100
    bool usesPersonalInfo;
    bool usesSimplePattern;
};

string toLowerStr(string s) {
    for (char &c : s) c = static_cast<char>(tolower(static_cast<unsigned char>(c)));
    return s;
}

bool containsIgnoreCase(const string &text, const string &pat) {
    if (pat.empty()) return false;
    string t = toLowerStr(text);
    string p = toLowerStr(pat);
    return t.find(p) != string::npos;
}

bool isSimpleSequence(const string &s) {
    if (s.size() < 3) return false;

    bool inc = true, dec = true, same = true;
    for (size_t i = 1; i < s.size(); ++i) {
        if (s[i] != s[i-1] + 1) inc = false;
        if (s[i] != s[i-1] - 1) dec = false;
        if (s[i] != s[0])       same = false;
    }
    return inc || dec || same;   // e.g. 1234, 4321, 1111, abcd, qwer-like patterns[web:41]
}

Result evaluateStrength(const string &password,
                        const string &firstName,
                        const string &lastName,
                        const string &dob)   // dob like "2004", "2004-05-21", "21-05-2004"
{
    bool hasLower = false;
    bool hasUpper = false;
    bool hasDigit = false;
    bool hasSpecial = false;

    for (char ch : password) {
        unsigned char c = static_cast<unsigned char>(ch);
        if (islower(c)) hasLower = true;
        else if (isupper(c)) hasUpper = true;
        else if (isdigit(c)) hasDigit = true;
        else hasSpecial = true;
    }

    int length = password.length();
    int score = 0;

    // Length contribution (max 40)
    if (length >= 8) {
        score += 20;
        if (length >= 12) score += 20;
    } else if (length >= 6) {
        score += 10;
    }

    // Character type diversity (max 40)
    if (hasLower)   score += 10;
    if (hasUpper)   score += 10;
    if (hasDigit)   score += 10;
    if (hasSpecial) score += 10;

    int typeCount = hasLower + hasUpper + hasDigit + hasSpecial;

    // Bonus for long & diverse (max 20)
    if (typeCount >= 3 && length >= 10) score += 10;
    if (typeCount == 4 && length >= 14) score += 10;

    // PERSONAL INFO & WEAK PATTERNS PENALTIES
    bool usesPersonal = false;
    bool usesPattern = false;

    // 1) Contains first or last name or parts of them[web:38][web:41]
    if (!firstName.empty() && containsIgnoreCase(password, firstName)) {
        score -= 20;
        usesPersonal = true;
    }
    if (!lastName.empty() && containsIgnoreCase(password, lastName)) {
        score -= 20;
        usesPersonal = true;
    }

    // Also penalize if it contains 3+ char prefix of first/last name
    if (firstName.size() >= 3) {
        string part = firstName.substr(0, 3);
        if (containsIgnoreCase(password, part)) {
            score -= 10;
            usesPersonal = true;
        }
    }
    if (lastName.size() >= 3) {
        string part = lastName.substr(0, 3);
        if (containsIgnoreCase(password, part)) {
            score -= 10;
            usesPersonal = true;
        }
    }

    // 2) Date of birth patterns: full DOB string and year[web:38][web:42]
    if (!dob.empty() && containsIgnoreCase(password, dob)) {
        score -= 20;
        usesPersonal = true;
    }

    // Extract a 4-digit year from dob (very simple heuristic)
    string year = "";
    for (char c : dob) {
        if (isdigit(static_cast<unsigned char>(c))) year.push_back(c);
        if (year.size() == 4) break;
    }

    if (year.size() == 4) {
        if (password.find(year) != string::npos) {     // e.g. John2004
            score -= 20;
            usesPersonal = true;
        }
    }

    // 3) Simple numeric or letter sequences in the password[web:41][web:48]
    string digitsOnly, lettersOnly;
    for (char c : password) {
        if (isdigit(static_cast<unsigned char>(c)))
            digitsOnly.push_back(c);
        else if (isalpha(static_cast<unsigned char>(c)))
            lettersOnly.push_back(static_cast<char>(tolower(c)));
    }

    if (isSimpleSequence(digitsOnly) || isSimpleSequence(lettersOnly)) {
        score -= 15;
        usesPattern = true;
    }

    // Clamp score
    if (score < 0) score = 0;
    if (score > 100) score = 100;

    // Map score to label
    string label;
    if (score < 30)       label = "Very Weak";
    else if (score < 50)  label = "Weak";
    else if (score < 70)  label = "Fair";
    else if (score < 85)  label = "Good";
    else                  label = "Strong";

    return {label, score, usesPersonal, usesPattern};
}

bool isLeap(int year) {
    return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

// Basic validation for a Gregorian calendar date
bool isValidDate(int y, int m, int d) {
    if (y < 1900 || y > 2100) return false;
    if (m < 1 || m > 12) return false;

    int daysInMonth[] = {0,
        31, 28, 31, 30, 31, 30,
        31, 31, 30, 31, 30, 31
    };
    if (m == 2 && isLeap(y)) daysInMonth[2] = 29;

    if (d < 1 || d > daysInMonth[m]) return false;
    return true;
}

// Parse YYYY-MM-DD
bool parseYMD(const string &dob, int &year, int &month, int &day) {
    if (dob.size() != 10) return false;
    if (dob[4] != '-' || dob[7] != '-') return false;

    string yStr = dob.substr(0, 4);
    string mStr = dob.substr(5, 2);
    string dStr = dob.substr(8, 2);

    for (char c : yStr + mStr + dStr) {
        if (!isdigit(static_cast<unsigned char>(c))) return false;
    }

    year  = stoi(yStr);
    month = stoi(mStr);
    day   = stoi(dStr);

    return isValidDate(year, month, day);
}

// Returns true if DOB is on or before 2025-12-31
bool isDobAllowed(const string &dob) {
    int y, m, d;
    if (!parseYMD(dob, y, m, d)) {
        cout << "Invalid DOB format. Use YYYY-MM-DD (for example, 2004-09-15).\n";
        return false;
    }

    const int limitYear  = 2025;
    const int limitMonth = 12;
    const int limitDay   = 31;

    // If year is greater than 2025 → not possible
    if (y > limitYear) return false;

    // If same year, check month
    if (y == limitYear && m > limitMonth) return false;

    // If same year and month, check day
    if (y == limitYear && m == limitMonth && d > limitDay) return false;

    return true;
}

int main() {
    string firstName, lastName, dob;
    string password;

    cout << "Enter your first name: ";
    getline(cin, firstName);

    cout << "Enter your last name (or press Enter to skip): ";
    getline(cin, lastName);

    cout << "Enter your date of birth (e.g. 2004-05-21 or 2004): ";
    getline(cin, dob);

    if (!isDobAllowed(dob)) {
        cout << "It would not have been possible to be born after December 2025.\n";
        return 0; // stop program here
    }

    cout << "Enter a password: ";
    getline(cin, password);

    Result res = evaluateStrength(password, firstName, lastName, dob);

    cout << "\nPassword strength label: " << res.label << endl;
    cout << "Security score (0-100): " << res.score << endl;

    if (res.usesPersonalInfo) {
        cout << "Warning: Your password contains personal information "
             << "(name or date of birth), which makes it easier to guess.\n";
    }
    if (res.usesSimplePattern) {
        cout << "Warning: Your password contains simple sequences like "
             << "\"1234\" or repeated characters, which are easy to crack.\n";
    }

    // Basic improvement suggestions
    if (password.length() < 12)
        cout << "Suggestion: Use at least 12 characters.\n";
    if (!any_of(password.begin(), password.end(),
                [](char c){ return islower(static_cast<unsigned char>(c)); }))
        cout << "Suggestion: Add lowercase letters.\n";
    if (!any_of(password.begin(), password.end(),
                [](char c){ return isupper(static_cast<unsigned char>(c)); }))
        cout << "Suggestion: Add uppercase letters.\n";
    if (!any_of(password.begin(), password.end(),
                [](char c){ return isdigit(static_cast<unsigned char>(c)); }))
        cout << "Suggestion: Add digits.\n";
    if (!any_of(password.begin(), password.end(),
                [](char c){ return !isalnum(static_cast<unsigned char>(c)); }))
        cout << "Suggestion: Add special characters (e.g. !@#$%^&*).\n";

    return 0;
}
