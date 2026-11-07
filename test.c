int main() {
    int n = 5;
    int res = 1;
    int i = 1;

    while (i <= n) {
        res = res * i;
        i = i + 1;
    }

    if (res > 100) {
        res = res + 10;
    } else {
        res = 0;
    }

    return res;
}