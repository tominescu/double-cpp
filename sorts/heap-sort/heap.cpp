#include <iostream>
#include <vector>
using namespace std;

void swap(vector<int> & elements, int l, int r)
{
    int tmp = elements[l];
    elements[l] = elements[r];
    elements[r] = tmp;
}

void Heapify(vector<int> & elements, int pos, int n)
{
    int lchild = 2*pos + 1;
    int rchild = 2*pos + 2;

    if (lchild >= n) {
        return;
    }
    
    if (lchild < n && rchild >= n) {
        if (elements[pos] < elements[lchild]) {
            swap(elements, pos, lchild);
            Heapify(elements, lchild, n);
        }
    } else {
        int max = lchild;
        if (elements[lchild] < elements[rchild]) {
            max = rchild;
        }
        if (elements[pos] < elements[max]) {
            swap(elements, pos, max);
            Heapify(elements, max, n);
        }
    }
        
}

void sort(vector<int> & elements)
{
    int size = elements.size();
    for ( int i = size/2 - 1; i > 0; i-- ) {
        Heapify(elements, i, size);
    }

    for (int i = size-1; i > 0; i--) {
        swap(elements, 0, i);
        Heapify(elements, 0, i);
    }
}

void print(const vector<int> & elements)
{
    for (auto iter = elements.begin(); iter != elements.end(); ++iter) {
        cout<<*iter<<"\t";
    }
    cout<<endl;
}

int main()
{
    int arr[] = {9,7,5,3,1,8,6,4,2,0};
    vector<int> elements(arr, arr+sizeof(arr)/sizeof(int));
    sort(elements);
    print(elements);
}
