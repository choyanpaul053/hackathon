#include <iostream>
#include <string>
#include <cctype>
using namespace std;

struct Result {
    string label;
    int score;      // 0 - 100
};

Result evaluateStrength(const string &password) {
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
        if (length >= 12) score += 20;      // long passwords get more points[web:21]
    } else if (length >= 6) {
        score += 10;
    }

    // Character type diversity (max 40)
    if (hasLower)  score += 10;
    if (hasUpper)  score += 10;
    if (hasDigit)  score += 10;
    if (hasSpecial) score += 10;           // more character sets → higher complexity[web:21][web:24]

    // Simple bonus for very diverse and long passwords (max 20)
    int typeCount = hasLower + hasUpper + hasDigit + hasSpecial;
    if (typeCount >= 3 && length >= 10) score += 10;
    if (typeCount == 4 && length >= 14) score += 10;

    if (score > 100) score = 100;

    // Map score to label
    string label;
    if (score < 30)       label = "Very Weak";
    else if (score < 50)  label = "Weak";
    else if (score < 70)  label = "Fair";
    else if (score < 85)  label = "Good";
    else                  label = "Strong";

    return {label, score};
}

int main() {
    string password;

    cout << "Enter a password: ";
    getline(cin, password);

    Result res = evaluateStrength(password);
    cout << "Password strength label: " << res.label << endl;
    cout << "Security score (0-100): " << res.score << endl;

    // Optional feedback
    bool hasLower = false, hasUpper = false, hasDigit = false, hasSpecial = false;
    for (char ch : password) {
        unsigned char c = static_cast<unsigned char>(ch);
        if (islower(c)) hasLower = true;
        else if (isupper(c)) hasUpper = true;
        else if (isdigit(c)) hasDigit = true;
        else hasSpecial = true;
    }

    if (password.length() < 12)
        cout << "Suggestion: Use at least 12 characters.\n";
    if (!hasLower)
        cout << "Suggestion: Add lowercase letters.\n";
    if (!hasUpper)
        cout << "Suggestion: Add uppercase letters.\n";
    if (!hasDigit)
        cout << "Suggestion: Add digits.\n";
    if (!hasSpecial)
        cout << "Suggestion: Add special characters (e.g. !@#$%^&*).\n";

    return 0;
}
