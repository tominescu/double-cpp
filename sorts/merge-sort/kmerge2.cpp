#include <iostream>
#include <vector>
using namespace std;

typedef vector<int>::const_iterator Iter;

class Node{
    public:
        Node(){}
        Node(Iter iter, Iter end_iter):iter_(iter),end_iter_(end_iter){}
        bool hasNext() {
            if (iter_ != end_iter_) {
                return true;
            }
            return false;
        }
        void Next() {
            ++iter_;
        }
        int Val() {
            return *iter_;
        }
    private:
        Iter iter_;
        Iter end_iter_;
};

void heapify(Node* nodes, int root, int len) {
    int lchild = 2*root + 1;
    int rchild = 2*root + 2;

    if (lchild >= len) {
        return;
    }

    int min = lchild;
    if (rchild < len && nodes[rchild].Val() < nodes[lchild].Val()) {
        min = rchild;
    }

    if (nodes[root].Val() > nodes[min].Val()) {
        Node tmp = nodes[root];
        nodes[root] = nodes[min];
        nodes[min] = tmp;
        heapify(nodes, min, len);
    }
}

void sort(const vector<vector<int> >& vv, vector<int>& result) {
    Node nodes[vv.size()];
    int heapLen = 0;
    for (size_t i = 0; i < vv.size(); ++i) {
        if (vv[i].size() != 0) {
            nodes[heapLen++] = Node(vv[i].begin(), vv[i].end());
        }
    }

    for (int i = heapLen/2 - 1; i >= 0; i--) {
        heapify(nodes, i, heapLen);
    }

    while (heapLen > 0) {
        result.push_back(nodes[0].Val());
        nodes[0].Next();
        if (!nodes[0].hasNext()) {
            nodes[0] = nodes[--heapLen];
        }
        heapify(nodes, 0, heapLen);
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
