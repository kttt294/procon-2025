#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <random>
#include <chrono>
#include <algorithm>
#include <set>
#include <cstring>
#include <unordered_map>
#include <cmath>
#include <functional>
#include "json.hpp"
using json = nlohmann::json;
using namespace std;
ifstream input_file("input.json");
const int MAXN = 24;
const int NUM_RUNS = 1;
const int nhoo = 50;   
const int resetN = 1500;  
const int too = 200;   
const int resetT = 800;

int get_limit_time(int n) {
    return 10;
}

vector<int> get_ds_xoay(int n) {
    if (n <= 4) return {2, 3};
    else if (n == 6) return {2, 3, 4, 5};
    else if (n == 8) return {2, 3, 4, 5, 6};
    else if (n == 10) return {2, 3, 4, 5, 6, 7};
    else if (n == 12) return {2, 3, 4, 5, 6, 7, 8, 9};
    else if (n == 14) return {2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    else if (n == 16) return {2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13};
    else if (n == 18) return {2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17};
    else if (n == 20) return {2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18};
    else if (n == 22) return {2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21};
    return {2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23};
}

struct Move {
    int x, y, n;
};

struct Result {
    int score;
    vector<Move> ops;
    double runtime;
};

int count_pairs(int field[MAXN][MAXN], int n) {
    int pairs = 0;
    // ngang
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n - 1; j++) {
            if (field[i][j] == field[i][j + 1]) pairs++;
        }
    }
    // dọc
    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n; j++) {
            if (field[i][j] == field[i + 1][j]) pairs++;
        }
    }
    return pairs;
}

int get_local_score(int field[MAXN][MAXN], int n, int x, int y, int k) {
    int score = 0;
    for (int i = y; i < y + k; i++) {
        for (int j = x; j < x + k - 1; j++) {
            if (field[i][j] == field[i][j + 1]) score++;
        }
        if (x > 0 && field[i][x - 1] == field[i][x]) score++;
        if (x + k < n && field[i][x + k - 1] == field[i][x + k]) score++;
    }
    for (int j = x; j < x + k; j++) {
        for (int i = y; i < y + k - 1; i++) {
            if (field[i][j] == field[i + 1][j]) score++;
        }
        if (y > 0 && field[y - 1][j] == field[y][j]) score++;
        if (y + k < n && field[y + k - 1][j] == field[y + k][j]) score++;
    }
    return score;
}


int calculate_delta_score(int field[MAXN][MAXN], int n, int x, int y, int k) {
    int old_score = get_local_score(field, n, x, y, k);
    int sub[MAXN][MAXN];
    for (int i = 0; i < k; i++) {
        for (int j = 0; j < k; j++) {
            sub[i][j] = field[y + i][x + j];
        }
    }
    // Xoay
    int rotated[MAXN][MAXN];
    for (int i = 0; i < k; i++) {
        for (int j = 0; j < k; j++) {
            rotated[j][k - 1 - i] = sub[i][j];
        }
    }
    int new_score = 0;
    for (int i = 0; i < k; i++) {
        for (int j = 0; j < k - 1; j++) {
            if (rotated[i][j] == rotated[i][j + 1]) new_score++;
        }
        if (x > 0 && field[y + i][x - 1] == rotated[i][0]) new_score++;
        if (x + k < n && rotated[i][k - 1] == field[y + i][x + k]) new_score++;
    }

    for (int j = 0; j < k; j++) {
        for (int i = 0; i < k - 1; i++) {
            if (rotated[i][j] == rotated[i + 1][j]) new_score++;
        }
        if (y > 0 && field[y - 1][x + j] == rotated[0][j]) new_score++;
        if (y + k < n && rotated[k - 1][j] == field[y + k][x + j]) new_score++;
    }

    return new_score - old_score;
}

void rotate_submatrix(int field[MAXN][MAXN], int x, int y, int k) {
    int temp[MAXN][MAXN];

    for (int i = 0; i < k; i++) {
        for (int j = 0; j < k; j++) {
            temp[i][j] = field[y + i][x + j];
        }
    }

    for (int i = 0; i < k; i++) {
        for (int j = 0; j < k; j++) {
            field[y + j][x + k - 1 - i] = temp[i][j];
        }
    }
}

void copy_field(int dest[MAXN][MAXN], const int src[MAXN][MAXN], int n) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            dest[i][j] = src[i][j];
        }
    }
}

vector<Move> compress(const vector<Move>& ops) {
    if (ops.empty()) return ops;
    vector<Move> res;
    int i = 0;
    int L = (int)ops.size();
    while (i < L) {
        int x = ops[i].x, y = ops[i].y, n = ops[i].n;
        int cnt = 1;
        int j = i + 1;
        while (j < L &&
               ops[j].x == x &&
               ops[j].y == y &&
               ops[j].n == n) {
            cnt++;
            j++;
        }
        int remain = cnt % 4;
        for (int k = 0; k < remain; k++) {
            res.push_back({x, y, n});
        }
        i = j;
    }
    return res;
}

bool is_row_full(int field[MAXN][MAXN], int n, int row_idx, const set<int>& locked_rows) {
    if (locked_rows.count(row_idx)) return true;
    for (int j = 0; j < n - 1; j++) {
        if (field[row_idx][j] != field[row_idx][j + 1]) {
            return false;
        }
    }
    return true;
}

bool is_col_full(int field[MAXN][MAXN], int n, int col_idx, const set<int>& locked_cols) {
    if (locked_cols.count(col_idx)) return true;
    for (int i = 0; i < n - 1; i++) {
        if (field[i][col_idx] != field[i + 1][col_idx]) {
            return false;
        }
    }
    return true;
}

bool get_active_region(int field[MAXN][MAXN], int n, const set<int>& locked_rows, const set<int>& locked_cols, int& top, int& bottom, int& left, int& right) {
    top = 0;
    while (top < n && locked_rows.count(top)) top++;
    bottom = n - 1;
    while (bottom >= 0 && locked_rows.count(bottom)) bottom--;
    left = 0;
    while (left < n && locked_cols.count(left)) left++;
    right = n - 1;
    while (right >= 0 && locked_cols.count(right)) right--;
    return (top <= bottom && left <= right);
}

vector<int> adjust_sizes(const vector<int>& sizes, int top, int bottom, int left, int right) {
    int h = bottom - top + 1;
    int w = right - left + 1;
    int max_k = min(h, w);
    vector<int> res;
    for (int k : sizes) {
        if (2 <= k && k <= max_k) res.push_back(k);
    }
    return res;
}
struct UnmatchedInfo {
    char mask[MAXN][MAXN];
    int top, bottom, left, right;
    bool any;
};

UnmatchedInfo build_unmatched_mask(int field[MAXN][MAXN], int n) {
    unordered_map<int, vector<pair<int,int>>> pos;
    pos.reserve(n * n * 2);

    for (int r = 0; r < n; r++) {
        for (int c = 0; c < n; c++) {
            int v = field[r][c];
            pos[v].push_back({r, c});
        }
    }

    UnmatchedInfo info;
    memset(info.mask, 0, sizeof(info.mask));
    info.any = false;
    int top = n, bottom = -1, left = n, right = -1;

    for (auto &kv : pos) {
        auto &vec = kv.second;
        if (vec.size() != 2) continue;
        auto [r1, c1] = vec[0];
        auto [r2, c2] = vec[1];
        int dist = abs(r1 - r2) + abs(c1 - c2);
        if (dist > 1) {
            info.mask[r1][c1] = 1;
            info.mask[r2][c2] = 1;
            info.any = true;
            top    = min(top,    min(r1, r2));
            bottom = max(bottom, max(r1, r2));
            left   = min(left,   min(c1, c2));
            right  = max(right,  max(c1, c2));
        }
    }

    if (info.any) {
        info.top = top;
        info.bottom = bottom;
        info.left = left;
        info.right = right;
    } else {
        info.top = 0;
        info.bottom = n - 1;
        info.left = 0;
        info.right = n - 1;
    }
    return info;
}

bool move_touches_unmatched(const UnmatchedInfo &info, int x, int y, int k) {
    if (!info.any) return true;
    for (int r = y; r < y + k; r++) {
        for (int c = x; c < x + k; c++) {
            if (info.mask[r][c]) return true;
        }
    }
    return false;
}

void save_output_json(const vector<Move>& ops) {
    json j;
    json ops_array = json::array();
    for (const auto &op : ops) {
        ops_array.push_back({{"x", op.x}, {"y", op.y}, {"n", op.n}});
    }
    j["ops"] = ops_array;
    ofstream out("output.json");
    out << j.dump(2);
    out.close();
}

void save_solution_full(const vector<Move>& ops, int n, int max_pairs, int score) {
    vector<Move> compressed = compress(ops);
    save_output_json(compressed);

    int missing = max_pairs - score;
    string filename = "s" + to_string(n) + "_" + to_string(missing)
                      + "_" + to_string(compressed.size()) + ".json";
    ofstream f(filename);
    json j;
    json ops_array = json::array();
    for (auto &op : compressed) {
        ops_array.push_back({{"x", op.x}, {"y", op.y}, {"n", op.n}});
    }
    j["ops"] = ops_array;
    f << j.dump(2);
    f.close();
}


bool dfs(
    int field[MAXN][MAXN],
    int n,
    int &score,
    int max_pairs,
    vector<Move> &ops,
    const chrono::high_resolution_clock::time_point &t0,
    int time_limit
) {
    int missing = max_pairs - score;
    if (missing > 60) return false; 

    auto now0 = chrono::high_resolution_clock::now();
    double elapsed0 = chrono::duration<double>(now0 - t0).count();
    if (elapsed0 > time_limit - 0.1) return false;

    UnmatchedInfo um = build_unmatched_mask(field, n);
    if (!um.any) return false;

    int top = um.top, bottom = um.bottom;
    int left = um.left, right = um.right;

    vector<int> ks = {2, 3, 4, 5};
    vector<pair<int, tuple<int,int,int>>> scored; 

    for (int k : ks) {
        if (k > bottom - top + 1 || k > right - left + 1) continue;
        for (int y = top; y <= bottom - k + 1; ++y) {
            for (int x = left; x <= right - k + 1; ++x) {
                if (!move_touches_unmatched(um, x, y, k)) continue;
                int delta = calculate_delta_score(field, n, x, y, k);
                int ns = score + delta;
                scored.push_back({ns, {x, y, k}});
            }
        }
    }

    if (scored.empty()) return false;

    sort(scored.begin(), scored.end(),
         [](auto &a, auto &b){ return a.first > b.first; });

    const int L = 20;
    vector<tuple<int,int,int>> topMoves;
    for (int i = 0; i < (int)scored.size() && i < L; ++i) {
        topMoves.push_back(scored[i].second);
    }

    int best_score = score;
    vector<Move> best_seq;
    vector<Move> curr_path;

    function<void(int,int)> dfs = [&](int depth, int current_score) {
        auto now = chrono::high_resolution_clock::now();
        double elapsed = chrono::duration<double>(now - t0).count();
        if (elapsed > time_limit - 0.05) return;

        if (current_score > best_score) {
            best_score = current_score;
            best_seq = curr_path;
        }
        if (best_score >= max_pairs) return;
        if (depth == 3) return; 
        int branch_limit = (depth == 0 ? 12 : 8);
        int used = 0;

        for (auto [x, y, k] : topMoves) {
            if (used >= branch_limit) break;

            int delta = calculate_delta_score(field, n, x, y, k);
            int ns = current_score + delta;

            if (depth == 0 && delta < -2) {
                used++;
                continue;
            }

            rotate_submatrix(field, x, y, k);
            curr_path.push_back({x, y, k});

            dfs(depth + 1, ns);
            curr_path.pop_back();
            rotate_submatrix(field, x, y, k);
            rotate_submatrix(field, x, y, k);
            rotate_submatrix(field, x, y, k);

            if (best_score >= max_pairs) return;
            used++;
        }
    };

    dfs(0, score);

    if (best_score <= score || best_seq.empty()) return false;

    for (auto mv : best_seq) {
        rotate_submatrix(field, mv.x, mv.y, mv.n);
        ops.push_back(mv);
    }
    score = best_score;
    return true;
}

Result local_search_small(const int start_field[MAXN][MAXN], int n, int time_limit) {
    auto t0 = chrono::high_resolution_clock::now();

    int max_pairs = n * n / 2;

    int current_field[MAXN][MAXN];
    copy_field(current_field, start_field, n);
    int current_score = count_pairs(current_field, n);

    // trang thaiii
    int initial_field[MAXN][MAXN];
    copy_field(initial_field, current_field, n);
    int initial_score = current_score;

    int best_field[MAXN][MAXN];
    copy_field(best_field, current_field, n);
    int best_score = current_score;
    vector<Move> best_ops;
    vector<Move> current_ops;

    set<int> locked_rows, locked_cols;
    vector<int> base_sizes = get_ds_xoay(n);

    int sample_moves = 350;
    double base_accept_bad = 0.001;

    int no_improve = 0;
    int iter_count = 0;
    int consecutive_empty = 0;

    random_device rd;
    mt19937 gen(rd());
    uniform_real_distribution<> urd(0.0, 1.0);

    while (true) {
        auto now = chrono::high_resolution_clock::now();
        double elapsed = chrono::duration<double>(now - t0).count();
        if (elapsed > time_limit) break;
        if (current_score == max_pairs) break;

        iter_count++;

        if (iter_count % 50 == 0) {
            for (int i = 0; i < n; i++) {
                if (is_row_full(current_field, n, i, locked_rows))
                    locked_rows.insert(i);
            }
            for (int j = 0; j < n; j++) {
                if (is_col_full(current_field, n, j, locked_cols))
                    locked_cols.insert(j);
            }
        }

        int top, bottom, left, right;
        if (!get_active_region(current_field, n, locked_rows, locked_cols,
                               top, bottom, left, right)) {
            break;
        }

        int active_h = bottom - top + 1;
        int active_w = right - left + 1;
        int active_size = min(active_h, active_w);

        if (active_size <= 6) {
            locked_rows.clear();
            locked_cols.clear();
            if (!get_active_region(current_field, n, locked_rows, locked_cols,
                                   top, bottom, left, right)) {
                break;
            }
        }

        vector<int> allowed_sizes = adjust_sizes(base_sizes, top, bottom, left, right);
        if (allowed_sizes.empty()) {
            consecutive_empty++;
            if (consecutive_empty <= 5) {
                locked_rows.clear();
                locked_cols.clear();
                base_accept_bad = min(0.1, base_accept_bad * 1.5);
                continue;
            } else {
                break;
            }
        }

        vector<tuple<int,int,int>> moves;
        for (int k : allowed_sizes) {
            for (int y = top; y <= bottom - k + 1; y++) {
                for (int x = left; x <= right - k + 1; x++) {
                    moves.emplace_back(x, y, k);
                }
            }
        }
        if (moves.empty()) {
            consecutive_empty++;
            if (consecutive_empty <= 5) {
                locked_rows.clear();
                locked_cols.clear();
                base_accept_bad = min(0.1, base_accept_bad * 1.5);
                continue;
            } else {
                break;
            }
        }

        consecutive_empty = 0;

        int max_steps_total = 10000;
        if ((int)current_ops.size() >= max_steps_total) {
            copy_field(current_field, best_field, n);
            current_score = best_score;
            current_ops = best_ops;
            locked_rows.clear();
            locked_cols.clear();
            no_improve = 0;
            continue;
        }
        int M = (int)moves.size();
        uniform_int_distribution<> dis(0, M - 1);

        vector<tuple<int,int,int,int>> candidates;
        int samples = min(sample_moves, M);

        for (int i = 0; i < samples; i++) {
            auto [x, y, k] = moves[dis(gen)];
            int delta = calculate_delta_score(current_field, n, x, y, k);
            int ns = current_score + delta;
            candidates.push_back({ns, x, y, k});
        }

        sort(candidates.begin(), candidates.end(),
             [](auto &a, auto &b) {
                 return get<0>(a) > get<0>(b);
             });

        auto [best_ns, best_x, best_y, best_k] = candidates[0];

        int top_k = min(3, (int)candidates.size());
        uniform_int_distribution<> dis_top(0, top_k - 1);
        auto [rnd_ns, rnd_x, rnd_y, rnd_k] = candidates[dis_top(gen)];

        int delta = best_ns - current_score;

        double accept_bad_prob = base_accept_bad;

        bool accept = false;
        int use_x, use_y, use_k;
        int new_score;

        if (delta >= 0) {
            accept = true;
            use_x = best_x; use_y = best_y; use_k = best_k;
            new_score = best_ns;
        } else {
            if ((int)current_ops.size() < 9000) {
                double r = urd(gen);
                if (r < accept_bad_prob) {
                    accept = true;
                    use_x = rnd_x; use_y = rnd_y; use_k = rnd_k;
                    new_score = rnd_ns;
                }
            }
        }

        if (accept) {
            rotate_submatrix(current_field, use_x, use_y, use_k);
            current_score = new_score;
            current_ops.push_back({use_x, use_y, use_k});
            no_improve++;

            if (current_ops.size() % 50 == 0 && !current_ops.empty()) {
                current_ops = compress(current_ops);
            }

            if ((int)current_ops.size() >= max_steps_total) {
                break;
            }

            if (new_score > best_score) {
                best_score = new_score;
                copy_field(best_field, current_field, n);
                best_ops = current_ops;
                no_improve = 0;
                if (best_score >= max_pairs) {
                    break;
                }
            } else if (new_score == best_score) {
                if (best_ops.empty() || current_ops.size() < best_ops.size()) {
                    copy_field(best_field, current_field, n);
                    best_ops = current_ops;
                }
            }
        }
        if (no_improve > resetN) {
            copy_field(current_field, initial_field, n);
            current_score = initial_score;
            current_ops.clear();
            locked_rows.clear();
            locked_cols.clear();
            no_improve = 0;
            base_accept_bad = min(0.2, base_accept_bad * 1.5);
            continue;
        }
        if (no_improve > nhoo) {
            copy_field(current_field, best_field, n);
            current_score = best_score;
            current_ops = best_ops;
            locked_rows.clear();
            locked_cols.clear();
            no_improve = 0;
            continue;
        }
    }

    vector<Move> final_ops = compress(best_ops);
    auto t1 = chrono::high_resolution_clock::now();
    double runtime = chrono::duration<double>(t1 - t0).count();

    Result res;
    res.score = best_score;
    res.ops = move(final_ops);
    res.runtime = runtime;
    return res;
}

Result local_search_solver_large(const int start_field[MAXN][MAXN], int n, int time_limit) {
    auto t0 = chrono::high_resolution_clock::now();

    int max_pairs = n * n / 2;

    int current_field[MAXN][MAXN];
    copy_field(current_field, start_field, n);
    int current_score = count_pairs(current_field, n);
   // luu trang thai  
    int initial_field[MAXN][MAXN];
    copy_field(initial_field, current_field, n);
    int initial_score = current_score;

    int best_field[MAXN][MAXN];
    copy_field(best_field, current_field, n);
    int best_score = current_score;
    vector<Move> best_ops;
    vector<Move> current_ops;

    set<int> locked_rows, locked_cols;
    vector<int> base_sizes = get_ds_xoay(n);

    int sample_moves = 320;

    double base_accept_bad = 0.001;
    const double initial_accept_bad = 0.001;

    int no_improve = 0;
    int iter_count = 0;
    int restart_count = 0;

    random_device rd;
    mt19937 gen(rd());
    uniform_real_distribution<> urd(0.0, 1.0);

    const double MAIN_PHASE_RATIO = 0.9;   

    while (true) {
        auto now = chrono::high_resolution_clock::now();
        double elapsed = chrono::duration<double>(now - t0).count();
        if (elapsed > time_limit * MAIN_PHASE_RATIO) break;
        if (current_score == max_pairs) break;

        iter_count++;

        if (iter_count % 50 == 0) {
            for (int i = 0; i < n; i++) {
                if (is_row_full(current_field, n, i, locked_rows))
                    locked_rows.insert(i);
            }
            for (int j = 0; j < n; j++) {
                if (is_col_full(current_field, n, j, locked_cols))
                    locked_cols.insert(j);
            }
        }

        int top, bottom, left, right;
        if (!get_active_region(current_field, n, locked_rows, locked_cols, top, bottom, left, right)) {
            break;
        }

        int active_h = bottom - top + 1;
        int active_w = right - left + 1;
        int active_size = min(active_h, active_w);

        UnmatchedInfo um;
        if (active_size > 6) {
            um = build_unmatched_mask(current_field, n);
            if (um.any) {
                top    = max(top,    um.top);
                bottom = min(bottom, um.bottom);
                left   = max(left,   um.left);
                right  = min(right,  um.right);
                if (top > bottom || left > right) {
                    top = um.top;
                    bottom = um.bottom;
                    left = um.left;
                    right = um.right;
                }
            }
        }

        if (active_size <= 6) {
            locked_rows.clear();
            locked_cols.clear();
            if (!get_active_region(current_field, n, locked_rows, locked_cols, top, bottom, left, right)) {
                break;
            }
            active_h = bottom - top + 1;
            active_w = right - left + 1;
            active_size = min(active_h, active_w);
        }

        vector<int> allowed_sizes = adjust_sizes(base_sizes, top, bottom, left, right);
        if (allowed_sizes.empty()) {
            locked_rows.clear();
            locked_cols.clear();
            continue;
        }

        vector<tuple<int,int,int>> moves;
        for (int k : allowed_sizes) {
            for (int y = top; y <= bottom - k + 1; y++) {
                for (int x = left; x <= right - k + 1; x++) {
                    moves.emplace_back(x, y, k);
                }
            }
        }
        if (moves.empty()) {
            locked_rows.clear();
            locked_cols.clear();
            continue;
        }

        int max_steps_total = 10000;
        if ((int)current_ops.size() >= max_steps_total) {
            restart_count++;
            base_accept_bad = min(0.15, initial_accept_bad * pow(2.0, restart_count));

            copy_field(current_field, best_field, n);
            current_score = best_score;
            current_ops = best_ops;
            locked_rows.clear();
            locked_cols.clear();
            no_improve = 0;
            continue;
        }
        int M = (int)moves.size();
        uniform_int_distribution<> dis(0, M - 1);

        vector<tuple<int,int,int,int>> candidates;
        int samples = min(sample_moves, M);
        int picked = 0;
        int tries_limit = M * 4;

        while (picked < samples && tries_limit > 0) {
            auto [x, y, k] = moves[dis(gen)];
            tries_limit--;

            if (active_size > 6) {
                UnmatchedInfo local_um = um;
                if (local_um.any && !move_touches_unmatched(local_um, x, y, k)) {
                    continue;
                }
            }

            int delta = calculate_delta_score(current_field, n, x, y, k);
            int ns = current_score + delta;
            candidates.push_back({ns, x, y, k});
            picked++;
        }

        if (candidates.empty()) {
            for (int i = 0; i < samples; i++) {
                auto [x, y, k] = moves[dis(gen)];
                int delta = calculate_delta_score(current_field, n, x, y, k);
                int ns = current_score + delta;
                candidates.push_back({ns, x, y, k});
            }
        }

        sort(candidates.begin(), candidates.end(),
             [](auto &a, auto &b) {
                 return get<0>(a) > get<0>(b);
             });

        auto [best_ns, best_x, best_y, best_k] = candidates[0];

        int top_k = min(3, (int)candidates.size());
        uniform_int_distribution<> dis_top(0, top_k - 1);
        auto [rnd_ns, rnd_x, rnd_y, rnd_k] = candidates[dis_top(gen)];

        int delta = best_ns - current_score;

        auto now_adaptive = chrono::high_resolution_clock::now();
        double elapsed_adaptive = chrono::duration<double>(now_adaptive - t0).count();
        double time_progress = elapsed_adaptive / time_limit;
        double score_progress = (double)current_score / max_pairs;

        double accept_bad_prob = base_accept_bad;

        if (time_progress > 0.5 && score_progress < 0.99) {
            accept_bad_prob *= (1.0 + 2.0 * (time_progress - 0.5));
        }
        if (score_progress > 0.98) {
            accept_bad_prob *= 5.0;
        }
        accept_bad_prob = min(0.3, accept_bad_prob);

        bool accept = false;
        int use_x, use_y, use_k;
        int new_score;

        if (delta >= 0) {
            accept = true;
            use_x = best_x; use_y = best_y; use_k = best_k;
            new_score = best_ns;
        } else {
            if ((int)current_ops.size() < 9000) {
                double r = urd(gen);
                if (r < accept_bad_prob) {
                    accept = true;
                    use_x = rnd_x; use_y = rnd_y; use_k = rnd_k;
                    new_score = rnd_ns;
                }
            }
        }

        if (accept) {
            rotate_submatrix(current_field, use_x, use_y, use_k);
            current_score = new_score;
            current_ops.push_back({use_x, use_y, use_k});
            no_improve++;

            if (current_ops.size() % 50 == 0 && !current_ops.empty()) {
                current_ops = compress(current_ops);
            }

            if ((int)current_ops.size() >= max_steps_total) {
                break;
            }

            if (new_score > best_score) {
                best_score = new_score;
                copy_field(best_field, current_field, n);
                best_ops = current_ops;
                no_improve = 0;
                if (best_score >= max_pairs) {
                    break;
                }
            } else if (new_score == best_score) {
                if (best_ops.empty() || current_ops.size() < best_ops.size()) {
                    copy_field(best_field, current_field, n);
                    best_ops = current_ops;
                }
            }
        }
        if (no_improve > resetT) {
            restart_count++;
            base_accept_bad = min(0.15, initial_accept_bad * pow(2.0, restart_count));
            copy_field(current_field, initial_field, n);
            current_score = initial_score;
            current_ops.clear();
            locked_rows.clear();
            locked_cols.clear();
            no_improve = 0;
            gen.seed(rd());
            continue;
        }
        if (no_improve > too) {
            copy_field(current_field, best_field, n);
            current_score = best_score;
            current_ops = best_ops;
            locked_rows.clear();
            locked_cols.clear();
            no_improve = 0;
            continue;
        }
    }
    


   // try full code in end g  
    {
        bool improved = dfs(
            best_field,
            n,
            best_score,
            max_pairs,
            best_ops,
            t0,
            time_limit
        );
        (void)improved;
    }

    vector<Move> final_ops = compress(best_ops);
    auto t1 = chrono::high_resolution_clock::now();
    double runtime = chrono::duration<double>(t1 - t0).count();

    Result res;
    res.score = best_score;
    res.ops = move(final_ops);
    res.runtime = runtime;
    return res;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    json data;
    input_file >> data;
    input_file.close();

    vector<vector<int>> temp = data["problem"]["field"]["entities"].get<vector<vector<int>>>();
    int n = (int)temp.size();

    int field[MAXN][MAXN];
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            field[i][j] = temp[i][j];
        }
    }

    int time_limit = get_limit_time(n);
    int max_pairs = n * n / 2;

    Result global_best;
    global_best.score = -1;
    double total_runtime = 0.0;

    for (int run = 0; run < NUM_RUNS; ++run) {
        Result r;
        if (n >= 20) r = local_search_solver_large(field, n, time_limit);
        else         r = local_search_small(field, n, time_limit);

        total_runtime += r.runtime;

        if (global_best.score < 0 ||
            r.score > global_best.score ||
            (r.score == global_best.score && r.ops.size() < global_best.ops.size())) {
            global_best = std::move(r);
        }
    }

    save_solution_full(global_best.ops, n, max_pairs, global_best.score);

    cout << "Best over " << NUM_RUNS << " runs: "
         << global_best.score << "/" << max_pairs
         << ", steps: " << global_best.ops.size()
         << ", best run time: " << global_best.runtime << "s"
         << ", total runtime ~ " << total_runtime << "s\n";

    return 0;
}
