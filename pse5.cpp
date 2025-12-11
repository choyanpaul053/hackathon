// pse4.cpp
#include <iostream>
#include <string>
#include <cstdlib>
#include <cctype>
#include <map>
using namespace std;

// ---------- CGI helpers ----------
string urlDecode(const string &s) {
    string out;
    for (size_t i = 0; i < s.size(); ++i) {
        if (s[i] == '+') out.push_back(' ');
        else if (s[i] == '%' && i + 2 < s.size()) {
            int v = strtol(s.substr(i + 1, 2).c_str(), nullptr, 16);
            out.push_back(static_cast<char>(v));
            i += 2;
        } else out.push_back(s[i]);
    }
    return out;
}

map<string,string> parsePostData() {
    map<string,string> params;
    char *lenStr = getenv("CONTENT_LENGTH");
    if (!lenStr) return params;
    int len = atoi(lenStr);
    if (len <= 0) return params;

    string data(len, '\0');
    cin.read(&data[0], len);

    size_t start = 0;
    while (start < data.size()) {
        size_t amp = data.find('&', start);
        if (amp == string::npos) amp = data.size();
        string pair = data.substr(start, amp - start);
        size_t eq = pair.find('=');
        if (eq != string::npos) {
            string key = urlDecode(pair.substr(0, eq));
            string val = urlDecode(pair.substr(eq + 1));
            params[key] = val;
        }
        start = amp + 1;
    }
    return params;
}

// ---------- Date validation (YYYY-MM-DD, ≤ 2025-12-31) ----------
bool isLeap(int year) {
    return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

bool isValidDate(int y, int m, int d) {
    if (y < 1900 || y > 2100) return false;
    if (m < 1 || m > 12) return false;
    int daysInMonth[] = {0,31,28,31,30,31,30,31,31,30,31,30,31};
    if (m == 2 && isLeap(y)) daysInMonth[2] = 29;
    if (d < 1 || d > daysInMonth[m]) return false;
    return true;
}

bool parseYMD(const string &dob, int &year, int &month, int &day) {
    if (dob.size() != 10) return false;
    if (dob[4] != '-' || dob[7] != '-') return false;
    string yStr = dob.substr(0,4);
    string mStr = dob.substr(5,2);
    string dStr = dob.substr(8,2);
    for (char c : yStr + mStr + dStr) {
        if (!isdigit(static_cast<unsigned char>(c))) return false;
    }
    year  = stoi(yStr);
    month = stoi(mStr);
    day   = stoi(dStr);
    return isValidDate(year, month, day);
}

bool isDobAllowed(const string &dob, string &errorMsg) {
    int y, m, d;
    if (!parseYMD(dob, y, m, d)) {
        errorMsg = "Invalid DOB format. Use YYYY-MM-DD.";
        return false;
    }
    const int ly = 2025, lm = 12, ld = 31;
    if (y > ly || (y == ly && (m > lm || (m == lm && d > ld)))) {
        errorMsg = "It would not have been possible to be born after December 2025.";
        return false;
    }
    return true;
}

// ---------- Password strength logic ----------
struct Result {
    string label;
    int score;
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
    return inc || dec || same;
}

Result evaluateStrength(const string &password,
                        const string &firstName,
                        const string &lastName,
                        const string &dob) {
    bool hasLower = false, hasUpper = false, hasDigit = false, hasSpecial = false;
    for (char ch : password) {
        unsigned char c = static_cast<unsigned char>(ch);
        if (islower(c)) hasLower = true;
        else if (isupper(c)) hasUpper = true;
        else if (isdigit(c)) hasDigit = true;
        else hasSpecial = true;
    }

    int length = password.length();
    int score = 0;

    // length
    if (length >= 8) {
        score += 20;
        if (length >= 12) score += 20;
    } else if (length >= 6) {
        score += 10;
    }

    // character types
    if (hasLower)   score += 10;
    if (hasUpper)   score += 10;
    if (hasDigit)   score += 10;
    if (hasSpecial) score += 10;

    int typeCount = hasLower + hasUpper + hasDigit + hasSpecial;

    if (typeCount >= 3 && length >= 10) score += 10;
    if (typeCount == 4 && length >= 14) score += 10;

    bool usesPersonal = false;
    bool usesPattern = false;

    // personal info: first/last name
    if (!firstName.empty() && containsIgnoreCase(password, firstName)) {
        score -= 20; usesPersonal = true;
    }
    if (!lastName.empty() && containsIgnoreCase(password, lastName)) {
        score -= 20; usesPersonal = true;
    }
    if (firstName.size() >= 3 && containsIgnoreCase(password, firstName.substr(0,3))) {
        score -= 10; usesPersonal = true;
    }
    if (lastName.size() >= 3 && containsIgnoreCase(password, lastName.substr(0,3))) {
        score -= 10; usesPersonal = true;
    }

    // dob string and year
    if (!dob.empty() && containsIgnoreCase(password, dob)) {
        score -= 20; usesPersonal = true;
    }
    string yearDigits;
    for (char c : dob) {
        if (isdigit(static_cast<unsigned char>(c))) yearDigits.push_back(c);
        if (yearDigits.size() == 4) break;
    }
    if (yearDigits.size() == 4 && password.find(yearDigits) != string::npos) {
        score -= 20; usesPersonal = true;
    }

    // simple sequences
    string digitsOnly, lettersOnly;
    for (char c : password) {
        if (isdigit(static_cast<unsigned char>(c))) digitsOnly.push_back(c);
        else if (isalpha(static_cast<unsigned char>(c))) lettersOnly.push_back(static_cast<char>(tolower(c)));
    }
    if (isSimpleSequence(digitsOnly) || isSimpleSequence(lettersOnly)) {
        score -= 15; usesPattern = true;
    }

    if (score < 0) score = 0;
    if (score > 100) score = 100;

    string label;
    if (score < 30)       label = "Very Weak";
    else if (score < 50)  label = "Weak";
    else if (score < 70)  label = "Fair";
    else if (score < 85)  label = "Good";
    else                  label = "Strong";

    return {label, score, usesPersonal, usesPattern};
}

int main() {
    // CGI header
    cout << "Content-type:text/html\r\n\r\n";

    auto params   = parsePostData();
    string firstName = params["firstName"];
    string lastName  = params["lastName"];
    string dob       = params["dob"];
    string password  = params["password"];

    cout << "<html><head><title>Password Result</title></head><body>\n";

    if (password.empty() || dob.empty() || firstName.empty()) {
        cout << "<p>Please fill all required fields (first name, DOB, password).</p>";
        cout << "</body></html>";
        return 0;
    }

    string dobError;
    if (!isDobAllowed(dob, dobError)) {
        cout << "<p>" << dobError << "</p>";
        cout << "</body></html>";
        return 0;
    }

    Result r = evaluateStrength(password, firstName, lastName, dob);

    cout << "<h2>Password Evaluation Result</h2>\n";
    cout << "<p><strong>Strength label:</strong> " << r.label << "<br>";
    cout << "<strong>Score:</strong> " << r.score << "/100</p>\n";

    if (r.usesPersonalInfo) {
        cout << "<p style='color:#b00020;'>Warning: Your password contains your name or date of birth, ";
        cout << "which makes it easier to guess.</p>\n";
    }
    if (r.usesSimplePattern) {
        cout << "<p style='color:#b00020;'>Warning: Your password contains simple sequences or repeated ";
        cout << "characters (like 1234 or abcd).</p>\n";
    }

    if (password.length() < 12)
        cout << "<p>Suggestion: Use at least 12 characters.</p>\n";

    bool hasLower = false, hasUpper = false, hasDigit = false, hasSpecial = false;
    for (char ch : password) {
        unsigned char c = static_cast<unsigned char>(ch);
        if (islower(c)) hasLower = true;
        else if (isupper(c)) hasUpper = true;
        else if (isdigit(c)) hasDigit = true;
        else hasSpecial = true;
    }
    if (!hasLower)   cout << "<p>Suggestion: Add lowercase letters.</p>\n";
    if (!hasUpper)   cout << "<p>Suggestion: Add uppercase letters.</p>\n";
    if (!hasDigit)   cout << "<p>Suggestion: Add digits.</p>\n";
    if (!hasSpecial) cout << "<p>Suggestion: Add special characters (e.g. !@#$%^&*).</p>\n";

    cout << "</body></html>";
    return 0;
}
