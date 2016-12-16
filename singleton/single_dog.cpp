#include <iostream>
#include <pthread.h>

using namespace std;

class SingleDog{
public:
    ~SingleDog(){
        pthread_mutex_destroy(&name_mutex_);
        pthread_mutex_destroy(&age_mutex_);
    }

    static SingleDog* GetInstance() {
        if (!instance_) {
            SingleDog* dog = new SingleDog();
            if (!__sync_bool_compare_and_swap(&instance_, NULL, dog)) {
                delete dog;
            }
        }
        return instance_;
    }

    void SetName(const string& name) {
        pthread_mutex_lock(&name_mutex_);
        name_ = name;
        pthread_mutex_unlock(&name_mutex_);
    }

    const string& GetName() {
        return name_;
    }

    void SetAge(int age) {
        pthread_mutex_lock(&age_mutex_);
        age_ = age;
        pthread_mutex_unlock(&age_mutex_);
    }

    int GetAge() {
        return age_;
    }

private:
    SingleDog(){
        name_mutex_ = PTHREAD_MUTEX_INITIALIZER;
        age_mutex_  = PTHREAD_MUTEX_INITIALIZER;
    }
    SingleDog(const SingleDog& dog){
        name_ = dog.name_;
        age_  = dog.age_;
    }
    SingleDog& operator=(const SingleDog& dog) {
        name_ = dog.name_;
        age_  = dog.age_;
        return *this;
    }

private:
    string  name_;
    int     age_;

    pthread_mutex_t name_mutex_;
    pthread_mutex_t age_mutex_;

    static SingleDog* instance_;

};

SingleDog* SingleDog::instance_ = NULL;


int main() {
    SingleDog* dog = SingleDog::GetInstance();
    dog->SetName("xiaofu");
    dog->SetAge(29);

//    delete dog;

    cout<<"name: "<<SingleDog::GetInstance()->GetName()<<endl;
    cout<<"age:  "<<SingleDog::GetInstance()->GetAge()<<endl;
    return 0;
}
