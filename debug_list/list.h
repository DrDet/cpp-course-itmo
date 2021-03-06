//
// Created by drden on 02.09.2017.
//


#ifndef LIST_DEBUG_LIST_H
#define LIST_DEBUG_LIST_H

#include <vector>
#include <cassert>
#include <type_traits>

template <typename T>
class list {
private:
    struct node;
    struct iterator_base;
    template <typename> struct any_iterator;

    node* head;
    node* dummy;
public:
    typedef any_iterator<T> iterator;
    typedef any_iterator<const T> const_iterator;
    typedef std::reverse_iterator<iterator> reverse_iterator;
    typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

    list()
    {
        dummy = new node();
        head = dummy;
    }

    list(list const & other) : list()
    {
        for (auto it = other.begin(); it != other.end(); ++it)
            push_back(*it);
    }

    list& operator=(list const & other) ///инвалидирует итераторы на старый
    {
        list temp(other);
        swap(*this, temp);
        ///here
        return *this;
    }

    ~list() ///инвалидирует
    {
        node* cur = head;
        while (cur) {
            node* ptr_next = cur->next;
            delete cur;
            cur = ptr_next;
        }
    }

    void clear() ///инвалидирует итераторы
    {
        while (!empty())
            pop_back();
    }

    bool empty() const {
        return head == dummy;
    }

    void insert(const_iterator pos, T const & x)
    {
        assert(pos.is_valid && pos.owner == this);
        node* new_node = new node();
        new_node->data = x;
        new_node->next = pos.cur_node;
        new_node->prev = pos.cur_node->prev;
        if (pos.cur_node != head) {
            pos.cur_node->prev->next = new_node;
        } else {
            head = new_node;
        }
        pos.cur_node->prev = new_node;
    }

    iterator erase(const_iterator pos) { ///инвалидирует все итераторы на удаляемый
        assert(pos.is_valid && pos.owner == this && pos.cur_node != dummy);
        pos.cur_node->next->prev = pos.cur_node->prev;
        if (pos.cur_node != head) {
            pos.cur_node->prev->next = pos.cur_node->next;
        } else {
            head = pos.cur_node->next;
        }
        iterator res(this, pos.cur_node->next);
        delete pos.cur_node; ///here
        return res;
    }

    T& front() {
        assert(!empty());
        return head->data;
    };

    T const & front() const {
        assert(!empty());
        return head->data;
    };

    T& back() {
        assert(!empty());
        return dummy->prev->data;
    }

    T const & back() const {
        assert(!empty());
        return dummy->prev->data;
    }

    void push_front(T const & x) {
        insert(begin(), x);
    }

    void push_back(T const & x) {
        insert(end(), x);
    }

    void pop_front() {
        assert(!empty());
        erase(begin());
    }

    void pop_back() {
        assert(!empty());
        erase(--end());
    }

    void splice(const_iterator pos, list& other, const_iterator first, const_iterator last)
    {
        assert(pos.is_valid && first.is_valid && last.is_valid && first.owner == last.owner);
        if (first == last)
            return;
        for (auto it = first; it != last; ++it) {
            if (it.cur_node == nullptr || it.cur_node == pos.cur_node)
                assert(false); ///assert(first <= last) && !pos_between(first, last)
        }
        for (auto it = first; it != last; ++it) {
//            for (size_t i = 0; i < it.cur_node->v_it.size(); ++i)/
//                it.cur_node->v_it[i]->set_owner(this);
            typename list<T>::iterator_base* cur = it.cur_node->it_head;
            while (cur != nullptr) {
                cur->set_owner(this);
                cur = cur->it_next;
            }
        }

        last.cur_node->prev->next = pos.cur_node;
        node* pos_prev = pos.cur_node->prev;
        pos.cur_node->prev = last.cur_node->prev;

        last.cur_node->prev = first.cur_node->prev;
        if (first.cur_node != other.head) {
            first.cur_node->prev->next = last.cur_node;
        } else {
            other.head = last.cur_node;
        }

        first.cur_node->prev = pos_prev;
        if (pos.cur_node != head) {
            pos_prev->next = first.cur_node;
        } else {
            head = first.cur_node;
        }

    }

    iterator begin() const { return iterator(this, head); }
    iterator end() const { return iterator(this, dummy); }
    const_iterator cbegin() const { return const_iterator(this, head); }
    const_iterator cend() const { return const_iterator(this, dummy); }
    reverse_iterator rbegin() const { return reverse_iterator(end()); }
    reverse_iterator rend() const { return reverse_iterator(begin()); }
    const_reverse_iterator crbegin() const { return const_reverse_iterator(cend()); }
    const_reverse_iterator crend() const { return const_reverse_iterator(cbegin()); }

    template <typename TT>
    friend void swap(list<TT>& a, list<TT>& b);
};

template <typename T>
void swap(list<T>& a, list<T>& b) {
    std::swap(a.head, b.head);
    std::swap(a.dummy, b.dummy);
}

template <typename T>
struct list<T>::node
{
    T data;
    node* next;
    node* prev;

//    std::vector<typename list<T>::iterator_base*> v_it;
    typename list<T>::iterator_base* it_head;

    node() : next(nullptr), prev(nullptr), it_head(nullptr) { };
    node(node const & other) : data(other.data), next(nullptr), prev(nullptr), it_head(nullptr) { }

    void invalidate_all_its() {
//        for (size_t i = 0; i < v_it.size(); ++i)
//            v_it[i]->invalidate();
        typename list<T>::iterator_base* cur = it_head;
        while (cur != nullptr) {
            cur->invalidate();
            cur = cur->it_next;
        }
    }

    ~node()
    {
        invalidate_all_its();
    }

    void add_it(iterator_base * x) {
//        v_it.push_back(x);
        x->it_next = it_head;
        it_head = x;
    }

    void delete_it(iterator_base * x) {
//        size_t idx;
//        for (size_t i = 0; i < v_it.size(); ++i) {
//            if (v_it[i] == x) {
//                idx = i;
//                break;
//            }
//        }
//        v_it.erase(v_it.begin() + idx);
        typename list<T>::iterator_base* cur = it_head;
        typename list<T>::iterator_base* prev = nullptr;
        while (cur != nullptr) {
            if (cur == x) {
                if (prev != nullptr) {
                    prev->it_next = cur->it_next;
                } else {
                    it_head = cur->it_next;
                    cur->it_next = nullptr;
                }
                break;
            }
            prev = cur;
            cur = cur->it_next;
        }
    }
};

template <typename T>
struct list<T>::iterator_base {
    iterator_base* it_next;

    iterator_base(iterator_base* ptr) : it_next(ptr) { }

    virtual void set_owner(list const *) = 0;
    virtual void invalidate() = 0;
    virtual ~iterator_base() { }
};

template<typename TT> template <typename T>
struct list<TT>::any_iterator : iterator_base {
    list const * owner;
    node* cur_node;
    bool is_valid;

    any_iterator() : iterator_base(nullptr), owner(nullptr), cur_node(nullptr), is_valid(false) { };

    any_iterator(any_iterator const & other) : iterator_base(nullptr), owner(other.owner), cur_node(other.cur_node), is_valid(other.is_valid) {
        assert(other.is_valid);
        cur_node->add_it(this);
    }

    template <typename U>
    any_iterator(any_iterator<U> const & other, typename std::enable_if<std::is_const<T>::value && !std::is_const<U>::value>::type * = nullptr) : iterator_base(nullptr), owner(other.owner), cur_node(other.cur_node), is_valid(other.is_valid) {
        assert(other.is_valid);
        cur_node->add_it(this);
    }

    any_iterator& operator=(any_iterator const & rhs) {
        assert(is_valid && rhs.is_valid);
        cur_node->delete_it(this);
        rhs.cur_node->add_it(this);
        cur_node = rhs.cur_node;
        owner = rhs.owner;
        return *this;
    }

    any_iterator(list const * owner, node* cur_node) : iterator_base(nullptr), owner(owner), cur_node(cur_node), is_valid(true) {
        cur_node->add_it(this);
    }

    ~any_iterator() {
        if (is_valid)
            cur_node->delete_it(this);
    }

    void set_owner(list const * new_owner) {
        owner = new_owner;
    }

    void invalidate() {
        is_valid = false;
    }

    T& operator*() const {
        assert(is_valid && cur_node != owner->dummy);
        return cur_node->data;
    }

    any_iterator& operator++() {
        assert(is_valid && cur_node != owner->dummy);
        cur_node->delete_it(this);
        cur_node->next->add_it(this);
        cur_node = cur_node->next;
        return *this;
    }

    any_iterator operator++(int) {
        any_iterator t = *this;
        ++(*this);
        return t;
    }

    any_iterator& operator--() {
        assert(is_valid && cur_node != owner->head);
        cur_node->delete_it(this);
        cur_node->prev->add_it(this);
        cur_node = cur_node->prev;
        return *this;
    }

    any_iterator operator--(int) {
        any_iterator t = *this;
        --(*this);
        return t;
    }

    friend bool operator==(any_iterator const & a, any_iterator const & b)
    {
        assert(a.is_valid && b.is_valid);
        return a.cur_node == b.cur_node;
    }

    friend bool operator!=(any_iterator const & a, any_iterator const & b)
    {
        assert(a.is_valid && b.is_valid);
        return a.cur_node != b.cur_node;
    }

    friend void swap(any_iterator& a, any_iterator& b) {
        assert(a.owner == b.owner && a.is_valid && b.is_valid);
        a.cur_node->delete_it(&a);
        a.cur_node->add_it(&b);
        b.cur_node->delete_it(&b);
        b.cur_node->add_it(&a);
        std::swap(a.cur_node, b.cur_node);
    }

    typedef std::ptrdiff_t difference_type;
    typedef T value_type;
    typedef T* pointer;
    typedef T& reference;
    typedef std::bidirectional_iterator_tag iterator_category;
};

#endif //LIST_DEBUG_LIST_H