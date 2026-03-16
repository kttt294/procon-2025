#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <chrono>
#include <algorithm>
#include <set>
#include <cstring>
#include <unordered_map>
#include "json.hpp"
using json = nlohmann::json;
using namespace std;

const int MAXN = 24;

ifstream input_file("input.json");

struct Move {
    int x, y, n;
};

// Đếm cặp với mảng C
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

// Xoay submatrix - modify in-place
void rotate_submatrix(int field[MAXN][MAXN], int x, int y, int k) {
    int temp[MAXN][MAXN];

    // Extract
    for (int i = 0; i < k; i++) {
        for (int j = 0; j < k; j++) {
            temp[i][j] = field[y + i][x + j];
        }
    }

    // Xoay và ghi lại
    for (int i = 0; i < k; i++) {
        for (int j = 0; j < k; j++) {
            field[y + j][x + k - 1 - i] = temp[i][j];
        }
    }
}

// Copy field
void copy_field(int dest[MAXN][MAXN], const int src[MAXN][MAXN], int n) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            dest[i][j] = src[i][j];
        }
    }
}

// Compress operations
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

// Kiểm tra xem có ít nhất 1 ô "màu đỏ" trong vùng xoay không
// "Màu đỏ" = ô có giá trị xuất hiện đúng 2 lần và 2 ô không kề nhau
bool has_red_cell(int field[MAXN][MAXN], int n, int x, int y, int k, 
                  const unordered_map<int, vector<pair<int,int>>>& color_positions) {
    for (int i = y; i < y + k; i++) {
        for (int j = x; j < x + k; j++) {
            int val = field[i][j];
            auto it = color_positions.find(val);
            if (it != color_positions.end() && it->second.size() == 2) {
                auto [r1, c1] = it->second[0];
                auto [r2, c2] = it->second[1];
                int dist = abs(r1 - r2) + abs(c1 - c2);
                if (dist > 1) {
                    // Kiểm tra xem ô hiện tại có phải là 1 trong 2 ô đỏ không
                    if ((i == r1 && j == c1) || (i == r2 && j == c2)) {
                        return true;
                    }
                }
            }
        }
    }
    return false;
}

// Tạo map vị trí các màu
unordered_map<int, vector<pair<int,int>>> build_color_positions(int field[MAXN][MAXN], int n) {
    unordered_map<int, vector<pair<int,int>>> pos;
    for (int r = 0; r < n; r++) {
        for (int c = 0; c < n; c++) {
            int v = field[r][c];
            pos[v].push_back({r, c});
        }
    }
    return pos;
}

// Liệt kê tất cả các bước đi hợp lệ tại trạng thái hiện tại
vector<Move> enumerate_valid_moves(int field[MAXN][MAXN], int n, 
                                    const unordered_map<int, vector<pair<int,int>>>& color_positions) {
    vector<Move> valid_moves;
    
    // Thử tất cả các kích thước từ 2 đến n
    for (int k = 2; k <= n; k++) {
        for (int y = 0; y <= n - k; y++) {
            for (int x = 0; x <= n - k; x++) {
                // Kiểm tra xem vùng này có ít nhất 1 ô đỏ không
                if (has_red_cell(field, n, x, y, k, color_positions)) {
                    valid_moves.push_back({x, y, k});
                }
            }
        }
    }
    
    return valid_moves;
}

// Lưu output JSON
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

// Thuật toán backtracking
struct BacktrackState {
    int best_score;
    vector<Move> best_path;
    int max_pairs;
    int nodes_explored;
    chrono::time_point<chrono::high_resolution_clock> start_time;
    double time_limit;
};

// Hàm so sánh move để dùng trong set
struct MoveCompare {
    bool operator()(const tuple<int,int,int>& a, const tuple<int,int,int>& b) const {
        if (get<0>(a) != get<0>(b)) return get<0>(a) < get<0>(b);
        if (get<1>(a) != get<1>(b)) return get<1>(a) < get<1>(b);
        return get<2>(a) < get<2>(b);
    }
};

void backtrack(int field[MAXN][MAXN], int n, vector<Move>& current_path, 
               set<tuple<int,int,int>, MoveCompare>& excluded_moves, 
               BacktrackState& state, int depth = 0) {
    
    // Kiểm tra thời gian
    auto now = chrono::high_resolution_clock::now();
    double elapsed = chrono::duration<double>(now - state.start_time).count();
    if (elapsed > state.time_limit) return;
    
    state.nodes_explored++;
    
    // Tính điểm hiện tại
    int current_score = count_pairs(field, n);
    
    // Cập nhật best nếu tốt hơn
    if (current_score > state.best_score) {
        state.best_score = current_score;
        state.best_path = current_path;
        
        cout << "New best: " << current_score << "/" << state.max_pairs 
             << " (steps: " << current_path.size() << ", depth: " << depth << ")" << endl;
        
        // Lưu ngay khi tìm được solution tốt hơn
        vector<Move> compressed = compress(current_path);
        save_output_json(compressed);
    }
    
    // Nếu đã đạt max, dừng
    if (current_score >= state.max_pairs) {
        return;
    }
    
    // Giới hạn độ sâu để tránh quá lâu
    if (depth >= 50) {
        return;
    }
    
    // Tạo map vị trí màu
    auto color_positions = build_color_positions(field, n);
    
    // Liệt kê các bước đi hợp lệ
    vector<Move> valid_moves = enumerate_valid_moves(field, n, color_positions);
    
    if (valid_moves.empty()) {
        return; // Không còn bước đi hợp lệ nào
    }
    
    // Thử từng bước đi
    for (const auto& move : valid_moves) {
        // Kiểm tra xem move này đã bị loại bỏ chưa
        auto move_tuple = make_tuple(move.x, move.y, move.n);
        if (excluded_moves.count(move_tuple)) continue;
        
        // Lưu trạng thái trước khi xoay
        int backup_field[MAXN][MAXN];
        copy_field(backup_field, field, n);
        int old_score = current_score;
        
        // Thực hiện xoay
        rotate_submatrix(field, move.x, move.y, move.n);
        int new_score = count_pairs(field, n);
        
        // Nếu điểm tăng, tiếp tục đệ quy
        if (new_score > old_score) {
            current_path.push_back(move);
            
            // Tạo set excluded_moves mới cho nhánh này (không kế thừa excluded từ nhánh khác)
            set<tuple<int,int,int>, MoveCompare> new_excluded;
            
            backtrack(field, n, current_path, new_excluded, state, depth + 1);
            current_path.pop_back();
        } 
        // Nếu điểm giữ nguyên, cũng thử (nhưng cẩn thận)
        else if (new_score == old_score && depth < 30) {
            current_path.push_back(move);
            
            set<tuple<int,int,int>, MoveCompare> new_excluded;
            
            backtrack(field, n, current_path, new_excluded, state, depth + 1);
            current_path.pop_back();
        }
        // Nếu điểm giảm, đánh dấu move này là không tốt ở level hiện tại
        else {
            excluded_moves.insert(move_tuple);
        }
        
        // Khôi phục trạng thái
        copy_field(field, backup_field, n);
    }
}

// Hàm chính của backtracking solver
void backtracking_solver(int field[MAXN][MAXN], int n, double time_limit) {
    BacktrackState state;
    state.best_score = count_pairs(field, n);
    state.max_pairs = n * n / 2;
    state.nodes_explored = 0;
    state.start_time = chrono::high_resolution_clock::now();
    state.time_limit = time_limit;
    
    cout << "Starting backtracking solver..." << endl;
    cout << "Initial score: " << state.best_score << "/" << state.max_pairs << endl;
    
    vector<Move> current_path;
    set<tuple<int,int,int>, MoveCompare> excluded_moves;
    
    // Tạo bản sao để không làm thay đổi field gốc
    int work_field[MAXN][MAXN];
    copy_field(work_field, field, n);
    
    backtrack(work_field, n, current_path, excluded_moves, state);
    
    auto end_time = chrono::high_resolution_clock::now();
    double runtime = chrono::duration<double>(end_time - state.start_time).count();
    
    cout << "\n=== Backtracking Solver Results ===" << endl;
    cout << "Best score: " << state.best_score << "/" << state.max_pairs << endl;
    cout << "Steps: " << state.best_path.size() << endl;
    cout << "Compressed steps: " << compress(state.best_path).size() << endl;
    cout << "Nodes explored: " << state.nodes_explored << endl;
    cout << "Runtime: " << runtime << "s" << endl;
    
    // Lưu kết quả cuối cùng
    vector<Move> final_ops = compress(state.best_path);
    save_output_json(final_ops);
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

    // Thời gian giới hạn
    double time_limit = 60.0;
    if (n <= 8) time_limit = 20.0;
    else if (n <= 14) time_limit = 30.0;

    backtracking_solver(field, n, time_limit);

    return 0;
}
