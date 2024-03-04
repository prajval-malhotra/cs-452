#ifndef _linked_list_h_
#define _linked_list_h_ 1

#include "rpi.h"

template<typename T, int Capacity>
class LinkedList {
private:
    struct Node {
        T data;
        int prev;
        int next;
    };

    struct FreeList {
        int prev;
        int next;
    };

    // node can either be Node or FreeList
    union NodeOrFreeList {
        FreeList freelist;
        Node node;
    };

    NodeOrFreeList nodes[Capacity];
    int head;
    int prev;
    int free_list_head;

public:
    
    LinkedList() : head(-1) {
        // init lists
        for (int i = 0; i < Capacity - 1; ++i) {
            nodes[i].node.next = -1;
            nodes[i].node.prev = i - 1;
            nodes[i].freelist.next = i + 1;
            nodes[i].freelist.prev = i - 1;
        }
        prev = -1;
        // set last node as end of list
        nodes[Capacity-1].freelist.next = -1;
        nodes[Capacity-1].node.next = -1;
        free_list_head = 0;
    }

    void push(const T& value) {
        // check if free list is empty
        if (free_list_head == -1) {
            return;
        }

        // get next free node index
        int new_node_index = free_list_head;

        // update free list to point to the next free node
        free_list_head = nodes[new_node_index].freelist.next;

        // init new node
        nodes[new_node_index].node.data = value;
        nodes[new_node_index].node.next = -1;

        // update head if list is empty
        if (head == -1)
            head = new_node_index;

        // if prev is valid, update next to new node
        if (prev != -1)
            nodes[prev].node.next = new_node_index;

        // Update prev to current node index
        prev = new_node_index;

    }

    void pop(int tid) {
        int cur = head;
        int remove = -1;
        int temp_prev = -1; // Initialize prev here

        // Find the node with the specified tid
        while (cur != -1) {
            if (nodes[cur].node.data->tid == tid) {
                remove = cur;
                break;
            }
            temp_prev = cur; // Update prev here
            cur = nodes[cur].node.next;
        }

        // If tid not found, return
        if (remove == -1) {
            return;
        }

        // If the node to be removed is the head, update the head
        if (remove == head)
            head = nodes[remove].node.next;

        // Update the next pointer of the previous node
        if (temp_prev != -1)
            nodes[prev].node.next = nodes[remove].node.next;

        // Update the prev pointer of the next node
        if (nodes[remove].node.next != -1)
            nodes[nodes[remove].node.next].node.prev = temp_prev;

        // Update the free list
        nodes[remove].freelist.next = free_list_head;
        free_list_head = remove;
    }

    void display() const {
        uart_printf(CONSOLE, "List: ");
        int current = head;
        while (current != -1) {
            uart_printf(CONSOLE, "%d, ", nodes[current].node.data->tid);
            current = nodes[current].node.next;
        }
        uart_printf(CONSOLE, "\r\n");
    }
};

#endif
