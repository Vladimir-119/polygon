#include <iostream>
#include <vector>
#include <algorithm>
using namespace std;

int minTaps(int n, vector<int>& ranges) {
    // Создаем массив для хранения максимального покрытия из каждой точки
    vector<int> maxReach(n + 1, 0);
    for (int i = 0; i <= n; ++i) {
        int left = max(0, i - ranges[i]);
        int right = min(n, i + ranges[i]);
        maxReach[left] = max(maxReach[left], right);
    }

    // Жадно покрываем сад
    int tapsOpened = 0;
    int currentEnd = 0;
    int nextEnd = 0;

    for (int i = 0; i <= n; ++i) {
        // Если мы выходим за предел текущего покрытия
        if (i > nextEnd) {
            return -1; // Невозможно покрыть сад
        }

        // Если мы достигаем конца текущего покрытия
        if (i > currentEnd) {
            ++tapsOpened;
            currentEnd = nextEnd;
        }

        // Обновляем максимальное покрытие
        nextEnd = max(nextEnd, maxReach[i]);
    }

    return tapsOpened;
}

// Пример использования
int main() {
    int n1 = 5;
    vector<int> ranges1 = {3, 4, 1, 1, 0, 0};
    cout << minTaps(n1, ranges1) << endl; // Вывод: 1

    int n2 = 3;
    vector<int> ranges2 = {0, 0, 0, 0};
    cout << minTaps(n2, ranges2) << endl; // Вывод: -1

    return 0;
}
