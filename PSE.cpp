#include <iostream>
#include <string>
#include <cctype>
using namespace std;

string evaluateStrength(const string &password) {
    bool hasLower = false;
    bool hasUpper = false;
    bool hasDigit = false;
    bool hasSpecial = false;

    for (char ch : password) {
        if (islower(static_cast<unsigned char>(ch))) hasLower = true;
        else if (isupper(static_cast<unsigned char>(ch))) hasUpper = true;
        else if (isdigit(static_cast<unsigned char>(ch))) hasDigit = true;
        else hasSpecial = true;
    }

    int length = password.length();

    // Example rules:
    // Strong: length >= 8 and has all 4 types
    if (length >= 8 && hasLower && hasUpper && hasDigit && hasSpecial)
        return "Strong";

    // Moderate: length >= 6 and has at least 3 of the 4 types
    int typeCount = hasLower + hasUpper + hasDigit + hasSpecial;
    if (length >= 6 && typeCount >= 3)
        return "Medium";

    // Otherwise weak
    return "Weak";

}

int main() {
    string password;

    cout << "Enter a password: ";
    getline(cin, password);

    string strength = evaluateStrength(password);
    cout << "Password strength: " << strength << endl;

    // Optional: show which criteria are missing
    bool hasLower = false, hasUpper = false, hasDigit = false, hasSpecial = false;
    for (char ch : password) {
        if (islower(static_cast<unsigned char>(ch))) hasLower = true;
        else if (isupper(static_cast<unsigned char>(ch))) hasUpper = true;
        else if (isdigit(static_cast<unsigned char>(ch))) hasDigit = true;
        else hasSpecial = true;
    }

    if (password.length() < 8)
        cout << "Suggestion: Use at least 8 characters.\n";
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
