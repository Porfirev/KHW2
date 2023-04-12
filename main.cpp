#include <iostream>
#include <vector>
#include <map>
#include <chrono>
#include <random>
#include <fstream>

const int MOD = 1791791791;
const int p = 31;
int64_t pows[15000];

std::vector<int64_t> get_all_hash(std::string& str) {
    int n = str.size();
    std::vector<int64_t> ans(n + 1);
    for (int i = 1; i < n + 1; ++i) {
        ans[i] = (ans[i - 1] * p + (str[i - 1] - 'a')) % MOD;
    }
    return ans;
}

// Полуинтервал
int64_t get_hash(std::vector<int64_t>& hash, int i, int j) {
    return ((hash[j] - (hash[i] * pows[j - i]) % MOD) % MOD + MOD) % MOD;
}

std::vector<int> prefix(std::string& string) {
    int n = string.size();
    std::vector<int> pr(n);
    for (int i = 1; i < n; ++i) {
        int j = pr[i - 1];
        while (j > 0 && string[i] != string[j] && string[i] != '?' && string[j] != '?') {
            j = pr[j - 1];
        }
        if (string[i] == string[j] || string[i] == '?' || string[j] == '?') {
            j++;
        }
        pr[i] = j;
    }
    return pr;
}

std::vector<int> refined_prefix(std::string& string) {
    auto pr = prefix(string);
    int n = pr.size();
    std::vector<int> prs(n);
    for (int i = 1; i < n - 1; ++i) {
        if (string[pr[i]] != string[i + 1] && string[i + 1] != '?' && string[pr[i] + 1] != '?') {
            prs[i] = pr[i];
        } else {
            prs[i] = prs[pr[i]];
        }
    }
    return prs;
}

int stupid_algo(std::string& str, std::string& pattern) {
    int pos = -1;
    for (size_t i = 0; i <= str.size() - pattern.size(); i++) {
        size_t j = 0;
        while (j < pattern.size() && (str[i + j] == pattern[j] || pattern[j] == '?')) {
            j++;
        }
        if (j == pattern.size()) {
            pos = i;
        }
    }
    return pos;
}

int default_kmp(std::string& str, std::string& pattern) {
    int pos = -1;
    std::string s = pattern + "#" + str;
    auto pr = prefix(s);
    for (size_t i = 0; i < pr.size(); ++i) {
        if (pr[i] == pattern.size()) {
            pos = i - 2 * pattern.size();
        }
    }
    return pos;
}

int refined_kmp(std::string& str, std::string& pattern) {
    int pos = -1;
    auto pr = refined_prefix(pattern);
    for (size_t i = 0; i <= str.size() - pattern.size();) {
        int j = 0;
        while (j < pattern.size() && (str[i + j] == pattern[j] || pattern[j] == '?')) {
            j++;
        }
        if (j == pattern.size()) {
            pos = i;
        }
        if (j == 0) {
            i++;
        } else {
            i += j - pr[j - 1];
        }
    }
    return pos;
}

int hash_algo(std::string& str, std::string& pattern) {
    int pos = -1;
    auto hashes = get_all_hash(str);
    auto trash = get_all_hash(pattern);
    int64_t pattern_hash = get_hash(trash, 0, pattern.size());
    for (size_t i = 0; i <= str.size() - pattern.size(); ++i) {
        if (get_hash(hashes, i, i + pattern.size()) == pattern_hash) {
            pos = i;
        }
    }
    return pos;
}

template <typename Func>
std::pair<int, int64_t> get_res(Func func, std::string str, std::string pattern) {
    int pos;
    int64_t res = 0;
    for (int i = 0; i < 10; ++i) {
        auto start = std::chrono::high_resolution_clock::now();
        pos = func(str, pattern);
        auto elapsed = std::chrono::high_resolution_clock::now() - start;
        res += std::chrono::duration_cast<std::chrono::nanoseconds>(elapsed).count();
    }
    return {pos, res / 10};
}

int main() {
    srand(time(nullptr));
    pows[0] = 1;
    for (int i = 1; i < 14000; ++i) {
        pows[i] = (pows[i - 1] * p) % MOD;
    }
    std::string small2, big2, small4, big4;
    for (int i = 0; i < 10000; ++i) {
        small2.push_back(static_cast<char>('a' + std::abs(std::rand()) % 2));
        small4.push_back(static_cast<char>('a' + std::abs(std::rand()) % 4));
    }
    for (int i = 0; i < 100000; ++i) {
        big2.push_back(static_cast<char>('a' + std::abs(std::rand()) % 2));
        big4.push_back(static_cast<char>('a' + std::abs(std::rand()) % 4));
    }

    std::vector<std::string> all_str = {small2, small4, big2, big4};
    std::map<int, std::map<int, std::vector<std::string>>> mp;
    for (int i = 0; i < 5; ++i) {
        for (int j = 0; j < 4; ++j) {
            int ind = std::abs(std::rand()) % (all_str[j].size() - 3000);
            for (int q = 100; q <= 3000; q += 100) {
                std::string pattern(q, 'a');
                for (int k = 0; k < q; ++k) {
                    pattern[k] = all_str[j][k + ind];
                }
                for (int k = 0; k < i; ++k) {
                    int pos = std::abs(std::rand()) % (pattern.size());
                    pattern[pos] = '?';
                }
                mp[i][j].push_back(pattern);
            }
        }
    }

    std::map<int, std::string> names = {
            {0, "small2"},
            {1, "small4"},
            {2, "big2"},
            {3, "big4"}
    };
    for (int i = 0; i < 5; ++i) {
        for (int j = 0; j < 4; ++j) {
            std::string file_name = "../csv/" + names[j] + "_" + "count_q_is_" + std::to_string(i) + ".csv";
            std::ofstream fout(file_name);
            fout << "algo_name;pattern_len;time" << '\n';
            for (std::string pattern: mp[i][j]) {
                std::string str = all_str[j];

                auto stupid_info = get_res(stupid_algo, str, pattern);
                fout << "stupid;" << pattern.size() << ';' << stupid_info.second << '\n';

                auto kmp_info = get_res(default_kmp, str, pattern);
                fout << "kmp;" << pattern.size() << ';' << kmp_info.second << '\n';

                auto rkmp_info = get_res(refined_kmp, str, pattern);
                fout << "refined_kmp;" << pattern.size() << ';' << rkmp_info.second << '\n';

                if (i == 0) {
                    auto hash_info = get_res(hash_algo, str, pattern);
                    fout << "hash;" << pattern.size() << ';' << hash_info.second << '\n';
                }
            }
            fout.close();
        }
    }
}
