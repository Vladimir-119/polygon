#include <iostream>
#include <vector>
#include <queue>
#include <stack>
#include <cmath>
#include <unordered_map>
#include <limits>
#include <algorithm>
#include <functional>
#include <ctime>
#include <fstream>
#include <sstream>

using namespace std;

//Узел - это пара координат (долгота, широта).
using Node = pair<double, double>;

//Хэш-функция для узла
struct NodeHash {
    size_t operator()(const Node& node) const {
        auto hash1 = hash<double>{}(node.first);
        auto hash2 = hash<double>{}(node.second);
        return hash1 ^ (hash2 << 1);
    }
};

struct Graph {
    unordered_map<Node, vector<pair<Node, double>>, NodeHash> adj_list;

    // Добавляет ребро в граф между двумя узлами с указанным весом.
    void add_edge(double lon1, double lat1, double lon2, double lat2, double weight) {
        adj_list[{lon1, lat1}].push_back({{lon2, lat2}, weight});
        adj_list[{lon2, lat2}].push_back({{lon1, lat1}, weight});
    }

    // Читает данные из файла и заполняет граф.
    // Время: O(L + E), где L - количество строк, E - количество рёбер.
    // Память: O(V + E), где V - количество узлов, E - количество рёбер.
    void get_file_data(const string& filename) {
        ifstream file(filename);
        if (!file.is_open()) {
            cerr << "Error: Could not open file " << filename << endl;
            return;
        }

        string line;
        while (getline(file, line)) {
            size_t pos = line.find(':');
            if (pos == string::npos) {
                cerr << "Invalid line format: " << line << endl;
                continue;
            }

            // Разбор координат узлов
            string source_coords = line.substr(0, pos);
            stringstream source_stream(source_coords);
            double lon1, lat1;
            char comma;
            source_stream >> lon1 >> comma >> lat1;

            // Разбор рёбер
            string edges = line.substr(pos + 1);
            stringstream edge_stream(edges);
            string edge;
            while (getline(edge_stream, edge, ';')) {
                stringstream edge_data(edge);
                double lon2, lat2, weight;
                edge_data >> lon2 >> comma >> lat2 >> comma >> weight;

                // Добавляем ребро в граф
                add_edge(lon1, lat1, lon2, lat2, weight);
            }
        }

        file.close();
    }

    // Находит ближайший узел в графе к заданным координатам.
    // Время: O(V), где V - количество узлов.
    // Память: O(1).
    Node find_nearest_node(double lon, double lat) {
        Node closest;
        double min_distance = numeric_limits<double>::max();

        for (const auto& node : adj_list) {
            double distance = sqrt(pow(node.first.first - lon, 2) + pow(node.first.second - lat, 2));
            if (distance < min_distance) {
                min_distance = distance;
                closest = node.first;
            }
        }
        return closest;
    }

    // Выводит путь в читаемом формате.
    // Время: O(P), где P - длина пути.
    // Память: O(1).
    void print_path(const vector<Node>& path) {
        if (path.empty()) {
            cout << "Путь не найден" << endl;
            return;
        }
        for (size_t i = 0; i < path.size(); ++i) {
            cout << "(" << path[i].first << ", " << path[i].second << ")";
            if (i != path.size() - 1) cout << " -> ";
        }
        cout << endl;
    }
};

// BFS: Поиск в ширину
// Находит кратчайший путь в графе без учёта весов рёбер.
// Время: O(V + E), где V - количество узлов, E - количество рёбер.
// Память: O(V).
vector<Node> bfs(Graph& graph, Node start, Node end) {
    queue<Node> to_visit;
    unordered_map<Node, Node, NodeHash> parent;
    unordered_map<Node, bool, NodeHash> visited;

    to_visit.push(start);
    visited[start] = true;
    parent[start] = start;

    while (!to_visit.empty()) {
        Node current = to_visit.front();
        to_visit.pop();

        if (current == end) {
            vector<Node> path;
            for (Node at = end; at != start; at = parent[at]) {
                path.push_back(at);
            }
            path.push_back(start);
            reverse(path.begin(), path.end());
            return path;
        }

        for (const auto& neighbor : graph.adj_list[current]) {
            if (!visited[neighbor.first]) {
                visited[neighbor.first] = true;
                parent[neighbor.first] = current;
                to_visit.push(neighbor.first);
            }
        }
    }

    return {}; // Путь не найден
}

// DFS: Поиск в глубину
// Находит путь между двумя узлами. Не гарантирует кратчайший путь.
// Время: O(V + E), где V - количество узлов, E - количество рёбер.
// Память: O(V).
vector<Node> dfs(Graph& graph, Node start, Node end) {
    stack<Node> to_visit;
    unordered_map<Node, Node, NodeHash> parent;
    unordered_map<Node, bool, NodeHash> visited;

    to_visit.push(start);
    parent[start] = start;

    while (!to_visit.empty()) {
        Node current = to_visit.top();
        to_visit.pop();

        if (visited[current]) continue;
        visited[current] = true;

        if (current == end) {
            vector<Node> path;
            for (Node at = end; at != start; at = parent[at]) {
                path.push_back(at);
            }
            path.push_back(start);
            reverse(path.begin(), path.end());
            return path;
        }

        for (const auto& neighbor : graph.adj_list[current]) {
            if (!visited[neighbor.first]) {
                parent[neighbor.first] = current;
                to_visit.push(neighbor.first);
            }
        }
    }

    return {}; // Путь не найден
}

// Dijkstra: Алгоритм Дейкстры
// Находит кратчайший путь с учётом весов рёбер.
// Время: O((V + E) log V), где V - количество узлов, E - количество рёбер.
// Память: O(V).
vector<Node> dijkstra(Graph& graph, Node start, Node end) {
    unordered_map<Node, double, NodeHash> dist;
    unordered_map<Node, Node, NodeHash> parent;
    priority_queue<pair<double, Node>, vector<pair<double, Node>>, greater<>> pq;

    for (const auto& node : graph.adj_list) {
        dist[node.first] = numeric_limits<double>::max();
    }
    dist[start] = 0.0;
    pq.push({0.0, start});

    while (!pq.empty()) {
        auto [current_dist, current] = pq.top();
        pq.pop();

        if (current == end) {
            vector<Node> path;
            for (Node at = end; at != start; at = parent[at]) {
                path.push_back(at);
            }
            path.push_back(start);
            reverse(path.begin(), path.end());
            return path;
        }

        for (const auto& neighbor : graph.adj_list[current]) {
            double new_dist = current_dist + neighbor.second;
            if (new_dist < dist[neighbor.first]) {
                dist[neighbor.first] = new_dist;
                parent[neighbor.first] = current;
                pq.push({new_dist, neighbor.first});
            }
        }
    }

    return {}; // Путь не найден
}

// Тестирование алгоритмов поиска пути
void test_pathfinding_algorithms() {
    Graph graph;

    graph.add_edge(0, 0, 1, 0, 1.0);
    graph.add_edge(1, 0, 2, 0, 1.0);
    graph.add_edge(2, 0, 3, 0, 1.0);
    graph.add_edge(0, 0, 0, 1, 1.0);
    graph.add_edge(0, 1, 1, 1, 1.0);
    graph.add_edge(1, 1, 2, 1, 1.0);
    graph.add_edge(2, 1, 3, 1, 1.0);
    graph.add_edge(3, 0, 3, 1, 1.0);

    vector<tuple<Node, Node, vector<Node>>> test_cases = {
        {{0, 0}, {3, 0}, {{0, 0}, {1, 0}, {2, 0}, {3, 0}}},
        {{0, 0}, {2, 1}, {{0, 0}, {0, 1}, {1, 1}, {2, 1}}},
        {{1, 1}, {1, 1}, {{1, 1}}},
        {{3, 0}, {0, 2}, {}}
    };

    for (auto& [start, end, expected_path] : test_cases) {
        cout << "\nПример для теста (" << start.first << ", " << start.second << ") to ("
             << end.first << ", " << end.second << ")" << endl;

        auto bfs_path = bfs(graph, start, end);
        cout << "BFS результат: ";
        graph.print_path(bfs_path);

        auto dfs_path = dfs(graph, start, end);
        cout << "DFS результат: ";
        graph.print_path(dfs_path);

        auto dijkstra_path = dijkstra(graph, start, end);
        cout << "Дейкстра результат: ";
        graph.print_path(dijkstra_path);
    }
}

int main() {
    Graph graph;
    graph.get_file_data("/Users/thevaldemar/Downloads/spb_graph.txt");
    Node home = graph.find_nearest_node(30.285510, 60.028110);
    Node itmo = graph.find_nearest_node(30.308726, 59.956435);

    test_pathfinding_algorithms();

    cout << "===================================" << endl;
    clock_t start_time1 = clock();
    auto bfs_path = bfs(graph, home, itmo);
    clock_t end_time1 = clock();
    double duration1 = (double)(end_time1 - start_time1) / CLOCKS_PER_SEC;
    cout << "BFS результат: " << endl;
    cout << "Длина пути (ребра на пути): " << bfs_path.size() - 1 << endl;
    cout << "Время: " << duration1 << " секунд" << endl;
    graph.print_path(bfs_path);
    cout << "===================================" << endl;

    clock_t start_time4 = clock();
    auto dfs_path = dfs(graph, home, itmo);
    clock_t end_time4 = clock();
    double duration4 = (double)(end_time4 - start_time4) / CLOCKS_PER_SEC;
    cout << "DFS результат: " << endl;
    cout << "Длина пути (ребра на пути): " << dfs_path.size() - 1 << endl;
    cout << "Время: " << duration4 << " секунд" << endl;
    graph.print_path(dfs_path);
    cout << "===================================" << endl;

    clock_t start_time2 = clock();
    auto dijkstra_path = dijkstra(graph, home, itmo);
    clock_t end_time2 = clock();
    double duration2 = (double)(end_time2 - start_time2) / CLOCKS_PER_SEC;
    cout << "Дейкстра результат: " << endl;
    cout << "Длина пути (ребра на пути): " << dijkstra_path.size() - 1 << endl;
    cout << "Время: " << duration2 << " секунд" << endl;
    graph.print_path(dijkstra_path);
    cout << "===================================" << endl;

    return 0;
}
