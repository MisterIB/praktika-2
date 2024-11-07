#pragma once

#include <functional>

#include <iostream>
#include <arpa/inet.h>
#include <string>

struct HList {
private:
	struct Node {
		std::string value;
		Node* next;

		Node(std::string value) : value(value), next(nullptr) {}
	};
public:

	Node* head;
	Node* tail;
	int32_t amountOfItems = 0;

	HList() : head(nullptr), tail(nullptr) {}

	bool is_empty() {
		if (amountOfItems < 0 or amountOfItems > 100) return true;
		return amountOfItems == 0 or head == nullptr or tail == nullptr;
	}

	int32_t size() {
		return amountOfItems;
	}

	std::string get(int32_t numberOfElement) {
		if (amountOfItems < numberOfElement) throw std::runtime_error("The element is missing in the hash table");
		Node* tempNode = head;
		for (int32_t i = 1; i <= numberOfElement; i++) {
			tempNode = tempNode->next;
		}
		return tempNode->value;
	}

	void push_back(std::string value) {
		Node* newNode = new Node(value);
		if (is_empty()) {
			head = newNode;
			tail = newNode;
			amountOfItems++;
			return;
		}
		tail->next = newNode;
		tail = newNode;
		amountOfItems++;
	}

	void remove(int32_t numberOfElement) {
		if (is_empty()) throw std::runtime_error("Hash table is empty");
		if (amountOfItems < numberOfElement) throw std::runtime_error("Error number");
		Node* prevNode = NULL;
		Node* curNode = head;
		for (int32_t i = 1; i <= numberOfElement; i++) {
			if (i == numberOfElement) {
				if (curNode == head) head = curNode->next;
				else prevNode->next = curNode->next;
				delete curNode;
				return;
			}
			prevNode = curNode;
			curNode = curNode->next;
		}
		amountOfItems--;
	}

	void print() {
		if (is_empty()) return;
		Node* printNode = head;
		while (printNode) {
			std::cout << printNode->value << std::endl;
			printNode = printNode->next;
		}
	}
};

struct Hash {
private:
	struct mNode {
		int32_t  key;
		HList list;
		mNode* next;

		mNode(int32_t inputKey): next(nullptr), key(inputKey) {}
	};

	mNode* head;
	int32_t amountOfItems = 0, capacity = 1000;

	int32_t hashFunction(std::string key) {
		key += "abcde123456789";
		std::hash<std::string> index;
		return index(key) % 1000;
	}

public:
	Hash() {
		head = nullptr;
		for (int32_t i = 1; i <= capacity; i++) {
			int32_t index = hashFunction(std::to_string(i));
			mNode* newNode = new mNode(index);
			if (head == nullptr) head = newNode;
			else {
				mNode* tempNode = head;
				while (tempNode->next) tempNode = tempNode->next;
				tempNode->next = newNode;
			}
		}
	}

	void HSET(std::string key, std::string value) {
		int32_t index = hashFunction(key);
		mNode* tempNode = head;
		for (int32_t i = 1; i <= capacity; i++) {
			if (tempNode->key == index) {
				(tempNode->list).push_back(value);
				amountOfItems++;
				return;
			}
			tempNode = tempNode->next;
		}
		throw std::runtime_error("Error column name");
	}

	HList HGET(std::string key) {
		int32_t index = hashFunction(key);
		mNode* tempNode = head;
		while (tempNode) {
			if (tempNode->key == index) break;
			tempNode = tempNode->next;
		}
		if (tempNode == nullptr) throw std::runtime_error("Error key");
		else return tempNode->list;
	}

	void print(std::string& result) {
		int32_t counter = 0;
		for (int32_t i = 1; i <= capacity; i++) {
			int32_t index = hashFunction(std::to_string(i));
			mNode* tempNode = head;
			while (tempNode) {
				if (tempNode->key == index) break;
				tempNode = tempNode->next;
			}
			if (tempNode->list.is_empty() or tempNode->list.size() <= 1) {
				counter++;
				continue;
			}
			else if (counter > 3) return;
			else {
				for (int32_t j = 0; j < tempNode->list.size(); j++) {
					result += tempNode->list.get(j) + "	";
				}
				result += "\n";
			}
		}
	}
};
