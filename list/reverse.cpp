#include <iostream>
using namespace std;

struct Node{
    int data;
    Node* next;
};

void reverse(Node **head) {
    Node* prev = NULL;
    Node* curr = *head;

    while(curr != NULL) {
        Node* next = curr->next;
        curr->next = prev;
        prev = curr;
        curr = next;
    }
    *head = prev;
}


void print(const Node *head) {
    while (head != NULL) {
        cout<<head->data<<"\t";
        head = head->next;
    }
    cout<<endl;
}

void head_insert(Node **head, int data) {
    Node *node = new Node;
    node->data = data;
    node->next = *head;

    *head = node;
}


int main() {
    Node *head = NULL;
    int arr[] = {1,2,3,4,5,6,7,8,9};
    for (size_t i = 0; i < sizeof(arr)/sizeof(int); ++i) {
        head_insert(&head, arr[i]);
    }
    print(head);
    reverse(&head);
    print(head);
}
