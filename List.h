#pragma once

#include <iostream>

template <typename T>
struct List {
private:
	template <typename U>
	struct Node {
		U value;
		Node<U>* next;
		Node<U>* prev;

		Node(U value) : value(value), next(nullptr), prev(nullptr) {}
	};

	Node<T>* head;
	Node<T>* tail;

public:

	int32_t size;

	List() : head(nullptr), tail(nullptr) { size = 0; }

	bool is_empty() {
		return head == nullptr;
	}

	Node<T>* find(T value) {
		Node<T>* tempNode = head;
		while (tempNode and tempNode->value != value) tempNode = tempNode->next;
		if (tempNode and tempNode->value == value) return tempNode;
		else return nullptr;
	}

	T& get(int32_t index) {
		if (is_empty()) throw std::runtime_error("List is empty");	
		if (index >= size) throw std::runtime_error("Incorrect index");
		Node<T>* tempNode = head;
		for (int32_t i = 0; i < index; i++) tempNode = tempNode->next;
		return tempNode->value;
	}

	void push(int32_t index, T value) {
		if (is_empty()) return;
		Node<T>* curNode = head;
		Node<T>* prevNode = NULL;
		for (int32_t i = 1; curNode; i++) {
			if (index == 0) {
				push_front(value);
				return;
			}
			prevNode = curNode;
			curNode = curNode->next;
			if (i == index) {
				Node<T>* newNode = new Node<T>(value);
				prevNode->next = newNode;
				newNode->next = curNode;
				size++;
				return;
			}
			if (curNode->next == nullptr) {
				push_back(value);
				return;
			}
		}

	}

	void push_front(T value) {
		Node<T>* newNode = new Node<T>(value);
		if (is_empty()) {
			head = newNode;
			tail = newNode;
			size++;
			return;
		}
		newNode->next = head;
		head->prev = newNode;
		head = newNode;
		size++;
	}

	void push_back(T value) {
		Node<T>* newNode = new Node<T>(value);
		if (is_empty()) {
			head = newNode;
			tail = newNode;
			size++;
			return;
		}
		tail->next = newNode;
		newNode->prev = tail;
		tail = newNode;
		size++;
	}

	void remove_front() {
		if (is_empty()) return;
		Node<T>* tempNode = head;
		head = head->next;
		head->prev = nullptr;
		delete tempNode;
		size--;
	}

	void remove_back() {
		if (is_empty()) return;
		Node<T>* tempNode = tail;
		tail = tail->prev;
		tail->next = nullptr;
		delete tempNode;
		size--;
	}

	void print() {
		if (is_empty()) return;
		Node<T>* printNode = head;
		while (printNode) {
			std::cout << printNode->value << " ";
			printNode = printNode->next;
		}
		std::cout << std::endl;
	}

};
