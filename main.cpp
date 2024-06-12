#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

struct Node
{
	int value;
	Node* next;
	std::mutex node_mutex;
};

class FineGrainedQueue
{
private:
	Node* head;
	std::mutex queue_mutex;

public:
	FineGrainedQueue() : head(nullptr) {}

	~FineGrainedQueue()
	{
		Node* current = head;
		while (current)
		{
			Node* next = current->next;
			delete current;
			current = next;
		}
		head = nullptr;
	}

	void insertIntoMiddle(int value, int pos)
	{
		Node* newNode = new Node;
		newNode->value = value;
		newNode->next = nullptr;

		std::lock_guard<std::mutex> queue_lock(queue_mutex);

		if (head == nullptr)
		{
			head = newNode;
		}
		else if (pos <= 1)
		{
			newNode->next = head;
			head = newNode;
		}
		else
		{
			Node* current = head;
			Node* previous = nullptr;
			int currPos = 1;

			while (current != nullptr && currPos < pos)
			{
				previous = current;
				current = current->next;
				currPos++;
			}

			if (previous != nullptr)
			{
				{
					std::lock_guard<std::mutex> lock_previous(previous->node_mutex);
					if (current != nullptr)
					{
						std::lock_guard<std::mutex> lock_current(current->node_mutex);
						newNode->next = current;
					}
					previous->next = newNode;
				}
			}
			else
			{
				newNode->next = head;
				head = newNode;
			}
		}
	}

	Node* getHead() const
	{
		return head;
	}
};


int main()
{
	FineGrainedQueue queue;

	std::vector<std::thread> threads;
	threads.emplace_back(&FineGrainedQueue::insertIntoMiddle, &queue, 1, 1);
	threads.emplace_back(&FineGrainedQueue::insertIntoMiddle, &queue, 2, 2);
	threads.emplace_back(&FineGrainedQueue::insertIntoMiddle, &queue, 3, 3);
	threads.emplace_back(&FineGrainedQueue::insertIntoMiddle, &queue, 4, 4);
	threads.emplace_back(&FineGrainedQueue::insertIntoMiddle, &queue, 5, 5);
	threads.emplace_back(&FineGrainedQueue::insertIntoMiddle, &queue, 777, 1);
	threads.emplace_back(&FineGrainedQueue::insertIntoMiddle, &queue, 555, 4);
	threads.emplace_back(&FineGrainedQueue::insertIntoMiddle, &queue, 444, 444);

	for (auto& thread : threads)
	{
		thread.join();
	}

	Node* current = queue.getHead();
	while (current != nullptr)
	{
		std::cout << current->value << " ";
		current = current->next;
	}
	std::cout << std::endl;

	return 0;
}
