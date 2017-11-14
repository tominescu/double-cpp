#include <iostream>
#include <vector>
using namespace std;

void merge(const vector<int> & a, const vector<int>& b, vector<int>& result) {
    auto i1 = a.begin();
    auto i2 = b.begin();
    while(i1 != a.end() && i2 != b.end()) {
        if (*i1 < *i2) {
            result.push_back(*i1);
            ++i1;
        } else {
            result.push_back(*i2);
            ++i2;
        }
    }
    while(i1 != a.end()) {
        result.push_back(*i1);
        ++i1;
    }
    while(i2 != b.end()) {
        result.push_back(*i2);
        ++i2;
    }
}

void print(const vector<int>& a) {
    for (auto iter=a.begin(); iter != a.end(); ++iter) {
        cout<<*iter<<"\t";
    }
    cout<<endl;
}

int main() {
    int arr1[] = {1,3,5,7,9};
    int arr2[] = {2,4,6};

    vector<int> a(arr1, arr1 + sizeof(arr1)/sizeof(int));
    vector<int> b(arr2, arr2 + sizeof(arr2)/sizeof(int));

    vector<int> result;
    merge(a,b, result);
    print(result);
}
