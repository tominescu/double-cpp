#include <iostream>
#include <vector>
using namespace std;

void sort(const vector<vector<int> >& vv, vector<int>& result) {
    int k = vv.size();
    vector<int> posvec(k, 0);

    bool empty = false;
    while (!empty) {
        empty = true;
        int minpos = -1;
        int minval = 0;
        for (int i = 0; i < k; i++) {
            if (posvec[i] < vv[i].size()) {
                empty = false;
                if (minpos == -1) {
                    minpos = i;
                    minval = vv[i][posvec[i]];
                } else if (minval > vv[i][posvec[i]]) {
                    minpos = i;
                    minval = vv[i][posvec[i]];
                }
            }
        }
        if (minpos != -1) {
            result.push_back(minval);
            posvec[minpos]++;
        }
    }
}

void print(const vector<int>& vec) {
    for (vector<int>::const_iterator iter = vec.begin(); iter != vec.end(); ++iter) {
        cout<<*iter<<"\t";
    }
    cout<<endl;
}

int main() {
    int arr1[] = {1,4,8,12,25,99};
    int arr2[] = {2,3,8,22,35,77};
    int arr3[] = {0,12,77,98,107,202,1987};
    int arr4[] = {1,3,5};

    vector<vector<int> > vv;
    vector<int> a(arr1, arr1+sizeof(arr1)/sizeof(int));
    vv.push_back(a);

    vector<int> b(arr2, arr2+sizeof(arr2)/sizeof(int));
    vv.push_back(b);

    vector<int> c(arr3, arr3+sizeof(arr3)/sizeof(int));
    vv.push_back(c);

    vector<int> d(arr4, arr4+sizeof(arr4)/sizeof(int));
    vv.push_back(d);

    vector<int> result;
    print(a);
    print(b);
    print(c);
    print(d);
    sort(vv, result);

    print(result);
    return 0;
}
