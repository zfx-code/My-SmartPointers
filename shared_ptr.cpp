#include <iostream>
#include <cassert>
#include <mutex>
using namespace std;

class Person {
public:
    string name;
    int age;
    Person() {
        name = "testName";
        age = 18;
        cout << "调用构造函数" << endl;
    }
    ~Person() {
        cout << "调用析构函数" << endl;
    }
};

// 指向所有类型的智能指针(使用模板类)
template<class T>
class SharedPointer {
private:
    T* _ptr;
    // 共享引用计数器
    size_t* _count;
    // 共享锁
    mutex* _mutex;
    // 封装引用计数器操作, 使用锁保证线程安全
    void add_count() {
        _mutex->lock();
        (*_count)++;
        _mutex->unlock();
    }

    void remove_count() {
        // 标记是否释放锁, 解锁后释放
        bool deleteMutex = false;
        _mutex->lock();
        (*_count)--;
        if (*_count == 0) {
            delete _ptr;
            delete _count;
            deleteMutex = true;
            cout << "智能指针指向的对象已释放" << endl;
        }
        _mutex->unlock();
        if (deleteMutex == true) {
            delete _mutex;
        }
    }

public:
    // 构造函数, 指针初始化
    SharedPointer(T* p = nullptr) {
        _ptr = p;
        _mutex = new mutex;
        if (p == nullptr) {
            _count = new size_t(0);
        }
        else {
            _count = new size_t(1);
        }
    }
    
    // 拷贝构造函数
    SharedPointer(SharedPointer& sp) {
        _ptr = sp._ptr;
        _count = sp._count;
        _mutex = sp._mutex;
        add_count();
    }

    // 运算符重载: "->", "*", "="
    T* operator->() {
        assert(_ptr != nullptr);
        return _ptr;
    }

    T& operator*() {
        assert(_ptr != nullptr);
        return *_ptr;
    }

    SharedPointer& operator=(SharedPointer& sp) {
        // 相同则不需要赋值
        if (_ptr == sp._ptr) {
            return *this;
        }
        // 1. 解决旧的
        remove_count();
        // 2. 指向新的
        _ptr = sp._ptr;
        _count = sp._count;
        _mutex = sp._mutex;
        add_count();
        return *this;
    }

    // 引用计数
    size_t use_count() {
        return *_count;
    }

    // 析构函数
    ~SharedPointer() {
        if (_ptr == nullptr) {
            cout << "指针为空" << endl;
            delete _count;
            return;
        }
        remove_count();
    }
};
int main() {
    // g++ -o main shared_ptr.cpp && ./main
    SharedPointer<Person> sp(new Person());
    cout << sp->name << endl;
    cout << (*sp).age << endl;
    cout << sp.use_count() << endl;
    SharedPointer<Person> sp2(sp);
    cout << sp2.use_count() << endl;
    SharedPointer<Person> sp3(new Person());
    sp3 = sp;
    cout << sp3.use_count() << endl;
    return 0;
}