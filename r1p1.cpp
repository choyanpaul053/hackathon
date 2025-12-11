//phishing email classifier
// #include <iostream>
// #include <string>
// #include <vector>
// #include <algorithm>
// #include <cctype>
// #include <unordered_set>
// #include <sstream>
// #include <map>
// using namespace std;
// class PhishingEmailClassifier {
// public:
//     PhishingEmailClassifier() {
//         initializeKeywords();
//     }

//     string classifyEmail(const string& emailContent) {
//         int phishingScore = 0;
//         vector<string> words = tokenize(emailContent);
        
//         for (const string& word : words) {
//             string lowerWord = toLowerCase(word);
//             if (phishingKeywords.find(lowerWord) != phishingKeywords.end()) {
//                 phishingScore++;
//             }
//         }

//         return (phishingScore >= phishingThreshold) ? "Phishing" : "Legitimate";
//     }
// private:
//     unordered_set<string> phishingKeywords;
//     const int phishingThreshold = 3;



#include <bits/stdc++.h>
using namespace std;

/*
 Simple phishing email classifier using Multinomial Naive Bayes.
 Dataset format (training_data.txt):
 label<TAB>email text ...
 where label is "phishing" or "legit"

 Example lines:
 phishing  Verify your account now by clicking this link
 legit     Meeting schedule for tomorrow

 This is a teaching/demo implementation, not production-grade.
*/

struct NaiveBayesEmailClassifier {
    // Vocabulary: word -> index
    unordered_map<string, int> vocab;
    // Class labels
    enum ClassLabel { PHISHING = 0, LEGIT = 1 };
    // Prior probabilities P(class)
    double prior[2] = {0.0, 0.0};
    // Likelihoods: P(word | class) stored as log-probabilities
    vector<vector<double>> log_likelihood; // [class][wordIndex]
    // Total word counts per class (for smoothing)
    int total_words[2] = {0, 0};
    // Class document counts
    int doc_count[2] = {0, 0};
    bool trained = false;

    // Basic lowercase + alphanumeric tokenizer
    vector<string> tokenize(const string &text) const {
        string clean;
        clean.reserve(text.size());
        for (char c : text) {
            if (isalpha(static_cast<unsigned char>(c))) {
                clean.push_back(tolower(static_cast<unsigned char>(c)));
            } else if (isdigit(static_cast<unsigned char>(c))) {
                clean.push_back(c);
            } else {
                clean.push_back(' ');
            }
        }
        vector<string> tokens;
        stringstream ss(clean);
        string tok;
        while (ss >> tok) {
            tokens.push_back(tok);
        }
        return tokens;
    }

    ClassLabel labelFromString(const string &s) {
        if (s == "phishing" || s == "spam" || s == "malicious") {
            return PHISHING;
        }
        return LEGIT;
    }

    string labelToString(ClassLabel c) const {
        return (c == PHISHING) ? "PHISHING" : "LEGIT";
    }

    void train(const string &trainFile) {
        // First pass: build vocabulary and count words per class
        ifstream in(trainFile);
        if (!in.is_open()) {
            cerr << "Cannot open training file: " << trainFile << endl;
            return;
        }

        // Temporary counts: wordIndex -> count per class
        vector<unordered_map<int, int>> word_counts(2);
        string line;

        while (getline(in, line)) {
            if (line.empty()) continue;
            string labelStr;
            string text;

            // Split label and text at first whitespace
            stringstream ss(line);
            if (!(ss >> labelStr)) continue;
            getline(ss, text); // rest of line
            if (!text.empty() && text[0] == ' ') text.erase(text.begin());

            ClassLabel cls = labelFromString(labelStr);
            doc_count[cls]++;

            vector<string> tokens = tokenize(text);
            for (const string &w : tokens) {
                int idx;
                auto it = vocab.find(w);
                if (it == vocab.end()) {
                    idx = (int)vocab.size();
                    vocab[w] = idx;
                } else {
                    idx = it->second;
                }
                word_counts[cls][idx]++;
                total_words[cls]++;
            }
        }
        in.close();

        int V = (int)vocab.size();
        if (V == 0 || (doc_count[0] + doc_count[1]) == 0) {
            cerr << "Empty dataset or vocabulary.\n";
            return;
        }

        // Compute priors
        int total_docs = doc_count[0] + doc_count[1];
        prior[PHISHING] = log((double)doc_count[PHISHING] / total_docs);
        prior[LEGIT]    = log((double)doc_count[LEGIT] / total_docs);

        // Allocate likelihoods
        log_likelihood.assign(2, vector<double>(V, 0.0));

        // Laplace smoothing
        const double alpha = 1.0;

        for (int c = 0; c < 2; ++c) {
            for (int i = 0; i < V; ++i) {
                int count_wc = 0;
                auto it = word_counts[c].find(i);
                if (it != word_counts[c].end()) {
                    count_wc = it->second;
                }
                double num = count_wc + alpha;
                double denom = total_words[c] + alpha * V;
                log_likelihood[c][i] = log(num / denom);
            }
        }

        trained = true;
        cout << "Training completed. Documents: " << total_docs
             << ", Vocab size: " << V << endl;
    }

    ClassLabel predictLabel(const string &text) const {
        if (!trained) {
            cerr << "Model not trained.\n";
            return LEGIT;
        }
        vector<string> tokens = tokenize(text);
        double log_prob[2] = {prior[PHISHING], prior[LEGIT]};

        for (const string &w : tokens) {
            auto it = vocab.find(w);
            if (it == vocab.end()) continue; // unseen word
            int idx = it->second;
            log_prob[PHISHING] += log_likelihood[PHISHING][idx];
            log_prob[LEGIT]    += log_likelihood[LEGIT][idx];
        }

        return (log_prob[PHISHING] > log_prob[LEGIT]) ? PHISHING : LEGIT;
    }

    double phishingProbability(const string &text) const {
        if (!trained) return 0.0;
        vector<string> tokens = tokenize(text);
        double log_prob[2] = {prior[PHISHING], prior[LEGIT]};

        for (const string &w : tokens) {
            auto it = vocab.find(w);
            if (it == vocab.end()) continue;
            int idx = it->second;
            log_prob[PHISHING] += log_likelihood[PHISHING][idx];
            log_prob[LEGIT]    += log_likelihood[LEGIT][idx];
        }

        // Convert from log-space to probability
        double max_log = max(log_prob[PHISHING], log_prob[LEGIT]);
        double p0 = exp(log_prob[PHISHING] - max_log);
        double p1 = exp(log_prob[LEGIT] - max_log);
        double sum = p0 + p1;
        return p0 / sum;
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    NaiveBayesEmailClassifier clf;

    cout << "=== Phishing Email Classifier (Naive Bayes, C++) ===\n";
    cout << "Enter path to training file (e.g., training_data.txt): ";
    string trainFile;
    getline(cin, trainFile);

    clf.train(trainFile);

    cout << "\nType email text to classify (empty line to exit):\n";
    while (true) {
        cout << "\nEmail> ";
        string email;
        getline(cin, email);
        if (email.empty()) break;
        auto label = clf.predictLabel(email);
        double p_phish = clf.phishingProbability(email);
        cout << "Predicted: " << clf.labelToString(label)
             << " (P(phishing) = " << fixed << setprecision(4)
             << p_phish << ")\n";
    }

    return 0;
}
